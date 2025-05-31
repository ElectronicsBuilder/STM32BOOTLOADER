#include "boot_transport.h"
#include "boot_defs.h"

#if BOOT_TRANSPORT_BACKEND_SELECTED == BOOT_TRANSPORT_BACKEND_UART
#include "transport_uart.h"
static const BootTransportDriver* active_driver = &boot_uart_driver;

#elif BOOT_TRANSPORT_BACKEND_SELECTED == BOOT_TRANSPORT_BACKEND_WIFI
// #include "transport_wifi.h"
// static const BootTransportDriver* active_driver = &boot_wifi_driver;

#elif BOOT_TRANSPORT_BACKEND_SELECTED == BOOT_TRANSPORT_BACKEND_ETHERNET
// #include "transport_eth.h"
// static const BootTransportDriver* active_driver = &boot_eth_driver;

#elif BOOT_TRANSPORT_BACKEND_SELECTED == BOOT_TRANSPORT_BACKEND_USB
// #include "transport_usb.h"
// static const BootTransportDriver* active_driver = &boot_usb_driver;

#else
#error "Unsupported transport backend selected."
#endif

bool boot_transport_init(void) {
    if (active_driver && active_driver->init) {
        active_driver->init();
        return true;
    }
    return false;
}

bool boot_transport_poll(void) {
    return (active_driver && active_driver->poll) ? active_driver->poll() : false;
}

int boot_transport_read(uint8_t *buf, size_t len) {
    return (active_driver && active_driver->read) ? active_driver->read(buf, len) : -1;
}

int boot_transport_write(const uint8_t *buf, size_t len) {
    return (active_driver && active_driver->write) ? active_driver->write(buf, len) : -1;
}

void boot_transport_flush(void) {
    if (active_driver && active_driver->flush)
        active_driver->flush();
}
