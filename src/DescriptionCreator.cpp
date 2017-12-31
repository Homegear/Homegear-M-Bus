/* Copyright 2013-2017 Homegear UG (haftungsbeschränkt) */

#include "DescriptionCreator.h"
#include "GD.h"

namespace MyFamily
{

DescriptionCreator::DescriptionCreator()
{
    _vifVariableNameMap[0] = "ENERGY";
    _vifUnit[0] = "mWh";
    _vifVariableNameMap[1] = "ENERGY";
    _vifUnit[1] = "10^-2 Wh";
    _vifVariableNameMap[2] = "ENERGY";
    _vifUnit[2] = "10^-1 Wh";
    _vifVariableNameMap[3] = "ENERGY";
    _vifUnit[3] = "Wh";
    _vifVariableNameMap[4] = "ENERGY";
    _vifUnit[4] = "10^-2 kWh";
    _vifVariableNameMap[5] = "ENERGY";
    _vifUnit[5] = "10^-1 kWh";
    _vifVariableNameMap[6] = "ENERGY";
    _vifUnit[6] = "kWh";
    _vifVariableNameMap[7] = "ENERGY";
    _vifUnit[7] = "10^-2 MWh";
    _vifVariableNameMap[8] = "ENERGY";
    _vifUnit[8] = "J";
    _vifVariableNameMap[9] = "ENERGY";
    _vifUnit[9] = "10^-2 kJ";
    _vifVariableNameMap[10] = "ENERGY";
    _vifUnit[10] = "10^-1 kJ";
    _vifVariableNameMap[11] = "ENERGY";
    _vifUnit[11] = "kJ";
    _vifVariableNameMap[12] = "ENERGY";
    _vifUnit[12] = "10^-2 MJ";
    _vifVariableNameMap[13] = "ENERGY";
    _vifUnit[13] = "10^-1 MJ";
    _vifVariableNameMap[14] = "ENERGY";
    _vifUnit[14] = "MJ";
    _vifVariableNameMap[15] = "ENERGY";
    _vifUnit[15] = "10^-2 GJ";
    _vifVariableNameMap[16] = "VOLUME";
    _vifUnit[16] = "cm³";
    _vifVariableNameMap[17] = "VOLUME";
    _vifUnit[17] = "10^1 cm³";
    _vifVariableNameMap[18] = "VOLUME";
    _vifUnit[18] = "10^2 cm³";
    _vifVariableNameMap[19] = "VOLUME";
    _vifUnit[19] = "l";
    _vifVariableNameMap[20] = "VOLUME";
    _vifUnit[20] = "10^-2 m³";
    _vifVariableNameMap[21] = "VOLUME";
    _vifUnit[21] = "10^-1 m³";
    _vifVariableNameMap[22] = "VOLUME";
    _vifUnit[22] = "m³";
    _vifVariableNameMap[23] = "VOLUME";
    _vifUnit[23] = "10^1 m³";
    _vifVariableNameMap[24] = "ON_TIME";
    _vifUnit[24] = "s";
    _vifVariableNameMap[25] = "ON_TIME";
    _vifUnit[25] = "m";
    _vifVariableNameMap[26] = "ON_TIME";
    _vifUnit[26] = "h";
    _vifVariableNameMap[27] = "ON_TIME";
    _vifUnit[27] = "d";

    _vifFdVariableNameMap[23] = "ERROR_FLAGS_BINARY";
}

DescriptionCreator::PeerInfo DescriptionCreator::createDescription(PMyPacket packet)
{
    try
    {
        createDirectories();

        std::string id = BaseLib::HelperFunctions::getHexString(packet->senderAddress(), 8);

        std::shared_ptr<HomegearDevice> device = std::make_shared<HomegearDevice>(GD::bl);
        device->version = 1;
        PSupportedDevice supportedDevice = std::make_shared<SupportedDevice>(GD::bl, device.get());
        supportedDevice->id = id;
        supportedDevice->description = packet->getMediumString(packet->getMedium());
        supportedDevice->typeNumber = (uint32_t)packet->senderAddress();
        device->supportedDevices.push_back(supportedDevice);

        createXmlMaintenanceChannel(device);

        PFunction function = std::make_shared<Function>(GD::bl);
        function->channel = 1;
        function->type = "MBUS_CHANNEL_1";
        function->variablesId = "mbus_values_1";
        device->functions[1] = function;

        PPacket devicePacket = std::make_shared<Packet>(GD::bl);
        devicePacket->id = "INFO";
        device->packetsById[devicePacket->id] = devicePacket;
        devicePacket->channel = 1;
        devicePacket->direction = Packet::Direction::Enum::toCentral;
        devicePacket->type = packet->getControlInformation();

        auto dataRecords = packet->getDataRecords();
        for(auto& dataRecord : dataRecords)
        {
            PParameter parameter = std::make_shared<Parameter>(GD::bl, function->variables.get());
            parameter->readable = true;
            parameter->writeable = false;

            parseDataRecord(dataRecord, parameter, function, devicePacket);

            if(!parameter->casts.empty())
            {
                function->variables->parametersOrdered.push_back(parameter);
                function->variables->parameters[parameter->id] = parameter;
            }
        }

        std::string filename = _xmlPath + id + ".xml";
        device->save(filename);

        PeerInfo peerInfo;
        peerInfo.address = packet->senderAddress();
        peerInfo.serialNumber = "MBUS" + id;
        peerInfo.type = packet->senderAddress();
        return peerInfo;
    }
    catch(const std::exception& ex)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }

