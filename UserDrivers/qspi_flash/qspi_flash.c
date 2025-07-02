#include "qspi_flash.h"
#include "w25q128.h"
#include "log.h"  // Or replace with your own LOG macro

#define QSPI_TIMEOUT_DEFAULT HAL_QPSI_TIMEOUT_DEFAULT_VALUE

volatile bool qspi_dma_tx_done = false;

//static void qspi_flash_write_enable(QspiFlash* qf);
bool qspi_flash_write_enable(QspiFlash* qf);
 //void qspi_flash_auto_poll_ready(QspiFlash* qf, uint32_t timeout);
static uint8_t qspi_flash_get_status(QspiFlash* qf);

void qspi_flash_init(QspiFlash* qf, QSPI_HandleTypeDef* handle)
{
    qf->hqspi = handle;
    qspi_flash_reset(qf);
}

void qspi_flash_reset(QspiFlash* qf)
{
    QSPI_CommandTypeDef cmd = {0};
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction = RESET_ENABLE_CMD;
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    HAL_QSPI_Command(qf->hqspi, &cmd, QSPI_TIMEOUT_DEFAULT);
    cmd.Instruction = RESET_MEMORY_CMD;
    HAL_QSPI_Command(qf->hqspi, &cmd, QSPI_TIMEOUT_DEFAULT);

    qspi_flash_auto_poll_ready(qf, QSPI_TIMEOUT_DEFAULT);
}

void qspi_flash_read_id(QspiFlash* qf, uint8_t* idBuffer)
{
    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = READ_JEDEC_ID_CMD;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.NbData = 3;

    HAL_QSPI_Command(qf->hqspi, &cmd, QSPI_TIMEOUT_DEFAULT);
    HAL_QSPI_Receive(qf->hqspi, idBuffer, QSPI_TIMEOUT_DEFAULT);
}

void qspi_flash_read(QspiFlash* qf, uint32_t addr, uint8_t* buf, size_t size)
{
    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = READ_DATA_CMD;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = addr;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.NbData = size;

    HAL_QSPI_Command(qf->hqspi, &cmd, QSPI_TIMEOUT_DEFAULT);
    HAL_QSPI_Receive(qf->hqspi, buf, QSPI_TIMEOUT_DEFAULT);
}

void qspi_flash_read_quad(QspiFlash* qf, uint32_t addr, uint8_t* buf, size_t size)
{
    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = FAST_READ_QUAD_OUT_CMD;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = addr;
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.DummyCycles = W25Q128J_DUMMY_CYCLES_READ;
    cmd.NbData = size;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    HAL_QSPI_Command(qf->hqspi, &cmd, QSPI_TIMEOUT_DEFAULT);
    HAL_QSPI_Receive(qf->hqspi, buf, QSPI_TIMEOUT_DEFAULT);
}

void qspi_flash_write(QspiFlash* qf, uint32_t addr, const uint8_t* data, size_t size)
{
    while (size > 0) {
        uint32_t chunk = 256 - (addr % 256);
        if (chunk > size) chunk = size;

        qspi_flash_write_enable(qf);

        QSPI_CommandTypeDef cmd = {0};
        cmd.Instruction = PAGE_PROG_CMD;
        cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
        cmd.AddressMode = QSPI_ADDRESS_1_LINE;
        cmd.AddressSize = QSPI_ADDRESS_24_BITS;
        cmd.Address = addr;
        cmd.DataMode = QSPI_DATA_1_LINE;
        cmd.NbData = chunk;

        HAL_QSPI_Command(qf->hqspi, &cmd, QSPI_TIMEOUT_DEFAULT);
        HAL_QSPI_Transmit(qf->hqspi, (uint8_t*)data, QSPI_TIMEOUT_DEFAULT);
        qspi_flash_auto_poll_ready(qf, QSPI_TIMEOUT_DEFAULT);

        addr += chunk;
        data += chunk;
        size -= chunk;
    }
}

