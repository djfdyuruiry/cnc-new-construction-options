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
#include "logger.h"
#include "paths.h"
#include "debugstring.h"
#include "utf.h"
#include <winerror.h>
#include <shlobj.h>
#include <tchar.h>

std::string PathsClass::Try_Get_Program_Path()
{
#ifndef REMASTER_BUILD
        /*
        ** Adapted from https://github.com/gpakosz/whereami
        ** dual licensed under the WTFPL v2 and MIT licenses without any warranty. by Gregory Pakosz (@gpakosz)
        */
        TCHAR buffer1[MAX_PATH];
        TCHAR buffer2[MAX_PATH];
        TCHAR* path = NULL;
        int length = -1;

        while (true) {
            DWORD size;

            size = GetModuleFileName(nullptr, buffer1, sizeof(buffer1) / sizeof(buffer1[0]));

            if (size == 0) {
                throw std::runtime_error(std::format("Failed to get EXE filename: {}", GetLastError()));
            }

            if (size == (DWORD)(sizeof(buffer1) / sizeof(buffer1[0]))) {
                DWORD size_ = size;
                do {
                    TCHAR* path_ = (TCHAR*)realloc(path, sizeof(TCHAR) * size_ * 2);

                    if (path_ == nullptr) {
                        break;
                    }

                    size_ *= 2;
                    path = path_;
                    size = GetModuleFileName(nullptr, path, size_);
                } while (size == size_);

                if (size == size_) {
                    throw std::runtime_error(std::format("Failed to get EXE filename: {}", GetLastError()));;
                }
            } else {
                path = buffer1;
            }

            if (!_tfullpath(buffer2, path, MAX_PATH)) {
                throw std::runtime_error(std::format("Failed to get EXE filename: {}", GetLastError()));
            }

            std::string tmp(static_cast<const char*>(TCHARToUTF8(buffer2)));

            if (path != buffer1) {
                free(path);
            }

            return tmp.substr(0, tmp.find_last_of("\\/"));
        }
#else
        TCHAR path[MAX_PATH];
        HMODULE hm = nullptr;

        // Get path to mod DLL (not remastered host exe)
        if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                reinterpret_cast<LPCWSTR>(&PathsClass::Create_Directory), &hm) == 0) {
            throw std::runtime_error(std::format("Failed to get DLL filename: {}", GetLastError()));
        }

        if (GetModuleFileName(hm, path, sizeof(path)) == 0) {
            throw std::runtime_error(std::format("Failed to get DLL filename: {}", GetLastError()));
        }

        const std::string tmp((TCHARToUTF8(path)));
        return tmp.substr(0, tmp.find_last_of('\\'));
#endif
}

const char* PathsClass::Program_Path()
{
    if (ProgramPath.empty()) {
        try {
            ProgramPath = Try_Get_Program_Path();
            CNC_LOGGER_INFO("Resolved ProgramPath: {}", ProgramPath);
        } catch (const std::runtime_error& e) {
            CNC_LOGGER_FATAL("Failed to resolve ProgramPath: {}", e.what());
        }
    }

    return ProgramPath.c_str();
}

const char* PathsClass::Data_Path()
{
    if (DataPath.empty()) {
        if (ProgramPath.empty()) {
            // Init the program path first if it hasn't been done already.
            Program_Path();
        }

        DataPath = ProgramPath.substr(0, ProgramPath.find_last_of("\\/")) + SEP + "share";

        if (!Suffix.empty()) {
            DataPath += SEP + Suffix;
        }

        CNC_LOGGER_DEBUG("Resolved DataPath: {}", DataPath);
    }

    return DataPath.c_str();
}

const char* PathsClass::User_Path()
{
    if (UserPath.empty()) {
        TCHAR path[MAX_PATH];

        if (!SHGetSpecialFolderPath(nullptr, path, CSIDL_APPDATA, TRUE)) {
            CNC_LOGGER_FATAL("Failed to retrieve FOLDERID_RoamingAppData for PathsClass::User_Path(): {}", GetLastError());
        }

        UserPath = std::format("{}{}{}", static_cast<const char*>(TCHARToUTF8(path)), SEP, "nco");

        if (!Suffix.empty()) {
            UserPath += SEP + Suffix;
        }

        Create_Directory(UserPath.c_str());

        CNC_LOGGER_DEBUG("Resolved UserPath: {}", UserPath);
    }

    return UserPath.c_str();
}

bool PathsClass::Create_Directory(const char* dirname)
{
    bool ret = true;

    if (dirname == nullptr) {
        return ret;
    }

    std::string temp = dirname;
    size_t pos = 0;
    do {
        pos = temp.find_first_of("\\/", pos + 1);
        if (CreateDirectory(UTF8ToTCHAR(temp.substr(0, pos).c_str()), nullptr) == FALSE) {
            if (GetLastError() != ERROR_ALREADY_EXISTS) {
                ret = false;
                break;
            }
        }
    } while (pos != std::string::npos);

    return ret;
}

bool PathsClass::Is_Absolute(const char* path)
{
    if (strlen(path) < 2) {
        return false;
    }

    return path != nullptr && (path[1] == ':' || (path[0] == '\\' && path[1] == '\\'));
}

std::string PathsClass::Concatenate_Paths(const char* path1, const char* path2)
{
    return std::string(path1) + SEP + path2;
}

std::string PathsClass::Get_Filename(const char* path)
{
    char temppath[_MAX_PATH];
    char name[_MAX_FNAME];
    char ext[_MAX_EXT];

    _splitpath(path, NULL, NULL, name, ext);
    _makepath(temppath, NULL, NULL, name, ext);

    return std::string(temppath);
}

std::string PathsClass::Argv_Path(const char* cmd_arg)
{
    TCHAR base_buff[MAX_PATH];
    TCHAR* buff = base_buff;
    unsigned len = GetFullPathName(UTF8ToTCHAR(cmd_arg), MAX_PATH, buff, nullptr);
    std::string ret;

    // If we have a path longer than the standard max path, allocate a buffer of the correct size.
    if (len >= MAX_PATH) {
        buff = new TCHAR[len];
        len = GetFullPathName(UTF8ToTCHAR(cmd_arg), len, buff, nullptr);

        if (len > 0) {
            ret = TCHARToUTF8(buff);
        }

        delete[] buff;
    } else if (len > 0) {
        ret = TCHARToUTF8(buff);
    }

    return ret;
}