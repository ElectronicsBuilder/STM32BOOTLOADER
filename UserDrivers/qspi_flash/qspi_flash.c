#include "qspi_flash.h"
#include "w25q128.h"
#include "log.hpp"  // Or replace with your own LOG macro

#define QSPI_TIMEOUT_DEFAULT HAL_QPSI_TIMEOUT_DEFAULT_VALUE

volatile bool qspi_dma_tx_done = false;

static void qspi_flash_write_enable(QspiFlash* qf);
static void qspi_flash_auto_poll_ready(QspiFlash* qf, uint32_t timeout);
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
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.NbData = size;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(qf->hqspi, &cmd, QSPI_TIMEOUT_DEFAULT) != HAL_OK) return false;
    if (HAL_QSPI_Transmit_DMA(qf->hqspi, (uint8_t*)data) != HAL_OK) return false;

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

void qspi_flash_erase_sector(QspiFlash* qf, uint32_t addr)
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

QFlashDeviceInfo qspi_flash_get_info(void)
{
    QFlashDeviceInfo info = {
        "W25Q128JVEIQ",
        128,
        256,
        4096,
        true
    };
    return info;
}

static void qspi_flash_write_enable(QspiFlash* qf)
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

static void qspi_flash_auto_poll_ready(QspiFlash* qf, uint32_t timeout)
{
    QSPI_CommandTypeDef cmd = {0};
    QSPI_AutoPollingTypeDef cfg = {0};

    cmd.Instruction = READ_STATUS_REG_CMD;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.DataMode = QSPI_DATA_1_LINE;

    cfg.Match = 0x00;
    cfg.Mask = 0x01;
    cfg.MatchMode = QSPI_MATCH_MODE_AND;
    cfg.StatusBytesSize = 1;
    cfg.Interval = 0x10;
    cfg.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;

    HAL_QSPI_AutoPolling(qf->hqspi, &cmd, &cfg, timeout);
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
