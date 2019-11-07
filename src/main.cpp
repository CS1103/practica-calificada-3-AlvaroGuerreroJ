#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <utility>

#include <prettyprint.hpp>

bool is_product_name(std::string const& s);

int main()
{
    // The list of the total sum and number of bids for each product
    std::map<std::string, std::multiset<int>> products_all_bids;
    // The list of valid bids (a bidder and a bid amount) for each product
    std::map<std::string, std::map<std::string, int>> products_valid_bids;
    //  The list of the product for which each bidder made a bid, if the option
    // is not a value, the bidderer has been invalidated.
    std::map<std::string, std::optional<std::string>> bidder_product_offer;

    // We need to remember the order in which the bidders first appeared
    std::map<std::string, unsigned> appearance_order;
    unsigned cur_order = 0;

    std::string bids_filename = "bid_example.txt";
    std::ifstream bids_file(bids_filename);
    if (!bids_file)
    {
        std::cerr << "Couldn't open " << bids_filename << std::endl;
        return 1;
    }

    std::string cur_product;

    // Because the input is well formed, the first line must be a product name
    std::getline(bids_file, cur_product);

    std::string temp;
    while (getline(bids_file, temp))
    {
        if (is_product_name(temp))
        {
            cur_product = temp;
            products_valid_bids[cur_product];
            products_all_bids[cur_product];
        }
        else
        {
            std::istringstream temp_stream(temp);
            std::string bidder;
            int bid_amount;
            temp_stream >> bidder >> bid_amount;

            if (appearance_order.find(bidder) == appearance_order.end())
            {
                appearance_order[bidder] = cur_order;
                cur_order++;
            }

            products_all_bids[cur_product].insert(bid_amount);

            // A bid is invalid if it's negative or the bidder has made a bid
            // for another product. Bids are invalidated retroactively.
            bool bidder_already_seen = bidder_product_offer.count(bidder) != 0;
            if (bid_amount < 0 ||
                (bidder_already_seen &&
                 (!bidder_product_offer[bidder].has_value() ||
                  bidder_product_offer[bidder].value() != cur_product)))
            {
                if (bidder_product_offer[bidder].has_value())
                {
                    std::string bidder_last_offer_product = bidder_product_offer[bidder].value();
                    auto& bids_for_last_offer = products_valid_bids[bidder_last_offer_product];
                    bids_for_last_offer.erase(bids_for_last_offer.find(bidder));
                }

                bidder_product_offer[bidder].reset();
            }
            else
            {
                products_valid_bids[cur_product][bidder] = bid_amount;
                bidder_product_offer[bidder] = cur_product;
            }
        }
    }

    // Each entry is a tuple that contains the product's name, the maximun bid,
    // the average_bid of all bids, the minimun bid, and the valids bids (each with
    // the bidder's name and the bid's value).
    std::vector<std::tuple<std::string, double, double, double, std::vector<std::pair<std::string, int>>>>
        products;

    // Maps are already sorted
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
        std::sort(bids.begin(), bids.end(),
                  [&appearance_order](bid const& l, bid const& r) {
                      if (l.second != r.second)
                      {
                          return l.second > r.second;
                      }
                      else
                      {
                          return appearance_order[l.first] < appearance_order[r.first];
                      }
                  });

        products.emplace_back(product_name,
                              max_bid, bid_average, min_bid,
                              bids);
    }

    for (auto const& p : products)
    {
        auto const& name = std::get<0>(p);
        auto const& max_bid = std::get<1>(p);
        auto const& average_bid = std::get<2>(p);
        auto const& min_bid = std::get<3>(p);
        auto const& bids = std::get<4>(p);

        std::cout << name << ","
                  << max_bid << ","
                  << average_bid << ","
                  << min_bid << std::endl;

        for (auto const& bid : bids)
        {
            std::cout << bid.first << " " << bid.second << std::endl;
        }
    }
}

bool is_product_name(std::string const& s)
{
    static std::regex product_name("[A-Z0-9 ]+");

    return std::regex_match(s, product_name);
}
