#pragma once

#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include <unordered_map>

#include "geo.h"

namespace transport_catalogue {

struct Stop {
	std::string name;
	Coordinates coordinates{ .0, .0 };
};

struct Bus {
	std::string name;
	std::vector<Stop*> stops{};
};

class TransportCatalogue {
public:
	void AddStop(const Stop& new_stop);

	Stop* FindStop(const std::string_view name) const;

	void AddBus(const Bus& new_bus);

	Bus* FindBus(const std::string_view name) const;

	std::tuple<size_t, size_t, double> GetBusInfo(const std::string_view name) const;

	std::set<std::string_view> GetStopInfo(const std::string_view name) const;

private:
	std::deque<Stop> stops_;
	std::unordered_map<std::string_view, Stop*> stopname_to_bus_;
	std::deque<Bus> buses_;
	std::unordered_map<std::string_view, Bus*> busname_to_stop_;
};

} //namespace transport_catalogue