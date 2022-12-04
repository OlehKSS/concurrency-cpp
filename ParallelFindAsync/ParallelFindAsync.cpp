#include <iostream>
#include <iterator>
#include <vector>

#include "find.h"

int main()
{
    std::vector<int> test = { 6, 4, 17, 12, 10, 2, 11, 15, 5, 18, 19, 3, 9, 11, 12, 20, 7, 12, 14, 4, 1, 0, 2};
    //std::vector<int> test = { 6, 4, 17, 12, 10, 0, 2, 11 };

    auto res = parallel_find(test.begin(), test.end(), 0);

    std::cout << "Val: " << *res << std::endl;
    std::cout << "Pos: " << std::distance(test.begin(), res) << std::endl;
}
