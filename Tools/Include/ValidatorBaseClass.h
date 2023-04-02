#pragma once

#include <string>

class ValidatorBaseClass
{
public:
    void virtual getValidation(const std::string&, bool&) = 0;
    virtual ~ValidatorBaseClass(){}
};
