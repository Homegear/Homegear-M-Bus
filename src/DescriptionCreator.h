/* Copyright 2013-2019 Homegear GmbH */

#ifndef HOMEGEAR_MBUS_DESCRIPTIONCREATOR_H
#define HOMEGEAR_MBUS_DESCRIPTIONCREATOR_H

#include <homegear-base/BaseLib.h>
#include "MbusPacket.h"

#include <sys/stat.h>

#include <utility>

namespace Mbus {

class DescriptionCreator {
 public:
  struct PeerInfo {
    std::string serialNumber;
    uint32_t secondary_address = -1;
    uint64_t type = 0;
  };

  DescriptionCreator();
  virtual ~DescriptionCreator() = default;

  DescriptionCreator::PeerInfo CreateDescription(const PMbusPacket &packet);
  DescriptionCreator::PeerInfo CreateEmptyDescription(int32_t secondary_address);
 private:
  enum class VifScaleOperation {
    kMultiplication, //Multiply received value with factor
    kDivision //Divide received value by factor
  };

  struct VifInfo {
    VifInfo() = default;
    VifInfo(std::string name, std::string unit, BaseLib::DeviceDescription::UnitCode unit_code, int32_t unit_scale_factor = 1, VifScaleOperation unit_scale_operation = VifScaleOperation::kMultiplication)
        : name(std::move(name)), unit(std::move(unit)), unit_code(unit_code), unit_scale_factor(unit_scale_factor), unit_scale_operation(unit_scale_operation) {}

    std::string name;
    std::string unit;
    BaseLib::DeviceDescription::UnitCode unit_code = BaseLib::DeviceDescription::UnitCode::kUndefined;
    int32_t unit_scale_factor = 1;
    VifScaleOperation unit_scale_operation = VifScaleOperation::kMultiplication;
    std::unordered_map<uint16_t, int64_t> medium_role_map;
  };

  std::map<uint8_t, VifInfo> vif_info_;
  std::map<uint8_t, VifInfo> vif_fb_info_;
  std::map<uint8_t, VifInfo> vif_fd_info_;
  std::map<std::string, std::map<uint8_t, VifInfo>> vif_ff_info_;
  std::string _xmlPath;

  void createDirectories();
  static void createXmlMaintenanceChannel(PHomegearDevice &device);
  std::string getFreeParameterId(std::string baseId, PFunction &function);
  void parseDataRecord(const std::string &manufacturer, uint8_t medium, MbusPacket::DataRecord &dataRecord, PParameter &parameter, PFunction &function, PPacket &packet, std::unordered_set<uint64_t> &used_roles);
  static void setVifInfo(PParameter &parameter, const VifInfo &vif_info, const MbusPacket::DataRecord &dataRecord, uint8_t medium, std::unordered_set<uint64_t> &used_roles);
};

}

#endif
