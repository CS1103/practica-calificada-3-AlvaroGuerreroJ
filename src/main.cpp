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

#include "BidsReader.hpp"

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "You must provide the filename as argument" << std::endl;
        return 1;
    }
    std::string bids_filename = argv[1];

    //  The list of the total sum and number of bids for each product
    std::map<std::string, std::multiset<int>> products_all_bids;
    // The list of valid bids (a bidder and a bid amount) for each product
    std::map<std::string, std::map<std::string, int>> products_valid_bids;
    // We need to remember the order in which the bidders first appeared
    std::map<std::string, unsigned> appearance_order;

    std::ifstream bids_file(bids_filename);
    if (!bids_file)
    {
        std::cerr << "Couldn't open " << bids_filename << std::endl;
        return 1;
    }

    BidsReader bids;
    bids_file >> bids;

    // Each entry is a tuple that contains the product's name, the maximun bid,
    // the average bid of all bids, the minimun bid, and the valids bids (each
    // with the bidder's name and the bid's value).
    std::vector<std::tuple<std::string, double, double, double, std::vector<std::pair<std::string, int>>>>
        products = bids.get_products_list();


    std::ofstream file_out("resultado.txt");
    for (auto const& p : products)
    {
        auto const& name = std::get<0>(p);
        auto const& max_bid = std::get<1>(p);
        auto const& average_bid = std::get<2>(p);
        auto const& min_bid = std::get<3>(p);
        auto const& bids = std::get<4>(p);

        file_out << name << ","
                 << max_bid << ","
                 << average_bid << ","
                 << min_bid << std::endl;

        for (auto const& bid : bids)
        {
            file_out << bid.first << " " << bid.second << std::endl;
        }
    }
}
