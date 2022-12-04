#include <iostream>

#include "parallel_partial_sum.h"

int main()
{
	std::vector<int> test = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	// std::vector<int> test = { 1, 2, 3 };

	parallel_partial_sum(test.begin(), test.end());

	for (auto t : test)
	{
		std::cout << t << " ";
	}
}
