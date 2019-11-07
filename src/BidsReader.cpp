#include <istream>
#include <map>
#include <numeric>
#include <regex>
#include <set>

#include "BidsReader.hpp"

std::istream& operator>>(std::istream& in, BidsReader& bids)
{
    //  The list of the product for which each bidder made a bid, if the option
    // is not a value, the bidderer has been invalidated.
    std::map<std::string, std::optional<std::string>> bidder_product_offer;

    // The order of appearance of the next bidder.
    unsigned cur_order = 0;

    std::string cur_product;
    // Because the input is well formed, the first line must be a product name
    std::getline(in, cur_product);

    std::string temp;
    while (getline(in, temp))
    {
        if (BidsReader::is_product_name(temp))
        {
            cur_product = temp;
            bids.products_valid_bids[cur_product];
            bids.products_all_bids[cur_product];
        }
        else
        {
            std::istringstream temp_stream(temp);
            std::string bidder;
            int bid_amount;
            temp_stream >> bidder >> bid_amount;

            if (bids.appearance_order.find(bidder) ==
                bids.appearance_order.end())
            {
                bids.appearance_order[bidder] = cur_order;
                cur_order++;
            }

            bids.products_all_bids[cur_product].insert(bid_amount);

            bool bidder_already_seen = bidder_product_offer.count(bidder) != 0;
            if (bid_amount < 0 ||
                (bidder_already_seen &&
                 (!bidder_product_offer[bidder].has_value() ||
                  bidder_product_offer[bidder].value() != cur_product)))
            {
                if (bidder_product_offer[bidder].has_value())
                {
                    std::string bidder_last_offer_product = bidder_product_offer[bidder].value();
                    auto& bids_for_last_offer = bids.products_valid_bids[bidder_last_offer_product];
                    bids_for_last_offer.erase(bids_for_last_offer.find(bidder));
                }

                bidder_product_offer[bidder].reset();
            }
            else
            {
                bids.products_valid_bids[cur_product][bidder] = bid_amount;
                bidder_product_offer[bidder] = cur_product;
            }
        }
    }

    return in;
}

std::vector<std::tuple<std::string, double, double, double, std::vector<std::pair<std::string, int>>>>
    BidsReader::get_products_list() const
{
    std::vector<std::tuple<std::string, double, double, double, std::vector<std::pair<std::string, int>>>>
        ret;

    //  Maps are already sorted
    for (auto const& product_bids : products_valid_bids)
    {
        std::string product_name = product_bids.first;

        auto& product_all_bids = products_all_bids.find(product_name)->second;
        double bid_average = 0;
        int max_bid = 0;
        int min_bid = 0;
        if (!product_all_bids.empty())
        {
            bid_average = std::accumulate(product_all_bids.begin(),
                                          product_all_bids.end(), 0.0)
                          / product_all_bids.size();
            max_bid = *product_all_bids.rbegin();
            min_bid = *product_all_bids.begin();
        }

        using bid = std::pair<std::string, int>;
        std::vector<bid> bids(product_bids.second.begin(), product_bids.second.end());
        auto const& appearance_order = this->appearance_order;
        std::sort(bids.begin(), bids.end(),
                  [&appearance_order](bid const& l, bid const& r) {
                      if (l.second != r.second)
                      {
                          return l.second > r.second;
                      }
                      else
                      {
                          return appearance_order.at(l.first) <
                                 appearance_order.at(r.first);
                      }
                  });

        ret.emplace_back(product_name,
                         max_bid, bid_average, min_bid,
                         bids);
    }

    return ret;
}

bool BidsReader::is_product_name(std::string const& s)
{
    static std::regex product_name("[A-Z0-9 ]+");

    return std::regex_match(s, product_name);
}
