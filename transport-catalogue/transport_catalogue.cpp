#include <algorithm>
#include <numeric>
#include <tuple>

#include "transport_catalogue.h"


namespace transport_catalogue {
	using namespace information_base;
void TransportCatalogue::AddDistanceBetweenStops(const std::string_view stop1, const std::string_view stop2, size_t dist) {
	distance_between_stops_.insert({ std::pair{ FindStop(stop1), FindStop(stop2) }, dist });
}

size_t TransportCatalogue::GetDistanceBetweenStops(const Stop* stop1, const Stop* stop2) const {
	std::pair<const Stop*, const Stop*> stops{ stop1, stop2 };
	if (distance_between_stops_.count(stops)) {
		return distance_between_stops_.at(stops);
	}
	else if (distance_between_stops_.count(std::pair{stop2, stop1})) {
		return distance_between_stops_.at(std::pair{ stop2, stop1 });
	}
	return 0;

}

void TransportCatalogue::AddStop(Stop new_stop) {
	stops_.emplace_back(std::move(new_stop));
	stopname_to_bus_[stops_.back().name] = &stops_.back();
}

const Stop* TransportCatalogue::FindStop(const std::string_view name) const {
	if (stopname_to_bus_.count(name)) {
		return stopname_to_bus_.at(name);
	}
	return {};
}

void TransportCatalogue::AddBus(Bus new_bus) {
	buses_.emplace_back(std::move(new_bus));
	busname_to_stop_[buses_.back().name] = &buses_.back();
}

const Bus* TransportCatalogue::FindBus(const std::string_view name) const {
	if (busname_to_stop_.count(name)) {
		return busname_to_stop_.at(name);
	}
	return {};
}

std::optional<BusInfo> TransportCatalogue::GetBusInfo(std::string_view name) const {
	
	if (busname_to_stop_.count(name) && !(*busname_to_stop_.at(name)).stops.empty()) {
		Bus bus = *busname_to_stop_.at(name);
		size_t stops_on_route = bus.stops.size();
		size_t unique_stops = std::set(bus.stops.begin(), bus.stops.end()).size();
		
		double route_length = .0;// �������������� ����������
		double actual_length = .0;// ����������� ����������

		for (size_t i = 0; i + 1 != bus.stops.size(); ++i) {
			const Stop* current_stop = bus.stops[i];
			const Stop* next_stop = bus.stops[i + 1];
			route_length += geo::ComputeGeographicalDistance(current_stop->coordinates, next_stop->coordinates);
			actual_length += GetDistanceBetweenStops(current_stop, next_stop);
		}
		return BusInfo{ stops_on_route, unique_stops, actual_length, (actual_length * 1.) / route_length };
		
	}
	return {};
}

std::set<std::string_view> TransportCatalogue::GetStopInfo(std::string_view name) const {
	std::set<std::string_view> buses;
	if (stopname_to_bus_.count(name)) {
		for (const auto& i : busname_to_stop_) {
			if (std::find_if(i.second->stops.begin(), i.second->stops.end(),
				[&name](const Stop* stop) {
					return stop->name == name;
				}) != i.second->stops.end()) {
				buses.insert(i.first);
			}
		}
	}
	return buses;
}

}// namespace transport_catalogue
