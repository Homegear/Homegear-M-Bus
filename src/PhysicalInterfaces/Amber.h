/* Copyright 2013-2019 Homegear GmbH */

#ifndef USB300_H_
#define USB300_H_

#include "../MyPacket.h"
#include "IMBusInterface.h"
#include <homegear-base/BaseLib.h>

namespace MyFamily
{

class Amber : public IMBusInterface
{
public:
	Amber(std::shared_ptr<BaseLib::Systems::PhysicalInterfaceSettings> settings);
	virtual ~Amber();

	virtual void startListening();
	virtual void stopListening();
	virtual void setup(int32_t userID, int32_t groupID, bool setPermissions);

	virtual bool isOpen() { return _serial && _serial->isOpen() && !_stopped; }
protected:
	std::unique_ptr<BaseLib::SerialReaderWriter> _serial;
    std::atomic_bool _initComplete;
	std::thread _initThread;

	std::unordered_set<uint8_t> _securityModeWhitelist;

	void init();
	void reconnect();
	void listen();
	bool setParameter(uint8_t address, uint8_t value);
	virtual void rawSend(std::vector<uint8_t>& packet);
	void processPacket(std::vector<uint8_t>& data);
};

}

#endif
