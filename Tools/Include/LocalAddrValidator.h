#pragma once

#include "ValidatorBaseClass.h"

class LocalAddrValidator : public ValidatorBaseClass
{
public:
    void getValidation(const std::string&, bool&) override;
};

