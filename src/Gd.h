/* Copyright 2013-2019 Homegear GmbH */

#ifndef GD_H_
#define GD_H_

#define MY_FAMILY_ID 23
#define MY_FAMILY_NAME "M-Bus"

#include <homegear-base/BaseLib.h>
#include "Mbus.h"
#include "Interfaces.h"

namespace Mbus {

class Gd {
 public:
  Gd() = delete;

  static BaseLib::SharedObjects *bl;
  static Mbus *family;
  static std::shared_ptr<Interfaces> interfaces;
  static BaseLib::Output out;
 private:
};

}

#endif /* GD_H_ */
