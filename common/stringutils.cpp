#include <algorithm>

#include "stringutils.h"

bool CncStringUtils::Is_Blank(const std::string& subject)
{
    return subject.empty() || std::ranges::all_of(subject, [](const auto c){ return std::isspace(c); });
}

bool CncStringUtils::Is_Blank(const std::string_view& subject)
{
    return subject.empty() || std::ranges::all_of(subject, [](const auto c){ return std::isspace(c); });
}

void CncStringUtils::To_Lower(std::string& subject)
{
    std::ranges::transform(subject, subject.begin(), ::tolower);
}

void CncStringUtils::To_Upper(std::string& subject)
{
    std::ranges::transform(subject, subject.begin(), ::toupper);
}

/**
 * Transforms a given string from any casing to title case, e.x. 'I am kane' -> 'I Am Kane'.
 *
 * @param subject String to transform.
 * @param word_separators List of characters that can be used to determine the start/end of a word.
 */
void CncStringUtils::To_Title_Case(std::string& subject, const std::vector<char>& word_separators)
{
    if (Is_Blank(subject)) {
        return;
    }

    if (subject.length() == 1) {
        To_Upper(subject);
        return;
    }

    // normalize string to lowercase
    To_Lower(subject);

    auto last_char_was_separator = false;

    // transform the first character to uppercase
    subject[0] = static_cast<char>(std::toupper(subject[0]));

    // transform any character that comes directly after a word separator to uppercase
    for (auto& c : subject) {
        if (last_char_was_separator) {
            c = static_cast<char>(std::toupper(c));
        }

        last_char_was_separator = c == ' ' || c == '/';
    }
}
