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
            if (ForwardMap.contains(pair.first)) {
                CNC_LOGGER_FATAL("Duplicate key in forward map");
            }

            // Check for duplicate backward mapping
            if (BackwardMap.contains(pair.second)) {
                CNC_LOGGER_FATAL("Duplicate key in backward map");
            }

            ForwardMap[pair.first] = pair.second;
            BackwardMap[pair.second] = pair.first;
        }
    }

    bool Has_Key(const A& key) const
    {
        return ForwardMap.contains(key);
    }

    bool Has_Key(const B& key) const
    {
        return BackwardMap.contains(key);
    }

    std::optional<B> operator[](const A& key) const
    {
        if (!ForwardMap.contains(key)) {
            return std::nullopt;
        }

        return ForwardMap.at(key);
    }

    std::optional<A> operator[](const B& key) const
    {
        if (!BackwardMap.contains(key)) {
            return std::nullopt;
        }

        return BackwardMap.at(key);
    }

    std::vector<A> Get_Forward_Keys() const
    {
        std::vector<A> keys;
        keys.reserve(ForwardMap.size());

        for (const auto& key : ForwardMap | std::views::keys) {
            keys.emplace_back(key);
        }

        return keys;
    }

    std::vector<B> Get_Backward_Keys() const
    {
        std::vector<B> keys;
        keys.reserve(BackwardMap.size());

        for (const auto& key : BackwardMap | std::views::keys) {
            keys.emplace_back(key);
        }

        return keys;
    }
};