void qspi_flash_write_quad(QspiFlash* qf, uint32_t addr, const uint8_t* data, size_t size)
{
    while (size > 0) {
        uint32_t chunk = 256 - (addr % 256);
        if (chunk > size) chunk = size;

        qspi_flash_write_enable(qf);

        QSPI_CommandTypeDef cmd = {0};
        cmd.Instruction = QUAD_PAGE_PROG_CMD;
        cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
        cmd.AddressMode = QSPI_ADDRESS_1_LINE;
        cmd.AddressSize = QSPI_ADDRESS_24_BITS;
        cmd.Address = addr;
        cmd.DataMode = QSPI_DATA_4_LINES;
        cmd.NbData = chunk;
        cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

        HAL_QSPI_Command(qf->hqspi, &cmd, QSPI_TIMEOUT_DEFAULT);
        HAL_QSPI_Transmit(qf->hqspi, (uint8_t*)data, QSPI_TIMEOUT_DEFAULT);
        qspi_flash_auto_poll_ready(qf, QSPI_TIMEOUT_DEFAULT);

        addr += chunk;
        data += chunk;
        size -= chunk;
    }
}

bool qspi_flash_write_quad_dma(QspiFlash* qf, uint32_t addr, const uint8_t* data, size_t size)
{
    if (size == 0 || data == NULL) return false;

    qspi_flash_write_enable(qf);

    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = QUAD_PAGE_PROG_CMD;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = addr;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.DummyCycles = 0;
    cmd.NbData = size;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;


    


    if (HAL_QSPI_Command(qf->hqspi, &cmd, QSPI_TIMEOUT_DEFAULT) != HAL_OK) 
    {
        LOG_ERROR("[ERROR] HAL_QSPI_Command failed before DMA transmit");
        return false;
    }
    if (HAL_QSPI_Transmit_DMA(qf->hqspi, (uint8_t*)data) != HAL_OK) 
    {
         LOG_ERROR("[ERROR] HAL_QSPI_Transmit_DMA failed to start");
        return false;
    }
    return true;
}

bool qspi_flash_write_quad_dma_split(QspiFlash* qf, uint32_t addr, const uint8_t* data, size_t size)
{
    if (size == 0 || data == NULL) return false;

    const uint32_t page_size = 256;
    uint32_t offset = 0;

    while (offset < size) {
        uint32_t page_offset = addr % page_size;
        uint32_t space_left = page_size - page_offset;
        uint32_t chunk_len = (size - offset < space_left) ? (size - offset) : space_left;

        // Write enable
        if (!qspi_flash_write_enable(qf)) {
            LOG_ERROR("[QSPI] Write Enable failed at 0x%08lX", addr);
            return false;
        }

        QSPI_CommandTypeDef cmd = {0};
        cmd.Instruction = QUAD_PAGE_PROG_CMD;
        cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
        cmd.AddressMode = QSPI_ADDRESS_1_LINE;
        cmd.AddressSize = QSPI_ADDRESS_24_BITS;
        cmd.Address = addr;
        cmd.DataMode = QSPI_DATA_4_LINES;
        cmd.NbData = chunk_len;
        cmd.DummyCycles = 0;
        cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

        qspi_dma_tx_done = false;

        if (HAL_QSPI_Command(qf->hqspi, &cmd, QSPI_TIMEOUT_DEFAULT) != HAL_OK) {
            LOG_ERROR("[QSPI] QSPI Command Failed at 0x%08lX", addr);
            return false;
        }

        if (HAL_QSPI_Transmit_DMA(qf->hqspi, (uint8_t*)&data[offset]) != HAL_OK) {
            LOG_ERROR("[QSPI] DMA Transmit Failed at 0x%08lX", addr);
            return false;
        }

        if (!qspi_flash_wait_dma_complete(1000)) {
            LOG_ERROR("[QSPI] DMA Timeout at 0x%08lX", addr);
            return false;
        }

        if (!qspi_flash_auto_poll_ready(qf, 5000)) {
            LOG_ERROR("[QSPI] Flash not ready after write at 0x%08lX", addr);
            return false;
        }

        offset += chunk_len;
        addr   += chunk_len;
    }

    return true;
}

bool qspi_flash_wait_dma_complete(uint32_t timeout)
{
    uint32_t start = HAL_GetTick();
    while (!qspi_dma_tx_done) {
        if ((HAL_GetTick() - start) > timeout) return false;
    }
    qspi_dma_tx_done = false;
    return true;
}

bool qspi_flash_read_quad_dma(QspiFlash* qf, uint32_t addr, uint8_t* buf, size_t size)
{
    if (size == 0 || buf == NULL) return false;

    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = FAST_READ_QUAD_OUT_CMD;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = addr;
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.DummyCycles = 8;
    cmd.NbData = size;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(qf->hqspi, &cmd, QSPI_TIMEOUT_DEFAULT) != HAL_OK) return false;
    if (HAL_QSPI_Receive_DMA(qf->hqspi, buf) != HAL_OK) return false;

    return true;
}