    return PeerInfo();
}

void DescriptionCreator::createDirectories()
{
    try
    {
        uid_t localUserId = GD::bl->hf.userId(GD::bl->settings.dataPathUser());
        gid_t localGroupId = GD::bl->hf.groupId(GD::bl->settings.dataPathGroup());
        if(((int32_t)localUserId) == -1 || ((int32_t)localGroupId) == -1)
        {
            localUserId = GD::bl->userId;
            localGroupId = GD::bl->groupId;
        }

        std::string path1 = GD::bl->settings.familyDataPath();
        std::string path2 = path1 + std::to_string(GD::family->getFamily()) + "/";
        _xmlPath = path2 + "desc/";
        if(!BaseLib::Io::directoryExists(path1)) BaseLib::Io::createDirectory(path1, GD::bl->settings.dataPathPermissions());
        if(localUserId != 0 || localGroupId != 0)
        {
            if(chown(path1.c_str(), localUserId, localGroupId) == -1) std::cerr << "Could not set owner on " << path1 << std::endl;
            if(chmod(path1.c_str(), GD::bl->settings.dataPathPermissions()) == -1) std::cerr << "Could not set permissions on " << path1 << std::endl;
        }
        if(!BaseLib::Io::directoryExists(path2)) BaseLib::Io::createDirectory(path2, GD::bl->settings.dataPathPermissions());
        if(localUserId != 0 || localGroupId != 0)
        {
            if(chown(path2.c_str(), localUserId, localGroupId) == -1) std::cerr << "Could not set owner on " << path2 << std::endl;
            if(chmod(path2.c_str(), GD::bl->settings.dataPathPermissions()) == -1) std::cerr << "Could not set permissions on " << path2 << std::endl;
        }
        if(!BaseLib::Io::directoryExists(_xmlPath)) BaseLib::Io::createDirectory(_xmlPath, GD::bl->settings.dataPathPermissions());
        if(localUserId != 0 || localGroupId != 0)
        {
            if(chown(_xmlPath.c_str(), localUserId, localGroupId) == -1) std::cerr << "Could not set owner on " << _xmlPath << std::endl;
            if(chmod(_xmlPath.c_str(), GD::bl->settings.dataPathPermissions()) == -1) std::cerr << "Could not set permissions on " << _xmlPath << std::endl;
        }
    }
    catch(const std::exception& ex)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void DescriptionCreator::createXmlMaintenanceChannel(PHomegearDevice& device)
{
    // {{{ Channel 0
    PFunction function(new Function(GD::bl));
    function->channel = 0;
    function->type = "MBUS_MAINTENANCE";
    function->variablesId = "mbus_maintenance_values";
    device->functions[function->channel] = function;

    PParameter parameter(new Parameter(GD::bl, function->variables.get()));
    parameter->id = "UNREACH";
    function->variables->parametersOrdered.push_back(parameter);
    function->variables->parameters[parameter->id] = parameter;
    parameter->writeable = false;
    parameter->service = true;
    parameter->logical = PLogicalBoolean(new LogicalBoolean(GD::bl));;
    parameter->physical = PPhysicalInteger(new PhysicalInteger(GD::bl));
    parameter->physical->groupId = parameter->id;
    parameter->physical->operationType = IPhysical::OperationType::internal;

    parameter.reset(new Parameter(GD::bl, function->variables.get()));
    parameter->id = "STICKY_UNREACH";
    function->variables->parametersOrdered.push_back(parameter);
    function->variables->parameters[parameter->id] = parameter;
    parameter->sticky = true;
    parameter->service = true;
    parameter->logical = PLogicalBoolean(new LogicalBoolean(GD::bl));;
    parameter->physical = PPhysicalInteger(new PhysicalInteger(GD::bl));
    parameter->physical->groupId = parameter->id;
    parameter->physical->operationType = IPhysical::OperationType::internal;
    // }}}
}

void DescriptionCreator::parseDataRecord(MyPacket::DataRecord& dataRecord, PParameter& parameter, PFunction& function, PPacket& packet)
{
    try
    {
        uint8_t dif = dataRecord.difs.front() & 0x0F;
        parameter->metadata = "0x" + BaseLib::HelperFunctions::getHexString(dataRecord.vifs);

        ParameterCast::PGeneric cast = std::make_shared<ParameterCast::Generic>(GD::bl);
        cast->type = "0x" + BaseLib::HelperFunctions::getHexString(dif, 2);

        parameter->physical = std::make_shared<PhysicalInteger>(GD::bl);
        parameter->physical->operationType = IPhysical::OperationType::Enum::command;
        std::shared_ptr<Parameter::Packet> eventPacket = std::make_shared<Parameter::Packet>();
        eventPacket->type = Parameter::Packet::Type::Enum::event;
        eventPacket->id = "INFO";
        parameter->eventPackets.push_back(eventPacket);

        if(dif == 0 || dif == 1 || dif == 2 || dif == 3 || dif == 4 || dif == 6 || dif == 7 || dif == 9 || dif == 10 || dif == 11 || dif == 12 || dif == 14)
        {
            parameter->logical = std::make_shared<LogicalInteger>(GD::bl);
        }
        else if(dif == 5)
        {
            parameter->logical = std::make_shared<LogicalDecimal>(GD::bl);
        }
        else if(dif == 8 || dif == 13 || dif == 15)
        {
            GD::out.printWarning("Warning: DIF 0x" + BaseLib::HelperFunctions::getHexString(dif) + " is currently not supported.");
            return;
        }

        if(dataRecord.vifs.size() == 1)
        {
            auto vifIterator = _vifVariableNameMap.find(dataRecord.vifs.front());
            if(vifIterator == _vifVariableNameMap.end())
            {
                GD::out.printWarning("Warning: VIF 0x" + BaseLib::HelperFunctions::getHexString(dataRecord.vifs) + " is currently not supported.");
                return;
            }
            parameter->id = vifIterator->second;

            auto unitIterator = _vifUnit.find(dataRecord.vifs.front());
            if(unitIterator != _vifUnit.end()) parameter->unit = unitIterator->second;
        }
        else if(dataRecord.vifs.size() == 2)
        {
            if(dataRecord.vifs.front() == 0xFD)
            {
                auto vifIterator = _vifFdVariableNameMap.find(dataRecord.vifs.at(1));
                if(vifIterator == _vifFdVariableNameMap.end())
                {
                    GD::out.printWarning("Warning: VIF 0x" + BaseLib::HelperFunctions::getHexString(dataRecord.vifs) + " is currently not supported.");
                    return;
                }
                parameter->id = vifIterator->second;

                auto unitIterator = _vifFdUnit.find(dataRecord.vifs.at(1));
                if(unitIterator != _vifFdUnit.end()) parameter->unit = unitIterator->second;
            }
            else
            {
                GD::out.printWarning("Warning: VIF 0x" + BaseLib::HelperFunctions::getHexString(dataRecord.vifs) + " is currently not supported.");
                return;
            }
        }
        else
        {
            GD::out.printWarning("Warning: VIF 0x" + BaseLib::HelperFunctions::getHexString(dataRecord.vifs) + " is currently not supported.");
            return;
        }

        if((int32_t)dataRecord.difFunction > 0) parameter->id += "_F" + std::to_string((int32_t)dataRecord.difFunction);
        if(dataRecord.subunit > 0) parameter->id += "_SU" + std::to_string(dataRecord.subunit);
        if(dataRecord.storageNumber > 0) parameter->id  += "_SN" + std::to_string(dataRecord.storageNumber);
        if(dataRecord.tariff > 0) parameter->id  += "_T" + std::to_string(dataRecord.tariff);
        parameter->id = getFreeParameterId(parameter->id, function);
        if(parameter->id.empty()) return;

        PBinaryPayload payload = std::make_shared<BinaryPayload>(GD::bl);
        payload->bitIndex = dataRecord.dataStart * 8;
        payload->bitSize = dataRecord.dataSize * 8;
        payload->metaInteger1 = (int32_t)dataRecord.difFunction;
        payload->metaInteger2 = dataRecord.subunit;
        payload->metaInteger3 = dataRecord.storageNumber;
        payload->metaInteger4 = dataRecord.tariff;
        payload->parameterId = parameter->id;
        packet->binaryPayloads.push_back(payload);

        parameter->casts.push_back(cast);
    }
    catch(const std::exception& ex)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

std::string DescriptionCreator::getFreeParameterId(std::string baseId, PFunction& function)
{
    try
    {
        if(function->variables->parameters.find(baseId) != function->variables->parameters.end())
        {
            int32_t i = 1;
            std::string currentId = baseId + "_" + std::to_string(i);
            while(function->variables->parameters.find(currentId) != function->variables->parameters.end())
            {
                i++;
                currentId = baseId + "_" + std::to_string(i);
            }
            return currentId;
        }
        return baseId;
    }
    catch(const std::exception& ex)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return "";
}

}