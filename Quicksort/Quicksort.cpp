#include <iostream>

#include "sorter.h"

int main()
{
    std::list<int> l = { 7, 5, 16, 8, 1, 2, 20, 19 };

    auto res = parallel_quick_sort(l);

    std::cout << "Hello World!\n";
}
