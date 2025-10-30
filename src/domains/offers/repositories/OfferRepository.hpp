#pragma once

#include <string>
#include <optional>
#include <vector>
#include "../entities/Offer.hpp"
#include "../../../core/database/ConnectionPool.hpp"

namespace Domains::Offers::Repositories {

using Domains::Offers::Entities::Offer;
using Core::Database::ConnectionPool;

struct OfferProfileProjection {
    std::string offerId;
    std::string freelancerId;
    std::string nome;
    double score;
    int numAvaliacoes;
    double distanciaKm;
    bool shortlisted;
    std::string createdAt;
};

class OfferRepository {
public:
    explicit OfferRepository(std::shared_ptr<ConnectionPool> pool) : pool_(std::move(pool)) {}

    void save(const Offer& offer);
    std::vector<OfferProfileProjection> listForLeilaoAsProfiles(const std::string& leilaoId);
    void setShortlist(const std::string& offerId, bool shortlisted);
    std::optional<Offer> findById(const std::string& id);
    void accept(const std::string& id);
    void reject(const std::string& id);
    std::string generateId();

private:
    std::shared_ptr<ConnectionPool> pool_;
};

} // namespace Domains::Offers::Repositories


