/* Copyright 2013-2017 Homegear UG (haftungsbeschränkt) */

#ifndef GD_H_
#define GD_H_

#define MY_FAMILY_ID 23
#define MY_FAMILY_NAME "M-Bus"

#include <homegear-base/BaseLib.h>
#include "MyFamily.h"
#include "PhysicalInterfaces/IMBusInterface.h"

namespace MyFamily
{

class GD
{
public:
	virtual ~GD();

	static BaseLib::SharedObjects* bl;
	static MyFamily* family;
	static std::map<std::string, std::shared_ptr<IMBusInterface>> physicalInterfaces;
	static std::shared_ptr<IMBusInterface> defaultPhysicalInterface;
	static BaseLib::Output out;
private:
	GD();
};

}

#endif /* GD_H_ */
