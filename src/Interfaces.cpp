/* Copyright 2013-2017 Homegear UG (haftungsbeschränkt) */

#include "Interfaces.h"
#include "GD.h"
#include "PhysicalInterfaces/Amber.h"

namespace MyFamily
{

Interfaces::Interfaces(BaseLib::SharedObjects* bl, std::map<std::string, Systems::PPhysicalInterfaceSettings> physicalInterfaceSettings) : Systems::PhysicalInterfaces(bl, GD::family->getFamily(), physicalInterfaceSettings)
{
	create();
}

Interfaces::~Interfaces()
{
}

void Interfaces::create()
{
	try
	{
		for(std::map<std::string, Systems::PPhysicalInterfaceSettings>::iterator i = _physicalInterfaceSettings.begin(); i != _physicalInterfaceSettings.end(); ++i)
		{
			std::shared_ptr<IMBusInterface> device;
			if(!i->second) continue;
			GD::out.printDebug("Debug: Creating physical device. Type defined in mbus.conf is: " + i->second->type);
			if(i->second->type == "amber") device.reset(new Amber(i->second));
			else GD::out.printError("Error: Unsupported physical device type: " + i->second->type);
			if(device)
			{
				if(_physicalInterfaces.find(i->second->id) != _physicalInterfaces.end()) GD::out.printError("Error: id used for two devices: " + i->second->id);
				_physicalInterfaces[i->second->id] = device;
				GD::physicalInterfaces[i->second->id] = device;
				if(i->second->isDefault || !GD::defaultPhysicalInterface) GD::defaultPhysicalInterface = device;
			}
		}
		if(!GD::defaultPhysicalInterface) GD::defaultPhysicalInterface = std::make_shared<IMBusInterface>(std::make_shared<BaseLib::Systems::PhysicalInterfaceSettings>());
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

}
