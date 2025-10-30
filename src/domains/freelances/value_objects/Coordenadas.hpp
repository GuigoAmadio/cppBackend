#pragma once

#include <stdexcept>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Domains::Freelances::ValueObjects {

/**
 * @brief Value Object para coordenadas geográficas
 */
class Coordenadas {
public:
    Coordenadas() : latitude_(0.0), longitude_(0.0) {}
    
    Coordenadas(double latitude, double longitude) 
        : latitude_(latitude), longitude_(longitude) {
        validate();
    }

    double getLatitude() const { return latitude_; }
    double getLongitude() const { return longitude_; }

    /**
     * @brief Calcula distância em KM entre duas coordenadas (fórmula de Haversine)
     */
    double distanciaEmKm(const Coordenadas& other) const {
        const double R = 6371.0; // Raio da Terra em km
        
        double lat1Rad = latitude_ * M_PI / 180.0;
        double lat2Rad = other.latitude_ * M_PI / 180.0;
        double deltaLat = (other.latitude_ - latitude_) * M_PI / 180.0;
        double deltaLon = (other.longitude_ - longitude_) * M_PI / 180.0;
        
        double a = std::sin(deltaLat / 2) * std::sin(deltaLat / 2) +
                   std::cos(lat1Rad) * std::cos(lat2Rad) *
                   std::sin(deltaLon / 2) * std::sin(deltaLon / 2);
        
        double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
        
        return R * c;
    }

    bool operator==(const Coordenadas& other) const {
        return std::abs(latitude_ - other.latitude_) < 0.0001 &&
               std::abs(longitude_ - other.longitude_) < 0.0001;
    }

private:
    double latitude_;
    double longitude_;

    void validate() const {
        if (latitude_ < -90.0 || latitude_ > 90.0) {
            throw std::invalid_argument("Latitude deve estar entre -90 e 90");
        }
        if (longitude_ < -180.0 || longitude_ > 180.0) {
            throw std::invalid_argument("Longitude deve estar entre -180 e 180");
        }
    }
};

} // namespace Domains::Freelances::ValueObjects

