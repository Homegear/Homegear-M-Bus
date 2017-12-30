/* Copyright 2013-2017 Homegear UG (haftungsbeschränkt) */

#include "DescriptionCreator.h"
#include "GD.h"

namespace MyFamily
{

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
        uint8_t vif = dataRecord.vifs.front();
        parameter->metadata = "0x" + BaseLib::HelperFunctions::getHexString(dataRecord.vifs);

        ParameterCast::PGeneric cast = std::make_shared<ParameterCast::Generic>(GD::bl);
        cast->type = "0x" + BaseLib::HelperFunctions::getHexString(dif, 2);
        //Todo: Create unit and variable name look-up table
        if(vif == 0x13)
        {
            parameter->id = "VOLUME";
            parameter->unit = "l";
        }
        else if(vif == (uint8_t)0xFD && dataRecord.vifs.at(1) == 0x17) parameter->id = "ERROR_FLAGS";
        else
        {
            GD::out.printWarning("Warning: Unknown VIF " + BaseLib::HelperFunctions::getHexString(dataRecord.vifs));
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