#include "LocalAddrValidator.h"

void LocalAddrValidator::getValidation(const std::string& userInput, bool& status)
{
	int portInt = 0;

	try
	{
		portInt = std::stoi(userInput);
	}
	catch (const std::exception&)
	{
		status = false;
		return;
	}

	if (portInt < 1 || portInt > 65535)
	{
		status = false;
		return;
	}
	status = true;
}