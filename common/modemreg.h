//
// Copyright 2020 Electronic Arts Inc.
//
// TiberianDawn.DLL and RedAlert.dll and corresponding source code is free
// software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.

// TiberianDawn.DLL and RedAlert.dll and corresponding source code is distributed
// in the hope that it will be useful, but with permitted additional restrictions
// under Section 7 of the GPL. See the GNU General Public License in LICENSE.TXT
// distributed with this program. You should have received a copy of the
// GNU General Public License along with permitted additional restrictions
// with this program. If not, see https://github.com/electronicarts/CnC_Remastered_Collection

#pragma once
class ModemRegistryEntryClass
{
public:
    ModemRegistryEntryClass(int modem_number)
        : ModemName("")
        , ModemDeviceName("")
        , ErrorCorrectionEnable("")
        , ErrorCorrectionDisable("")
        , CompressionEnable("")
        , CompressionDisable("")
        , HardwareFlowControl("")
        , NoFlowControl("")
    {
    }

    ~ModemRegistryEntryClass(void) {}

    char* Get_Modem_Name(void)
    {
        return (ModemName);
    }

    char* Get_Modem_Device_Name(void)
    {
        return (ModemDeviceName);
    }

    char* Get_Modem_Error_Correction_Enable(void)
    {
        return (ErrorCorrectionEnable);
    }

    char* Get_Modem_Error_Correction_Disable(void)
    {
        return (ErrorCorrectionDisable);
    }

    char* Get_Modem_Compression_Enable(void)
    {
        return (CompressionEnable);
    }

    char* Get_Modem_Compression_Disable(void)
    {
        return (CompressionDisable);
    }

    char* Get_Modem_Hardware_Flow_Control(void)
    {
        return (HardwareFlowControl);
    }

    char* Get_Modem_No_Flow_Control(void)
    {
        return (HardwareFlowControl);
    }

private:
    char ModemName[256];
    char ModemDeviceName[256];
    char ErrorCorrectionEnable[256];
    char ErrorCorrectionDisable[256];
    char CompressionEnable[256];
    char CompressionDisable[256];
    char HardwareFlowControl[256];
    char NoFlowControl[256];
};
