#ifndef ITERATEDECKS_OPT_OPTIMIZER_HPP
    #define ITERATEDECKS_OPT_OPTIMIZER_HPP

    #include <memory>
    #include <set>
    #include "../CORE/deckTemplate.hpp"
    #include "../CORE/iterateDecksCore.hpp"

    namespace IterateDecks {
        namespace Opt {

            class Optimizer {
                public:
                    typedef std::shared_ptr<Optimizer> Ptr;
                    
                public:
                    virtual IterateDecks::Core::DeckTemplate::Ptr optimizeMany(IterateDecks::Core::SimulationTaskClass const & task, bool attacker = true) = 0;
                    virtual void abort() = 0;
                    virtual ~Optimizer() {};
            };

        }
    }

#endif
