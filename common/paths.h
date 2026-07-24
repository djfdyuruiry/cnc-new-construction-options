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

#ifndef COMMON_PATHS_H
#define COMMON_PATHS_H

#include <filesystem>
#include <string>

class PathsClass
{
public:
    static constexpr char SEP = std::filesystem::path::preferred_separator;

    static std::string Try_Get_Program_Path();
    static std::string Try_Get_Program_Binary_Name();
    static std::string Try_Get_User_Path_Root();
    static bool Create_Directory(const char* path);
    static bool Is_Absolute(const char* path);
    static std::string Concatenate_Paths(const char* path1, const char* path2);
    static std::string Get_Filename(const char* path);

    PathsClass()
    {
    }

    PathsClass(PathsClass const&) = delete;
    void operator=(PathsClass const&) = delete;

    /**
    * @brief Initialise the paths from an ini file to override normal OS defaults.
    */
    void Init(const char* suffix = nullptr,
              const char* ini_name = nullptr,
              const char* data_name = nullptr,
              const char* cmd_arg = nullptr);

    const char* Program_Path();
    const char* Program_Lua_Path();
    const char* Data_Path();
    const char* User_Path();
    const char* User_Lua_Path();
    const char* User_Save_Path();
    const char* User_Screenshot_Path();

private:
    std::string Suffix;
    std::string ProgramPath;
    std::string ProgramLuaPath;
    std::string DataPath;
    std::string UserPath;
    std::string UserLuaPath;
    std::string UserSavePath;
    std::string UserScreenshotPath;

    static std::string Argv_Path(const char* cmd_arg);
};

extern PathsClass Paths;

#endif /* COMMON_PATHS_H */
