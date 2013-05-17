#ifndef ITERATEDECKS_CLI3_COMMANDS_HPP
    #define ITERATEDECKS_CLI3_COMMANDS_HPP

    #include <memory>
    #include <boost/program_options.hpp>

    namespace IterateDecks {
        namespace CLI3 {

            class Command {
                public:
                    typedef std::shared_ptr<Command> Ptr;
                public:
                    virtual ~Command();
                    virtual int execute() = 0;
            };

            class VersionCommand : public Command {
                public:
                    int execute();
            };

            class HelpCommand : public Command {
                private:
                    boost::program_options::options_description desc;
                public:
                    HelpCommand(boost::program_options::options_description const & desc);
                    int execute();
            };

        }
    }
#endif