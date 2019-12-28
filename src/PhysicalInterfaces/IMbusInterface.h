/* Copyright 2013-2019 Homegear GmbH */

#ifndef IENOCEANINTERFACE_H_
#define IENOCEANINTERFACE_H_

#include <homegear-base/BaseLib.h>

namespace Mbus
{

class IMbusInterface : public BaseLib::Systems::IPhysicalInterface
{
public:
	IMbusInterface(std::shared_ptr<BaseLib::Systems::PhysicalInterfaceSettings> settings);
	virtual ~IMbusInterface();

	virtual void startListening() {}
	virtual void stopListening() {}

	virtual void sendPacket(std::shared_ptr<BaseLib::Systems::Packet> packet) {}
protected:
	class Request
	{
	public:
		std::mutex mutex;
		std::condition_variable conditionVariable;
		bool mutexReady = false;
		std::vector<uint8_t> response;

		Request() {}
		virtual ~Request() {}
	private:
	};

	BaseLib::SharedObjects* _bl = nullptr;
	BaseLib::Output _out;

	std::mutex _sendPacketMutex;
	std::mutex _getResponseMutex;

	std::mutex _requestsMutex;
	std::map<uint8_t, std::shared_ptr<Request>> _requests;


	void getResponse(std::vector<uint8_t>& requestPacket, std::vector<uint8_t>& responsePacket);
	virtual void rawSend(std::vector<uint8_t>& packet) {}
    void addCrc8(std::vector<uint8_t>& packet);

	virtual void raisePacketReceived(std::shared_ptr<BaseLib::Systems::Packet> packet);
};

}

#endif
