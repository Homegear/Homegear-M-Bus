/* Copyright 2013-2017 Homegear UG (haftungsbeschränkt) */

#ifndef MYCENTRAL_H_
#define MYCENTRAL_H_

#include "MyPeer.h"
#include "MyPacket.h"
#include <homegear-base/BaseLib.h>

#include <memory>
#include <mutex>
#include <string>

namespace MyFamily
{

class MyCentral : public BaseLib::Systems::ICentral
{
public:
	MyCentral(ICentralEventSink* eventHandler);
	MyCentral(uint32_t deviceType, std::string serialNumber, ICentralEventSink* eventHandler);
	virtual ~MyCentral();
	virtual void dispose(bool wait = true);

	std::string handleCliCommand(std::string command);
	virtual bool onPacketReceived(std::string& senderId, std::shared_ptr<BaseLib::Systems::Packet> packet);

	uint64_t getPeerIdFromSerial(std::string& serialNumber) { std::shared_ptr<MyPeer> peer = getPeer(serialNumber); if(peer) return peer->getID(); else return 0; }
	PMyPeer getPeer(uint64_t id);
	PMyPeer getPeer(int32_t address);
	PMyPeer getPeer(std::string serialNumber);

	virtual PVariable deleteDevice(BaseLib::PRpcClientInfo clientInfo, std::string serialNumber, int32_t flags);
	virtual PVariable deleteDevice(BaseLib::PRpcClientInfo clientInfo, uint64_t peerId, int32_t flags);
	virtual PVariable getDeviceInfo(BaseLib::PRpcClientInfo clientInfo, uint64_t id, std::map<std::string, bool> fields);
	virtual PVariable getSniffedDevices(BaseLib::PRpcClientInfo clientInfo);
	virtual PVariable putParamset(BaseLib::PRpcClientInfo clientInfo, std::string serialNumber, int32_t channel, ParameterGroup::Type::Enum type, std::string remoteSerialNumber, int32_t remoteChannel, PVariable paramset);
	virtual PVariable putParamset(BaseLib::PRpcClientInfo clientInfo, uint64_t peerId, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteId, int32_t remoteChannel, PVariable paramset);
	virtual PVariable setInstallMode(BaseLib::PRpcClientInfo clientInfo, bool on, uint32_t duration = 60, bool debugOutput = true);
	virtual PVariable startSniffing(BaseLib::PRpcClientInfo clientInfo);
	virtual PVariable stopSniffing(BaseLib::PRpcClientInfo clientInfo);
protected:
	bool _sniff = false;
	std::mutex _sniffedPacketsMutex;
	std::map<int32_t, std::vector<PMyPacket>> _sniffedPackets;

	std::atomic_bool _pairing;
	std::atomic<uint32_t> _timeLeftInPairingMode;
	std::atomic_bool _stopPairingModeThread;
	std::mutex _pairingModeThreadMutex;
	std::thread _pairingModeThread;

	std::string getFreeSerialNumber(int32_t address);
	virtual void init();
	virtual void loadPeers();
	virtual void savePeers(bool full);
	virtual void loadVariables() {}
	virtual void saveVariables() {}
	std::shared_ptr<MyPeer> createPeer(uint32_t deviceType, int32_t address, std::string serialNumber, bool save = true);
	void deletePeer(uint64_t id);

	void pairingModeTimer(int32_t duration, bool debugOutput = true);
};

}

#endif
