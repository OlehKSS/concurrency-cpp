#include <iostream>
#include <vector>

#include "for_each.h"

int main()
{
	auto test_size = 100;
	std::vector<int> test_nums(test_size);

	for (int i = 0; i < test_size; ++i)
	{
		test_nums[i] = i;
	}

	parallel_for_each(test_nums.begin(), test_nums.end(),
		[](int i) { std::cout << i << " "; });
}
