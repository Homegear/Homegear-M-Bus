/* Copyright 2013-2019 Homegear GmbH */

#include "GD.h"

namespace Mbus {
BaseLib::SharedObjects *GD::bl = nullptr;
Mbus *GD::family = nullptr;
std::shared_ptr<Interfaces> GD::interfaces;
BaseLib::Output GD::out;
}
