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
    _vifUnit[1] = "10⁻² Wh";
    _vifVariableNameMap[2] = "ENERGY";
    _vifUnit[2] = "10⁻¹ Wh";
    _vifVariableNameMap[3] = "ENERGY";
    _vifUnit[3] = "Wh";
    _vifVariableNameMap[4] = "ENERGY";
    _vifUnit[4] = "10⁻² kWh";
    _vifVariableNameMap[5] = "ENERGY";
    _vifUnit[5] = "10⁻¹ kWh";
    _vifVariableNameMap[6] = "ENERGY";
    _vifUnit[6] = "kWh";
    _vifVariableNameMap[7] = "ENERGY";
    _vifUnit[7] = "10¹ kWh";

    _vifVariableNameMap[8] = "ENERGY";
    _vifUnit[8] = "J";
    _vifVariableNameMap[9] = "ENERGY";
    _vifUnit[9] = "10⁻² kJ";
    _vifVariableNameMap[10] = "ENERGY";
    _vifUnit[10] = "10⁻¹ kJ";
    _vifVariableNameMap[11] = "ENERGY";
    _vifUnit[11] = "kJ";
    _vifVariableNameMap[12] = "ENERGY";
    _vifUnit[12] = "10⁻² MJ";
    _vifVariableNameMap[13] = "ENERGY";
    _vifUnit[13] = "10⁻¹ MJ";
    _vifVariableNameMap[14] = "ENERGY";
    _vifUnit[14] = "MJ";
    _vifVariableNameMap[15] = "ENERGY";
    _vifUnit[15] = "10¹ MJ";

    _vifVariableNameMap[16] = "VOLUME";
    _vifUnit[16] = "cm³";
    _vifVariableNameMap[17] = "VOLUME";
    _vifUnit[17] = "10¹ cm³";
    _vifVariableNameMap[18] = "VOLUME";
    _vifUnit[18] = "10² cm³";
    _vifVariableNameMap[19] = "VOLUME";
    _vifUnit[19] = "l";
    _vifVariableNameMap[20] = "VOLUME";
    _vifUnit[20] = "10⁻² m³";
    _vifVariableNameMap[21] = "VOLUME";
    _vifUnit[21] = "10⁻¹ m³";
    _vifVariableNameMap[22] = "VOLUME";
    _vifUnit[22] = "m³";
    _vifVariableNameMap[23] = "VOLUME";
    _vifUnit[23] = "10¹ m³";

    _vifVariableNameMap[24] = "MASS";
    _vifUnit[24] = "g";
    _vifVariableNameMap[25] = "MASS";
    _vifUnit[25] = "10⁻² kg";
    _vifVariableNameMap[26] = "MASS";
    _vifUnit[26] = "10⁻¹ kg";
    _vifVariableNameMap[27] = "MASS";
    _vifUnit[27] = "kg";
    _vifVariableNameMap[28] = "MASS";
    _vifUnit[28] = "10⁻² t";
    _vifVariableNameMap[29] = "MASS";
    _vifUnit[29] = "10⁻¹ t";
    _vifVariableNameMap[30] = "MASS";
    _vifUnit[30] = "t";
    _vifVariableNameMap[31] = "MASS";
    _vifUnit[31] = "10¹ t";

    _vifVariableNameMap[32] = "ON_TIME";
    _vifUnit[32] = "s";
    _vifVariableNameMap[33] = "ON_TIME";
    _vifUnit[33] = "m";
    _vifVariableNameMap[34] = "ON_TIME";
    _vifUnit[34] = "h";
    _vifVariableNameMap[35] = "ON_TIME";
    _vifUnit[35] = "d";

    _vifVariableNameMap[36] = "OPERATING_TIME";
    _vifUnit[36] = "s";
    _vifVariableNameMap[37] = "OPERATING_TIME";
    _vifUnit[37] = "m";
    _vifVariableNameMap[38] = "OPERATING_TIME";
    _vifUnit[38] = "h";
    _vifVariableNameMap[39] = "OPERATING_TIME";
    _vifUnit[39] = "d";

    _vifVariableNameMap[40] = "POWER";
    _vifUnit[40] = "mW";
    _vifVariableNameMap[41] = "POWER";
    _vifUnit[41] = "10⁻² W";
    _vifVariableNameMap[42] = "POWER";
    _vifUnit[42] = "10⁻¹ W";
    _vifVariableNameMap[43] = "POWER";
    _vifUnit[43] = "W";
    _vifVariableNameMap[44] = "POWER";
    _vifUnit[44] = "10⁻² kW";
    _vifVariableNameMap[45] = "POWER";
    _vifUnit[45] = "10⁻¹ kW";
    _vifVariableNameMap[46] = "POWER";
    _vifUnit[46] = "kW";
    _vifVariableNameMap[47] = "POWER";
    _vifUnit[47] = "10¹ kW";

    _vifVariableNameMap[48] = "POWER";
    _vifUnit[48] = "J/h";
    _vifVariableNameMap[49] = "POWER";
    _vifUnit[49] = "10⁻² kJ/h";
    _vifVariableNameMap[50] = "POWER";
    _vifUnit[50] = "10⁻¹ kJ/h";
    _vifVariableNameMap[51] = "POWER";
    _vifUnit[51] = "kJ/J";
    _vifVariableNameMap[52] = "POWER";
    _vifUnit[52] = "10⁻² MJ/h";
    _vifVariableNameMap[53] = "POWER";
    _vifUnit[53] = "10⁻¹ MJ/h";
    _vifVariableNameMap[54] = "POWER";
    _vifUnit[54] = "MJ";
    _vifVariableNameMap[55] = "POWER";
    _vifUnit[55] = "10¹ MJ";

    _vifVariableNameMap[56] = "VOLUME_FLOW";
    _vifUnit[56] = "cm³/h";
    _vifVariableNameMap[57] = "VOLUME_FLOW";
    _vifUnit[57] = "10¹ cm³/h";
    _vifVariableNameMap[58] = "VOLUME_FLOW";
    _vifUnit[58] = "10² cm³/h";
    _vifVariableNameMap[59] = "VOLUME_FLOW";
    _vifUnit[59] = "l/h";
    _vifVariableNameMap[60] = "VOLUME_FLOW";
    _vifUnit[60] = "10⁻² m³/h";
    _vifVariableNameMap[61] = "VOLUME_FLOW";
    _vifUnit[61] = "10⁻¹ m³/h";
    _vifVariableNameMap[62] = "VOLUME_FLOW";
    _vifUnit[62] = "m³/h";
    _vifVariableNameMap[63] = "VOLUME_FLOW";
    _vifUnit[63] = "10¹ m³/h";

    _vifVariableNameMap[64] = "VOLUME_FLOW";
    _vifUnit[64] = "10⁻¹ cm³/min";
    _vifVariableNameMap[65] = "VOLUME_FLOW";
    _vifUnit[65] = "cm³/min";
    _vifVariableNameMap[66] = "VOLUME_FLOW";
    _vifUnit[66] = "10¹ cm³/min";
    _vifVariableNameMap[67] = "VOLUME_FLOW";
    _vifUnit[67] = "10² cm³/min";
    _vifVariableNameMap[68] = "VOLUME_FLOW";
    _vifUnit[68] = "l/min";
    _vifVariableNameMap[69] = "VOLUME_FLOW";
    _vifUnit[69] = "10⁻² m³/min";
    _vifVariableNameMap[70] = "VOLUME_FLOW";
    _vifUnit[70] = "10⁻¹ m³/min";
    _vifVariableNameMap[71] = "VOLUME_FLOW";
    _vifUnit[71] = "m³/min";

    _vifVariableNameMap[72] = "VOLUME_FLOW";
    _vifUnit[72] = "mm³/s";
    _vifVariableNameMap[73] = "VOLUME_FLOW";
    _vifUnit[73] = "10⁻² cm³/s";
    _vifVariableNameMap[74] = "VOLUME_FLOW";
    _vifUnit[74] = "10⁻¹ cm³/s";
    _vifVariableNameMap[75] = "VOLUME_FLOW";
    _vifUnit[75] = "cm³/s";
    _vifVariableNameMap[76] = "VOLUME_FLOW";
    _vifUnit[76] = "10¹ cm³/s";
    _vifVariableNameMap[77] = "VOLUME_FLOW";
    _vifUnit[77] = "10² cm³/s";
    _vifVariableNameMap[78] = "VOLUME_FLOW";
    _vifUnit[78] = "l/s";
    _vifVariableNameMap[79] = "VOLUME_FLOW";
    _vifUnit[79] = "10⁻² m³/s";

    _vifVariableNameMap[80] = "MASS_FLOW";
    _vifUnit[80] = "g/h";
    _vifVariableNameMap[81] = "MASS_FLOW";
    _vifUnit[81] = "10⁻² kg/h";
    _vifVariableNameMap[82] = "MASS_FLOW";
    _vifUnit[82] = "10⁻¹ kg/h";
    _vifVariableNameMap[83] = "MASS_FLOW";
    _vifUnit[83] = "kg/h";
    _vifVariableNameMap[84] = "MASS_FLOW";
    _vifUnit[84] = "10⁻² t/h";
    _vifVariableNameMap[85] = "MASS_FLOW";
    _vifUnit[85] = "10⁻¹ t/h";
    _vifVariableNameMap[86] = "MASS_FLOW";
    _vifUnit[86] = "t/h";
    _vifVariableNameMap[87] = "MASS_FLOW";
    _vifUnit[87] = "10¹ t/h";

    _vifVariableNameMap[88] = "FLOW_TEMPERATURE";
    _vifUnit[88] = "10⁻³ °C";
    _vifVariableNameMap[89] = "FLOW_TEMPERATURE";
    _vifUnit[89] = "10⁻² °C";
    _vifVariableNameMap[90] = "FLOW_TEMPERATURE";
    _vifUnit[90] = "10⁻¹ °C";
    _vifVariableNameMap[91] = "FLOW_TEMPERATURE";
    _vifUnit[91] = "°C";

    _vifVariableNameMap[92] = "RETURN_TEMPERATURE";
    _vifUnit[92] = "10⁻³ °C";
    _vifVariableNameMap[93] = "RETURN_TEMPERATURE";
    _vifUnit[93] = "10⁻² °C";
    _vifVariableNameMap[94] = "RETURN_TEMPERATURE";
    _vifUnit[94] = "10⁻¹ °C";
    _vifVariableNameMap[95] = "RETURN_TEMPERATURE";
    _vifUnit[95] = "°C";

    _vifVariableNameMap[96] = "TEMPERATURE_DIFFERENCE";
    _vifUnit[96] = "mK";
    _vifVariableNameMap[97] = "TEMPERATURE_DIFFERENCE";
    _vifUnit[97] = "10⁻² K";
    _vifVariableNameMap[98] = "TEMPERATURE_DIFFERENCE";
    _vifUnit[98] = "10⁻¹ K";
    _vifVariableNameMap[99] = "TEMPERATURE_DIFFERENCE";
    _vifUnit[99] = "K";

    _vifVariableNameMap[100] = "EXTERNAL_TEMPERATURE";
    _vifUnit[100] = "10⁻³ °C";
    _vifVariableNameMap[101] = "EXTERNAL_TEMPERATURE";
    _vifUnit[101] = "10⁻² °C";
    _vifVariableNameMap[102] = "EXTERNAL_TEMPERATURE";
    _vifUnit[102] = "10⁻¹ °C";
    _vifVariableNameMap[103] = "EXTERNAL_TEMPERATURE";
    _vifUnit[103] = "°C";

    _vifVariableNameMap[104] = "PRESSURE";
    _vifUnit[104] = "mbar";
    _vifVariableNameMap[105] = "PRESSURE";
    _vifUnit[105] = "10⁻² bar";
    _vifVariableNameMap[106] = "PRESSURE";
    _vifUnit[106] = "10⁻¹ bar";
    _vifVariableNameMap[107] = "PRESSURE";
    _vifUnit[107] = "bar";

    _vifVariableNameMap[108] = "DATE";
    _vifVariableNameMap[109] = "DATETIME";

    _vifVariableNameMap[110] = "HCA_UNITS";

    //111 reserved

    _vifVariableNameMap[112] = "AVERAGING_DURATION";
    _vifUnit[112] = "s";
    _vifVariableNameMap[113] = "AVERAGING_DURATION";
    _vifUnit[113] = "m";
    _vifVariableNameMap[114] = "AVERAGING_DURATION";
    _vifUnit[114] = "h";
    _vifVariableNameMap[115] = "AVERAGING_DURATION";
    _vifUnit[115] = "d";

    _vifVariableNameMap[116] = "ACTUALITY_DURATION";
    _vifUnit[116] = "s";
    _vifVariableNameMap[117] = "ACTUALITY_DURATION";
    _vifUnit[117] = "m";
    _vifVariableNameMap[118] = "ACTUALITY_DURATION";
    _vifUnit[118] = "h";
    _vifVariableNameMap[119] = "ACTUALITY_DURATION";
    _vifUnit[119] = "d";

    _vifVariableNameMap[120] = "FABRICATION_NO";
    _vifVariableNameMap[121] = "ENHANCED_IDENTIFICATION";
    _vifVariableNameMap[122] = "BUS_ADDRESS";

    _vifVariableNameMap[0x7C] = "CUSTOM_STRING";
    _vifVariableNameMap[0xFC] = "CUSTOM_STRING";
    _vifVariableNameMap[0x7E] = "ANY_VIF";
    _vifVariableNameMap[0xFE] = "ANY_VIF";
    _vifVariableNameMap[0x7F] = "MANUFACTURER_SPECIFIC";
    _vifVariableNameMap[0xFF] = "MANUFACTURER_SPECIFIC";


    _vifFbVariableNameMap[0] = "ENERGY";
    _vifFbUnit[0] = "10⁻¹ MWh";
    _vifFbVariableNameMap[1] = "ENERGY";
    _vifFbUnit[1] = "MWh";

    //2 to 7 reserved

    _vifFbVariableNameMap[8] = "ENERGY";
    _vifFbUnit[8] = "10⁻¹ GJ";
    _vifFbVariableNameMap[9] = "ENERGY";
    _vifFbUnit[9] = "GJ";

    //10 to 15 reserved

    _vifFbVariableNameMap[16] = "VOLUME";
    _vifFbUnit[16] = "10² m³";
    _vifFbVariableNameMap[17] = "VOLUME";
    _vifFbUnit[17] = "10³ m³";

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
        device->timeout = 86400;
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
        devicePacket->type = 1;

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
        parameter->metadata = BaseLib::HelperFunctions::getHexString(dataRecord.vifs);

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
            if(vifIterator == _vifVariableNameMap.end()) parameter->id = "UNKNOWN_" + BaseLib::HelperFunctions::getHexString(dataRecord.vifs.front(), 2);
            else parameter->id = vifIterator->second;

            auto unitIterator = _vifUnit.find(dataRecord.vifs.front());
            if(unitIterator != _vifUnit.end()) parameter->unit = unitIterator->second;
        }
        else if(dataRecord.vifs.size() == 2)
        {
            if(dataRecord.vifs.front() == 0xFB)
            {
                auto vifIterator = _vifFbVariableNameMap.find(dataRecord.vifs.at(1));
                if(vifIterator == _vifFbVariableNameMap.end()) parameter->id = "UNKNOWN_" + BaseLib::HelperFunctions::getHexString(dataRecord.vifs);
                else parameter->id = vifIterator->second;

                auto unitIterator = _vifFbUnit.find(dataRecord.vifs.at(1));
                if(unitIterator != _vifFbUnit.end()) parameter->unit = unitIterator->second;
            }
            else if(dataRecord.vifs.front() == 0xFD)
            {
                auto vifIterator = _vifFdVariableNameMap.find(dataRecord.vifs.at(1));
                if(vifIterator == _vifFdVariableNameMap.end()) parameter->id = "UNKNOWN_" + BaseLib::HelperFunctions::getHexString(dataRecord.vifs);
                else parameter->id = vifIterator->second;

                auto unitIterator = _vifFdUnit.find(dataRecord.vifs.at(1));
                if(unitIterator != _vifFdUnit.end()) parameter->unit = unitIterator->second;
            }
            else
            {
                GD::out.printWarning("Warning: Invalid VIF array: " + BaseLib::HelperFunctions::getHexString(dataRecord.vifs));
                return;
            }
        }
        else
        {
            GD::out.printWarning("Warning: Invalid VIF array." + BaseLib::HelperFunctions::getHexString(dataRecord.vifs));
            return;
        }

        if((int32_t)dataRecord.difFunction > 0) parameter->id += "_F" + std::to_string((int32_t)dataRecord.difFunction);
        if(dataRecord.subunit > 0) parameter->id += "_SU" + std::to_string(dataRecord.subunit);
        if(dataRecord.storageNumber > 0) parameter->id  += "_SN" + std::to_string(dataRecord.storageNumber);
        if(dataRecord.tariff > 0) parameter->id  += "_T" + std::to_string(dataRecord.tariff);
        parameter->id = getFreeParameterId(parameter->id, function);
        if(parameter->id.empty()) return;
        parameter->physical->groupId = parameter->id;

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