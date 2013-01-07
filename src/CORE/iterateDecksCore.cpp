#include "iterateDecksCore.hpp"
#include "activeDeck.hpp"
#include "assert.hpp"

#include <iostream>
#include <cstdlib>

namespace IterateDecks {
    namespace Core {

        SimulationTaskClass::SimulationTaskClass(SimulationTaskStruct const & task, CardDB const & cardDB)
        : minimalNumberOfGames(task.minimalNumberOfGames)
        , surge(task.surge)
        , delayFirstAttacker(task.delayFirstAttacker)
        , battleGround(task.battleGround)
        , achievementOptions(task.achievementOptions)
        , randomSeed(task.randomSeed)
        {
            this->attacker = DeckTemplate::create(task.attacker, cardDB);
            this->attacker->allowInvalid = task.allowInvalidDecks;
            this->defender = DeckTemplate::create(task.defender, cardDB);
            this->defender->allowInvalid = task.allowInvalidDecks;
        }

        SimulatorCore::~SimulatorCore() {}

        IterateDecksCore::IterateDecksCore()
        : logger(NULL)
        {
            this->cardDB.LoadAchievementXML("achievements.xml");
            this->cardDB.LoadCardXML("cards.xml");
            this->cardDB.LoadMissionXML("missions.xml");
            this->cardDB.LoadRaidXML("raids.xml");
            this->cardDB.LoadQuestXML("quests.xml");
        }

        Result IterateDecksCore::simulate(SimulationTaskStruct const & taskStruct)
        {
            // We first translate the simple struct-like simulationTask
            // into a more appropriate object like one
            SimulationTaskClass taskClass(taskStruct, cardDB);
            return this->simulate(taskClass);
        }

        Result IterateDecksCore::simulate(SimulationTaskClass const & task) {
            // Store results
            Result result;

            // Setup seed
            std::srand(task.randomSeed);
            
            unsigned int const n = task.minimalNumberOfGames;
            this->aborted = false;
            for(unsigned int i = 0; i < n; i++) {
                this->simulateOnce(task, result);

                // aborted?
                if (this->aborted) {
                    break;
                }
            }
            return result;
        }

        void IterateDecksCore::abort() {
            this->aborted = true;
        }

        void IterateDecksCore::simulateOnce(SimulationTaskClass const & task
                                           ,Result & result
                                           )
        {            
            // We play one game.
            // 1. What about the battleground ?
            BattleGroundEffect questEffect = task.battleGround;
            // If we have not specified one, we inherit the one from the
            // defense deck (which might be a quest)
            if(questEffect == BattleGroundEffect::normal) {
                questEffect = task.defender->getBattleGroundEffect();
            }
            // 2. We need the decks
            ActiveDeck attacker = task.attacker->instantiate();
            attacker.SetQuestEffect(questEffect);
            ActiveDeck defender = task.defender->instantiate();
            defender.SetQuestEffect(questEffect);

            // Surge or not?
            bool const surge = task.surge;
            bool defendersTurn = surge;
            

            unsigned int attAutoDamageToCommander = 0;
            unsigned int defAutoDamageToCommander = 0;
            unsigned int lastManualTurn = 1;
            unsigned int i;
            for(i = 1; i <= 50; i++) {

                // reset damage done and update last manual turn counter
                // P: I'm not sure whether I like this.
                //    This does not allow us to do "start on manual, then go auto" style
                if (attacker.Deck.size() >= 2) {
                    attAutoDamageToCommander += attacker.DamageToCommander;
                    defAutoDamageToCommander += defender.DamageToCommander;
                    attacker.DamageToCommander = 0;
                    defender.DamageToCommander = 0;
                    lastManualTurn = i;
                }
                
                // Attack
                LOG(logger,turnStart(i));
                if(defendersTurn) {
                    defender.AttackDeck(attacker);
                } else {
                    attacker.AttackDeck(defender);
                }
                LOG(logger,turnEnd(i));

                // One might be dead
                bool const attackerAlive = attacker.isAlive();
                bool const defenderAlive = defender.isAlive();

                // At least one should be still alive.
                // P: I can not think of many situations where you can actually lose in your turn.
                //    Only thing that comes to mind being backfire.
                assertX(attackerAlive || defenderAlive);
                bool const gameOver = !(attackerAlive && defenderAlive);

                if(gameOver) {
                    break;
                }

                // swap whose turn it is
                defendersTurn = !defendersTurn;
            } // for

            unsigned int const attManualDamageToCommander = attacker.DamageToCommander;
            unsigned int const defManualDamageToCommander = defender.DamageToCommander;
            attAutoDamageToCommander += attManualDamageToCommander;
            defAutoDamageToCommander += defManualDamageToCommander;
            bool const attackerAlive = attacker.isAlive();
            bool const defenderAlive = defender.isAlive();
            assertX(attackerAlive || defenderAlive);

            if(!attackerAlive) {
                // defender won
                result.gamesLost++;
                result.pointsDefender     += 10u;
                result.pointsDefenderAuto += 10u;
                // time bonus?
                if(i < lastManualTurn + 10) {
                    result.pointsDefender     += 5u;
                }
                if(i <= 10) {
                    result.pointsDefenderAuto += 5u;
                }
            } else if(!defenderAlive) {
                // attacker won
                //std::clog << "awarding points to attacker for winning" << std::endl;
                result.gamesWon++;
                result.pointsAttacker     += surge ? 30u : 10u;
                result.pointsAttackerAuto += surge ? 30u : 10u;
                // time bonus?                
                if(i < lastManualTurn + 10) {
                    //std::clog << "awarding points to attacker for time bonus in manual" << std::endl;
                    result.pointsAttacker     += 5u;
                }
                if(i <= 10) {
                    //std::clog << "awarding points to attacker for time bonus in auto" << std::endl;
                    result.pointsAttackerAuto += 5u;
                }                
            } else {
                // both alive
                result.gamesStalled++;
                result.pointsDefender     += 10u;
                result.pointsDefenderAuto += 10u;
                // certainly no time bonus on stalls
            }

            // damage
            //std::clog << "damage for attacker. "
            //          << "manual=" << attManualDamageToCommander << " "
            //          << "auto=" << attAutoDamageToCommander << std::endl;
            result.pointsAttacker     += std::min(attManualDamageToCommander, 10u);
            result.pointsAttackerAuto += std::min(attAutoDamageToCommander  , 10u);
            result.pointsDefender     += std::min(defManualDamageToCommander, 10u);
            result.pointsDefenderAuto += std::min(defAutoDamageToCommander  , 10u);

            // also increase number of games
            result.numberOfGames++;
        }
                                            

    }
}
