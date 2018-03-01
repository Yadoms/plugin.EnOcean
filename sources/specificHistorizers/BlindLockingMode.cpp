#include "stdafx.h"
#include "BlindLockingMode.h"
#include <shared/plugin/yPluginApi/StandardUnits.h>

namespace specificHistorizers
{
   DECLARE_ENUM_IMPLEMENTATION(EBlindLockingMode,
      ((DoNotChangeOrNoLock))
      ((Blockage))
      ((Alarm))
      ((Deblockage))
   );


   CBlindLockingMode::CBlindLockingMode(const std::string& keywordName)
      : CSingleHistorizableData<EBlindLockingMode>(keywordName,
                                                   shared::plugin::yPluginApi::CStandardCapacity("BlindLockingMode",
                                                                                                 shared::plugin::yPluginApi::CStandardUnits::NoUnit(),
                                                                                                 shared::plugin::yPluginApi::EKeywordDataType::kEnum),
                                                   shared::plugin::yPluginApi::EKeywordAccessMode::kGetSet)
   {
   }

   CBlindLockingMode::~CBlindLockingMode()
   {
   }
} // namespace specificHistorizers
