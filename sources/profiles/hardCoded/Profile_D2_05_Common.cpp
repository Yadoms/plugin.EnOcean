#include "stdafx.h"
#include "Profile_D2_05_Common.h"
#include "../../message/ResponseReceivedMessage.h"
#include "../bitsetHelpers.hpp"
#include "../../message/RadioErp1SendMessage.h"
#include <shared/Log.h>


void CProfile_D2_05_Common::sendkGoToPositionAndAngle(boost::shared_ptr<IMessageHandler> messageHandler,
                                                      const std::string& senderId,
                                                      const std::string& targetId,
                                                      boost::optional<unsigned int> verticalPosition,
                                                      boost::optional<unsigned int> rotationAngle,
                                                      specificHistorizers::EBlindLockingMode lockingMode)
{
   boost::dynamic_bitset<> userData(4 * 8);
   bitset_insert(userData, 1, 7, verticalPosition ? verticalPosition.value() : 127);
   bitset_insert(userData, 9, 7, rotationAngle ? rotationAngle.value() : 127);
   bitset_insert(userData, 17, 3, 0);
   bitset_insert(userData, 21, 3, lockingMode);
   bitset_insert(userData, 24, 4, 0);
   bitset_insert(userData, 28, 4, kGoToPositionAndAngle);

   sendMessage(messageHandler,
               senderId,
               targetId,
               userData,
               "Go to Position and Angle");
}

void CProfile_D2_05_Common::sendStop(boost::shared_ptr<IMessageHandler> messageHandler,
                                     const std::string& senderId,
                                     const std::string& targetId)
{
   boost::dynamic_bitset<> userData(1 * 8);
   bitset_insert(userData, 0, 4, 0);
   bitset_insert(userData, 4, 4, kStop);

   sendMessage(messageHandler,
               senderId,
               targetId,
               userData,
               "Stop");
}

void CProfile_D2_05_Common::sendQueryPositionAndAngle(boost::shared_ptr<IMessageHandler> messageHandler,
                                                      const std::string& senderId,
                                                      const std::string& targetId)
{
   boost::dynamic_bitset<> userData(1 * 8);
   bitset_insert(userData, 0, 4, 0);
   bitset_insert(userData, 4, 4, kQueryPositionAndAngle);

   sendMessage(messageHandler,
               senderId,
               targetId,
               userData,
               "Query Position and Angle");
}


const boost::shared_ptr<yApi::historization::CDimmable> CProfile_D2_05_Common::noVerticalPosition = boost::shared_ptr<yApi::historization::CDimmable>();
const boost::shared_ptr<yApi::historization::CDimmable> CProfile_D2_05_Common::noRotationAngle = boost::shared_ptr<yApi::historization::CDimmable>();

std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>> CProfile_D2_05_Common::extractReplyPositionAndAngleResponse(unsigned char rorg,
                                                                                                                                     const boost::dynamic_bitset<>& data,
                                                                                                                                     boost::shared_ptr<yApi::historization::CDimmable> verticalPosition,
                                                                                                                                     boost::shared_ptr<yApi::historization::CDimmable> rotationAngle,
                                                                                                                                     boost::shared_ptr<specificHistorizers::CBlindLockingMode> lockingMode)
{
   // Some devices supports several RORG telegrams, ignore non-VLD telegrams
   if (rorg != CRorgs::ERorgIds::kVLD_Telegram)
      return std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>>();

   if (bitset_extract(data, 28, 4) != kReplyPositionAndAngle)
      return std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>>();

   if (bitset_extract(data, 24, 4) != 0)
      return std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>>();

   // Return only the concerned historizer
   std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>> historizers;

   int position = bitset_extract(data, 1, 7);
   int angle = bitset_extract(data, 9, 7);
   auto locking = static_cast<specificHistorizers::EBlindLockingMode>(bitset_extract(data, 21, 3));

   if (position != 127)
   {
      verticalPosition->set(position);
      historizers.push_back(verticalPosition);
   }

   if (angle != 127)
   {
      rotationAngle->set(angle);
      historizers.push_back(rotationAngle);
   }

   switch (locking)
   {
   case specificHistorizers::EBlindLockingMode::kDoNotChangeOrNoLockValue:
   case specificHistorizers::EBlindLockingMode::kBlockageValue:
   case specificHistorizers::EBlindLockingMode::kAlarmValue:
      lockingMode->set(locking);
      historizers.push_back(lockingMode);
      break;
   default:
      YADOMS_LOG(warning) << "ReplyPositionAndAngleResponse : received unsupported locking mode value " << locking;
      break;
   }

   return historizers;
}

void CProfile_D2_05_Common::sendSetParameters(boost::shared_ptr<IMessageHandler> messageHandler,
                                              const std::string& senderId,
                                              const std::string& targetId,
                                              unsigned int measuredDurationOfVerticalRunMs,
                                              unsigned int measuredDurationOfRotationMs,
                                              EAlarmAction alarmAction)
{
   message::CRadioErp1SendMessage command(CRorgs::kVLD_Telegram,
                                          senderId,
                                          targetId,
                                          0);
   boost::dynamic_bitset<> data(5 * 8);

   if (measuredDurationOfVerticalRunMs < 5000 || measuredDurationOfVerticalRunMs > 300000)
      measuredDurationOfVerticalRunMs = 32767;
   else
      measuredDurationOfVerticalRunMs /= 10;

   if (measuredDurationOfRotationMs != 0)
   {
      if (measuredDurationOfRotationMs < 10 || measuredDurationOfRotationMs > 2540)
         measuredDurationOfRotationMs = 255;
      else
         measuredDurationOfRotationMs /= 10;
   }

   bitset_insert(data, 1, 15, measuredDurationOfVerticalRunMs);
   bitset_insert(data, 16, 8, measuredDurationOfRotationMs);
   bitset_insert(data, 29, 3, alarmAction);
   bitset_insert(data, 32, 4, 0);
   bitset_insert(data, 36, 4, kSetParameters);

   sendMessage(messageHandler,
               senderId,
               targetId,
               data,
               "Set parameters");
}


void CProfile_D2_05_Common::sendMessage(boost::shared_ptr<IMessageHandler> messageHandler,
                                        const std::string& senderId,
                                        const std::string& targetId,
                                        const boost::dynamic_bitset<>& userData,
                                        const std::string& commandName)
{
   message::CRadioErp1SendMessage command(CRorgs::kVLD_Telegram,
                                          senderId,
                                          targetId,
                                          0);

   command.userData(bitset_to_bytes(userData));

   boost::shared_ptr<const message::CEsp3ReceivedPacket> answer;
   if (!messageHandler->send(command,
                             [](boost::shared_ptr<const message::CEsp3ReceivedPacket> esp3Packet)
                          {
                             return esp3Packet->header().packetType() == message::RESPONSE;
                          },
                             [&](boost::shared_ptr<const message::CEsp3ReceivedPacket> esp3Packet)
                          {
                             answer = esp3Packet;
                          }))
      throw std::runtime_error((boost::format("Fail to send message to %1% : no answer to \"%2%\"") % targetId % commandName).str());

   auto response = boost::make_shared<message::CResponseReceivedMessage>(answer);

   if (response->returnCode() != message::CResponseReceivedMessage::RET_OK)
   YADOMS_LOG(error) << "Fail to send message to " << targetId << " : \"" << commandName << "\" returns " << response->returnCode();
}
