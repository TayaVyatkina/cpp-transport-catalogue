#include "domain.h"

namespace information_base {

	bool Stop::operator==(const Stop& other) const {
		return this->name == other.name && this->coordinates.lat == other.coordinates.lat && this->coordinates.lng == other.coordinates.lng;
	}

	bool Stop::operator<(const Stop& other) const {
		return std::lexicographical_compare(this->name.begin(), this->name.end(),
			other.name.begin(), other.name.end());
	}

	bool Bus::operator==(const Bus& other) const {
		return this->name == other.name && this->stops == other.stops;
	}

	bool Bus::operator<(const Bus& other) const {
		return std::lexicographical_compare(this->name.begin(), this->name.end(),
			other.name.begin(), other.name.end());
	}

	namespace detail {

	size_t DistanceHasher::operator()(std::pair<const Stop*, const Stop*> stops) const {
		return static_cast<size_t>(hasher_(stops.first)) * 7 + static_cast<size_t>(hasher_(stops.second)) * 13;
	}

	size_t StopsHasher::operator()(const Stop& stop) const {
		return static_cast<size_t>(std::hash<std::string>{}(stop.name));
	}

	bool BusesHasher::operator()(const Bus& lhs, const Bus& rhs) const {
		return lhs < rhs;
	}

	bool PairsHasher::operator()(const std::pair<information_base::Bus, bool >& lhs, const std::pair<information_base::Bus, bool>& rhs) const {
		BusesHasher bh;
		return bh(lhs.first, rhs.first);
	}

	}// namespace detail

}// namespace information_base 