#pragma once

#include <functional>
#include <sstream>
#include <string>
#include <vector>

class CncStringUtils final
{
public:
    template<class T = std::string>
    static std::string To_Csv(std::vector<T> items, std::function<std::string(T)> to_string = [](auto s) { return s;})
    {
        std::ostringstream oss;
        auto first = true;

        for (const auto& instance : items) {
            if (!first) {
                oss << ",";
            }

            oss << to_string(instance);
            first = false;
        }

        return oss.str();
    }

private:
    CncStringUtils() = delete;
};