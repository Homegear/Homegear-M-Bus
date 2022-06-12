/* Copyright 2013-2019 Homegear GmbH */

#include "DescriptionCreator.h"

#include <memory>
#include "Gd.h"

namespace Mbus {

DescriptionCreator::DescriptionCreator() {
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

  //18 to 23 reserved

  _vifFbVariableNameMap[24] = "MASS";
  _vifFbUnit[24] = "10² t";
  _vifFbVariableNameMap[25] = "MASS";
  _vifFbUnit[25] = "10³ t";

  //26 to 32 reserved

  _vifFbVariableNameMap[33] = "VOLUME";
  _vifFbUnit[33] = "10⁻¹ feet³";
  _vifFbVariableNameMap[34] = "VOLUME";
  _vifFbUnit[34] = "10⁻¹ american gallons";
  _vifFbVariableNameMap[35] = "VOLUME";
  _vifFbUnit[35] = "american gallon";

  _vifFbVariableNameMap[36] = "VOLUME_FLOW";
  _vifFbUnit[36] = "10⁻³ american gallons/min";
  _vifFbVariableNameMap[37] = "VOLUME_FLOW";
  _vifFbUnit[37] = "american gallons/min";
  _vifFbVariableNameMap[38] = "VOLUME_FLOW";
  _vifFbUnit[38] = "american gallons/hour";

  //39 reserved

  _vifFbVariableNameMap[40] = "POWER";
  _vifFbUnit[40] = "10⁻¹ MW";
  _vifFbVariableNameMap[41] = "POWER";
  _vifFbUnit[41] = "MW";

  //42 to 87 reserved

  _vifFbVariableNameMap[88] = "FLOW_TEMPERATURE";
  _vifFbUnit[88] = "10⁻³ °F";
  _vifFbVariableNameMap[89] = "FLOW_TEMPERATURE";
  _vifFbUnit[89] = "10⁻² °F";
  _vifFbVariableNameMap[90] = "FLOW_TEMPERATURE";
  _vifFbUnit[90] = "10⁻¹ °F";
  _vifFbVariableNameMap[91] = "FLOW_TEMPERATURE";
  _vifFbUnit[91] = "°F";

  _vifFbVariableNameMap[92] = "RETURN_TEMPERATURE";
  _vifFbUnit[92] = "10⁻³ °F";
  _vifFbVariableNameMap[93] = "RETURN_TEMPERATURE";
  _vifFbUnit[93] = "10⁻² °F";
  _vifFbVariableNameMap[94] = "RETURN_TEMPERATURE";
  _vifFbUnit[94] = "10⁻¹ °F";
  _vifFbVariableNameMap[95] = "RETURN_TEMPERATURE";
  _vifFbUnit[95] = "°F";

  _vifFbVariableNameMap[96] = "TEMPERATURE_DIFFERENCE";
  _vifFbUnit[96] = "10⁻³ °F";
  _vifFbVariableNameMap[97] = "TEMPERATURE_DIFFERENCE";
  _vifFbUnit[97] = "10⁻² °F";
  _vifFbVariableNameMap[98] = "TEMPERATURE_DIFFERENCE";
  _vifFbUnit[98] = "10⁻¹ °F";
  _vifFbVariableNameMap[99] = "TEMPERATURE_DIFFERENCE";
  _vifFbUnit[99] = "°F";

  _vifFbVariableNameMap[100] = "EXTERNAL_TEMPERATURE";
  _vifFbUnit[100] = "10⁻³ °F";
  _vifFbVariableNameMap[101] = "EXTERNAL_TEMPERATURE";
  _vifFbUnit[101] = "10⁻² °F";
  _vifFbVariableNameMap[102] = "EXTERNAL_TEMPERATURE";
  _vifFbUnit[102] = "10⁻¹ °F";
  _vifFbVariableNameMap[103] = "EXTERNAL_TEMPERATURE";
  _vifFbUnit[103] = "°F";

  //104 to 111

  _vifFbVariableNameMap[112] = "COLD_WARM_TEMPERATURE_LIMIT";
  _vifFbUnit[112] = "10⁻³ °F";
  _vifFbVariableNameMap[113] = "COLD_WARM_TEMPERATURE_LIMIT";
  _vifFbUnit[113] = "10⁻² °F";
  _vifFbVariableNameMap[114] = "COLD_WARM_TEMPERATURE_LIMIT";
  _vifFbUnit[114] = "10⁻¹ °F";
  _vifFbVariableNameMap[115] = "COLD_WARM_TEMPERATURE_LIMIT";
  _vifFbUnit[115] = "°F";

  _vifFbVariableNameMap[116] = "COLD_WARM_TEMPERATURE_LIMIT";
  _vifFbUnit[116] = "10⁻³ °C";
  _vifFbVariableNameMap[117] = "COLD_WARM_TEMPERATURE_LIMIT";
  _vifFbUnit[117] = "10⁻² °C";
  _vifFbVariableNameMap[118] = "COLD_WARM_TEMPERATURE_LIMIT";
  _vifFbUnit[118] = "10⁻¹ °C";
  _vifFbVariableNameMap[119] = "COLD_WARM_TEMPERATURE_LIMIT";
  _vifFbUnit[119] = "°C";

  _vifFbVariableNameMap[120] = "MAX_POWER_COUNT";
  _vifFbUnit[120] = "mW";
  _vifFbVariableNameMap[121] = "MAX_POWER_COUNT";
  _vifFbUnit[121] = "10⁻² W";
  _vifFbVariableNameMap[122] = "MAX_POWER_COUNT";
  _vifFbUnit[122] = "10⁻¹ W";
  _vifFbVariableNameMap[123] = "MAX_POWER_COUNT";
  _vifFbUnit[123] = "W";
  _vifFbVariableNameMap[124] = "MAX_POWER_COUNT";
  _vifFbUnit[124] = "10⁻² kW";
  _vifFbVariableNameMap[125] = "MAX_POWER_COUNT";
  _vifFbUnit[125] = "10⁻¹ kW";
  _vifFbVariableNameMap[126] = "MAX_POWER_COUNT";
  _vifFbUnit[126] = "kW";
  _vifFbVariableNameMap[127] = "MAX_POWER_COUNT";
  _vifFbUnit[127] = "10¹ kW";

  _vifFdVariableNameMap[0] = "CREDIT";
  _vifFdUnit[0] = "10⁻³ Currency Units";
  _vifFdVariableNameMap[1] = "CREDIT";
  _vifFdUnit[1] = "10⁻² Currency Units";
  _vifFdVariableNameMap[2] = "CREDIT";
  _vifFdUnit[2] = "10⁻¹ Currency Units";
  _vifFdVariableNameMap[3] = "CREDIT";
  _vifFdUnit[3] = "Currency Units";

  _vifFdVariableNameMap[4] = "DEBIT";
  _vifFdUnit[4] = "10⁻³ Currency Units";
  _vifFdVariableNameMap[5] = "DEBIT";
  _vifFdUnit[5] = "10⁻² Currency Units";
  _vifFdVariableNameMap[6] = "DEBIT";
  _vifFdUnit[6] = "10⁻¹ Currency Units";
  _vifFdVariableNameMap[7] = "DEBIT";
  _vifFdUnit[7] = "Currency Units";

  _vifFdVariableNameMap[8] = "ACCESS_NUMBER";
  _vifFdVariableNameMap[9] = "MEDIUM";
  _vifFdVariableNameMap[10] = "MANUFACTURER";
  _vifFdVariableNameMap[11] = "PARAMETER_SET_IDENTIFICATION";
  _vifFdVariableNameMap[12] = "MODEL";
  _vifFdVariableNameMap[13] = "HARDWARE_VERSION";
  _vifFdVariableNameMap[14] = "FIRMWARE_VERSION";
  _vifFdVariableNameMap[15] = "SOFTWARE_VERSION";
  _vifFdVariableNameMap[16] = "CUSTOMER_LOCATION";
  _vifFdVariableNameMap[17] = "CUSTOMER";
  _vifFdVariableNameMap[18] = "ACCESS_CODE_USER";
  _vifFdVariableNameMap[19] = "ACCESS_CODE_OPERATOR";
  _vifFdVariableNameMap[20] = "ACCESS_CODE_SYSTEM_OPERATOR";
  _vifFdVariableNameMap[21] = "ACCESS_CODE_DEVELOPER";
  _vifFdVariableNameMap[22] = "PASSWORD";
  _vifFdVariableNameMap[23] = "ERROR_FLAGS_BINARY";
  _vifFdVariableNameMap[24] = "ERROR_MASK";
  _vifFdVariableNameMap[26] = "DIGITAL_OUTPUT_BINARY";
  _vifFdVariableNameMap[27] = "DIGITAL_INPUT_BINARY";
  _vifFdVariableNameMap[28] = "BAUDRATE";
  _vifFdVariableNameMap[29] = "RESPONSE_DELAY";
  _vifFdVariableNameMap[30] = "RETRY";
  _vifFdVariableNameMap[32] = "FIRST_STORAGE_NUMBER";
  _vifFdVariableNameMap[33] = "LAST_STORAGE_NUMBER";
  _vifFdVariableNameMap[34] = "STORAGE_BLOCK_SIZE";

  _vifFdVariableNameMap[36] = "STORAGE_INTERVAL";
  _vifFdUnit[36] = "s";
  _vifFdVariableNameMap[37] = "STORAGE_INTERVAL";
  _vifFdUnit[37] = "m";
  _vifFdVariableNameMap[38] = "STORAGE_INTERVAL";
  _vifFdUnit[38] = "h";
  _vifFdVariableNameMap[39] = "STORAGE_INTERVAL";
  _vifFdUnit[39] = "d";

  _vifFdVariableNameMap[40] = "STORAGE_INTERVAL_MONTHS";
  _vifFdVariableNameMap[41] = "STORAGE_INTERVAL_YEARS";

  _vifFdVariableNameMap[44] = "DURATION_SINCE_LAST_READOUT";
  _vifFdUnit[44] = "s";
  _vifFdVariableNameMap[45] = "DURATION_SINCE_LAST_READOUT";
  _vifFdUnit[45] = "m";
  _vifFdVariableNameMap[46] = "DURATION_SINCE_LAST_READOUT";
  _vifFdUnit[46] = "h";
  _vifFdVariableNameMap[47] = "DURATION_SINCE_LAST_READOUT";
  _vifFdUnit[47] = "d";

  _vifFdVariableNameMap[48] = "TARIFF_START_DATETIME";

  _vifFdVariableNameMap[49] = "TARIFF_DURATION";
  _vifFdUnit[49] = "s";
  _vifFdVariableNameMap[50] = "TARIFF_DURATION";
  _vifFdUnit[50] = "m";
  _vifFdVariableNameMap[51] = "TARIFF_DURATION";
  _vifFdUnit[51] = "h";
  _vifFdVariableNameMap[52] = "TARIFF_DURATION";
  _vifFdUnit[52] = "d";

  _vifFdVariableNameMap[53] = "TARIFF_PERIOD";
  _vifFdUnit[53] = "s";
  _vifFdVariableNameMap[54] = "TARIFF_PERIOD";
  _vifFdUnit[54] = "m";
  _vifFdVariableNameMap[55] = "TARIFF_PERIOD";
  _vifFdUnit[55] = "h";
  _vifFdVariableNameMap[56] = "TARIFF_PERIOD";
  _vifFdUnit[56] = "d";

  _vifFdVariableNameMap[57] = "TARIFF_PERIOD_MONTHS";
  _vifFdVariableNameMap[58] = "TARIFF_PERIOD_YEARS";
  _vifFdVariableNameMap[59] = "DIMENSIONLESS";

  _vifFdVariableNameMap[64] = "VOLTAGE";
  _vifFdUnit[64] = "nV";
  _vifFdVariableNameMap[65] = "VOLTAGE";
  _vifFdUnit[65] = "10⁻² µV";
  _vifFdVariableNameMap[66] = "VOLTAGE";
  _vifFdUnit[66] = "10⁻¹ µV";
  _vifFdVariableNameMap[67] = "VOLTAGE";
  _vifFdUnit[67] = "µV";
  _vifFdVariableNameMap[68] = "VOLTAGE";
  _vifFdUnit[68] = "10⁻² mV";
  _vifFdVariableNameMap[69] = "VOLTAGE";
  _vifFdUnit[69] = "10⁻¹ mV";
  _vifFdVariableNameMap[70] = "VOLTAGE";
  _vifFdUnit[70] = "mV";
  _vifFdVariableNameMap[71] = "VOLTAGE";
  _vifFdUnit[71] = "10⁻² V";
  _vifFdVariableNameMap[72] = "VOLTAGE";
  _vifFdUnit[72] = "10⁻¹ V";
  _vifFdVariableNameMap[73] = "VOLTAGE";
  _vifFdUnit[73] = "V";
  _vifFdVariableNameMap[74] = "VOLTAGE";
  _vifFdUnit[74] = "10⁻² kV";
  _vifFdVariableNameMap[75] = "VOLTAGE";
  _vifFdUnit[75] = "10⁻¹ kV";
  _vifFdVariableNameMap[76] = "VOLTAGE";
  _vifFdUnit[76] = "kV";
  _vifFdVariableNameMap[77] = "VOLTAGE";
  _vifFdUnit[77] = "10⁻² MV";
  _vifFdVariableNameMap[78] = "VOLTAGE";
  _vifFdUnit[78] = "10⁻¹ MV";
  _vifFdVariableNameMap[79] = "VOLTAGE";
  _vifFdUnit[79] = "MV";

  _vifFdVariableNameMap[80] = "CURRENT";
  _vifFdUnit[80] = "pA";
  _vifFdVariableNameMap[81] = "CURRENT";
  _vifFdUnit[81] = "10⁻² nA";
  _vifFdVariableNameMap[82] = "CURRENT";
  _vifFdUnit[82] = "10⁻¹ nA";
  _vifFdVariableNameMap[83] = "CURRENT";
  _vifFdUnit[83] = "nA";
  _vifFdVariableNameMap[84] = "CURRENT";
  _vifFdUnit[84] = "10⁻² µA";
  _vifFdVariableNameMap[85] = "CURRENT";
  _vifFdUnit[85] = "10⁻¹ µA";
  _vifFdVariableNameMap[86] = "CURRENT";
  _vifFdUnit[86] = "µA";
  _vifFdVariableNameMap[87] = "CURRENT";
  _vifFdUnit[87] = "10⁻² mA";
  _vifFdVariableNameMap[88] = "CURRENT";
  _vifFdUnit[88] = "10⁻¹ mA";
  _vifFdVariableNameMap[89] = "CURRENT";
  _vifFdUnit[89] = "mA";
  _vifFdVariableNameMap[90] = "CURRENT";
  _vifFdUnit[90] = "10⁻² A";
  _vifFdVariableNameMap[91] = "CURRENT";
  _vifFdUnit[91] = "10⁻¹ A";
  _vifFdVariableNameMap[92] = "CURRENT";
  _vifFdUnit[92] = "A";
  _vifFdVariableNameMap[93] = "CURRENT";
  _vifFdUnit[93] = "10⁻² kA";
  _vifFdVariableNameMap[94] = "CURRENT";
  _vifFdUnit[94] = "10⁻¹ kA";
  _vifFdVariableNameMap[95] = "CURRENT";
  _vifFdUnit[95] = "kA";

  _vifFdVariableNameMap[96] = "RESET_COUNTER";
  _vifFdVariableNameMap[97] = "CUMULATION_COUNTER";
  _vifFdVariableNameMap[98] = "CONTROL_SIGNAL";
  _vifFdVariableNameMap[99] = "DAY_OF_WEEK";
  _vifFdVariableNameMap[100] = "WEEK_NUMBER";
  _vifFdVariableNameMap[101] = "DAY_CHANGE_TIMEPOINT";
  _vifFdVariableNameMap[102] = "PARAMETER_ACTIVATION_STATE";
  _vifFdVariableNameMap[103] = "SPECIAL_SUPPLIER_INFORMATION";

  _vifFdVariableNameMap[104] = "DURATION_SINCE_LAST_CUMULATION";
  _vifFdUnit[104] = "h";
  _vifFdVariableNameMap[105] = "DURATION_SINCE_LAST_CUMULATION";
  _vifFdUnit[105] = "d";
  _vifFdVariableNameMap[106] = "DURATION_SINCE_LAST_CUMULATION";
  _vifFdUnit[106] = "m";
  _vifFdVariableNameMap[107] = "DURATION_SINCE_LAST_CUMULATION";
  _vifFdUnit[107] = "y";

  _vifFdVariableNameMap[108] = "BATTERY_OPERATING_TIME";
  _vifFdUnit[108] = "h";
  _vifFdVariableNameMap[109] = "BATTERY_OPERATING_TIME";
  _vifFdUnit[109] = "d";
  _vifFdVariableNameMap[110] = "BATTERY_OPERATING_TIME";
  _vifFdUnit[110] = "m";
  _vifFdVariableNameMap[111] = "BATTERY_OPERATING_TIME";
  _vifFdUnit[111] = "y";

  _vifFdVariableNameMap[112] = "BATTERY_CHANGE_DATETIME";
}

DescriptionCreator::PeerInfo DescriptionCreator::createDescription(PMbusPacket packet) {
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

      parseDataRecord(packet->getManufacturer(), dataRecord, parameter, function, devicePacket);

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

  return PeerInfo();
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

void DescriptionCreator::parseDataRecord(const std::string &manufacturer, MbusPacket::DataRecord &dataRecord, PParameter &parameter, PFunction &function, PPacket &packet) {
  try {
    uint8_t dif = dataRecord.difs.front() & 0x0Fu;
    parameter->metadata = BaseLib::HelperFunctions::getHexString(dataRecord.vifs);

    ParameterCast::PGeneric cast = std::make_shared<ParameterCast::Generic>(Gd::bl);
    cast->type = "0x" + BaseLib::HelperFunctions::getHexString(dif, 2);

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
      auto vifIterator = _vifVariableNameMap.find(dataRecord.vifs.front());
      if (vifIterator == _vifVariableNameMap.end()) parameter->id = "UNKNOWN_" + BaseLib::HelperFunctions::getHexString(dataRecord.vifs.front(), 2);
      else parameter->id = vifIterator->second;

      auto unitIterator = _vifUnit.find(dataRecord.vifs.front());
      if (unitIterator != _vifUnit.end()) parameter->unit = unitIterator->second;
    } else if (dataRecord.vifs.size() == 2) {
      if (dataRecord.vifs.front() == 0xFB) {
        auto vifIterator = _vifFbVariableNameMap.find(dataRecord.vifs.at(1));
        if (vifIterator == _vifFbVariableNameMap.end()) parameter->id = "UNKNOWN_" + BaseLib::HelperFunctions::getHexString(dataRecord.vifs);
        else parameter->id = vifIterator->second;

        auto unitIterator = _vifFbUnit.find(dataRecord.vifs.at(1));
        if (unitIterator != _vifFbUnit.end()) parameter->unit = unitIterator->second;
      } else if (dataRecord.vifs.front() == 0xFD) {
        auto vifIterator = _vifFdVariableNameMap.find(dataRecord.vifs.at(1));
        if (vifIterator == _vifFdVariableNameMap.end()) parameter->id = "UNKNOWN_" + BaseLib::HelperFunctions::getHexString(dataRecord.vifs);
        else parameter->id = vifIterator->second;

        auto unitIterator = _vifFdUnit.find(dataRecord.vifs.at(1));
        if (unitIterator != _vifFdUnit.end()) parameter->unit = unitIterator->second;
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

    parameter->casts.push_back(cast);
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
