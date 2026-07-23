// common/serialport_posix.cpp
#include "serialport.h"

#if !defined(_WIN32)

#include <cstring>
#include <algorithm>
#include <dirent.h>
#include <cstdio>

std::vector<SerialPortInfo> SerialPortClass::Enumerate() {
    std::vector<SerialPortInfo> ports;

    DIR* dir = opendir("/sys/class/tty/");
    if (!dir) {
        dir = opendir("/dev/");
        if (!dir) {
            return ports;
        }

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            const char* name = entry->d_name;

            if (name[0] == '.') continue;

            bool is_serial = false;
            if (strncmp(name, "ttyS", 4) == 0) {
                is_serial = true;
            } else if (strncmp(name, "ttyUSB", 6) == 0) {
                is_serial = true;
            } else if (strncmp(name, "ttyACM", 6) == 0) {
                is_serial = true;
            } else if (strncmp(name, "ttyAMA", 6) == 0) {
                is_serial = true;
            } else if (strncmp(name, "ttyGS", 5) == 0) {
                is_serial = true;
            }

            if (is_serial) {
                SerialPortInfo info;
                info.name = "/dev/";
                info.name += name;

                if (strncmp(name, "ttyUSB", 6) == 0) {
                    info.description = "USB Serial Adapter";
                } else if (strncmp(name, "ttyACM", 6) == 0) {
                    info.description = "USB CDC ACM Device";
                } else if (strncmp(name, "ttyAMA", 6) == 0) {
                    info.description = "AMBA Serial Port";
                } else if (strncmp(name, "ttyGS", 5) == 0) {
                    info.description = "Gadget Serial";
                } else {
                    info.description = "Serial Port";
                }

                char dev_path[256];
                snprintf(dev_path, sizeof(dev_path), "/sys/class/tty/%s/device", name);

                char id_path[256];

                snprintf(id_path, sizeof(id_path), "%s/idVendor", dev_path);
                FILE* f = fopen(id_path, "r");
                if (f) {
                    char vendor[16] = {0};
                    if (fgets(vendor, sizeof(vendor), f)) {
                        vendor[strcspn(vendor, "\n")] = 0;
                        info.hardware_id = vendor;
                    }
                    fclose(f);

                    snprintf(id_path, sizeof(id_path), "%s/idProduct", dev_path);
                    f = fopen(id_path, "r");
                    if (f) {
                        char product[16] = {0};
                        if (fgets(product, sizeof(product), f)) {
                            product[strcspn(product, "\n")] = 0;
                            if (!info.hardware_id.empty()) {
                                info.hardware_id += ":";
                            }
                            info.hardware_id += product;
                        }
                        fclose(f);
                    }

                    snprintf(id_path, sizeof(id_path), "%s/serial", dev_path);
                    f = fopen(id_path, "r");
                    if (f) {
                        char serial[64] = {0};
                        if (fgets(serial, sizeof(serial), f)) {
                            serial[strcspn(serial, "\n")] = 0;
                            info.hardware_id += ".";
                            info.hardware_id += serial;
                        }
                        fclose(f);
                    }
                }

                ports.push_back(info);
            }
        }
        closedir(dir);

        std::sort(ports.begin(), ports.end(),
                  [](const SerialPortInfo& a, const SerialPortInfo& b) {
                      return a.name < b.name;
                  });
        return ports;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        const char* name = entry->d_name;

        if (name[0] == '.') continue;

        bool is_serial = false;
        if (strncmp(name, "ttyS", 4) == 0) {
            is_serial = true;
        } else if (strncmp(name, "ttyUSB", 6) == 0) {
            is_serial = true;
        } else if (strncmp(name, "ttyACM", 6) == 0) {
            is_serial = true;
        } else if (strncmp(name, "ttyAMA", 6) == 0) {
            is_serial = true;
        } else if (strncmp(name, "ttyGS", 5) == 0) {
            is_serial = true;
        }

        if (is_serial) {
            SerialPortInfo info;
            info.name = "/dev/";
            info.name += name;

            if (strncmp(name, "ttyUSB", 6) == 0) {
                info.description = "USB Serial Adapter";
            } else if (strncmp(name, "ttyACM", 6) == 0) {
                info.description = "USB CDC ACM Device";
            } else if (strncmp(name, "ttyAMA", 6) == 0) {
                info.description = "AMBA Serial Port";
            } else if (strncmp(name, "ttyGS", 5) == 0) {
                info.description = "Gadget Serial";
            } else {
                info.description = "Serial Port";
            }

            char dev_path[256];
            snprintf(dev_path, sizeof(dev_path), "/sys/class/tty/%s/device\0", name);

            char id_path[256];

            snprintf(id_path, sizeof(id_path), "%s/idVendor\0", dev_path);
            FILE* f = fopen(id_path, "r");
            if (f) {
                char vendor[16] = {0};
                if (fgets(vendor, sizeof(vendor), f)) {
                    vendor[strcspn(vendor, "\n")] = 0;
                    info.hardware_id = vendor;
                }
                fclose(f);

                snprintf(id_path, sizeof(id_path), "%s/idProduct", dev_path);
                f = fopen(id_path, "r");
                if (f) {
                    char product[16] = {0};
                    if (fgets(product, sizeof(product), f)) {
                        product[strcspn(product, "\n")] = 0;
                        if (!info.hardware_id.empty()) {
                            info.hardware_id += ":";
                        }
                        info.hardware_id += product;
                    }
                    fclose(f);
                }

                snprintf(id_path, sizeof(id_path), "%s/serial", dev_path);
                f = fopen(id_path, "r");
                if (f) {
                    char serial[64] = {0};
                    if (fgets(serial, sizeof(serial), f)) {
                        serial[strcspn(serial, "\n")] = 0;
                        info.hardware_id += ".";
                        info.hardware_id += serial;
                    }
                    fclose(f);
                }
            }

            ports.push_back(info);
        }
    }
    closedir(dir);

    std::sort(ports.begin(), ports.end(),
              [](const SerialPortInfo& a, const SerialPortInfo& b) {
                  return a.name < b.name;
              });

    return ports;
}

