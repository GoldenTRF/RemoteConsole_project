#pragma once

#include "ValidatorBaseClass.h"

class IpValidator : public ValidatorBaseClass
{
public:
    void getValidation(const std::string&, bool&) override;
};
