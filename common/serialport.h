// common/serialport.h
#pragma once

#include <cstdint>
#include <vector>
#include <string>

#if !defined(_WIN32)
#include <libserialport.h>
#endif

enum class SerialParity : uint8_t { None = 0, Even, Odd, Mark, Space };
enum class SerialFlowControl : uint8_t { None = 0, Hardware, Software };

struct SerialPortInfo
{
    std::string name;
    std::string description;
    std::string hardware_id;
    std::string display_name;
};

class SerialPortClass
{
public:
    SerialPortClass();
    ~SerialPortClass();

    static std::vector<SerialPortInfo> Enumerate();

    bool Open(const char* device, int baud, SerialParity parity,
              int wordlen, int stopbits, SerialFlowControl flowcontrol);
    void Close();

    int Read(unsigned char* dest, int buffer_len);
    int Write(const unsigned char* buf, int len);
    void Flush();

    bool GetDTR() const;
    void SetDTR(bool state);
    bool GetRTS() const;
    void SetRTS(bool state);

    bool CTS() const;
    bool DSR() const;
    bool RI() const;
    bool CD() const;

    // Diagnostics
    int FramingErrors = 0;
    int IOErrors = 0;
    int BufferOverruns = 0;
    int InBufferOverflows = 0;
    int ParityErrors = 0;
    int OutBufferOverflows = 0;
    int InQueue = 0;
    int OutQueue = 0;

    bool IsOpen() const;

private:
#if defined(_WIN32)
    void* m_handle;
#else
    struct sp_port* m_port;
#endif
};