SerialPortClass::SerialPortClass() : m_port(nullptr) {}

SerialPortClass::~SerialPortClass() {
    Close();
}

bool SerialPortClass::Open(const char* device, int baud, SerialParity parity,
                           int wordlen, int stopbits, SerialFlowControl flowcontrol)
{
    if (sp_get_port_by_name(device, &m_port) != SP_OK) {
        return false;
    }

    if (sp_open(m_port, SP_MODE_READ_WRITE) != SP_OK) {
        sp_free_port(m_port);
        m_port = nullptr;
        return false;
    }

    if (sp_set_baudrate(m_port, baud) != SP_OK) {
        Close();
        return false;
    }

    sp_parity sp_p;
    switch (parity) {
        case SerialParity::None:  sp_p = SP_PARITY_NONE; break;
        case SerialParity::Even:  sp_p = SP_PARITY_EVEN; break;
        case SerialParity::Odd:   sp_p = SP_PARITY_ODD;  break;
        case SerialParity::Mark:  sp_p = SP_PARITY_MARK; break;
        case SerialParity::Space: sp_p = SP_PARITY_SPACE;break;
    }
    if (sp_set_parity(m_port, sp_p) != SP_OK) {
        Close();
        return false;
    }

    if (sp_set_bits(m_port, wordlen) != SP_OK) {
        Close();
        return false;
    }

    if (sp_set_stopbits(m_port, stopbits) != SP_OK) {
        Close();
        return false;
    }

    sp_flowcontrol sp_fc;
    switch (flowcontrol) {
        case SerialFlowControl::None:      sp_fc = SP_FLOWCONTROL_NONE;      break;
        case SerialFlowControl::Hardware:  sp_fc = SP_FLOWCONTROL_RTSCTS;    break;
        case SerialFlowControl::Software:  sp_fc = SP_FLOWCONTROL_XONXOFF;   break;
        default:                           sp_fc = SP_FLOWCONTROL_NONE;      break;
    }
    if (sp_set_flowcontrol(m_port, sp_fc) != SP_OK) {
        Close();
        return false;
    }

    return true;
}

void SerialPortClass::Close() {
    if (m_port) {
        sp_close(m_port);
        sp_free_port(m_port);
        m_port = nullptr;
    }
}

int SerialPortClass::Read(unsigned char* dest, int buffer_len) {
    if (!m_port) return -1;
    return sp_blocking_read(m_port, dest, buffer_len, 100);
}

int SerialPortClass::Write(const unsigned char* buf, int len) {
    if (!m_port) return -1;
    return sp_blocking_write(m_port, buf, len, 100);
}

void SerialPortClass::Flush() {
    if (!m_port) return;
    sp_flush(m_port, SP_BUF_BOTH);
}

bool SerialPortClass::GetDTR() const {
    if (!m_port) return false;
    int signal = 0;
    if (sp_get_signals(const_cast<struct sp_port*>(m_port), (enum sp_signal*)&signal) == SP_OK) {
        return (signal & SP_SIG_DCD) != 0;
    }
    return false;
}

void SerialPortClass::SetDTR(bool state) {
    if (!m_port) return;
    if (state) {
        sp_set_dtr(m_port, SP_DTR_ON);
    } else {
        sp_set_dtr(m_port, SP_DTR_OFF);
    }
}

bool SerialPortClass::GetRTS() const {
    if (!m_port) return false;
    int signal = 0;
    if (sp_get_signals(const_cast<struct sp_port*>(m_port), (enum sp_signal*)&signal) == SP_OK) {
        return (signal & SP_SIG_CTS) != 0;
    }
    return false;
}

void SerialPortClass::SetRTS(bool state) {
    if (!m_port) return;
    if (state) {
        sp_set_rts(m_port, SP_RTS_ON);
    } else {
        sp_set_rts(m_port, SP_RTS_OFF);
    }
}

bool SerialPortClass::CTS() const {
    if (!m_port) return false;
    int signal = 0;
    if (sp_get_signals(const_cast<struct sp_port*>(m_port), (enum sp_signal*)&signal) == SP_OK) {
        return (signal & SP_SIG_CTS) != 0;
    }
    return false;
}

bool SerialPortClass::DSR() const {
    if (!m_port) return false;
    int signal = 0;
    if (sp_get_signals(const_cast<struct sp_port*>(m_port), (enum sp_signal*)&signal) == SP_OK) {
        return (signal & SP_SIG_DSR) != 0;
    }
    return false;
}

bool SerialPortClass::RI() const {
    if (!m_port) return false;
    int signal = 0;
    if (sp_get_signals(const_cast<struct sp_port*>(m_port), (enum sp_signal*)&signal) == SP_OK) {
        return (signal & SP_SIG_RI) != 0;
    }
    return false;
}

bool SerialPortClass::CD() const {
    if (!m_port) return false;
    int signal = 0;
    if (sp_get_signals(const_cast<struct sp_port*>(m_port), (enum sp_signal*)&signal) == SP_OK) {
        return (signal & SP_SIG_DCD) != 0;
    }
    return false;
}

bool SerialPortClass::IsOpen() const {
    return m_port != nullptr;
}

#endif
