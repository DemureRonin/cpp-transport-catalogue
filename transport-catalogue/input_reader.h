#pragma once
#include <string>
#include <string_view>
#include <vector>
#include "transport_catalogue.h"

namespace transport_catalogue{
    namespace readers{
        struct CommandDescription {
            explicit operator bool() const {
                return !command.empty();
            }
            bool operator!() const {
                return !operator bool();
            }

            std::string command;      
            std::string id;          
            std::string description; 
            std::vector<std::pair<std::string, int>> distances; 
        };

        class InputReader {
        public:
            /**
             * Парсит строку в структуру CommandDescription и сохраняет результат в commands_
             */
            void ParseLine(std::string_view line);

            /**
             * Наполняет данными транспортный справочник, используя команды из commands_
             */
            void ApplyCommands(transport_catalogue::TransportCatalogue& catalogue) const;

        private:
            std::vector<CommandDescription> commands_;
        };
    }
}