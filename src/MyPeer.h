/* Copyright 2013-2017 Homegear UG (haftungsbeschränkt) */

#ifndef MYPEER_H_
#define MYPEER_H_

#include "PhysicalInterfaces/IMBusInterface.h"
#include "MyPacket.h"
#include <homegear-base/BaseLib.h>

using namespace BaseLib;
using namespace BaseLib::DeviceDescription;

namespace MyFamily
{
class MyCentral;

class MyPeer : public BaseLib::Systems::Peer, public BaseLib::Rpc::IWebserverEventSink
{
public:
	MyPeer(uint32_t parentID, IPeerEventSink* eventHandler);
	MyPeer(int32_t id, int32_t address, std::string serialNumber, uint32_t parentID, IPeerEventSink* eventHandler);
	virtual ~MyPeer();
	void init();
	void dispose();

	//{{{ Features
	virtual bool wireless() { return true; }
	//}}}

	//{{{ In table variables
	std::vector<uint8_t> getAesKey() { return _aesKey; }
	void setAesKey(std::vector<uint8_t>& value) { _aesKey = value; saveVariable(21, value); }
	int32_t getControlInformation() { return _controlInformation; }
	void setControlInformation(int32_t value) { _controlInformation = value; saveVariable(22, value); }
    int32_t getDataRecordCount() { return _dataRecordCount; }
    void setDataRecordCount(int32_t value) { _dataRecordCount = value; saveVariable(23, value); }
	//}}}

	bool expectsEncryption() { return !_aesKey.empty(); }

	virtual std::string handleCliCommand(std::string command);
	void packetReceived(PMyPacket& packet);

	virtual bool load(BaseLib::Systems::ICentral* central);
    virtual void savePeers() {}

	virtual int32_t getChannelGroupedWith(int32_t channel) { return -1; }
	virtual int32_t getNewFirmwareVersion() { return 0; }
	virtual std::string getFirmwareVersionString(int32_t firmwareVersion) { return "1.0"; }
    virtual bool firmwareUpdateAvailable() { return false; }

    std::string printConfig();

    /**
	 * {@inheritDoc}
	 */
    virtual void homegearStarted();

    /**
	 * {@inheritDoc}
	 */
    virtual void homegearShuttingDown();

	//RPC methods
	virtual PVariable putParamset(BaseLib::PRpcClientInfo clientInfo, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteID, int32_t remoteChannel, PVariable variables, bool onlyPushing = false);
	virtual PVariable setValue(BaseLib::PRpcClientInfo clientInfo, uint32_t channel, std::string valueKey, PVariable value, bool wait);
	//End RPC methods
protected:
	//In table variables:
	std::vector<uint8_t> _aesKey;
	int32_t _controlInformation = -1;
    int32_t _dataRecordCount = -1;
	//End

	bool _shuttingDown = false;

	PMyPacket _lastPacket;
	uint32_t _lastRssiDevice = 0;

	virtual void loadVariables(BaseLib::Systems::ICentral* central, std::shared_ptr<BaseLib::Database::DataTable>& rows);
    virtual void saveVariables();

    void setRssiDevice(uint8_t rssi);

	virtual std::shared_ptr<BaseLib::Systems::ICentral> getCentral();

	virtual PParameterGroup getParameterSet(int32_t channel, ParameterGroup::Type::Enum type);

	// {{{ Hooks
		/**
		 * {@inheritDoc}
		 */
		virtual bool getAllValuesHook2(PRpcClientInfo clientInfo, PParameter parameter, uint32_t channel, PVariable parameters);

		/**
		 * {@inheritDoc}
		 */
		virtual bool getParamsetHook2(PRpcClientInfo clientInfo, PParameter parameter, uint32_t channel, PVariable parameters);
	// }}}
};

typedef std::shared_ptr<MyPeer> PMyPeer;

}

#endif
