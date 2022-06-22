/* Copyright 2013-2019 Homegear GmbH */

#include "DescriptionCreator.h"

#include <memory>
#include "Gd.h"

namespace Mbus {

DescriptionCreator::DescriptionCreator() {
  //mWh
  vif_info_[0] = VifInfo("ENERGY", "Wh", BaseLib::DeviceDescription::UnitCode::kWattHours, 1000, VifScaleOperation::kDivision);
  //10^-2 Wh
  vif_info_[1] = VifInfo("ENERGY", "Wh", BaseLib::DeviceDescription::UnitCode::kWattHours, 100, VifScaleOperation::kDivision);
  //10^-1 Wh
  vif_info_[2] = VifInfo("ENERGY", "Wh", BaseLib::DeviceDescription::UnitCode::kWattHours, 10, VifScaleOperation::kDivision);
  //Wh
  vif_info_[3] = VifInfo("ENERGY", "Wh", BaseLib::DeviceDescription::UnitCode::kWattHours);
  //10^-2 kWh
  vif_info_[4] = VifInfo("ENERGY", "kWh", BaseLib::DeviceDescription::UnitCode::kKilowattHours, 100, VifScaleOperation::kDivision);
  //10^-1 kWh
  vif_info_[5] = VifInfo("ENERGY", "kWh", BaseLib::DeviceDescription::UnitCode::kKilowattHours, 10, VifScaleOperation::kDivision);
  //kWh
  vif_info_[6] = VifInfo("ENERGY", "kWh", BaseLib::DeviceDescription::UnitCode::kKilowattHours);
  //10^1 kWh
  vif_info_[7] = VifInfo("ENERGY", "kWh", BaseLib::DeviceDescription::UnitCode::kKilowattHours, 10, VifScaleOperation::kMultiplication);
  for(uint32_t i = 0; i <= 7; i++) {
    vif_info_.at(i).medium_role_map.emplace(0x02, 900201);
    vif_info_.at(i).medium_role_map.emplace(0x04, 900401);
    vif_info_.at(i).medium_role_map.emplace(0x0C, 900401);
  }

  //J
  vif_info_[8] = VifInfo("ENERGY", "J", BaseLib::DeviceDescription::UnitCode::kJoules);
  //10^-2 kJ
  vif_info_[9] = VifInfo("ENERGY", "kJ", BaseLib::DeviceDescription::UnitCode::kKilojoules, 100, VifScaleOperation::kDivision);
  //10^-1 kJ
  vif_info_[10] = VifInfo("ENERGY", "kJ", BaseLib::DeviceDescription::UnitCode::kKilojoules, 10, VifScaleOperation::kDivision);
  //kJ
  vif_info_[11] = VifInfo("ENERGY", "kJ", BaseLib::DeviceDescription::UnitCode::kKilojoules);
  //10^-2 MJ
  vif_info_[12] = VifInfo("ENERGY", "MJ", BaseLib::DeviceDescription::UnitCode::kMegajoules, 100, VifScaleOperation::kDivision);
  //10^-1 MJ
  vif_info_[13] = VifInfo("ENERGY", "MJ", BaseLib::DeviceDescription::UnitCode::kMegajoules, 10, VifScaleOperation::kDivision);
  //MJ
  vif_info_[14] = VifInfo("ENERGY", "MJ", BaseLib::DeviceDescription::UnitCode::kMegajoules);
  //10^1 MJ
  vif_info_[15] = VifInfo("ENERGY", "MJ", BaseLib::DeviceDescription::UnitCode::kMegajoules, 10, VifScaleOperation::kMultiplication);

  //cm^3
  vif_info_[16] = VifInfo("VOLUME", "l", BaseLib::DeviceDescription::UnitCode::kLiters, 1000, VifScaleOperation::kDivision);
  //10^1 cm^3
  vif_info_[17] = VifInfo("VOLUME", "l", BaseLib::DeviceDescription::UnitCode::kLiters, 100, VifScaleOperation::kDivision);
  //10^2 cm^3
  vif_info_[18] = VifInfo("VOLUME", "l", BaseLib::DeviceDescription::UnitCode::kLiters, 10, VifScaleOperation::kDivision);
  //l
  vif_info_[19] = VifInfo("VOLUME", "l", BaseLib::DeviceDescription::UnitCode::kLiters);
  //10^-2 m^3
  vif_info_[20] = VifInfo("VOLUME", "m³", BaseLib::DeviceDescription::UnitCode::kCubicMeters, 100, VifScaleOperation::kDivision);
  //10^-1 m^3
  vif_info_[21] = VifInfo("VOLUME", "m³", BaseLib::DeviceDescription::UnitCode::kCubicMeters, 10, VifScaleOperation::kDivision);
  //m^3
  vif_info_[22] = VifInfo("VOLUME", "m³", BaseLib::DeviceDescription::UnitCode::kCubicMeters);
  //10^1 m^3
  vif_info_[23] = VifInfo("VOLUME", "m³", BaseLib::DeviceDescription::UnitCode::kCubicMeters, 10, VifScaleOperation::kMultiplication);
  for(uint32_t i = 16; i <= 23; i++) {
    vif_info_.at(i).medium_role_map.emplace(0x03, 900301);
    vif_info_.at(i).medium_role_map.emplace(0x04, 900101);
    vif_info_.at(i).medium_role_map.emplace(0x0C, 900101);
    vif_info_.at(i).medium_role_map.emplace(0x06, 900101);
    vif_info_.at(i).medium_role_map.emplace(0x07, 900101);
    vif_info_.at(i).medium_role_map.emplace(0x15, 900101);
    vif_info_.at(i).medium_role_map.emplace(0x16, 900101);
    vif_info_.at(i).medium_role_map.emplace(0x17, 900101);
    vif_info_.at(i).medium_role_map.emplace(0x28, 900101);
  }

  //g
  vif_info_[24] = VifInfo("MASS", "g", BaseLib::DeviceDescription::UnitCode::kGrams, 1, VifScaleOperation::kMultiplication);
  //10^-2 kg
  vif_info_[25] = VifInfo("MASS", "kg", BaseLib::DeviceDescription::UnitCode::kKilograms, 100, VifScaleOperation::kDivision);
  //10^-1 kg
  vif_info_[26] = VifInfo("MASS", "kg", BaseLib::DeviceDescription::UnitCode::kKilograms, 10, VifScaleOperation::kDivision);
  //kg
  vif_info_[27] = VifInfo("MASS", "kg", BaseLib::DeviceDescription::UnitCode::kKilograms);
  //10^-2 t
  vif_info_[28] = VifInfo("MASS", "t", BaseLib::DeviceDescription::UnitCode::kTons, 100, VifScaleOperation::kDivision);
  //10^-1 t
  vif_info_[29] = VifInfo("MASS", "t", BaseLib::DeviceDescription::UnitCode::kTons, 10, VifScaleOperation::kDivision);
  //t
  vif_info_[30] = VifInfo("MASS", "t", BaseLib::DeviceDescription::UnitCode::kTons);
  //10^1 t
  vif_info_[31] = VifInfo("MASS", "t", BaseLib::DeviceDescription::UnitCode::kTons, 10, VifScaleOperation::kMultiplication);

  vif_info_[32] = VifInfo("ON_TIME", "s", BaseLib::DeviceDescription::UnitCode::kSeconds);
  vif_info_[33] = VifInfo("ON_TIME", "m", BaseLib::DeviceDescription::UnitCode::kMinutes);
  vif_info_[34] = VifInfo("ON_TIME", "h", BaseLib::DeviceDescription::UnitCode::kHours);
  vif_info_[35] = VifInfo("ON_TIME", "d", BaseLib::DeviceDescription::UnitCode::kDays);

  vif_info_[36] = VifInfo("OPERATING_TIME", "s", BaseLib::DeviceDescription::UnitCode::kSeconds);
  vif_info_[37] = VifInfo("OPERATING_TIME", "m", BaseLib::DeviceDescription::UnitCode::kMinutes);
  vif_info_[38] = VifInfo("OPERATING_TIME", "h", BaseLib::DeviceDescription::UnitCode::kHours);
  vif_info_[39] = VifInfo("OPERATING_TIME", "d", BaseLib::DeviceDescription::UnitCode::kDays);

  //mW
  vif_info_[40] = VifInfo("POWER", "mW", BaseLib::DeviceDescription::UnitCode::kMilliwatts);
  //10^-2 W
  vif_info_[41] = VifInfo("POWER", "W", BaseLib::DeviceDescription::UnitCode::kWatts, 100, VifScaleOperation::kDivision);
  //10^-1 W
  vif_info_[42] = VifInfo("POWER", "W", BaseLib::DeviceDescription::UnitCode::kWatts, 10, VifScaleOperation::kDivision);
  //W
  vif_info_[43] = VifInfo("POWER", "W", BaseLib::DeviceDescription::UnitCode::kWatts);
  //10^-2 kW
  vif_info_[44] = VifInfo("POWER", "kW", BaseLib::DeviceDescription::UnitCode::kKilowatts, 100, VifScaleOperation::kDivision);
  //10^-1 kW
  vif_info_[45] = VifInfo("POWER", "kW", BaseLib::DeviceDescription::UnitCode::kKilowatts, 10, VifScaleOperation::kDivision);
  //kW
  vif_info_[46] = VifInfo("POWER", "kW", BaseLib::DeviceDescription::UnitCode::kKilowatts);
  //10^1 kW
  vif_info_[47] = VifInfo("POWER", "kW", BaseLib::DeviceDescription::UnitCode::kKilowatts, 10, VifScaleOperation::kMultiplication);

  //J/h
  vif_info_[48] = VifInfo("POWER", "J/h", BaseLib::DeviceDescription::UnitCode::kJoulePerHours);
  //10^-2 kJ/h
  vif_info_[49] = VifInfo("POWER", "J/h", BaseLib::DeviceDescription::UnitCode::kJoulePerHours, 10, VifScaleOperation::kMultiplication);
  //10^-1 kJ/h
  vif_info_[50] = VifInfo("POWER", "J/h", BaseLib::DeviceDescription::UnitCode::kJoulePerHours, 100, VifScaleOperation::kMultiplication);
  //kJ/j
  vif_info_[51] = VifInfo("POWER", "J/h", BaseLib::DeviceDescription::UnitCode::kJoulePerHours, 1000, VifScaleOperation::kMultiplication);
  //10^-2 MJ/h
  vif_info_[52] = VifInfo("POWER", "J/h", BaseLib::DeviceDescription::UnitCode::kJoulePerHours, 10000, VifScaleOperation::kMultiplication);
  //10^-1 MJ/h
  vif_info_[53] = VifInfo("POWER", "J/h", BaseLib::DeviceDescription::UnitCode::kJoulePerHours, 100000, VifScaleOperation::kMultiplication);
  //MJ/h
  vif_info_[54] = VifInfo("POWER", "J/h", BaseLib::DeviceDescription::UnitCode::kJoulePerHours, 1000000, VifScaleOperation::kMultiplication);
  //10^1 MJ/h
  vif_info_[55] = VifInfo("POWER", "J/h", BaseLib::DeviceDescription::UnitCode::kJoulePerHours, 10000000, VifScaleOperation::kMultiplication);

  //cm^3/h
  vif_info_[56] = VifInfo("VOLUME_FLOW", "l/h", BaseLib::DeviceDescription::UnitCode::kLitersPerHour, 1000, VifScaleOperation::kDivision);
  //10^1 cm^3/h
  vif_info_[57] = VifInfo("VOLUME_FLOW", "l/h", BaseLib::DeviceDescription::UnitCode::kLitersPerHour, 100, VifScaleOperation::kDivision);
  //10^2 cm^3/h
  vif_info_[58] = VifInfo("VOLUME_FLOW", "l/h", BaseLib::DeviceDescription::UnitCode::kLitersPerHour, 10, VifScaleOperation::kDivision);
  //l/h
  vif_info_[59] = VifInfo("VOLUME_FLOW", "l/h", BaseLib::DeviceDescription::UnitCode::kLitersPerHour);
  //10^-2 m^3/h
  vif_info_[60] = VifInfo("VOLUME_FLOW", "m³/h", BaseLib::DeviceDescription::UnitCode::kCubicMetersPerHour, 100, VifScaleOperation::kDivision);
  //10^-1 m^3/h
  vif_info_[61] = VifInfo("VOLUME_FLOW", "m³/h", BaseLib::DeviceDescription::UnitCode::kCubicMetersPerHour, 10, VifScaleOperation::kDivision);
  //m^3/h
  vif_info_[62] = VifInfo("VOLUME_FLOW", "m³/h", BaseLib::DeviceDescription::UnitCode::kCubicMetersPerHour);
  //10^1 m^3/h
  vif_info_[63] = VifInfo("VOLUME_FLOW", "m³/h", BaseLib::DeviceDescription::UnitCode::kCubicMetersPerHour, 10, VifScaleOperation::kMultiplication);
  for(uint32_t i = 56; i <= 63; i++) {
    vif_info_.at(i).medium_role_map.emplace(0x04, 900403);
    vif_info_.at(i).medium_role_map.emplace(0x0C, 900403);
  }

  //10⁻¹ cm³/min
  vif_info_[64] = VifInfo("VOLUME_FLOW", "l/min", BaseLib::DeviceDescription::UnitCode::kLitersPerMinute, 10000, VifScaleOperation::kDivision);
  //cm³/min
  vif_info_[65] = VifInfo("VOLUME_FLOW", "l/min", BaseLib::DeviceDescription::UnitCode::kLitersPerMinute, 1000, VifScaleOperation::kDivision);
  //10¹ cm³/min
  vif_info_[66] = VifInfo("VOLUME_FLOW", "l/min", BaseLib::DeviceDescription::UnitCode::kLitersPerMinute, 100, VifScaleOperation::kDivision);
  //10² cm³/min
  vif_info_[67] = VifInfo("VOLUME_FLOW", "l/min", BaseLib::DeviceDescription::UnitCode::kLitersPerMinute, 10, VifScaleOperation::kDivision);
  //l/min
  vif_info_[68] = VifInfo("VOLUME_FLOW", "l/min", BaseLib::DeviceDescription::UnitCode::kLitersPerMinute);
  //10⁻² m³/min
  vif_info_[69] = VifInfo("VOLUME_FLOW", "m³/min", BaseLib::DeviceDescription::UnitCode::kCubicMetersPerMinute, 100, VifScaleOperation::kDivision);
  //10⁻¹ m³/min
  vif_info_[70] = VifInfo("VOLUME_FLOW", "m³/min", BaseLib::DeviceDescription::UnitCode::kCubicMetersPerMinute, 10, VifScaleOperation::kDivision);
  //m³/min
  vif_info_[71] = VifInfo("VOLUME_FLOW", "m³/min", BaseLib::DeviceDescription::UnitCode::kCubicMetersPerMinute);

  //mm³/s
  vif_info_[72] = VifInfo("VOLUME_FLOW", "l/s", BaseLib::DeviceDescription::UnitCode::kLitersPerSecond, 1000000, VifScaleOperation::kDivision);
  //10⁻² cm³/s
  vif_info_[73] = VifInfo("VOLUME_FLOW", "l/s", BaseLib::DeviceDescription::UnitCode::kLitersPerSecond, 100000, VifScaleOperation::kDivision);
  //10⁻¹ cm³/s
  vif_info_[74] = VifInfo("VOLUME_FLOW", "l/s", BaseLib::DeviceDescription::UnitCode::kLitersPerSecond, 10000, VifScaleOperation::kDivision);
  //cm³/s
  vif_info_[75] = VifInfo("VOLUME_FLOW", "l/s", BaseLib::DeviceDescription::UnitCode::kLitersPerSecond, 1000, VifScaleOperation::kDivision);
  //10¹ cm³/s
  vif_info_[76] = VifInfo("VOLUME_FLOW", "l/s", BaseLib::DeviceDescription::UnitCode::kLitersPerSecond, 100, VifScaleOperation::kDivision);
  //10² cm³/s
  vif_info_[77] = VifInfo("VOLUME_FLOW", "l/s", BaseLib::DeviceDescription::UnitCode::kLitersPerSecond, 10, VifScaleOperation::kDivision);
  //l/s
  vif_info_[78] = VifInfo("VOLUME_FLOW", "l/s", BaseLib::DeviceDescription::UnitCode::kLitersPerSecond);
  //10⁻² m³/s
  vif_info_[79] = VifInfo("VOLUME_FLOW", "m³/s", BaseLib::DeviceDescription::UnitCode::kCubicMetersPerSecond, 100, VifScaleOperation::kDivision);

  //g/h
  vif_info_[80] = VifInfo("MASS_FLOW", "kg/h", BaseLib::DeviceDescription::UnitCode::kKilogramsPerHour, 1000, VifScaleOperation::kDivision);
  //10⁻² kg/h
  vif_info_[81] = VifInfo("MASS_FLOW", "kg/h", BaseLib::DeviceDescription::UnitCode::kKilogramsPerHour, 100, VifScaleOperation::kDivision);
  //10⁻¹ kg/h
  vif_info_[82] = VifInfo("MASS_FLOW", "kg/h", BaseLib::DeviceDescription::UnitCode::kKilogramsPerHour, 10, VifScaleOperation::kDivision);
  //kg/h
  vif_info_[83] = VifInfo("MASS_FLOW", "kg/h", BaseLib::DeviceDescription::UnitCode::kKilogramsPerHour);
  //10⁻² t/h
  vif_info_[84] = VifInfo("MASS_FLOW", "t/h", BaseLib::DeviceDescription::UnitCode::kTonsPerHour, 100, VifScaleOperation::kDivision);
  //10⁻¹ t/h
  vif_info_[85] = VifInfo("MASS_FLOW", "t/h", BaseLib::DeviceDescription::UnitCode::kTonsPerHour, 10, VifScaleOperation::kDivision);
  //t/h
  vif_info_[86] = VifInfo("MASS_FLOW", "t/h", BaseLib::DeviceDescription::UnitCode::kTonsPerHour);
  //10¹ t/h
  vif_info_[87] = VifInfo("MASS_FLOW", "t/h", BaseLib::DeviceDescription::UnitCode::kTonsPerHour, 10, VifScaleOperation::kMultiplication);

  //10⁻³ °C
  vif_info_[88] = VifInfo("FLOW_TEMPERATURE", "°C", BaseLib::DeviceDescription::UnitCode::kDegreesCelsius, 1000, VifScaleOperation::kDivision);
  //10⁻² °C
  vif_info_[89] = VifInfo("FLOW_TEMPERATURE", "°C", BaseLib::DeviceDescription::UnitCode::kDegreesCelsius, 100, VifScaleOperation::kDivision);
  //10⁻¹ °C
  vif_info_[90] = VifInfo("FLOW_TEMPERATURE", "°C", BaseLib::DeviceDescription::UnitCode::kDegreesCelsius, 10, VifScaleOperation::kDivision);
  //°C
  vif_info_[91] = VifInfo("FLOW_TEMPERATURE", "°C", BaseLib::DeviceDescription::UnitCode::kDegreesCelsius);
  for(uint32_t i = 88; i <= 91; i++) {
    vif_info_.at(i).medium_role_map.emplace(0x04, 900404);
    vif_info_.at(i).medium_role_map.emplace(0x0C, 900404);
  }

  //10⁻³ °C
  vif_info_[92] = VifInfo("RETURN_TEMPERATURE", "°C", BaseLib::DeviceDescription::UnitCode::kDegreesCelsius, 1000, VifScaleOperation::kDivision);
  //10⁻² °C
  vif_info_[93] = VifInfo("RETURN_TEMPERATURE", "°C", BaseLib::DeviceDescription::UnitCode::kDegreesCelsius, 100, VifScaleOperation::kDivision);
  //10⁻¹ °C
  vif_info_[94] = VifInfo("RETURN_TEMPERATURE", "°C", BaseLib::DeviceDescription::UnitCode::kDegreesCelsius, 10, VifScaleOperation::kDivision);
  //°C
  vif_info_[95] = VifInfo("RETURN_TEMPERATURE", "°C", BaseLib::DeviceDescription::UnitCode::kDegreesCelsius);
  for(uint32_t i = 92; i <= 95; i++) {
    vif_info_.at(i).medium_role_map.emplace(0x04, 900405);
    vif_info_.at(i).medium_role_map.emplace(0x0C, 900405);
  }

  //mK
  vif_info_[96] = VifInfo("TEMPERATURE_DIFFERENCE", "K", BaseLib::DeviceDescription::UnitCode::kKelvins, 1000, VifScaleOperation::kDivision);
  //10⁻² K
  vif_info_[97] = VifInfo("TEMPERATURE_DIFFERENCE", "K", BaseLib::DeviceDescription::UnitCode::kKelvins, 100, VifScaleOperation::kDivision);
  //10⁻¹ K
  vif_info_[98] = VifInfo("TEMPERATURE_DIFFERENCE", "K", BaseLib::DeviceDescription::UnitCode::kKelvins, 10, VifScaleOperation::kDivision);
  //K
  vif_info_[99] = VifInfo("TEMPERATURE_DIFFERENCE", "K", BaseLib::DeviceDescription::UnitCode::kKelvins);

  //10⁻³ °C
  vif_info_[100] = VifInfo("EXTERNAL_TEMPERATURE", "°C", BaseLib::DeviceDescription::UnitCode::kDegreesCelsius, 1000, VifScaleOperation::kDivision);
  //10⁻² °C
  vif_info_[101] = VifInfo("EXTERNAL_TEMPERATURE", "°C", BaseLib::DeviceDescription::UnitCode::kDegreesCelsius, 100, VifScaleOperation::kDivision);
  //10⁻¹ °C
  vif_info_[102] = VifInfo("EXTERNAL_TEMPERATURE", "°C", BaseLib::DeviceDescription::UnitCode::kDegreesCelsius, 10, VifScaleOperation::kDivision);
  //°C
  vif_info_[103] = VifInfo("EXTERNAL_TEMPERATURE", "°C", BaseLib::DeviceDescription::UnitCode::kDegreesCelsius);

  //mbar
  vif_info_[104] = VifInfo("PRESSURE", "mbar", BaseLib::DeviceDescription::UnitCode::kMillibars);
  //10⁻² bar
  vif_info_[105] = VifInfo("PRESSURE", "bar", BaseLib::DeviceDescription::UnitCode::kBars, 100, VifScaleOperation::kDivision);
  //10⁻¹ bar
  vif_info_[106] = VifInfo("PRESSURE", "bar", BaseLib::DeviceDescription::UnitCode::kBars, 10, VifScaleOperation::kDivision);
  //bar
  vif_info_[107] = VifInfo("PRESSURE", "bar", BaseLib::DeviceDescription::UnitCode::kBars);

  vif_info_[108] = VifInfo("DATE", "s", BaseLib::DeviceDescription::UnitCode::kTimestampSeconds);
  vif_info_[109] = VifInfo("DATETIME", "s", BaseLib::DeviceDescription::UnitCode::kTimestampSeconds);

  vif_info_[110] = VifInfo("HCA_UNITS", "", BaseLib::DeviceDescription::UnitCode::kHeatingCostAllocatorUnits);

  //111 reserved

  vif_info_[112] = VifInfo("AVERAGING_DURATION", "s", BaseLib::DeviceDescription::UnitCode::kSeconds);
  vif_info_[113] = VifInfo("AVERAGING_DURATION", "m", BaseLib::DeviceDescription::UnitCode::kMinutes);
  vif_info_[114] = VifInfo("AVERAGING_DURATION", "h", BaseLib::DeviceDescription::UnitCode::kHours);
  vif_info_[115] = VifInfo("AVERAGING_DURATION", "d", BaseLib::DeviceDescription::UnitCode::kDays);

  vif_info_[116] = VifInfo("ACTUALITY_DURATION", "s", BaseLib::DeviceDescription::UnitCode::kSeconds);
  vif_info_[117] = VifInfo("ACTUALITY_DURATION", "m", BaseLib::DeviceDescription::UnitCode::kMinutes);
  vif_info_[118] = VifInfo("ACTUALITY_DURATION", "h", BaseLib::DeviceDescription::UnitCode::kHours);
  vif_info_[119] = VifInfo("ACTUALITY_DURATION", "d", BaseLib::DeviceDescription::UnitCode::kDays);

  vif_info_[120] = VifInfo("FABRICATION_NO", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_info_[121] = VifInfo("ENHANCED_IDENTIFICATION", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_info_[122] = VifInfo("BUS_ADDRESS", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);

  vif_info_[0x7C] = VifInfo("CUSTOM_STRING", "", BaseLib::DeviceDescription::UnitCode::kUndefined);
  vif_info_[0xFC] = VifInfo("CUSTOM_STRING", "", BaseLib::DeviceDescription::UnitCode::kUndefined);
  vif_info_[0x7E] = VifInfo("ANY_VIF", "", BaseLib::DeviceDescription::UnitCode::kUndefined);
  vif_info_[0xFE] = VifInfo("ANY_VIF", "", BaseLib::DeviceDescription::UnitCode::kUndefined);
  vif_info_[0x7F] = VifInfo("MANUFACTURER_SPECIFIC", "", BaseLib::DeviceDescription::UnitCode::kUndefined);
  vif_info_[0xFF] = VifInfo("MANUFACTURER_SPECIFIC", "", BaseLib::DeviceDescription::UnitCode::kUndefined);

  //10^-1 MWh
  vif_fb_info_[0] = VifInfo("ENERGY", "MWh", BaseLib::DeviceDescription::UnitCode::kMegawattHours, 10, VifScaleOperation::kDivision);
  //MWh
  vif_fb_info_[1] = VifInfo("ENERGY", "MWh", BaseLib::DeviceDescription::UnitCode::kMegawattHours);

  //2 to 7 reserved

  //10^-1 GJ
  vif_fb_info_[8] = VifInfo("ENERGY", "MJ", BaseLib::DeviceDescription::UnitCode::kMegajoules, 100, VifScaleOperation::kMultiplication);
  //GJ
  vif_fb_info_[9] = VifInfo("ENERGY", "MJ", BaseLib::DeviceDescription::UnitCode::kMegajoules, 1000, VifScaleOperation::kMultiplication);

  //10 to 15 reserved

  //10^2 m^2
  vif_fb_info_[16] = VifInfo("VOLUME", "m³", BaseLib::DeviceDescription::UnitCode::kCubicMeters, 100, VifScaleOperation::kMultiplication);
  //10^3 m^3
  vif_fb_info_[17] = VifInfo("VOLUME", "m³", BaseLib::DeviceDescription::UnitCode::kCubicMeters, 1000, VifScaleOperation::kMultiplication);

  //18 to 23 reserved

  //10^2 t
  vif_fb_info_[24] = VifInfo("MASS", "t", BaseLib::DeviceDescription::UnitCode::kTons, 100, VifScaleOperation::kMultiplication);
  //10^3 t
  vif_fb_info_[25] = VifInfo("MASS", "t", BaseLib::DeviceDescription::UnitCode::kTons, 1000, VifScaleOperation::kMultiplication);

  //26 to 32 reserved

  //10^-1 feet^3
  vif_fb_info_[33] = VifInfo("VOLUME", "feet³", BaseLib::DeviceDescription::UnitCode::kCubicFeet, 10, VifScaleOperation::kDivision);
  //10^-1 american gallons
  vif_fb_info_[34] = VifInfo("VOLUME", "american gallons", BaseLib::DeviceDescription::UnitCode::kUsGallons, 10, VifScaleOperation::kDivision);
  //american gallons
  vif_fb_info_[35] = VifInfo("VOLUME", "american gallons", BaseLib::DeviceDescription::UnitCode::kUsGallons);

  //10⁻³ american gallons/min
  vif_fb_info_[36] = VifInfo("VOLUME_FLOW", "american gallons/min", BaseLib::DeviceDescription::UnitCode::kUsGallonsPerMinute, 1000, VifScaleOperation::kDivision);
  //american gallons/min
  vif_fb_info_[37] = VifInfo("VOLUME_FLOW", "american gallons/min", BaseLib::DeviceDescription::UnitCode::kUsGallonsPerMinute);
  //american gallons/hour
  vif_fb_info_[38] = VifInfo("VOLUME_FLOW", "american gallons/h", BaseLib::DeviceDescription::UnitCode::kUsGallonsPerHours);

  //39 reserved

  //10^-1 MW
  vif_fb_info_[40] = VifInfo("POWER", "MW", BaseLib::DeviceDescription::UnitCode::kMegawatts, 10, VifScaleOperation::kDivision);
  //MW
  vif_fb_info_[41] = VifInfo("POWER", "MW", BaseLib::DeviceDescription::UnitCode::kMegawatts);

  //42 to 87 reserved

  vif_fb_info_[88] = VifInfo("FLOW_TEMPERATURE", "°F", BaseLib::DeviceDescription::UnitCode::kDegreesFahrenheit, 1000, VifScaleOperation::kDivision);
  vif_fb_info_[89] = VifInfo("FLOW_TEMPERATURE", "°F", BaseLib::DeviceDescription::UnitCode::kDegreesFahrenheit, 100, VifScaleOperation::kDivision);
  vif_fb_info_[90] = VifInfo("FLOW_TEMPERATURE", "°F", BaseLib::DeviceDescription::UnitCode::kDegreesFahrenheit, 10, VifScaleOperation::kDivision);
  vif_fb_info_[91] = VifInfo("FLOW_TEMPERATURE", "°F", BaseLib::DeviceDescription::UnitCode::kDegreesFahrenheit);
  for(uint32_t i = 88; i <= 91; i++) {
    vif_fb_info_.at(i).medium_role_map.emplace(0x04, 900404);
    vif_fb_info_.at(i).medium_role_map.emplace(0x0C, 900404);
  }

  vif_fb_info_[92] = VifInfo("RETURN_TEMPERATURE", "°F", BaseLib::DeviceDescription::UnitCode::kDegreesFahrenheit, 1000, VifScaleOperation::kDivision);
  vif_fb_info_[93] = VifInfo("RETURN_TEMPERATURE", "°F", BaseLib::DeviceDescription::UnitCode::kDegreesFahrenheit, 100, VifScaleOperation::kDivision);
  vif_fb_info_[94] = VifInfo("RETURN_TEMPERATURE", "°F", BaseLib::DeviceDescription::UnitCode::kDegreesFahrenheit, 10, VifScaleOperation::kDivision);
  vif_fb_info_[95] = VifInfo("RETURN_TEMPERATURE", "°F", BaseLib::DeviceDescription::UnitCode::kDegreesFahrenheit);
  for(uint32_t i = 92; i <= 95; i++) {
    vif_fb_info_.at(i).medium_role_map.emplace(0x04, 900405);
    vif_fb_info_.at(i).medium_role_map.emplace(0x0C, 900405);
  }

  vif_fb_info_[96] = VifInfo("TEMPERATURE_DIFFERENCE", "°F", BaseLib::DeviceDescription::UnitCode::kDegreesFahrenheit, 1000, VifScaleOperation::kDivision);
  vif_fb_info_[97] = VifInfo("TEMPERATURE_DIFFERENCE", "°F", BaseLib::DeviceDescription::UnitCode::kDegreesFahrenheit, 100, VifScaleOperation::kDivision);
  vif_fb_info_[98] = VifInfo("TEMPERATURE_DIFFERENCE", "°F", BaseLib::DeviceDescription::UnitCode::kDegreesFahrenheit, 10, VifScaleOperation::kDivision);
  vif_fb_info_[99] = VifInfo("TEMPERATURE_DIFFERENCE", "°F", BaseLib::DeviceDescription::UnitCode::kDegreesFahrenheit);

  vif_fb_info_[100] = VifInfo("EXTERNAL_TEMPERATURE", "°F", BaseLib::DeviceDescription::UnitCode::kDegreesFahrenheit, 1000, VifScaleOperation::kDivision);
  vif_fb_info_[101] = VifInfo("EXTERNAL_TEMPERATURE", "°F", BaseLib::DeviceDescription::UnitCode::kDegreesFahrenheit, 100, VifScaleOperation::kDivision);
  vif_fb_info_[102] = VifInfo("EXTERNAL_TEMPERATURE", "°F", BaseLib::DeviceDescription::UnitCode::kDegreesFahrenheit, 10, VifScaleOperation::kDivision);
  vif_fb_info_[103] = VifInfo("EXTERNAL_TEMPERATURE", "°F", BaseLib::DeviceDescription::UnitCode::kDegreesFahrenheit);

  //104 to 111 reserved

  vif_fb_info_[112] = VifInfo("COLD_WARM_TEMPERATURE_LIMIT", "°F", BaseLib::DeviceDescription::UnitCode::kDegreesFahrenheit, 1000, VifScaleOperation::kDivision);
  vif_fb_info_[113] = VifInfo("COLD_WARM_TEMPERATURE_LIMIT", "°F", BaseLib::DeviceDescription::UnitCode::kDegreesFahrenheit, 100, VifScaleOperation::kDivision);
  vif_fb_info_[114] = VifInfo("COLD_WARM_TEMPERATURE_LIMIT", "°F", BaseLib::DeviceDescription::UnitCode::kDegreesFahrenheit, 10, VifScaleOperation::kDivision);
  vif_fb_info_[115] = VifInfo("COLD_WARM_TEMPERATURE_LIMIT", "°F", BaseLib::DeviceDescription::UnitCode::kDegreesFahrenheit);

  vif_fb_info_[116] = VifInfo("COLD_WARM_TEMPERATURE_LIMIT", "°C", BaseLib::DeviceDescription::UnitCode::kDegreesCelsius, 1000, VifScaleOperation::kDivision);
  vif_fb_info_[117] = VifInfo("COLD_WARM_TEMPERATURE_LIMIT", "°C", BaseLib::DeviceDescription::UnitCode::kDegreesCelsius, 100, VifScaleOperation::kDivision);
  vif_fb_info_[118] = VifInfo("COLD_WARM_TEMPERATURE_LIMIT", "°C", BaseLib::DeviceDescription::UnitCode::kDegreesCelsius, 10, VifScaleOperation::kDivision);
  vif_fb_info_[119] = VifInfo("COLD_WARM_TEMPERATURE_LIMIT", "°C", BaseLib::DeviceDescription::UnitCode::kDegreesCelsius);

  //mW
  vif_fb_info_[120] = VifInfo("MAX_POWER_COUNT", "W", BaseLib::DeviceDescription::UnitCode::kWatts, 1000, VifScaleOperation::kDivision);
  //10^-2 W
  vif_fb_info_[121] = VifInfo("MAX_POWER_COUNT", "W", BaseLib::DeviceDescription::UnitCode::kWatts, 100, VifScaleOperation::kDivision);
  //10^-1 W
  vif_fb_info_[122] = VifInfo("MAX_POWER_COUNT", "W", BaseLib::DeviceDescription::UnitCode::kWatts, 10, VifScaleOperation::kDivision);
  //W
  vif_fb_info_[123] = VifInfo("MAX_POWER_COUNT", "W", BaseLib::DeviceDescription::UnitCode::kWatts);
  //10^-2 kW
  vif_fb_info_[124] = VifInfo("MAX_POWER_COUNT", "kW", BaseLib::DeviceDescription::UnitCode::kKilowatts, 100, VifScaleOperation::kDivision);
  //10^-1 kW
  vif_fb_info_[125] = VifInfo("MAX_POWER_COUNT", "kW", BaseLib::DeviceDescription::UnitCode::kKilowatts, 10, VifScaleOperation::kDivision);
  //kW
  vif_fb_info_[126] = VifInfo("MAX_POWER_COUNT", "kW", BaseLib::DeviceDescription::UnitCode::kKilowatts);
  //10^1 kW
  vif_fb_info_[127] = VifInfo("MAX_POWER_COUNT", "kW", BaseLib::DeviceDescription::UnitCode::kKilowatts, 10, VifScaleOperation::kMultiplication);

  vif_fd_info_[0] = VifInfo("CREDIT", "", BaseLib::DeviceDescription::UnitCode::kCurrency1, 1000, VifScaleOperation::kDivision);
  vif_fd_info_[1] = VifInfo("CREDIT", "", BaseLib::DeviceDescription::UnitCode::kCurrency1, 100, VifScaleOperation::kDivision);
  vif_fd_info_[2] = VifInfo("CREDIT", "", BaseLib::DeviceDescription::UnitCode::kCurrency1, 10, VifScaleOperation::kDivision);
  vif_fd_info_[3] = VifInfo("CREDIT", "", BaseLib::DeviceDescription::UnitCode::kCurrency1);

  vif_fd_info_[4] = VifInfo("DEBIT", "", BaseLib::DeviceDescription::UnitCode::kCurrency1, 1000, VifScaleOperation::kDivision);
  vif_fd_info_[5] = VifInfo("DEBIT", "", BaseLib::DeviceDescription::UnitCode::kCurrency1, 100, VifScaleOperation::kDivision);
  vif_fd_info_[6] = VifInfo("DEBIT", "", BaseLib::DeviceDescription::UnitCode::kCurrency1, 10, VifScaleOperation::kDivision);
  vif_fd_info_[7] = VifInfo("DEBIT", "", BaseLib::DeviceDescription::UnitCode::kCurrency1);

  vif_fd_info_[8] = VifInfo("ACCESS_NUMBER", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[9] = VifInfo("MEDIUM", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[10] = VifInfo("MANUFACTURER", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[11] = VifInfo("PARAMETER_SET_IDENTIFICATION", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[12] = VifInfo("MODEL", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[13] = VifInfo("HARDWARE_VERSION", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[14] = VifInfo("FIRMWARE_VERSION", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[15] = VifInfo("SOFTWARE_VERSION", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[16] = VifInfo("CUSTOMER_LOCATION", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[17] = VifInfo("CUSTOMER", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[18] = VifInfo("ACCESS_CODE_USER", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[19] = VifInfo("ACCESS_CODE_OPERATOR", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[20] = VifInfo("ACCESS_CODE_SYSTEM_OPERATOR", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[21] = VifInfo("ACCESS_CODE_DEVELOPER", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[22] = VifInfo("PASSWORD", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[23] = VifInfo("ERROR_FLAGS_BINARY", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[24] = VifInfo("ERROR_MASK", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[26] = VifInfo("DIGITAL_OUTPUT_BINARY", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[27] = VifInfo("DIGITAL_INPUT_BINARY", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[28] = VifInfo("BAUDRATE", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[29] = VifInfo("RESPONSE_DELAY", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[30] = VifInfo("RETRY", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[32] = VifInfo("FIRST_STORAGE_NUMBER", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[33] = VifInfo("LAST_STORAGE_NUMBER", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[34] = VifInfo("STORAGE_BLOCK_SIZE", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);

  vif_fd_info_[36] = VifInfo("STORAGE_INTERVAL", "s", BaseLib::DeviceDescription::UnitCode::kSeconds);
  vif_fd_info_[37] = VifInfo("STORAGE_INTERVAL", "m", BaseLib::DeviceDescription::UnitCode::kMinutes);
  vif_fd_info_[38] = VifInfo("STORAGE_INTERVAL", "h", BaseLib::DeviceDescription::UnitCode::kHours);
  vif_fd_info_[39] = VifInfo("STORAGE_INTERVAL", "d", BaseLib::DeviceDescription::UnitCode::kDays);
  vif_fd_info_[40] = VifInfo("STORAGE_INTERVAL", "months", BaseLib::DeviceDescription::UnitCode::kMonths);
  vif_fd_info_[41] = VifInfo("STORAGE_INTERVAL", "years", BaseLib::DeviceDescription::UnitCode::kYears);

  vif_fd_info_[44] = VifInfo("DURATION_SINCE_LAST_READOUT", "s", BaseLib::DeviceDescription::UnitCode::kSeconds);
  vif_fd_info_[45] = VifInfo("DURATION_SINCE_LAST_READOUT", "m", BaseLib::DeviceDescription::UnitCode::kMinutes);
  vif_fd_info_[46] = VifInfo("DURATION_SINCE_LAST_READOUT", "h", BaseLib::DeviceDescription::UnitCode::kHours);
  vif_fd_info_[47] = VifInfo("DURATION_SINCE_LAST_READOUT", "d", BaseLib::DeviceDescription::UnitCode::kDays);

  vif_fd_info_[48] = VifInfo("TARIFF_START_DATETIME", "s", BaseLib::DeviceDescription::UnitCode::kTimestampSeconds);

  vif_fd_info_[49] = VifInfo("TARIFF_DURATION", "s", BaseLib::DeviceDescription::UnitCode::kSeconds);
  vif_fd_info_[50] = VifInfo("TARIFF_DURATION", "m", BaseLib::DeviceDescription::UnitCode::kMinutes);
  vif_fd_info_[51] = VifInfo("TARIFF_DURATION", "h", BaseLib::DeviceDescription::UnitCode::kHours);
  vif_fd_info_[52] = VifInfo("TARIFF_DURATION", "d", BaseLib::DeviceDescription::UnitCode::kDays);

  vif_fd_info_[53] = VifInfo("TARIFF_PERIOD", "s", BaseLib::DeviceDescription::UnitCode::kSeconds);
  vif_fd_info_[54] = VifInfo("TARIFF_PERIOD", "m", BaseLib::DeviceDescription::UnitCode::kMinutes);
  vif_fd_info_[55] = VifInfo("TARIFF_PERIOD", "h", BaseLib::DeviceDescription::UnitCode::kHours);
  vif_fd_info_[56] = VifInfo("TARIFF_PERIOD", "d", BaseLib::DeviceDescription::UnitCode::kDays);
  vif_fd_info_[57] = VifInfo("TARIFF_PERIOD", "months", BaseLib::DeviceDescription::UnitCode::kMonths);
  vif_fd_info_[58] = VifInfo("TARIFF_PERIOD", "years", BaseLib::DeviceDescription::UnitCode::kYears);

  vif_fd_info_[59] = VifInfo("DIMENSIONLESS", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);

  vif_fd_info_[64] = VifInfo("VOLTAGE", "mV", BaseLib::DeviceDescription::UnitCode::kMillivolts, 1000000, VifScaleOperation::kDivision);
  vif_fd_info_[65] = VifInfo("VOLTAGE", "mV", BaseLib::DeviceDescription::UnitCode::kMillivolts, 100000, VifScaleOperation::kDivision);
  vif_fd_info_[66] = VifInfo("VOLTAGE", "mV", BaseLib::DeviceDescription::UnitCode::kMillivolts, 10000, VifScaleOperation::kDivision);
  vif_fd_info_[67] = VifInfo("VOLTAGE", "mV", BaseLib::DeviceDescription::UnitCode::kMillivolts, 1000, VifScaleOperation::kDivision);
  vif_fd_info_[68] = VifInfo("VOLTAGE", "mV", BaseLib::DeviceDescription::UnitCode::kMillivolts, 100, VifScaleOperation::kDivision);
  vif_fd_info_[69] = VifInfo("VOLTAGE", "mV", BaseLib::DeviceDescription::UnitCode::kMillivolts, 10, VifScaleOperation::kDivision);
  vif_fd_info_[70] = VifInfo("VOLTAGE", "mV", BaseLib::DeviceDescription::UnitCode::kMillivolts);
  vif_fd_info_[71] = VifInfo("VOLTAGE", "V", BaseLib::DeviceDescription::UnitCode::kVolts, 100, VifScaleOperation::kDivision);
  vif_fd_info_[72] = VifInfo("VOLTAGE", "V", BaseLib::DeviceDescription::UnitCode::kVolts, 10, VifScaleOperation::kDivision);
  vif_fd_info_[73] = VifInfo("VOLTAGE", "V", BaseLib::DeviceDescription::UnitCode::kVolts);
  vif_fd_info_[74] = VifInfo("VOLTAGE", "kV", BaseLib::DeviceDescription::UnitCode::kKilovolts, 100, VifScaleOperation::kDivision);
  vif_fd_info_[75] = VifInfo("VOLTAGE", "kV", BaseLib::DeviceDescription::UnitCode::kKilovolts, 10, VifScaleOperation::kDivision);
  vif_fd_info_[76] = VifInfo("VOLTAGE", "kV", BaseLib::DeviceDescription::UnitCode::kKilovolts);
  vif_fd_info_[77] = VifInfo("VOLTAGE", "MV", BaseLib::DeviceDescription::UnitCode::kMegavolts, 100, VifScaleOperation::kDivision);
  vif_fd_info_[78] = VifInfo("VOLTAGE", "MV", BaseLib::DeviceDescription::UnitCode::kMegavolts, 10, VifScaleOperation::kDivision);
  vif_fd_info_[79] = VifInfo("VOLTAGE", "MV", BaseLib::DeviceDescription::UnitCode::kMegavolts);

  vif_fd_info_[80] = VifInfo("CURRENT", "mA", BaseLib::DeviceDescription::UnitCode::kMilliamperes, 1000000000, VifScaleOperation::kDivision);
  vif_fd_info_[81] = VifInfo("CURRENT", "mA", BaseLib::DeviceDescription::UnitCode::kMilliamperes, 100000000, VifScaleOperation::kDivision);
  vif_fd_info_[82] = VifInfo("CURRENT", "mA", BaseLib::DeviceDescription::UnitCode::kMilliamperes, 10000000, VifScaleOperation::kDivision);
  vif_fd_info_[83] = VifInfo("CURRENT", "mA", BaseLib::DeviceDescription::UnitCode::kMilliamperes, 1000000, VifScaleOperation::kDivision);
  vif_fd_info_[84] = VifInfo("CURRENT", "mA", BaseLib::DeviceDescription::UnitCode::kMilliamperes, 100000, VifScaleOperation::kDivision);
  vif_fd_info_[85] = VifInfo("CURRENT", "mA", BaseLib::DeviceDescription::UnitCode::kMilliamperes, 10000, VifScaleOperation::kDivision);
  vif_fd_info_[86] = VifInfo("CURRENT", "mA", BaseLib::DeviceDescription::UnitCode::kMilliamperes, 1000, VifScaleOperation::kDivision);
  vif_fd_info_[87] = VifInfo("CURRENT", "mA", BaseLib::DeviceDescription::UnitCode::kMilliamperes, 100, VifScaleOperation::kDivision);
  vif_fd_info_[88] = VifInfo("CURRENT", "mA", BaseLib::DeviceDescription::UnitCode::kMilliamperes, 10, VifScaleOperation::kDivision);
  vif_fd_info_[89] = VifInfo("CURRENT", "mA", BaseLib::DeviceDescription::UnitCode::kMilliamperes);
  vif_fd_info_[90] = VifInfo("CURRENT", "A", BaseLib::DeviceDescription::UnitCode::kAmperes, 100, VifScaleOperation::kDivision);
  vif_fd_info_[91] = VifInfo("CURRENT", "A", BaseLib::DeviceDescription::UnitCode::kAmperes, 10, VifScaleOperation::kDivision);
  vif_fd_info_[92] = VifInfo("CURRENT", "A", BaseLib::DeviceDescription::UnitCode::kAmperes);
  vif_fd_info_[93] = VifInfo("CURRENT", "A", BaseLib::DeviceDescription::UnitCode::kAmperes, 10, VifScaleOperation::kMultiplication);
  vif_fd_info_[94] = VifInfo("CURRENT", "A", BaseLib::DeviceDescription::UnitCode::kAmperes, 100, VifScaleOperation::kMultiplication);
  vif_fd_info_[95] = VifInfo("CURRENT", "A", BaseLib::DeviceDescription::UnitCode::kAmperes, 1000, VifScaleOperation::kMultiplication);

  vif_fd_info_[96] = VifInfo("RESET_COUNTER", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[97] = VifInfo("CUMULATION_COUNTER", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[98] = VifInfo("CONTROL_SIGNAL", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[99] = VifInfo("DAY_OF_WEEK", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[100] = VifInfo("WEEK_NUMBER", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[101] = VifInfo("DAY_CHANGE_TIMEPOINT", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[102] = VifInfo("PARAMETER_ACTIVATION_STATE", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);
  vif_fd_info_[103] = VifInfo("SPECIAL_SUPPLIER_INFORMATION", "", BaseLib::DeviceDescription::UnitCode::kNoUnits);

  vif_fd_info_[104] = VifInfo("DURATION_SINCE_LAST_CUMULATION", "h", BaseLib::DeviceDescription::UnitCode::kHours);
  vif_fd_info_[105] = VifInfo("DURATION_SINCE_LAST_CUMULATION", "d", BaseLib::DeviceDescription::UnitCode::kDays);
  vif_fd_info_[106] = VifInfo("DURATION_SINCE_LAST_CUMULATION", "m", BaseLib::DeviceDescription::UnitCode::kMonths);
  vif_fd_info_[107] = VifInfo("DURATION_SINCE_LAST_CUMULATION", "y", BaseLib::DeviceDescription::UnitCode::kYears);

  vif_fd_info_[108] = VifInfo("BATTERY_OPERATING_TIME", "h", BaseLib::DeviceDescription::UnitCode::kHours);
  vif_fd_info_[109] = VifInfo("BATTERY_OPERATING_TIME", "d", BaseLib::DeviceDescription::UnitCode::kDays);
  vif_fd_info_[110] = VifInfo("BATTERY_OPERATING_TIME", "m", BaseLib::DeviceDescription::UnitCode::kMonths);
  vif_fd_info_[111] = VifInfo("BATTERY_OPERATING_TIME", "y", BaseLib::DeviceDescription::UnitCode::kYears);

  vif_fd_info_[112] = VifInfo("BATTERY_CHANGE_DATETIME", "s", BaseLib::DeviceDescription::UnitCode::kTimestampSeconds);
}

DescriptionCreator::PeerInfo DescriptionCreator::CreateDescription(const PMbusPacket &packet) {
  try {
    createDirectories();

    std::string id = BaseLib::HelperFunctions::getHexString(packet->secondaryAddress(), 8);

    std::shared_ptr<HomegearDevice> device = std::make_shared<HomegearDevice>(Gd::bl);
    device->version = 1;
    device->timeout = 86400;
    PSupportedDevice supportedDevice = std::make_shared<SupportedDevice>(Gd::bl);
    supportedDevice->id = id;
    supportedDevice->description = packet->getMediumString(packet->getMedium());
    supportedDevice->typeNumber = (uint32_t)packet->secondaryAddress();
    device->supportedDevices.push_back(supportedDevice);

    createXmlMaintenanceChannel(device);

    PFunction function = std::make_shared<Function>(Gd::bl);
    function->channel = 1;
    function->type = "MBUS_CHANNEL_1";
    function->variablesId = "mbus_values_1";
    device->functions[1] = function;

    PPacket devicePacket = std::make_shared<Packet>(Gd::bl);
    devicePacket->id = "INFO";
    device->packetsById[devicePacket->id] = devicePacket;
    devicePacket->channel = 1;
    devicePacket->direction = Packet::Direction::Enum::toCentral;
    devicePacket->type = 1;

    auto dataRecords = packet->getDataRecords();
    for (auto &dataRecord: dataRecords) {
      PParameter parameter = std::make_shared<Parameter>(Gd::bl, function->variables);
      parameter->readable = true;
      parameter->writeable = false;

      parseDataRecord(packet->getManufacturer(), packet->getMedium(), dataRecord, parameter, function, devicePacket);

      if (!parameter->casts.empty()) {
        function->variables->parametersOrdered.push_back(parameter);
        function->variables->parameters[parameter->id] = parameter;
      }
    }

    std::string filename = _xmlPath + id + ".xml";
    device->save(filename);

    PeerInfo peerInfo;
    peerInfo.secondary_address = packet->secondaryAddress();
    peerInfo.serialNumber = "MBUS" + id;
    peerInfo.type = packet->secondaryAddress();
    return peerInfo;
  }
  catch (const std::exception &ex) {
    Gd::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }

  return {};
}

DescriptionCreator::PeerInfo DescriptionCreator::CreateEmptyDescription(int32_t secondary_address) {
  try {
    createDirectories();

    std::string id = BaseLib::HelperFunctions::getHexString(secondary_address, 8);

    std::shared_ptr<HomegearDevice> device = std::make_shared<HomegearDevice>(Gd::bl);
    device->version = 1;
    device->timeout = 86400;
    PSupportedDevice supportedDevice = std::make_shared<SupportedDevice>(Gd::bl);
    supportedDevice->id = id;
    supportedDevice->typeNumber = (uint32_t)secondary_address;
    device->supportedDevices.push_back(supportedDevice);

    createXmlMaintenanceChannel(device);

    PFunction function = std::make_shared<Function>(Gd::bl);
    function->channel = 1;
    function->type = "MBUS_CHANNEL_1";
    function->variablesId = "mbus_values_1";
    device->functions[1] = function;

    PPacket devicePacket = std::make_shared<Packet>(Gd::bl);
    devicePacket->id = "INFO";
    device->packetsById[devicePacket->id] = devicePacket;
    devicePacket->channel = 1;
    devicePacket->direction = Packet::Direction::Enum::toCentral;
    devicePacket->type = 1;

    std::string filename = _xmlPath + id + ".xml";
    device->save(filename);

    PeerInfo peerInfo;
    peerInfo.secondary_address = secondary_address;
    peerInfo.serialNumber = "MBUS" + id;
    peerInfo.type = secondary_address;
    return peerInfo;
  }
  catch (const std::exception &ex) {
    Gd::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }

  return {};
}

void DescriptionCreator::createDirectories() {
  try {
    uid_t localUserId = BaseLib::HelperFunctions::userId(Gd::bl->settings.dataPathUser());
    gid_t localGroupId = BaseLib::HelperFunctions::groupId(Gd::bl->settings.dataPathGroup());
    if (((int32_t)localUserId) == -1 || ((int32_t)localGroupId) == -1) {
      localUserId = Gd::bl->userId;
      localGroupId = Gd::bl->groupId;
    }

    std::string path1 = Gd::bl->settings.familyDataPath();
    std::string path2 = path1 + std::to_string(Gd::family->getFamily()) + "/";
    _xmlPath = path2 + "desc/";
    if (!BaseLib::Io::directoryExists(path1)) BaseLib::Io::createDirectory(path1, Gd::bl->settings.dataPathPermissions());
    if (localUserId != 0 || localGroupId != 0) {
      if (chown(path1.c_str(), localUserId, localGroupId) == -1) Gd::out.printWarning("Could not set owner on " + path1);
      if (chmod(path1.c_str(), Gd::bl->settings.dataPathPermissions()) == -1) Gd::out.printWarning("Could not set permissions on " + path1);
    }
    if (!BaseLib::Io::directoryExists(path2)) BaseLib::Io::createDirectory(path2, Gd::bl->settings.dataPathPermissions());
    if (localUserId != 0 || localGroupId != 0) {
      if (chown(path2.c_str(), localUserId, localGroupId) == -1) Gd::out.printWarning("Could not set owner on " + path2);
      if (chmod(path2.c_str(), Gd::bl->settings.dataPathPermissions()) == -1) Gd::out.printWarning("Could not set permissions on " + path2);
    }
    if (!BaseLib::Io::directoryExists(_xmlPath)) BaseLib::Io::createDirectory(_xmlPath, Gd::bl->settings.dataPathPermissions());
    if (localUserId != 0 || localGroupId != 0) {
      if (chown(_xmlPath.c_str(), localUserId, localGroupId) == -1) Gd::out.printWarning("Could not set owner on " + _xmlPath);
      if (chmod(_xmlPath.c_str(), Gd::bl->settings.dataPathPermissions()) == -1) Gd::out.printWarning("Could not set permissions on " + _xmlPath);
    }
  }
  catch (const std::exception &ex) {
    Gd::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void DescriptionCreator::createXmlMaintenanceChannel(PHomegearDevice &device) {
  // {{{ Channel 0
  PFunction function(new Function(Gd::bl));
  function->channel = 0;
  function->type = "MBUS_MAINTENANCE";
  function->variablesId = "mbus_maintenance_values";
  device->functions[function->channel] = function;

  PParameter parameter(new Parameter(Gd::bl, function->variables));
  parameter->id = "UNREACH";
  function->variables->parametersOrdered.push_back(parameter);
  function->variables->parameters[parameter->id] = parameter;
  parameter->writeable = false;
  parameter->service = true;
  parameter->logical = std::make_shared<LogicalBoolean>(Gd::bl);;
  parameter->physical = std::make_shared<PhysicalInteger>(Gd::bl);
  parameter->physical->groupId = parameter->id;
  parameter->physical->operationType = IPhysical::OperationType::internal;
  parameter->roles.emplace(800002, Role(800002, RoleDirection::input, false, false, {}));

  parameter.reset(new Parameter(Gd::bl, function->variables));
  parameter->id = "STICKY_UNREACH";
  function->variables->parametersOrdered.push_back(parameter);
  function->variables->parameters[parameter->id] = parameter;
  parameter->sticky = true;
  parameter->service = true;
  parameter->logical = std::make_shared<LogicalBoolean>(Gd::bl);;
  parameter->physical = std::make_shared<PhysicalInteger>(Gd::bl);
  parameter->physical->groupId = parameter->id;
  parameter->physical->operationType = IPhysical::OperationType::internal;

  parameter.reset(new Parameter(Gd::bl, function->variables));
  parameter->id = "POSSIBLE_HACKING_ATTEMPT";
  function->variables->parametersOrdered.push_back(parameter);
  function->variables->parameters[parameter->id] = parameter;
  parameter->sticky = true;
  parameter->service = true;
  parameter->logical = std::make_shared<LogicalBoolean>(Gd::bl);;
  parameter->physical = std::make_shared<PhysicalInteger>(Gd::bl);
  parameter->physical->groupId = parameter->id;
  parameter->physical->operationType = IPhysical::OperationType::internal;

  parameter.reset(new Parameter(Gd::bl, function->variables));
  parameter->id = "RSSI_DEVICE";
  function->variables->parametersOrdered.push_back(parameter);
  function->variables->parameters[parameter->id] = parameter;
  parameter->service = true;
  parameter->logical = std::make_shared<LogicalInteger>(Gd::bl);;
  parameter->physical = std::make_shared<PhysicalInteger>(Gd::bl);
  parameter->physical->groupId = parameter->id;
  parameter->physical->operationType = IPhysical::OperationType::internal;
  // }}}
}

void DescriptionCreator::parseDataRecord(const std::string &manufacturer, uint8_t medium, MbusPacket::DataRecord &dataRecord, PParameter &parameter, PFunction &function, PPacket &packet) {
  try {
    uint8_t dif = dataRecord.difs.front() & 0x0Fu;
    parameter->metadata = BaseLib::HelperFunctions::getHexString(dataRecord.vifs);

    ParameterCast::PGeneric cast1 = std::make_shared<ParameterCast::Generic>(Gd::bl);
    cast1->type = "0x" + BaseLib::HelperFunctions::getHexString(dif, 2);
    parameter->casts.push_back(cast1);

    parameter->physical = std::make_shared<PhysicalInteger>(Gd::bl);
    parameter->physical->operationType = IPhysical::OperationType::Enum::command;
    std::shared_ptr<Parameter::Packet> eventPacket = std::make_shared<Parameter::Packet>();
    eventPacket->type = Parameter::Packet::Type::Enum::event;
    eventPacket->id = "INFO";
    parameter->eventPackets.push_back(eventPacket);

    if (dif == 0 || dif == 1 || dif == 2 || dif == 3 || dif == 4 || dif == 6 || dif == 7 || dif == 9 || dif == 10 || dif == 11 || dif == 12 || dif == 14) {
      parameter->logical = std::make_shared<LogicalInteger>(Gd::bl);
    } else if (dif == 5) {
      parameter->logical = std::make_shared<LogicalDecimal>(Gd::bl);
    } else if (dif == 8 || dif == 13 || dif == 15) {
      parameter->logical = std::make_shared<LogicalString>(Gd::bl);
    }

    if (!dataRecord.vifCustomName.empty()) {
      parameter->id = dataRecord.vifCustomName;
    } else if (dataRecord.vifs.size() == 1) {
      auto vifIterator = vif_info_.find(dataRecord.vifs.front());
      if (vifIterator == vif_info_.end()) parameter->id = "UNKNOWN_" + BaseLib::HelperFunctions::getHexString(dataRecord.vifs.front(), 2);
      else setVifInfo(parameter, vifIterator->second, dataRecord, medium);
    } else if (dataRecord.vifs.size() == 2) {
      if (dataRecord.vifs.front() == 0xFB) {
        auto vifIterator = vif_fb_info_.find(dataRecord.vifs.at(1));
        if (vifIterator == vif_fb_info_.end()) parameter->id = "UNKNOWN_" + BaseLib::HelperFunctions::getHexString(dataRecord.vifs);
        else setVifInfo(parameter, vifIterator->second, dataRecord, medium);
      } else if (dataRecord.vifs.front() == 0xFD) {
        auto vifIterator = vif_fd_info_.find(dataRecord.vifs.at(1));
        if (vifIterator == vif_fd_info_.end()) parameter->id = "UNKNOWN_" + BaseLib::HelperFunctions::getHexString(dataRecord.vifs);
        else setVifInfo(parameter, vifIterator->second, dataRecord, medium);
      } else if (dataRecord.vifs.front() == 0xFF) {
        //Manufacturer specific
        if (manufacturer == "KAM") {
          if (dataRecord.vifs.at(1) == 0x20) parameter->id = "INFOCODE";
          else parameter->id = "MANUFACTURER_SPECIFIC_" + BaseLib::HelperFunctions::getHexString(dataRecord.vifs);
        } else parameter->id = "MANUFACTURER_SPECIFIC_" + BaseLib::HelperFunctions::getHexString(dataRecord.vifs);
      } else {
        parameter->id = "UNKNOWN_" + BaseLib::HelperFunctions::getHexString(dataRecord.vifs);
      }
    } else if (dataRecord.difs.front() == 0xF) { //Manufacturer specific
      parameter->id = "UNKNOWN_MANUFACTURER_SPECIFIC";
    } else {
      parameter->id = "UNKNOWN_" + BaseLib::HelperFunctions::getHexString(dataRecord.vifs);
    }

    if ((int32_t)dataRecord.difFunction > 0) parameter->id += "_F" + std::to_string((int32_t)dataRecord.difFunction);
    if (dataRecord.subunit > 0) parameter->id += "_SU" + std::to_string(dataRecord.subunit);
    if (dataRecord.storageNumber > 0) parameter->id += "_SN" + std::to_string(dataRecord.storageNumber);
    if (dataRecord.tariff > 0) parameter->id += "_T" + std::to_string(dataRecord.tariff);
    parameter->id = getFreeParameterId(parameter->id, function);
    if (parameter->id.empty()) return;
    parameter->physical->groupId = parameter->id;

    PBinaryPayload payload = std::make_shared<BinaryPayload>(Gd::bl);
    payload->bitIndex = dataRecord.dataStart * 8;
    payload->bitSize = dataRecord.dataSize * 8;
    payload->metaInteger1 = (int32_t)dataRecord.difFunction;
    payload->metaInteger2 = dataRecord.subunit;
    payload->metaInteger3 = dataRecord.storageNumber;
    payload->metaInteger4 = dataRecord.tariff;
    payload->parameterId = parameter->id;
    packet->binaryPayloads.push_back(payload);
  }
  catch (const std::exception &ex) {
    Gd::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void DescriptionCreator::setVifInfo(PParameter &parameter, const VifInfo &vif_info, const MbusPacket::DataRecord &dataRecord, uint8_t medium) {
  try {
    parameter->id = vif_info.name;
    parameter->unit = vif_info.unit;
    parameter->unit_code = vif_info.unit_code;

    if (vif_info.unit_scale_factor != 1) {
      auto cast2 = std::make_shared<DecimalIntegerScale>(Gd::bl);
      if (vif_info.unit_scale_operation == VifScaleOperation::kDivision) cast2->factor = vif_info.unit_scale_factor;
      else cast2->factor = 1.0 / (double)vif_info.unit_scale_factor;
      parameter->casts.emplace_back(std::move(cast2));
    }

    if (dataRecord.difFunction == MbusPacket::DifFunction::instantaneousValue && dataRecord.subunit == 0 && dataRecord.storageNumber == 0 && dataRecord.tariff == 0) {
      auto role_iterator = vif_info.medium_role_map.find(medium);
      if (role_iterator != vif_info.medium_role_map.end()) {
        parameter->roles.emplace(role_iterator->second, Role(role_iterator->second, RoleDirection::input, false, false, {}));
      }
    }
  }
  catch (const std::exception &ex) {
    Gd::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

std::string DescriptionCreator::getFreeParameterId(std::string baseId, PFunction &function) {
  try {
    if (function->variables->parameters.find(baseId) != function->variables->parameters.end()) {
      int32_t i = 1;
      std::string currentId = baseId + "_" + std::to_string(i);
      while (function->variables->parameters.find(currentId) != function->variables->parameters.end()) {
        i++;
        currentId = baseId + "_" + std::to_string(i);
      }
      return currentId;
    }
    return baseId;
  }
  catch (const std::exception &ex) {
    Gd::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return "";
}

}
