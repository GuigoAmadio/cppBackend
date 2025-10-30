#pragma once

#include <string>
#include "../repositories/OfferRepository.hpp"

namespace Domains::Offers::UseCases {

using Domains::Offers::Repositories::OfferRepository;

class ShortlistOfferUseCase {
public:
    explicit ShortlistOfferUseCase(OfferRepository* repo) : repo_(repo) {}

    void execute(const std::string& offerId, bool shortlisted) {
        repo_->setShortlist(offerId, shortlisted);
    }

private:
    OfferRepository* repo_;
};

} // namespace Domains::Offers::UseCases


