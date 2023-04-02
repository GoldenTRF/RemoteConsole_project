#include "UtilFuncs.h"
#include "client.h"



std::string utils::getAddress()
{
    std::string addressInput, portInput;
    std::unique_ptr<ValidatorBaseClass> ipValidator(new IpValidator());
    std::unique_ptr<ValidatorBaseClass> portValidator(new LocalAddrValidator());

    bool status = false;
    while (!status)
    {
        bool ipStatus = false;
        bool portStatus = false;

        std::cout << "Enter IP address and port: ";
        std::cin >> addressInput >> portInput;

        ipValidator->getValidation(addressInput, ipStatus);
        portValidator->getValidation(portInput, portStatus);

        if (!ipStatus || !portStatus)
        {
            system("cls");
            std::cout << "Invalid address input!\n";
        }
        else
        {
            status = true;
        }
    }

    return std::string(addressInput + ":" + portInput);
}

bool utils::connectionAlive(std::shared_ptr<GrpcClient> client)
{
    return client->connectionAlive();
}

void utils::errorExit()
{
    system("cls");
    std::cout << "Your connection with server has been lost!\n";
    system("pause");
    exit(CTRL_CLOSE_EVENT);
}