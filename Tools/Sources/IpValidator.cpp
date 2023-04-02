#include "IpValidator.h"

#include <vector>

void IpValidator::getValidation(const std::string& ipAddr, bool& status)
{
    if (ipAddr == "localhost")
    {
        status = true;
        return;
    }
        
    std::vector<std::string> ipNumbersList;
    std::string addressNumber{};

    for (std::size_t pos = 0; pos < ipAddr.size(); pos++)
    {
        if (ipAddr[pos] == '.')
        {
            ipNumbersList.push_back(addressNumber);
            addressNumber.clear();
            continue;
        }

        if (pos == (ipAddr.size() - 1))
        {
            addressNumber += ipAddr[pos];
            ipNumbersList.push_back(addressNumber);
            addressNumber.clear();
            break;
        }
        addressNumber += ipAddr[pos];
    }

    if (ipNumbersList.size() != 4)
    {
        status = false;
        return;
    }

    auto isNumber = [](const std::string& str) {
        return !str.empty() && (str.find_first_not_of("[0123456789]") == std::string::npos);
    };

    for (auto& str : ipNumbersList)
    {
        if (!isNumber(str) || std::stoi(str) > 255 || std::stoi(str) < 0)
        {
            status = false;
            return;
        }
    }

    status = true;
    return;
}