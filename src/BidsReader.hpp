#ifndef __UTIL_HPP_
#define __UTIL_HPP_

#include <istream>
#include <map>
#include <set>
#include <string>
#include <vector>

class BidsReader
{
    std::map<std::string, std::multiset<int>> products_all_bids;
    std::map<std::string, std::map<std::string, int>> products_valid_bids;
    std::map<std::string, unsigned> appearance_order;

public:
    friend std::istream& operator>>(std::istream& in, BidsReader& bids);

    std::vector<std::tuple<std::string, double, double, double, std::vector<std::pair<std::string, int>>>>
        get_products_list() const;

private:
    static bool is_product_name(std::string const& s);
};


#endif // __UTIL_HPP_
