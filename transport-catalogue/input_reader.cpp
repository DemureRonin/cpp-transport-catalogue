#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
namespace transport_catalogue{
	namespace readers{
		geo::Coordinates ParseCoordinates(std::string_view str) {
			static const double nan = std::nan("");

			auto not_space = str.find_first_not_of(' ');
			auto comma = str.find(',');

			if (comma == str.npos) {
				return {nan, nan};
			}

			auto not_space2 = str.find_first_not_of(' ', comma + 1);

			double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
			double lng = std::stod(std::string(str.substr(not_space2)));

			return {lat, lng};
		}

		/**
		 * Удаляет пробелы в начале и конце строки
		 */
		std::string_view Trim(std::string_view string) {
			const auto start = string.find_first_not_of(' ');
			if (start == string.npos) {
				return {};
			}
			return string.substr(start, string.find_last_not_of(' ') + 1 - start);
		}

		/**
		 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
		 */
		std::vector<std::string_view> Split(std::string_view string, char delim) {
			std::vector<std::string_view> result;

			size_t pos = 0;
			while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
				auto delim_pos = string.find(delim, pos);
				if (delim_pos == string.npos) {
					delim_pos = string.size();
				}
				if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
					result.push_back(substr);
				}
				pos = delim_pos + 1;
			}

			return result;
		}

		/**
		 * Парсит маршрут.
		 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
		 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
		 */
		std::vector<std::string_view> ParseRoute(std::string_view route) {
			if (route.find('>') != route.npos) {
				return Split(route, '>');
			}

			auto stops = Split(route, '-');
			std::vector<std::string_view> results(stops.begin(), stops.end());
			results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

			return results;
		}

		CommandDescription ParseCommandDescription(std::string_view line) {
			auto colon_pos = line.find(':');
			if (colon_pos == line.npos) {
				return {};
			}

			auto space_pos = line.find(' ');
			if (space_pos >= colon_pos) {
				return {};
			}

			auto not_space = line.find_first_not_of(' ', space_pos);
			if (not_space >= colon_pos) {
				return {};
			}

			CommandDescription cmd;
			cmd.command = std::string(line.substr(0, space_pos));
			cmd.id = std::string(line.substr(not_space, colon_pos - not_space));

			std::string_view rest = line.substr(colon_pos + 1);
			if (cmd.command == "Stop") {
				auto comma1 = rest.find(',');
				if (comma1 == rest.npos) {
					return {};
				}
				auto comma2 = rest.find(',', comma1 + 1);

				if (comma2 == rest.npos) {
					cmd.description = std::string(rest);
					return cmd;
				}

				cmd.description = std::string(rest.substr(0, comma2));
				std::string_view dists = Trim(rest.substr(comma2 + 1));

				while (!dists.empty()) {
					auto m_pos = dists.find("m to ");
					if (m_pos == dists.npos) break;

					int dist = std::stoi(std::string(dists.substr(0, m_pos)));
					dists.remove_prefix(m_pos + 4);

					auto comma = dists.find(',');
					std::string stop_name;
					if (comma == dists.npos) {
						stop_name = std::string(Trim(dists));
						dists = {};
					}
					else {
						stop_name = std::string(Trim(dists.substr(0, comma)));
						dists.remove_prefix(comma + 1);
					}

					cmd.distances.emplace_back(std::move(stop_name), dist);
					dists = Trim(dists);
				}
			}
			else {
				cmd.description = std::string(rest);
			}

			return cmd;
		}


		void InputReader::ParseLine(std::string_view line) {
			if (auto command_description = ParseCommandDescription(line)) {
				commands_.push_back(std::move(command_description));
			}
		}

		void InputReader::ApplyCommands(transport_catalogue::TransportCatalogue& catalogue) const {
			std::vector<const CommandDescription*> bus_commands;
			std::vector<const CommandDescription*> stop_distance_commands;

			for (const auto& command : commands_) {
				if (command.command == "Stop") {
					catalogue.AddStop(command.id, ParseCoordinates(command.description));
					if (!command.distances.empty()) {
						stop_distance_commands.push_back(&command);
					}
				}
				else if (command.command == "Bus") {
					bus_commands.push_back(&command);
				}
			}

			for (const auto* command : stop_distance_commands) {
				for (const auto& [other_stop, dist] : command->distances) {
					catalogue.SetDistance(command->id, other_stop, dist);
				}
			}

			for (const auto* bus_command : bus_commands) {
				catalogue.AddBus(bus_command->id, ParseRoute(bus_command->description));
			}
		}
	}
}
