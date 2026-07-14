#pragma once

#include <format>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

class CncStringUtils final
{
public:
    static bool Is_Blank(const std::string& subject);
    static bool Is_Blank(const std::string_view& subject);

    static void To_Lower(std::string& subject);
    static void To_Upper(std::string& subject);
    static void To_Title_Case(std::string& subject, const std::vector<char>& word_separators = { ' ', '/' });

    template<class T = std::string>
    static std::string To_Csv(
        const std::vector<T>& items,
        const std::function<std::string(const T&)>& to_string = [](const T& s) { return std::format("{}", s); }
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