void qspi_flash_erase_sector2(QspiFlash* qf, uint32_t addr)
{
    qspi_flash_write_enable(qf);

    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = SECTOR_ERASE_CMD;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = addr;
    cmd.DataMode = QSPI_DATA_NONE;

    HAL_QSPI_Command(qf->hqspi, &cmd, QSPI_TIMEOUT_DEFAULT);
    qspi_flash_auto_poll_ready(qf, W25Q128J_SECTOR_ERASE_MAX_TIME);
}

bool qspi_flash_erase_sector(QspiFlash* qf, uint32_t addr)
{
    if (!qspi_flash_write_enable(qf)) {
        LOG_ERROR("[QSPI] Write Enable failed before sector erase");
        return false;
    }

    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = SECTOR_ERASE_CMD;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = addr;
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.DummyCycles = 0;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(qf->hqspi, &cmd, QSPI_TIMEOUT_DEFAULT) != HAL_OK) {
        LOG_ERROR("[QSPI] Failed to issue sector erase command at 0x%08lX", addr);
        return false;
    }

    if (!qspi_flash_auto_poll_ready(qf, W25Q128J_SECTOR_ERASE_MAX_TIME)) {
        LOG_ERROR("[QSPI] Timeout waiting for sector erase at 0x%08lX", addr);
        return false;
    }

    return true;
}


void qspi_flash_erase_chip(QspiFlash* qf)
{
    qspi_flash_write_enable(qf);

    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = CHIP_ERASE_CMD;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.AddressMode = QSPI_ADDRESS_NONE;

    HAL_QSPI_Command(qf->hqspi, &cmd, QSPI_TIMEOUT_DEFAULT);
    qspi_flash_auto_poll_ready(qf, W25Q128J_CHIP_ERASE_MAX_TIME);
}

void qspi_flash_set_quad_enable(QspiFlash* qf)
{
    QSPI_CommandTypeDef cmd = {0};
    uint8_t sr2 = 0;

    cmd.Instruction = READ_STATUS_REG_2_CMD;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.NbData = 1;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    HAL_QSPI_Command(qf->hqspi, &cmd, QSPI_TIMEOUT_DEFAULT);
    HAL_QSPI_Receive(qf->hqspi, &sr2, QSPI_TIMEOUT_DEFAULT);

    cmd.Instruction = VOL_SR_WRITE_ENABLE_CMD;
    cmd.DataMode = QSPI_DATA_NONE;
    HAL_QSPI_Command(qf->hqspi, &cmd, QSPI_TIMEOUT_DEFAULT);

    sr2 |= W25Q128J_SR2_QE;
    cmd.Instruction = WRITE_STATUS_REG_2_CMD;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.NbData = 1;

    HAL_QSPI_Command(qf->hqspi, &cmd, QSPI_TIMEOUT_DEFAULT);
    HAL_QSPI_Transmit(qf->hqspi, &sr2, QSPI_TIMEOUT_DEFAULT);
}

void qspi_flash_enable_mmap(QspiFlash* qf)
{
    QSPI_CommandTypeDef cmd = {0};
    QSPI_MemoryMappedTypeDef cfg = {0};

    cmd.Instruction = READ_DATA_CMD;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;

    HAL_QSPI_MemoryMapped(qf->hqspi, &cmd, &cfg);
}

void qspi_flash_enable_quad_mmap(QspiFlash* qf)
{
    QSPI_CommandTypeDef cmd = {0};
    QSPI_MemoryMappedTypeDef cfg = {0};

    cmd.Instruction = FAST_READ_QUAD_INOUT_CMD;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_4_LINES;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
    cmd.AlternateBytes = 0xFF;
    cmd.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.DummyCycles = W25Q128J_DUMMY_CYCLES_READ_QUAD;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;

    if (HAL_QSPI_MemoryMapped(qf->hqspi, &cmd, &cfg) != HAL_OK) {
        LOG_ERROR("[QSPI] Memory mapped (quad) failed");
    }
}

