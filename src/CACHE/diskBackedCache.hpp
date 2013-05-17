#ifndef ITERATEDECKS_CACHE_DISKBACKEDCACHE_HPP
    #define ITERATEDECKS_CACHE_DISKBACKEDCACHE_HPP

    #include "simulatorCache.hpp"
    #include "../CORE/iterateDecksCore.hpp"
    #include "sqliteWrapper.hpp"

    namespace IterateDecks {
        namespace Cache {    

            class DiskBackedCache : public SimulatorCache {
                private:
                    bool ignoreCoreVersion;
                    bool ignoreXMLVersion;
                    SQLiteWrapper database;
                    
                    PreparedStatement * insertStatement;
                    PreparedStatement * selectStatement;
                    
                
                    Result loadCache(SimulationTaskClass const & task);
                    void addToCache(SimulationTaskClass const & task, Result const & result);
                
                public:
                    DiskBackedCache(SimulatorCore::Ptr & delegate);
                    virtual ~DiskBackedCache();

                    virtual Result simulate(SimulationTaskClass const &);
                    virtual Result simulate(SimulationTaskClass const &, unsigned long numberOfNewSamples);
                    virtual void abort();                    
            };
        }
    }

#endif
