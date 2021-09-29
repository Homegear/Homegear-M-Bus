/* Copyright 2013-2019 Homegear GmbH */

#ifndef INTERFACES_H_
#define INTERFACES_H_

#include "PhysicalInterfaces/IMbusInterface.h"

#include <homegear-base/BaseLib.h>

namespace Mbus {

using namespace BaseLib;

class Interfaces : public BaseLib::Systems::PhysicalInterfaces {
 public:
  Interfaces(BaseLib::SharedObjects *bl, std::map<std::string, Systems::PPhysicalInterfaceSettings> physicalInterfaceSettings);
  virtual ~Interfaces();

  void addEventHandlers(BaseLib::Systems::IPhysicalInterface::IPhysicalInterfaceEventSink *central);
  void removeEventHandlers();
  void startListening() override;
  void stopListening() override;
  std::shared_ptr<IMbusInterface> getDefaultInterface();
  bool hasInterface(const std::string &name);
  std::shared_ptr<IMbusInterface> getInterface(const std::string &name);
  std::vector<std::shared_ptr<IMbusInterface>> getInterfaces();
  void worker();
 protected:
  BaseLib::PVariable _updatedHgdcModules;

  std::atomic_bool _stopped{true};
  std::atomic_bool _hgdcReconnected{false};
  int32_t _hgdcModuleUpdateEventHandlerId = -1;
  int32_t _hgdcReconnectedEventHandlerId = -1;
  BaseLib::Systems::IPhysicalInterface::IPhysicalInterfaceEventSink *_central = nullptr;
  std::shared_ptr<IMbusInterface> _defaultPhysicalInterface;
  std::map<std::string, PEventHandler> _physicalInterfaceEventhandlers;

  void create() override;
  void hgdcReconnected();
  void createHgdcInterfaces(bool reconnected);
  void hgdcModuleUpdate(const BaseLib::PVariable &modules);
  void hgdcReconnectedThread();
  void hgdcModuleUpdateThread();
};

}

#endif
