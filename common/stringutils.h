#pragma once

#include <functional>
#include <sstream>
#include <string>
#include <vector>

class CncStringUtils final
{
public:
    [[nodiscard]] static bool Is_Blank(const std::string& subject)
    {
       return subject.empty() || std::all_of(
           subject.begin(), subject.end(), [](const auto c){ return std::isspace(c); }
       );
    }

    static void To_Lower(std::string& subject)
    {
        std::transform(subject.begin(), subject.end(), subject.begin(), ::tolower);
    }

    static void To_Upper(std::string& subject)
    {
        std::transform(subject.begin(), subject.end(), subject.begin(), ::toupper);
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