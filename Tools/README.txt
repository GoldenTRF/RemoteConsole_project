This folder containes the static lib (.lib) "Tools_lib.lib" and using mostly for client "user interface".

The server application has a real low chance to get used this lib, so this folder can be called as "Client Application Heplful Functions".

For Server application will be created another one lib that will containe the functions can help in server app working.


This lib containes:
1. Validators files (IpValidator, AddressValidator, OperationValidator, LocalAddrValidator and ValidatorBaseClass) FILES: ".cpp"/".h"
2. EnumController namespace that containes the helpful functions for validators ans user input functions. Also there are all of enums that been used in "Client"-app. FILES: "EnumController.h"
3. Utility functions that responsible for "user interface" operation. There are lot of functions that regulate the scenario going through user can run different command (ex. add connection information to file, make connection, enter the ip or local address) and other. FILES: "UtilsFuncs.h"/"UtilsFuncs.cpp"