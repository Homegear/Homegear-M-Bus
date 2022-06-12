/* Copyright 2013-2019 Homegear GmbH */

#include "Gd.h"

namespace Mbus {
BaseLib::SharedObjects *Gd::bl = nullptr;
Mbus *Gd::family = nullptr;
std::shared_ptr<Interfaces> Gd::interfaces;
BaseLib::Output Gd::out;
}
