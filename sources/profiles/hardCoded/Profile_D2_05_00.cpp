#include "stdafx.h"
#include "Profile_D2_05_00.h"
#include "Profile_D2_05_Common.h"

CProfile_D2_05_00::CProfile_D2_05_00(const std::string& deviceId,
                                     boost::shared_ptr<yApi::IYPluginApi> api)
   : m_deviceId(deviceId),
     m_dimmable(boost::make_shared<yApi::historization::CSwitch>("Channel", yApi::EKeywordAccessMode::kGetSet)),
     m_historizers({m_channel})
{
}

CProfile_D2_05_00::~CProfile_D2_05_00()
{
}

const std::string& CProfile_D2_05_00::profile() const
{
   static const std::string profile("D2-01-00");
   return profile;
}

const std::string& CProfile_D2_05_00::title() const
{
   static const std::string title("Electronic switch with energy measurement and local control");
   return title;
}

std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>> CProfile_D2_05_00::allHistorizers() const
{
   return m_historizers;
}

void CProfile_D2_05_00::readInitialState(const std::string& senderId,
                                         boost::shared_ptr<IMessageHandler> messageHandler) const
{
   // Need to wait a bit between outgoing messages, to be sure to receive answer
   boost::this_thread::sleep(boost::posix_time::milliseconds(500));
   CProfile_D2_05_Common::sendActuatorStatusQuery(messageHandler,
                                                  senderId,
                                                  m_deviceId,
                                                  CProfile_D2_05_Common::kAllOutputChannels);
}

std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>> CProfile_D2_05_00::states(unsigned char rorg,
                                                                                                   const boost::dynamic_bitset<>& data,
                                                                                                   const boost::dynamic_bitset<>& status,
                                                                                                   const std::string& senderId,
                                                                                                   boost::shared_ptr<IMessageHandler> messageHandler) const
{
   return CProfile_D2_05_Common::extractActuatorStatusResponse(rorg,
                                                               data,
                                                               m_channel,
                                                               CProfile_D2_05_Common::noDimmable,
                                                               CProfile_D2_05_Common::noPowerFailure,
                                                               CProfile_D2_05_Common::noOverCurrent);
}

void CProfile_D2_05_00::sendCommand(const std::string& keyword,
                                    const std::string& commandBody,
                                    const std::string& senderId,
                                    boost::shared_ptr<IMessageHandler> messageHandler) const
{
   if (keyword != m_channel->getKeyword())
      return;

   m_channel->setCommand(commandBody);

   CProfile_D2_05_Common::sendActuatorSetOutputCommandSwitching(messageHandler,
                                                                senderId,
                                                                m_deviceId,
                                                                CProfile_D2_05_Common::kAllOutputChannels,
                                                                m_channel->get());
}

void CProfile_D2_05_00::sendConfiguration(const shared::CDataContainer& deviceConfiguration,
                                          const std::string& senderId,
                                          boost::shared_ptr<IMessageHandler> messageHandler) const
{
   // Device supports no configuration
}
