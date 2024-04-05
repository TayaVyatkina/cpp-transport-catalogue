#pragma once

#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>

#include "domain.h"

namespace transport_catalogue {
using namespace information_base;

class TransportCatalogue {
public:
	void AddDistanceBetweenStops(const std::string_view stop1, const std::string_view stop2, size_t dist);

	size_t GetDistanceBetweenStops(const Stop* stop1, const Stop* stop2) const;

	void AddStop(Stop new_stop);

	const Stop* FindStop(const std::string_view name) const;

	void AddBus(Bus new_bus);

	const Bus* FindBus(const std::string_view name) const;

	std::optional<BusInfo> GetBusInfo(std::string_view name) const;

	std::set<std::string_view> GetStopInfo(std::string_view name) const;

private:
	Stops stops_;
	StopsForBus stopname_to_bus_;
	Buses buses_;
	BusesForStop busname_to_stop_;
	DistanceBetweenStops distance_between_stops_;
};

} //namespace transport_catalogue