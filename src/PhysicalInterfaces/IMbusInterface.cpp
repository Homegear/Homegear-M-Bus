/* Copyright 2013-2019 Homegear GmbH */

#include "IMbusInterface.h"
#include "../GD.h"
#include "../MbusPacket.h"

namespace Mbus
{

IMbusInterface::IMbusInterface(std::shared_ptr<BaseLib::Systems::PhysicalInterfaceSettings> settings) : IPhysicalInterface(GD::bl, GD::family->getFamily(), settings)
{
	_bl = GD::bl;

	if(settings->listenThreadPriority == -1)
	{
		settings->listenThreadPriority = 0;
		settings->listenThreadPolicy = SCHED_OTHER;
	}
}

IMbusInterface::~IMbusInterface()
{

}

void IMbusInterface::getResponse(std::vector<uint8_t>& requestPacket, std::vector<uint8_t>& responsePacket)
{
	try
    {
        if(_stopped || requestPacket.size() < 4) return;
        responsePacket.clear();

        uint8_t responsePacketType = requestPacket.at(1) | 0x80;

        std::lock_guard<std::mutex> sendPacketGuard(_sendPacketMutex);
        std::lock_guard<std::mutex> getResponseGuard(_getResponseMutex);
        std::shared_ptr<Request> request(new Request());
        std::unique_lock<std::mutex> requestsGuard(_requestsMutex);
        _requests[responsePacketType] = request;
        requestsGuard.unlock();
        std::unique_lock<std::mutex> lock(request->mutex);

        try
        {
            if(_bl->debugLevel >= 5) GD::out.printDebug("Debug: Sending packet " + BaseLib::HelperFunctions::getHexString(requestPacket));
            rawSend(requestPacket);
        }
        catch(BaseLib::SocketOperationException ex)
        {
            _out.printError("Error sending packet: " + std::string(ex.what()));
            return;
        }

        if(!request->conditionVariable.wait_for(lock, std::chrono::milliseconds(10000), [&] { return request->mutexReady; }))
        {
            _out.printError("Error: No response received to packet: " + BaseLib::HelperFunctions::getHexString(requestPacket));
        }
        responsePacket = request->response;

        requestsGuard.lock();
        _requests.erase(responsePacketType);
        requestsGuard.unlock();
	}
	catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

void IMbusInterface::addCrc8(std::vector<uint8_t>& packet)
{
    try
    {
        if(packet.size() < 4) return;

        uint8_t crc8 = 0;
        for(uint32_t i = 0; i < packet.size() - 1; i++)
        {
            crc8 = crc8 ^ (uint8_t)packet[i];
        }
        packet.back() = crc8;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

void IMbusInterface::raisePacketReceived(std::shared_ptr<BaseLib::Systems::Packet> packet)
{
    try
    {
        PMbusPacket myPacket(std::dynamic_pointer_cast<MbusPacket>(packet));
        if(!myPacket) return;

        BaseLib::Systems::IPhysicalInterface::raisePacketReceived(packet);
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}
}
