/* Copyright 2013-2019 Homegear GmbH */

#ifndef HOMEGEAR_MBUS_DESCRIPTIONCREATOR_H
#define HOMEGEAR_MBUS_DESCRIPTIONCREATOR_H

#include <homegear-base/BaseLib.h>
#include "MbusPacket.h"

#include <sys/stat.h>

namespace Mbus {

class DescriptionCreator {
 public:
  struct PeerInfo {
    std::string serialNumber;
    int32_t secondary_address = -1;
    int32_t type = -1;
  };

  DescriptionCreator();
  virtual ~DescriptionCreator() = default;

  DescriptionCreator::PeerInfo CreateDescription(PMbusPacket packet);
  DescriptionCreator::PeerInfo CreateEmptyDescription(int32_t secondary_address);
 private:
  std::map<uint8_t, std::string> _vifVariableNameMap;
  std::map<uint8_t, std::string> _vifUnit;
  std::map<uint8_t, std::string> _vifFbVariableNameMap;
  std::map<uint8_t, std::string> _vifFbUnit;
  std::map<uint8_t, std::string> _vifFdVariableNameMap;
  std::map<uint8_t, std::string> _vifFdUnit;
  std::string _xmlPath;

  void createDirectories();
  static void createXmlMaintenanceChannel(PHomegearDevice &device);
  std::string getFreeParameterId(std::string baseId, PFunction &function);
  void parseDataRecord(const std::string &manufacturer, MbusPacket::DataRecord &dataRecord, PParameter &parameter, PFunction &function, PPacket &packet);
};

}

#endif
