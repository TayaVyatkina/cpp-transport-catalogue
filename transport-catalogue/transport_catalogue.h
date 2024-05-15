#pragma once

#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>

#include "domain.h"

namespace transport_catalogue {

class TransportCatalogue {
public:
	void AddDistanceBetweenStops(const std::string_view stop1, const std::string_view stop2, size_t dist);

	size_t GetDistanceBetweenStops(const information_base::Stop* stop1, const information_base::Stop* stop2) const;

	void AddStop(information_base::Stop new_stop);

	const information_base::Stop* FindStop(const std::string_view name) const;

	void AddBus(information_base::Bus new_bus);

	const information_base::Bus* FindBus(const std::string_view name) const;

	std::optional<information_base::BusInfo> GetBusInfo(std::string_view name) const;

	std::set<std::string_view> GetStopInfo(std::string_view name) const;

	std::vector<information_base::Stop> GetStops() const{
		return { stops_.begin(), stops_.end()};
	}
	std::vector<information_base::Bus> GetBuses() const {
		return { buses_.begin(), buses_.end() };
	}
private:
	information_base::Stops stops_;
	information_base::StopsForBus stopname_to_bus_;
	information_base::Buses buses_;
	information_base::BusesForStop busname_to_stop_;
	information_base::DistanceBetweenStops distance_between_stops_;
};

} //namespace transport_catalogue