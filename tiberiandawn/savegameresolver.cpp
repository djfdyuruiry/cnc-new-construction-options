#include <ranges>
#include <vector>

#include "function.h"
#include "savegame_v1.h"
#include "savegameresolver.h"

//The implementation of the current save game version
#define CURRENT_SAVE_CLASS SaveGame_v1
#define CURRENT_SAVE_CLASS_NAME NAMEOF(SaveGame_v1)

const std::string_view& SaveGameResolver::Current_Save_Version = CURRENT_SAVE_CLASS::Version_Name;
const std::vector<std::string_view> SaveGameResolver::Supported_Save_Versions = std::vector { CURRENT_SAVE_CLASS::Version_Name };

bool SaveGameResolver::Save(CDFileClass& file, const char* description)
{
    std::string error_message;

    try {
        // build header
        SaveGameHeader header;

        header.Version = Current_Save_Version;
        header.Description = description;
        header.Read_Globals();

        // build save
        CURRENT_SAVE_CLASS save;

        save.Read_Globals();

        return save.To_File(file, header);
    } catch (const CncJsonException& e) {
        error_message = e.what();
    } catch (const nlohmann::json::exception& e) {
        error_message = e.what();
    }

    CNC_LOG_ERROR("Error reading game state into {} instance: {}", CURRENT_SAVE_CLASS_NAME, error_message);
    return false;
}

std::optional<SaveGameHeader> SaveGameResolver::Load_Header(const std::string& path)
{
    SaveGameHeader header;

    if (!SaveGameHeader::From_File(path, header)) {
        return std::nullopt;
    }

    if (!std::ranges::contains(Supported_Save_Versions, header.Version)) {
        CNC_LOGGER_ERROR("Save game version '{}' is unsupported, file path: {}", header.Version, path);

        return std::nullopt;
    }

    return header;
}

std::optional<SaveGameHeader> SaveGameResolver::Load(const std::string& path)
{
    SaveGameHeader header;

    if (!SaveGameHeader::From_File(path, header)) {
        return std::nullopt;
    }

    if (header.Version == SaveGame_v1::Version_Name) { \
        CNC_LOGGER_INFO("Loading save game version '{}' from file: {}", SaveGame_v1::Version_Name, path);

            if (SaveGame_v1 save; save.Load_From_File(path)) {
                std::string error_message;

                try {
                    if (header.Write_Globals() && save.Write_Globals()) {
                        return header;
                    }
                } catch (const CncJsonException& e) {
                    error_message = e.what();
                } catch (const nlohmann::json::exception& e) {
                    error_message = e.what();
                }

                CNC_LOGGER_ERROR("Save game is corrupt, JSON parse error: {}", error_message);
            }
    }
    else {
        CNC_LOGGER_ERROR("Save game version '{}' is unsupported, file path: {}", header.Version, path);
    }

    return std::nullopt;
}
