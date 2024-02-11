#pragma once

#include <deque>
#include <optional>
#include <set>
#include <string>
#include <string_view>
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
	std::vector<const Stop*> stops{};
};
struct BusInfo {
	size_t stops_on_route = 0;
	size_t unique_stops = 0;
	double route_length = .0;
};
class TransportCatalogue {
public:
	void AddStop(Stop new_stop);

	const Stop* FindStop(const std::string_view name) const;

	void AddBus(Bus new_bus);

	const Bus* FindBus(const std::string_view name) const;

	std::optional<BusInfo> GetBusInfo(std::string_view name) const;

	std::set<std::string_view> GetStopInfo(std::string_view name) const;

private:
	std::deque<Stop> stops_;
	std::unordered_map<std::string_view, Stop*> stopname_to_bus_;
	std::deque<Bus> buses_;
	std::unordered_map<std::string_view, Bus*> busname_to_stop_;
};

} //namespace transport_catalogue