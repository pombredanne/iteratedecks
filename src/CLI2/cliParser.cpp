/**
 * Copyright 2012 Fabian "Praetorian" Kürten.
 *
 * This file is part of IterateDecks.
 *
 *  IterateDecks is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, only version 3 of the License.
 *
 *  IterateDecks is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with IterateDecks.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  In addition this file can be redistributed and/or modified under
 *  version 3 of the GNU General Public License or any later version
 *  of the License. (This does not extend to other files unless noted.)
 */

#include "cliParser.hpp"
#include "../CORE/exceptions.hpp"

#ifdef _WIN32
    #define __windows__
#endif
#ifdef _WIN64
    #define __windows__
#endif

#if defined(__windows__) && !defined(__MINGW32__)
    #include "../../EvaluateDecks/getopt_mb_uni_vc10/getopt_mb_uni_vc10_dll/getopt.h"
#else
    #include <getopt.h>
#endif

#include <sstream>
#include <ctime>

namespace IterateDecks {
    namespace CLI {

        CliOptions parseCliOptions(int const & argc, char * const argv[]) throw (std::invalid_argument, std::logic_error)
        {
            // get option table
            option long_options[numberOfOptions];
            std::string shortOptions;
            for(unsigned int i = 0; i < numberOfOptions; i++) {
                long_options[i] = options[i].getOptPart;
                if (    long_options[i].flag == NULL
                     && long_options[i].val != 0
                     && long_options[i].val < 256
                   ){
                    shortOptions += long_options[i].val;
                    if(long_options[i].has_arg == required_argument) {
                        shortOptions += ":";
                    } else if (long_options[i].has_arg == optional_argument) {
                        shortOptions += "::";
                    }
                }
            }
            char const * const short_options(shortOptions.c_str());

            CliOptions options;

            if (argc == 1) {
                options.printHelpAndExit = true;
            }

            // gnu getopt stuff
            while(true) {
                int option_index = 0;
                int c = getopt_long(argc, argv, short_options, long_options, &option_index);
                if(c == -1) {
                    break;
                }

                switch(c) {
                    case 'n': {
                            std::stringstream ssNumberOfIterations(optarg);
                            ssNumberOfIterations >> options.numberOfIterations;
                            if(ssNumberOfIterations.fail()) {
                                throw InvalidUserInputError("-n --number-of-iterations requires an integer argument");
                            }
                        } break;
                    case 'o': {
                            if (options.attackDeck.getType() == DeckArgument::HASH) {
                                options.attackDeck.setOrdered(true);
                            } else {
                                throw InvalidUserInputError("ordered deck only makes sense for hash decks");
                            }
                        } break;
                    case 'a': {
                            std::stringstream ssAchievementId(optarg);
                            int achievementId;
                            ssAchievementId >> achievementId;
                            if(ssAchievementId.fail()) {
                                throw InvalidUserInputError ("-a --achievement-id requires an integer argument");
                            }
                            if (achievementId >= 0) {
                                options.achievementOptions.enableCheck(achievementId);
                            } else {
                                options.achievementOptions.disableCheck();
                            }
                        } break;
                    case 'v': {
                            options.verbosity++;
                        } break;
                    case 'h': {
                            options.printHelpAndExit = true;
                        } break;
                    case 's': {
                            options.surge = true;
                        } break;
                    case 'Q': {
                            std::stringstream ssQuestId(optarg);
                            int questId;
                            ssQuestId>> questId;
                            if(ssQuestId.fail()) {
                                throw InvalidUserInputError ("-Q --quest-id requires an integer argument");
                            }
                            options.defenseDeck.setQuest(questId);
                        } break;
                    case 'r': {
                            std::stringstream ssRaidId(optarg);
                            int raidId;
                            ssRaidId>> raidId;
                            if(ssRaidId.fail()) {
                                throw InvalidUserInputError ("-r --raid-id requires an integer argument");
                            }
                            options.defenseDeck.setRaid(raidId);
                        } break;
                    case 'm': {
                            std::stringstream ssMissionId(optarg);
                            int missionId;
                            ssMissionId>> missionId;
                            if(ssMissionId.fail()) {
                                throw InvalidUserInputError ("-m --mission-id requires an integer argument");
                            }
                            options.defenseDeck.setMission(missionId);
                        } break;
                    case 'b': {
                            std::stringstream ssBattleGroundEffectId(optarg);
                            int battleGroundEffectId;
                            ssBattleGroundEffectId >> battleGroundEffectId;
                            if(ssBattleGroundEffectId.fail()) {
                                throw std::invalid_argument ("-b --battleground-id requires an integer argument");
                            }
                            BattleGroundEffect battleGroundEffect = static_cast<BattleGroundEffect>(battleGroundEffectId);
                            options.battleGroundEffect = battleGroundEffect;
                        } break;
                    case VERIFY: {
                            //std::clog << "verify" << std::endl;
                            options.verifyOptions = VerifyOptions(optarg);
                        } break;
                     case SEED: {
                            if (optarg == NULL) {
                                // no data, random seed
                                options.seed = static_cast<unsigned int>(time(NULL));
                            } else {
                                std::stringstream ssSeed(optarg);
                                ssSeed >> options.seed;
                                if (ssSeed.fail()) {
                                    throw InvalidUserInputError("--seed requires an positive integer argument");
                                }
                            }
                        } break;
                    case COLOR: {
                            // TODO better logic
                            options.colorMode = Logger::COLOR_ANSI;                            
                        } break;
                    case ALLOW_INVALID_DECKS: {
                            options.allowInvalidDecks = true;
                        } break;
                    case VERSION: {
                            options.printVersion = true;
                        } break;
                    case '?':
                        throw InvalidUserInputError("no such option");
                    case 0: {
                            std::stringstream message;
                            message << "0 default: " << (int)option_index;
                            throw InvalidUserInputError(message.str());
                        } break;
                    default: {
                            std::stringstream message;
                            message << "default: " << c;
                            throw InvalidUserInputError(message.str());
                        }
                }
            }

            if (options.printHelpAndExit) {
                // no more arguments expected
            } else if((options.defenseDeck.getType() == DeckArgument::RAID_ID
                        || options.defenseDeck.getType() == DeckArgument::QUEST_ID
                        || options.defenseDeck.getType() == DeckArgument::MISSION_ID
                      )
                      && optind+1 == argc) {
                // if we are raid deck, we only need the attack hash
                options.attackDeck.setHash(argv[optind+0]);
            } else if(optind+2 == argc) {
                // other arguments, we expect exactly two decks
                options.attackDeck.setHash(argv[optind+0]);
                options.defenseDeck.setHash(argv[optind+1]);
            } else if(options.printVersion) { // --version can no op safely
            } else {
                throw std::invalid_argument("please specify exactly two decks to test");
            }
            return options;
        }

    } // namespace CLI
} // namespace EvaluateDecks
