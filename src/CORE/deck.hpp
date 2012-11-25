#ifndef DECK_HPP_2
    #define DECK_HPP_2

#endif

#ifndef DECK_HPP_1
    #define DECK_HPP_1

    #include <vector>
    #include <list>
    #include <set>
    #include <string>

    #include "simpleTypes.hpp"

    #include "logger.forward.hpp"

    #include "constants.hpp"

    #include "playedCard.forward.hpp"

    namespace IterateDecks { namespace Core {

    char const * const FACTIONS[6] = {0,"Imperial","Raider","Bloodthirsty","Xeno","Righteous"};

    class ActiveDeck;
    class PlayedCard;
    class LOG_RECORD;
    class LOG_CARD;
    class CardDB;

    struct REQUIREMENT {
        UCHAR SkillID;
        UCHAR Procs;
        REQUIREMENT() { SkillID = 0; };
    };

    }}


    #include "playedCard.hpp"

#endif

