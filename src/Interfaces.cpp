/* Copyright 2013-2019 Homegear GmbH */

#include "Interfaces.h"
#include "GD.h"
#include "PhysicalInterfaces/Amber.h"
#include "PhysicalInterfaces/Hgdc.h"

namespace Mbus {

Interfaces::Interfaces(BaseLib::SharedObjects *bl, std::map<std::string, Systems::PPhysicalInterfaceSettings> physicalInterfaceSettings) : Systems::PhysicalInterfaces(bl, GD::family->getFamily(), physicalInterfaceSettings) {
  create();
}

Interfaces::~Interfaces() {
  _physicalInterfaces.clear();
  _defaultPhysicalInterface.reset();
  _physicalInterfaceEventhandlers.clear();
}

void Interfaces::addEventHandlers(BaseLib::Systems::IPhysicalInterface::IPhysicalInterfaceEventSink *central) {
  try {
    std::lock_guard<std::mutex> interfaceGuard(_physicalInterfacesMutex);
    _central = central;
    for (const auto &interface: _physicalInterfaces) {
      if (_physicalInterfaceEventhandlers.find(interface.first) != _physicalInterfaceEventhandlers.end()) continue;
      _physicalInterfaceEventhandlers[interface.first] = interface.second->addEventHandler(central);
    }
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Interfaces::removeEventHandlers() {
  try {
    std::lock_guard<std::mutex> interfaceGuard(_physicalInterfacesMutex);
    for (const auto &interface: _physicalInterfaces) {
      auto physicalInterfaceEventhandler = _physicalInterfaceEventhandlers.find(interface.first);
      if (physicalInterfaceEventhandler == _physicalInterfaceEventhandlers.end()) continue;
      interface.second->removeEventHandler(physicalInterfaceEventhandler->second);
      _physicalInterfaceEventhandlers.erase(physicalInterfaceEventhandler);
    }
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Interfaces::create() {
  try {
    for (std::map<std::string, Systems::PPhysicalInterfaceSettings>::iterator i = _physicalInterfaceSettings.begin(); i != _physicalInterfaceSettings.end(); ++i) {
      std::shared_ptr<IMbusInterface> device;
      if (!i->second) continue;
      GD::out.printDebug("Debug: Creating physical device. Type defined in mbus.conf is: " + i->second->type);
      if (i->second->type == "amber") device.reset(new Amber(i->second));
      else GD::out.printError("Error: Unsupported physical device type: " + i->second->type);
      if (device) {
        if (_physicalInterfaces.find(i->second->id) != _physicalInterfaces.end()) GD::out.printError("Error: id used for two devices: " + i->second->id);
        _physicalInterfaces[i->second->id] = device;
        if (i->second->isDefault || !_defaultPhysicalInterface) _defaultPhysicalInterface = device;
      }
    }
    if (!_defaultPhysicalInterface) _defaultPhysicalInterface = std::make_shared<IMbusInterface>(std::make_shared<BaseLib::Systems::PhysicalInterfaceSettings>());
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Interfaces::startListening() {
  try {
    _stopped = false;

    if (GD::bl->hgdc) {
      _hgdcModuleUpdateEventHandlerId = GD::bl->hgdc->registerModuleUpdateEventHandler(std::function<void(const BaseLib::PVariable &)>(std::bind(&Interfaces::hgdcModuleUpdate, this, std::placeholders::_1)));
      _hgdcReconnectedEventHandlerId = GD::bl->hgdc->registerReconnectedEventHandler(std::function<void()>(std::bind(&Interfaces::hgdcReconnected, this)));

      createHgdcInterfaces(false);
    }

    PhysicalInterfaces::startListening();
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Interfaces::stopListening() {
  try {
    _stopped = true;

    if (GD::bl->hgdc) {
      GD::bl->hgdc->unregisterModuleUpdateEventHandler(_hgdcModuleUpdateEventHandlerId);
      GD::bl->hgdc->unregisterModuleUpdateEventHandler(_hgdcReconnectedEventHandlerId);
    }

    PhysicalInterfaces::stopListening();
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

std::vector<std::shared_ptr<IMbusInterface>> Interfaces::getInterfaces() {
  std::vector<std::shared_ptr<IMbusInterface>> interfaces;
  try {
    std::lock_guard<std::mutex> interfaceGuard(_physicalInterfacesMutex);
    interfaces.reserve(_physicalInterfaces.size());
    for (auto interfaceBase: _physicalInterfaces) {
      std::shared_ptr<IMbusInterface> interface(std::dynamic_pointer_cast<IMbusInterface>(interfaceBase.second));
      if (!interface) continue;
      if (interface->isOpen()) interfaces.push_back(interface);
    }
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return interfaces;
}

std::shared_ptr<IMbusInterface> Interfaces::getDefaultInterface() {
  std::lock_guard<std::mutex> interfaceGuard(_physicalInterfacesMutex);
  return _defaultPhysicalInterface;
}

bool Interfaces::hasInterface(const std::string &name) {
  std::lock_guard<std::mutex> interfaceGuard(_physicalInterfacesMutex);
  auto interfaceBase = _physicalInterfaces.find(name);
  return interfaceBase != _physicalInterfaces.end();
}

std::shared_ptr<IMbusInterface> Interfaces::getInterface(const std::string &name) {
  std::lock_guard<std::mutex> interfaceGuard(_physicalInterfacesMutex);
  auto interfaceBase = _physicalInterfaces.find(name);
  if (interfaceBase == _physicalInterfaces.end()) return _defaultPhysicalInterface;
  std::shared_ptr<IMbusInterface> interface(std::dynamic_pointer_cast<IMbusInterface>(interfaceBase->second));
  return interface;
}

void Interfaces::hgdcReconnected() {
  try {
    int32_t cycles = BaseLib::HelperFunctions::getRandomNumber(40, 100);
    for (int32_t i = 0; i < cycles; i++) {
      if (_stopped) return;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    _hgdcReconnected = true;
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Interfaces::createHgdcInterfaces(bool reconnected) {
  try {
    if (GD::bl->hgdc) {
      std::lock_guard<std::mutex> interfacesGuard(_physicalInterfacesMutex);
      auto modules = GD::bl->hgdc->getModules(MY_FAMILY_ID);
      if (modules->errorStruct) {
        GD::out.printError("Error getting HGDC modules: " + modules->structValue->at("faultString")->stringValue);
      }
      for (auto &module: *modules->arrayValue) {
        auto deviceId = module->structValue->at("serialNumber")->stringValue;
        auto deviceType = module->structValue->at("deviceType")->integerValue;

        if (_physicalInterfaces.find(deviceId) == _physicalInterfaces.end()) {
          std::shared_ptr<IMbusInterface> device;
          GD::out.printDebug("Debug: Creating HGDC device.");
          auto settings = std::make_shared<Systems::PhysicalInterfaceSettings>();
          settings->type = "hgdc" + BaseLib::HelperFunctions::getHexString(deviceType);
          settings->id = deviceId;
          settings->serialNumber = settings->id;
          device = std::make_shared<Hgdc>(settings);
          _physicalInterfaces[settings->id] = device;
          if (settings->isDefault || !_defaultPhysicalInterface || _defaultPhysicalInterface->getID().empty()) _defaultPhysicalInterface = device;

          if (_central) {
            if (_physicalInterfaceEventhandlers.find(settings->id) != _physicalInterfaceEventhandlers.end()) continue;
            _physicalInterfaceEventhandlers[settings->id] = device->addEventHandler(_central);
          }

          if (reconnected) device->startListening();
        } else if (reconnected) {
          std::shared_ptr<Hgdc> interface(std::dynamic_pointer_cast<Hgdc>(_physicalInterfaces.at(deviceId)));
          if (interface) interface->init();
        }
      }
    }
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Interfaces::hgdcModuleUpdate(const BaseLib::PVariable &modules) {
  try {
    std::lock_guard<std::mutex> interfaceGuard(_physicalInterfacesMutex);
    _updatedHgdcModules = modules;
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Interfaces::hgdcReconnectedThread() {
  try {
    if (!_hgdcReconnected) return;
    _hgdcReconnected = false;
    createHgdcInterfaces(true);
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Interfaces::hgdcModuleUpdateThread() {
  try {
    BaseLib::PVariable modules;

    {
      std::lock_guard<std::mutex> interfaceGuard(_physicalInterfacesMutex);
      modules = _updatedHgdcModules;
    }

    if (!modules) return;

    auto addedModules = std::make_shared<std::list<std::shared_ptr<BaseLib::Systems::IPhysicalInterface>>>();

    for (auto &module: *modules->structValue) {
      auto familyIdIterator = module.second->structValue->find("familyId");
      if (familyIdIterator == module.second->structValue->end() || familyIdIterator->second->integerValue64 != MY_FAMILY_ID) continue;

      auto removedIterator = module.second->structValue->find("removed");
      if (removedIterator != module.second->structValue->end()) {
        std::unique_lock<std::mutex> interfaceGuard(_physicalInterfacesMutex);
        auto interfaceIterator = _physicalInterfaces.find(module.first);
        if (interfaceIterator != _physicalInterfaces.end()) {
          auto interface = interfaceIterator->second;
          interfaceGuard.unlock();
          interface->stopListening();
          continue;
        }
      }

      auto restartedIterator = module.second->structValue->find("restarted");
      if (restartedIterator != module.second->structValue->end()) {
        std::unique_lock<std::mutex> interfaceGuard(_physicalInterfacesMutex);
        auto interfaceIterator = _physicalInterfaces.find(module.first);
        if (interfaceIterator != _physicalInterfaces.end()) {
          std::shared_ptr<Hgdc> interface(std::dynamic_pointer_cast<Hgdc>(interfaceIterator->second));
          interfaceGuard.unlock();
          if (!interface) continue;
          interface->init();
          continue;
        }
      }

      auto addedIterator = module.second->structValue->find("added");
      if (addedIterator != module.second->structValue->end()) {
        std::unique_lock<std::mutex> interfaceGuard(_physicalInterfacesMutex);
        auto interfaceIterator = _physicalInterfaces.find(module.first);
        if (interfaceIterator == _physicalInterfaces.end()) {
          interfaceGuard.unlock();
          std::shared_ptr<IMbusInterface> device;
          GD::out.printDebug("Debug: Creating HGDC device.");
          auto settings = std::make_shared<Systems::PhysicalInterfaceSettings>();
          settings->type = "hgdc";
          settings->id = module.first;
          settings->serialNumber = settings->id;
          device = std::make_shared<Hgdc>(settings);

          if (_physicalInterfaces.find(settings->id) != _physicalInterfaces.end()) GD::out.printError("Error: id used for two devices: " + settings->id);
          _physicalInterfaces[settings->id] = device;
          if (settings->isDefault || !_defaultPhysicalInterface || _defaultPhysicalInterface->getID().empty()) _defaultPhysicalInterface = device;

          addedModules->push_back(device);
        } else {
          auto interface = interfaceIterator->second;
          interfaceGuard.unlock();
          if (interface->getType() == "hgdc" && !interface->isOpen()) {
            interface->startListening();
          }
        }
      }
    }

    for (auto &module: *addedModules) {
      if (_central) {
        if (_physicalInterfaceEventhandlers.find(module->getID()) != _physicalInterfaceEventhandlers.end()) continue;
        _physicalInterfaceEventhandlers[module->getID()] = module->addEventHandler(_central);
      }

      module->startListening();
    }
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }

  try {
    std::lock_guard<std::mutex> interfaceGuard(_physicalInterfacesMutex);
    _updatedHgdcModules.reset();
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Interfaces::worker() {
  try {
    hgdcModuleUpdateThread();
    hgdcReconnectedThread();
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

BaseLib::PVariable Interfaces::listInterfaces() {
  try {
    auto array = Systems::PhysicalInterfaces::listInterfaces();

    BaseLib::PVariable interfaceStruct(new BaseLib::Variable(BaseLib::VariableType::tStruct));

    interfaceStruct->structValue->insert(BaseLib::StructElement("FAMILYID", std::make_shared<BaseLib::Variable>(MY_FAMILY_ID)));
    interfaceStruct->structValue->insert(BaseLib::StructElement("VIRTUAL", std::make_shared<BaseLib::Variable>(true)));
    interfaceStruct->structValue->insert(BaseLib::StructElement("ID", std::make_shared<BaseLib::Variable>(std::to_string(MY_FAMILY_ID) + ".virtual")));
    interfaceStruct->structValue->insert(BaseLib::StructElement("CONNECTED", std::make_shared<BaseLib::Variable>(true)));

    array->arrayValue->emplace_back(interfaceStruct);

    return array;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return BaseLib::Variable::createError(-32500, "Unknown application error.");
}

}
