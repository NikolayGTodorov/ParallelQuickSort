#include "Sorter.cpp"
#include <iostream>


#define ELEM_COUNT 400000
#define ELEM_COUNT_MINUS_1 399999

template<typename T>
std::list<T> parallel_quick_sort(std::list<T> input)
{
	if (input.empty())
	{
		return input;
	}
	sorter<T> s;
	return s.do_sort(input);
}

int main()
{
	std::list<int> l;
	/*l.push_back(2);
	l.push_back(1);
	l.push_back(5);
	l.push_back(6);
	l.push_back(3);
	l.push_back(9);
	l.push_back(8);
	l.push_back(-2);*/
	for (int i = ELEM_COUNT_MINUS_1; i >= 0; i--)
	{
		if (i % 2 == 0)
		{
			l.push_back(ELEM_COUNT - i);
		}
		else
		{
			l.push_back(i);
		}
	}

	std::list<int> res = parallel_quick_sort<int>(l);
	for (auto& i : res)
	{
		std::cout << i << '\n';
	}
}