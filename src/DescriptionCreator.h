/* Copyright 2013-2017 Homegear UG (haftungsbeschränkt) */

#ifndef HOMEGEAR_MBUS_DESCRIPTIONCREATOR_H
#define HOMEGEAR_MBUS_DESCRIPTIONCREATOR_H

#include <homegear-base/BaseLib.h>
#include "MyPacket.h"

#include <sys/stat.h>

namespace MyFamily
{

class DescriptionCreator
{
public:
    struct PeerInfo
    {
        std::string serialNumber;
        int32_t address = -1;
        int32_t type = -1;
    };

    DescriptionCreator();
    virtual ~DescriptionCreator() = default;

    DescriptionCreator::PeerInfo createDescription(PMyPacket packet);
private:
    std::map<uint8_t, std::string> _vifVariableNameMap;
    std::map<uint8_t, std::string> _vifUnit;
    std::map<uint8_t, std::string> _vifFbVariableNameMap;
    std::map<uint8_t, std::string> _vifFbUnit;
    std::map<uint8_t, std::string> _vifFdVariableNameMap;
    std::map<uint8_t, std::string> _vifFdUnit;
    std::string _xmlPath;

    void createDirectories();
    void createXmlMaintenanceChannel(PHomegearDevice& device);
    std::string getFreeParameterId(std::string baseId, PFunction& function);
    void parseDataRecord(MyPacket::DataRecord& dataRecord, PParameter& parameter, PFunction& function, PPacket& packet);
};

}

#endif
