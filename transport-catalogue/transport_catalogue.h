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
	double route_length = 0.;
	double curvature = .0;
};

namespace detail {
	class DistanceHasher {
	public:
		size_t operator()(std::pair<const Stop*, const Stop*> stops) const {
			return static_cast<size_t>(hasher_(stops.first)) * 7 + static_cast<size_t>(hasher_(stops.second)) * 13;
		}
	private:
		std::hash<const void*> hasher_;
	};
}// namespace detail

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
	std::deque<Stop> stops_;
	std::unordered_map<std::string_view, Stop*> stopname_to_bus_;
	std::deque<Bus> buses_;
	std::unordered_map<std::string_view, Bus*> busname_to_stop_;
	std::unordered_map<std::pair<const Stop*, const Stop*>, size_t, detail::DistanceHasher> distance_between_stops_;
};

} //namespace transport_catalogue