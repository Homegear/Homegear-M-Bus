/* Copyright 2013-2017 Homegear UG (haftungsbeschränkt) */

#ifndef HOMEGEAR_MBUS_VIFCONVERTER_H
#define HOMEGEAR_MBUS_VIFCONVERTER_H

#include <homegear-base/BaseLib.h>

using namespace BaseLib;

namespace MyFamily
{

class VifConverter
{
public:
    VifConverter() = default;
    virtual ~VifConverter() = default;

    PVariable getVariable(uint8_t type, std::vector<uint8_t>& vifs, const std::vector<uint8_t>& value);
private:

};

}

#endif
