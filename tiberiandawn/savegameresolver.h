#pragma once

#include <string>

#include "common/cdfile.h"
#include "common/logger.h"

#include "savegameheader.h"

/**
 * Main API for saving/loading JSON format save game files.
 *
 *   - Uses the nlohmann::json library
 *   - Reads the game state from Global variables into JSON objects (Saving)
 *   - Provides validation after reading globals (to ensure game state is valid)
 *   - Writes JSON to a file in JSON Lines (JSONL) format, first line is the header info and second is the main data
 *   - Reads JSON from a file in JSONL format
 *   - Provides validation after reading JSON
 *   - Writes to game state from JSON objects via globals (Loading)
 *   - Each save is 'versioned', with the intention to be backwards compatible
 *     (new versions should be subclasses of SaveGame_v1 and define sane defaults for new values/behavior)
 *
 * Note: Serialization/deserialization is made possible by declaring TO_JSON/FROM_JSON friend functions
 *       for each involved C++ type (Units, Triggers, Objects, Map etc.)
 */
class SaveGameResolver final
{
public:
    // The current version to use when saving the game
    static const std::string_view& Current_Save_Version;
    // A list of supported versions as strings
    static const std::vector<std::string_view>& Supported_Save_Versions;

    /**
     * Attempt to write a save game to a file handle, the header will have the given description.
     *
     * Uses the latest version of the JSON save game format.
     */
    static bool Save(CDFileClass& file, const char* description);

    /**
     * Reads the header (only) from a save game file at the given path. Asserts the save version
     * is supported before returning the header object.
     */
    static std::optional<SaveGameHeader> Load_Header(const std::string& path);

    /**
     * Attempt to load a save game from a file path. The save game version in the header
     * will determine load behaviour.
     */
    static std::optional<SaveGameHeader> Load(const std::string& path);

private:
    static inline const CncLogger Logger = CncLogger::For(SaveGameResolver);

    SaveGameResolver() = delete;

};