void qspi_flash_enable_dual_mmap(QspiFlash* qf)
{
    QSPI_CommandTypeDef cmd = {0};
    QSPI_MemoryMappedTypeDef cfg = {0};

    cmd.Instruction = FAST_READ_DUAL_OUT_CMD;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.DataMode = QSPI_DATA_2_LINES;
    cmd.DummyCycles = 4;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;

    if (HAL_QSPI_MemoryMapped(qf->hqspi, &cmd, &cfg) != HAL_OK) {
        LOG_ERROR("[QSPI] Memory mapped (dual) failed");
    }
}

bool qspi_flash_disable_mmap(QspiFlash* qf)
{
    if (!qf || !qf->hqspi) return false;

    // Abort MMAP mode
    if (HAL_QSPI_Abort(qf->hqspi) != HAL_OK) {
        LOG_ERROR("[QSPI] HAL_QSPI_Abort failed");
        return false;
    }

    // Clear FMODE
    qf->hqspi->Instance->CCR &= ~(QUADSPI_CCR_FMODE);

    // Wait for BUSY to clear
    while (qf->hqspi->Instance->SR & QUADSPI_SR_BUSY) {
        __NOP();
    }

    LOG_INFO("[QSPI] MMAP mode disabled and flash ready");
    return true;
}





QFlashDeviceInfo qspi_flash_get_info(void)
{
    QFlashDeviceInfo info = {
        .name = "W25Q128JVEIQ",
        .size_mbit = 128,
        .page_size = 256,
        .sector_size = 4096,
        .quad_enabled = true
    };
    return info;
}

static void qspi_flash_write_enable2(QspiFlash* qf)
{
    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = WRITE_ENABLE_CMD;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.DataMode = QSPI_DATA_NONE;

    HAL_QSPI_Command(qf->hqspi, &cmd, QSPI_TIMEOUT_DEFAULT);

    if (!(qspi_flash_get_status(qf) & W25Q128J_SR_WEL)) {
        LOG_ERROR("[QSPI] Write Enable failed");
    }
}

bool qspi_flash_write_enable(QspiFlash* qf)
{
    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = WRITE_ENABLE_CMD;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.DummyCycles = 0;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(qf->hqspi, &cmd, QSPI_TIMEOUT_DEFAULT) != HAL_OK) {
        LOG_ERROR("[QSPI] Failed to issue Write Enable command");
        return false;
    }

    // Poll SR1 for WEL bit
    uint32_t start = HAL_GetTick();
    while (!(qspi_flash_get_status(qf) & W25Q128J_SR_WEL)) {
        if ((HAL_GetTick() - start) > 10) {
            LOG_ERROR("[QSPI] Write Enable failed (WEL bit not set)");
            return false;
        }
    }

    return true;
}


bool qspi_flash_auto_poll_ready(QspiFlash* qf, uint32_t timeout)
{
    QSPI_CommandTypeDef cmd = {0};
    QSPI_AutoPollingTypeDef cfg = {0};

    cmd.Instruction = READ_STATUS_REG_CMD;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.DummyCycles = 0;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    cfg.Match = 0x00;                         // Wait for BUSY == 0
    cfg.Mask = 0x01;                          // Mask for BUSY bit (bit 0)
    cfg.MatchMode = QSPI_MATCH_MODE_AND;
    cfg.StatusBytesSize = 1;
    cfg.Interval = 0x10;
    cfg.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;

    if (HAL_QSPI_AutoPolling(qf->hqspi, &cmd, &cfg, timeout) != HAL_OK) {
        LOG_ERROR("[QSPI] AutoPolling timeout or failure");
        return false;
    }

    return true;
}


static uint8_t qspi_flash_get_status(QspiFlash* qf)
{
    QSPI_CommandTypeDef cmd = {0};
    uint8_t reg = 0;

    cmd.Instruction = READ_STATUS_REG_CMD;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.NbData = 1;

    HAL_QSPI_Command(qf->hqspi, &cmd, QSPI_TIMEOUT_DEFAULT);
    HAL_QSPI_Receive(qf->hqspi, &reg, QSPI_TIMEOUT_DEFAULT);
    return reg;
}



void HAL_QSPI_TxCpltCallback(QSPI_HandleTypeDef *hqspi)
{
    qspi_dma_tx_done = true; // A global or static volatile flag
}

void HAL_QSPI_RxCpltCallback(QSPI_HandleTypeDef *hqspi)
{
    qspi_dma_tx_done = true;
}
