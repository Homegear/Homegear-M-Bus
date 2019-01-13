/* Copyright 2013-2019 Homegear GmbH */

#ifndef MYCENTRAL_H_
#define MYCENTRAL_H_

#include "MyPeer.h"
#include "MyPacket.h"
#include "DescriptionCreator.h"
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
	virtual PVariable getSniffedDevices(BaseLib::PRpcClientInfo clientInfo);
	virtual PVariable setInstallMode(BaseLib::PRpcClientInfo clientInfo, bool on, uint32_t duration, BaseLib::PVariable metadata, bool debugOutput = true);
	virtual PVariable startSniffing(BaseLib::PRpcClientInfo clientInfo);
	virtual PVariable stopSniffing(BaseLib::PRpcClientInfo clientInfo);
protected:
	bool _sniff = false;
	std::mutex _sniffedPacketsMutex;
	std::map<int32_t, std::vector<PMyPacket>> _sniffedPackets;

	std::atomic_bool _stopPairingModeThread;
	std::mutex _pairingModeThreadMutex;
	std::thread _pairingModeThread;
	std::mutex _devicesToPairMutex;
	std::unordered_map<int32_t, std::string> _devicesToPair;
	std::mutex _pairMutex;
	DescriptionCreator _descriptionCreator;

    std::atomic_bool _stopWorkerThread;
    std::thread _workerThread;

	virtual void init();
    virtual void worker();
	virtual void loadPeers();
	virtual void savePeers(bool full);
	virtual void loadVariables() {}
	virtual void saveVariables() {}
	std::shared_ptr<MyPeer> createPeer(uint32_t deviceType, int32_t address, std::string serialNumber, bool save = true);
	void deletePeer(uint64_t id);

	void pairingModeTimer(int32_t duration, bool debugOutput = true);
	void pairDevice(PMyPacket packet, std::vector<uint8_t>& key);
};

}

#endif
