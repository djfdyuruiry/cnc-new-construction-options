#include <algorithm>

#include "stringutils.h"

bool CncStringUtils::Is_Blank(const std::string& subject)
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
