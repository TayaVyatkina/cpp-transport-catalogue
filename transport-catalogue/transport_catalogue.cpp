#include <algorithm>
#include <numeric>
#include <tuple>

#include "transport_catalogue.h"

namespace transport_catalogue {

void TransportCatalogue::AddStop(const Stop& new_stop) {
	stops_.emplace_back(new_stop);
	stopname_to_bus_[stops_.back().name] = &stops_.back();
}

Stop* TransportCatalogue::FindStop(const std::string_view name) const {
	if (stopname_to_bus_.count(name)) {
		return stopname_to_bus_.at(name);
	}
	return {};
}

void TransportCatalogue::AddBus(const Bus& new_bus) {
	buses_.emplace_back(new_bus);
	busname_to_stop_[buses_.back().name] = &buses_.back();
}

Bus* TransportCatalogue::FindBus(const std::string_view name) const {
	if (busname_to_stop_.count(name)) {
		return busname_to_stop_.at(name);
	}
	return {};
}

std::tuple<size_t, size_t, double> TransportCatalogue::GetBusInfo(const std::string_view name) const {

	if (busname_to_stop_.count(name)) {
		Bus bus = *busname_to_stop_.at(name);
		size_t stops_on_route = bus.stops.size();
		size_t unique_stops = std::set(bus.stops.begin(), bus.stops.end()).size();
		double route_length = .0;
		Coordinates last_stop = bus.stops[0]->coordinates;
		for (const auto& i : bus.stops) {
			route_length += ComputeDistance(last_stop, FindStop(i->name)->coordinates);
			last_stop = FindStop(i->name)->coordinates;
		}
		return { std::move(stops_on_route), std::move(unique_stops), std::move(route_length) };
	}
	return {};
}

std::set<std::string_view> TransportCatalogue::GetStopInfo(const std::string_view name) const {
	std::set<std::string_view> buses;
	if (stopname_to_bus_.count(name)) {
		for (const auto& i : busname_to_stop_) {
			if (std::find_if(i.second->stops.begin(), i.second->stops.end(),
				[&name](Stop* stop) {
					return stop->name == name;
				}) != i.second->stops.end()) {
				buses.insert(i.first);
			}
		}
	}
	return buses;
}

}// namespace transport_catalogue
