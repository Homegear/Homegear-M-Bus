/* Copyright 2013-2019 Homegear GmbH */

#ifndef MYPEER_H_
#define MYPEER_H_

#include "PhysicalInterfaces/IMbusInterface.h"
#include "MbusPacket.h"
#include "VifConverter.h"
#include <homegear-base/BaseLib.h>

using namespace BaseLib;
using namespace BaseLib::DeviceDescription;

namespace Mbus
{
class MbusCentral;

class MbusPeer : public BaseLib::Systems::Peer, public BaseLib::Rpc::IWebserverEventSink
{
public:
	MbusPeer(uint32_t parentID, IPeerEventSink* eventHandler);
	MbusPeer(int32_t id, int32_t address, std::string serialNumber, uint32_t parentID, IPeerEventSink* eventHandler);
	virtual ~MbusPeer();
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
	int32_t getFormatCrc() { return _formatCrc; }
	void setFormatCrc(int32_t value) { _formatCrc = value; saveVariable(24, (int32_t)value); }
	int32_t getEncryptionMode() { return _encryptionMode; }
	void setEncryptionMode(int32_t value) { _encryptionMode = value; saveVariable(25, (int32_t)value); }
    void setLastTime(int32_t value) { _lastTime = value; saveVariable(26, (int32_t)value); }
	//}}}

	bool expectsEncryption() { return !_aesKey.empty(); }

    void worker();
	virtual std::string handleCliCommand(std::string command);
	void packetReceived(PMbusPacket& packet);

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
	virtual PVariable putParamset(BaseLib::PRpcClientInfo clientInfo, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteID, int32_t remoteChannel, PVariable variables, bool checkAcls, bool onlyPushing = false);
	virtual PVariable setValue(BaseLib::PRpcClientInfo clientInfo, uint32_t channel, std::string valueKey, PVariable value, bool wait);
	//End RPC methods
protected:
	struct FrameValue
	{
		std::list<uint32_t> channels;
		std::vector<uint8_t> value;
	};

	struct FrameValues
	{
		std::string frameID;
		std::list<uint32_t> paramsetChannels;
		ParameterGroup::Type::Enum parameterSetType;
		std::map<std::string, FrameValue> values;
	};

	//In table variables:
	std::vector<uint8_t> _aesKey;
	int32_t _controlInformation = -1;
    int32_t _dataRecordCount = -1;
	uint16_t _formatCrc = 0;
	uint8_t _encryptionMode = 0;
    int32_t _lastTime = 0;
	//End

	bool _shuttingDown = false;

	uint32_t _lastRssiDevice = 0;

	void loadVariables(BaseLib::Systems::ICentral* central, std::shared_ptr<BaseLib::Database::DataTable>& rows) override;
    void saveVariables() override;

    void setRssiDevice(uint8_t rssi);

	std::shared_ptr<BaseLib::Systems::ICentral> getCentral() override;

	void getValuesFromPacket(PMbusPacket packet, std::vector<FrameValues>& frameValue);

	PParameterGroup getParameterSet(int32_t channel, ParameterGroup::Type::Enum type) override;

	// {{{ Hooks
		/**
		 * {@inheritDoc}
		 */
		bool getAllValuesHook2(PRpcClientInfo clientInfo, PParameter parameter, uint32_t channel, PVariable parameters) override;

		/**
		 * {@inheritDoc}
		 */
		bool getParamsetHook2(PRpcClientInfo clientInfo, PParameter parameter, uint32_t channel, PVariable parameters) override;

		/**
		 * {@inheritDoc}
		 */
		bool convertFromPacketHook(BaseLib::Systems::RpcConfigurationParameter& parameter, std::vector<uint8_t>& data, PVariable& result) override;
	// }}}
};

typedef std::shared_ptr<MbusPeer> PMyPeer;

}

#endif
