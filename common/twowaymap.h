#pragma once

#include <map>

/**
 * Map that enables mapping between two lists of values, in
 * either direction. Useful for converting between enums/constants
 * and strings, for example.
 */
template<typename A, typename B>
class TwoWayMap {
private:
    std::map<A, B> forward_map;
    std::map<B, A> backward_map;

public:
    TwoWayMap(std::initializer_list<std::pair<A, B>> pairs) {
        for (const auto& pair : pairs) {
            // TODO: Validate uniqueness of all pairs
            forward_map[pair.first] = pair.second;
            backward_map[pair.second] = pair.first;
        }
    }

    bool Has_Key(const A& key) const {
        return forward_map.find(key) != forward_map.end();
    }

    bool Has_Key(const B& key) const {
        return backward_map.find(key) != backward_map.end();
    }

    std::optional<B> operator[](const A& key) const {
        auto it = forward_map.find(key);
        return (it != forward_map.end()) ? std::make_optional(it->second) : std::nullopt;
    }

    std::optional<A> operator[](const B& key) const {
        auto it = backward_map.find(key);
        return (it != backward_map.end()) ? std::make_optional(it->second) : std::nullopt;
    }
};
