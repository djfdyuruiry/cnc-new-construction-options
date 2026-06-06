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

void CncStringUtils::To_Title_Case(std::string& subject)
{
    if (Is_Blank(subject)) {
        return;
    }

    if (subject.length() == 1) {
        To_Upper(subject);
        return;
    }

    To_Lower(subject);
    subject[0] = std::toupper(subject[0]);
}

