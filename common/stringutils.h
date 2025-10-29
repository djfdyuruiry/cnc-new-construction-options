#pragma once

#include <algorithm>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

class CncStringUtils final
{
public:
    [[nodiscard]] static bool Is_Blank(const std::string& subject)
    {
       return subject.empty() || std::ranges::all_of(subject, [](const auto c){ return std::isspace(c); }
       );
    }

    static void To_Lower(std::string& subject)
    {
        std::ranges::transform(subject, subject.begin(), ::tolower);
    }

    static void To_Upper(std::string& subject)
    {
        std::ranges::transform(subject, subject.begin(), ::toupper);
    }

    template<class T = std::string>
    static std::string To_Csv(std::vector<T> items, std::function<std::string(T)> to_string = [](auto s) { return s; })
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