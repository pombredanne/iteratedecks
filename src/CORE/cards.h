// Warning: Strange charset:  windows-1251
// *****************************************
// EvaluateDecks
// Tyrant card game simulator
//
// My kongregate account:
// http://www.kongregate.com/accounts/NETRAT
// 
// Project pages:
// http://code.google.com/p/evaluatedecks
// http://www.kongregate.com/forums/65-tyrant/topics/195043-yet-another-battlesim-evaluate-decks
// *****************************************
//
// this module contains card database related classes - info, xml loader, card storage

#include <map>
#include <vector>
#include <string>

#include "compat.h"

#include "cards.hpp"

#include "achievementInfo.hpp"

#include "cardPool.hpp"

namespace IterateDecks { namespace Core {

// FIXME: After fixing class CardDB, move this to Logger.cpp
std::string Logger::abilityIdToString(AbilityId const & abilityId) const
{
    char const * cStrName = cDB->GetSkill(abilityId);
    return std::string(cStrName);
}

}}
