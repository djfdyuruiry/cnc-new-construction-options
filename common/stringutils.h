#pragma once

#include <functional>
#include <sstream>
#include <string>
#include <vector>

class CncStringUtils final
{
public:
    static bool Is_Blank(const std::string& subject);

    static void To_Lower(std::string& subject);

    static void To_Upper(std::string& subject);

    template<class T = std::string>
    static std::string To_Csv(
        const std::vector<T>& items,
        const std::function<std::string(T)>& to_string = [](auto s) { return s; }
    )
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