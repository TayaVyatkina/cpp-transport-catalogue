#pragma once
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

#include "geo.h"

namespace information_base {
	
	struct Stop {
		std::string name;
		geo::Coordinates coordinates{ .0, .0 };

		bool operator==(const Stop& other) const;
		bool operator<(const Stop& other) const;
	};

	struct Bus {
		std::string name;
		std::vector<const Stop*> stops{};

		bool operator==(const Bus& other) const;
		bool operator<(const Bus& other) const;
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
		size_t operator()(std::pair<const Stop*, const Stop*> stops) const;
	private:
		std::hash<const void*> hasher_;
	};

	class StopsHasher {
	public:
		size_t operator()(const Stop& stop) const;
	};

	class BusesHasher {
	public:
		bool operator()(const Bus& lhs, const Bus& rhs) const;
	};

	class PairsHasher {
	public:
		bool operator()(const std::pair<information_base::Bus, bool >& lhs, const std::pair<information_base::Bus, bool>& rhs) const;
	};

	} // namespace detail

	using Stops = std::deque<Stop>;
	using StopsForBus = std::unordered_map<std::string_view, Stop*>;
	using Buses = std::deque<Bus>;
	using BusesForStop = std::unordered_map<std::string_view, Bus*>;
	using DistanceBetweenStops = std::unordered_map<std::pair<const Stop*, const Stop*>, size_t, detail::DistanceHasher>;

}// namespace information_base 