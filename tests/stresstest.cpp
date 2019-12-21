#include <iostream>
#include <pltables/linear_open_address.h>
#include <sstream>
#include <string>

int
main(int argc, char** argv)
{
    using Table = loatable<int, int>;
    Table table;
    std::string input;
    std::string op;
    while (std::getline(std::cin, input)) {
        std::stringstream ss{ input };
        ss >> op;
        if (op == "INSERT") {
            int k, v, r;
            ss >> k >> v >> r;
            auto result = table.insert(k, v);
            assert(result.second != Table::InsertResult::Error);
            assert(result.first.key() == k);
            assert(result.first.val() == r);
        } else if (op == "ERASE") {
            int k, r;
            ss >> k >> r;
            auto result = table.erase(k);
            assert(result == size_t(r));
        } else if (op == "FIND") {
            int k, v;
            ss >> k >> v;
            auto iter = table.find(k);
            assert(iter.key() == k);
            assert(iter.val() == v);
        } else if (op == "MISS") {
            int k;
            ss >> k;
            auto iter = table.find(k);
            assert(iter == table.end());
        } else if (op == "SIZE") {
            int s;
            ss >> s;
            assert(table.size() == size_t(s));
        } else {
            std::cerr << "error: unknown operation: " << op << std::endl;
            return 1;
        }
    }
    std::cerr << "Passed." << std::endl;
    return 0;
}
