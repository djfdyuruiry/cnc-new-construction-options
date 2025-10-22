#pragma once

#include <map>

#include "logger.h"

/**
 * Map that enables mapping between two lists of values, in
 * either direction. Useful for converting between enums/constants
 * and strings, for example.
 */
template<typename A, typename B>
class TwoWayMap
{
private:
    static inline const auto& Logger = CncLogger::For(TwoWayMap);

    std::map<A, B> ForwardMap;
    std::map<B, A> BackwardMap;

public:
    TwoWayMap(std::initializer_list<std::pair<A, B>> pairs)
    {
        // TODO: Include duplicate value in exception message
        for (const auto& pair : pairs) {
            // Check for duplicate forward mapping
            if (ForwardMap.find(pair.first) != ForwardMap.end()) {
                CNC_LOGGER_FATAL("Duplicate key in forward map");
            }

            // Check for duplicate backward mapping
            if (BackwardMap.find(pair.second) != BackwardMap.end()) {
                CNC_LOGGER_FATAL("Duplicate key in backward map");
            }

            ForwardMap[pair.first] = pair.second;
            BackwardMap[pair.second] = pair.first;
        }
    }

    bool Has_Key(const A& key) const
    {
        return ForwardMap.find(key) != ForwardMap.end();
    }

    bool Has_Key(const B& key) const
    {
        return BackwardMap.find(key) != BackwardMap.end();
    }

    std::optional<B> operator[](const A& key) const
    {
        auto it = ForwardMap.find(key);
        return (it != ForwardMap.end()) ? std::make_optional(it->second) : std::nullopt;
    }

    std::optional<A> operator[](const B& key) const
    {
        auto it = BackwardMap.find(key);
        return (it != BackwardMap.end()) ? std::make_optional(it->second) : std::nullopt;
    }
};
