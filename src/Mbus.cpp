/* Copyright 2013-2019 Homegear GmbH */

#include "GD.h"
#include "Interfaces.h"
#include "Mbus.h"
#include "MbusCentral.h"

namespace Mbus
{

Mbus::Mbus(BaseLib::SharedObjects* bl, BaseLib::Systems::IFamilyEventSink* eventHandler) : BaseLib::Systems::DeviceFamily(bl, eventHandler, MY_FAMILY_ID, MY_FAMILY_NAME)
{
	GD::bl = bl;
	GD::family = this;
	GD::out.init(bl);
	GD::out.setPrefix(std::string("Module ") + MY_FAMILY_NAME + ": ");
	GD::out.printDebug("Debug: Loading module...");
    GD::interfaces = std::make_shared<Interfaces>(bl, _settings->getPhysicalInterfaceSettings());
    _physicalInterfaces = GD::interfaces;
}

Mbus::~Mbus()
{

}

bool Mbus::init()
{
	_bl->out.printInfo("Loading XML RPC devices...");
	std::string xmlPath = _bl->settings.familyDataPath() + std::to_string(GD::family->getFamily()) + "/desc/";
	BaseLib::Io io;
	io.init(_bl);
	if(BaseLib::Io::directoryExists(xmlPath) && !io.getFiles(xmlPath).empty()) _rpcDevices->load(xmlPath);
	return true;
}

void Mbus::dispose()
{
	if(_disposed) return;
	DeviceFamily::dispose();
	_central.reset();
    GD::interfaces.reset();
    _physicalInterfaces.reset();
}

void Mbus::reloadRpcDevices()
{
	_bl->out.printInfo("Reloading XML RPC devices...");
	std::string xmlPath = _bl->settings.familyDataPath() + std::to_string(GD::family->getFamily()) + "/desc/";
	if(BaseLib::Io::directoryExists(xmlPath)) _rpcDevices->load(xmlPath);
}

void Mbus::createCentral()
{
	try
	{
		_central.reset(new MbusCentral(0, "VMBUS00001", this));
		GD::out.printMessage("Created central with id " + std::to_string(_central->getId()) + ".");
	}
	catch(const std::exception& ex)
    {
    	GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

std::shared_ptr<BaseLib::Systems::ICentral> Mbus::initializeCentral(uint32_t deviceId, int32_t address, std::string serialNumber)
{
	return std::shared_ptr<MbusCentral>(new MbusCentral(deviceId, serialNumber, this));
}

PVariable Mbus::getPairingInfo()
{
	try
	{
		if(!_central) return PVariable(new Variable(VariableType::tArray));
		PVariable array(new Variable(VariableType::tArray));
		array->arrayValue->push_back(PVariable(new Variable(std::string("setInstallMode"))));
		return array;
	}
	catch(const std::exception& ex)
	{
		GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	return Variable::createError(-32500, "Unknown application error.");
}
}
