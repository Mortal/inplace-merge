#include <iostream>
#include <vector>
#include <algorithm>
#include <sys/time.h>
#include <stdexcept>
#include <random>

//#define DEBUG(x) do { std::cout << x; } while (0)
#define DEBUG(x) do {} while (0)

struct timer {
	timer() {
		::gettimeofday(&t1, NULL);
	}

	double elapsed() {
		struct timeval t2;
		::gettimeofday(&t2, NULL);
		return (t2.tv_sec - t1.tv_sec) +
			(t2.tv_usec - t1.tv_usec) * 1e-6;
	}

	struct timeval t1;
};

size_t * merge(size_t * a, size_t * b, size_t * c, size_t * out) {
	size_t * i = a;
	size_t * j = b;
	while (i != b && j != c) {
		if (*i < *j) {
			std::iter_swap(i++, out++);
		} else {
			std::iter_swap(j++, out++);
		}
	}
	if (i != b) std::swap_ranges(i, b, c - (b - i));
	return out;
}

void inplace_merge(size_t * a, size_t * b, size_t * c) {
	size_t N = b - a;
	if (b + N != c) throw std::runtime_error("Bad input sizes");

	size_t sqrtN = 1;
	while (sqrtN * sqrtN < N) ++sqrtN;
	if (sqrtN * sqrtN != N) throw std::runtime_error("N is not a square");
	size_t bucketSize = sqrtN;
	// size_t buckets = sqrtN;

	// Scratch space
	std::swap_ranges(a, a + bucketSize, b - bucketSize);
	std::swap_ranges(a + bucketSize, a + 2 * bucketSize, c - bucketSize);

	for (size_t * i = a; i != c; ++i) {
		if (i == a) DEBUG("buckets: ");
		DEBUG(*i << ' ');
		if ((i - a + 1) % bucketSize == 0) DEBUG("| ");
	}
	DEBUG(std::endl);

	// Selection sort of buckets
	for (size_t * i = a + 2 * bucketSize; i != c; i += bucketSize) {
		size_t * smallest = i;
		for (size_t * j = i; j != c; j += bucketSize) {
			if (*j < *smallest) smallest = j;
		}
		DEBUG("Insert " << (smallest - (a + 2 * bucketSize)) << " at "
			<< (i - (a + 2 * bucketSize)) << '\n');
		std::swap_ranges(i, i + bucketSize, smallest);
	}

	for (size_t * i = a; i != c; ++i) {
		if (i == a) DEBUG("W: ");
		if (i == a + 2 * bucketSize) DEBUG("buckets: ");
		DEBUG(*i << ' ');
		if ((i - a + 1) % bucketSize == 0) DEBUG("| ");
	}
	DEBUG(std::endl);

	size_t * out = a;
	size_t * series1 = a + 2 * bucketSize;
	size_t * series2 = series1 + bucketSize;
	while (true) {
		while (series2 != c && series2[-1] <= series2[0])
			series2 += bucketSize;
		if (series2 == c) break;
		for (size_t * i = a; i != c; ++i) {
			if (i == out) DEBUG("| ");
			if (i == series1) DEBUG("| ");
			if (i == series2) DEBUG("| ");
			if (i == series2 + bucketSize) DEBUG("| ");
			DEBUG(*i << ' ');
		}
		DEBUG(std::endl);
		out = merge(series1, series2, series2 + bucketSize, out);
		series2 += bucketSize;
		series1 = out + 2 * bucketSize;
	}

	for (size_t * i = a; i != c; ++i) {
		if (i == out) DEBUG("| ");
		DEBUG(*i << ' ');
	}
	DEBUG(std::endl);

	// Insertion sort of remainder
	for (size_t * i = out; i != c; ++i) {
		size_t * insertionPoint = out;
		while (insertionPoint != i && *insertionPoint < *i)
			++insertionPoint;
		DEBUG("Insert " << (i - out) << " at "
			<< (insertionPoint - out) << '\n');
		std::rotate(insertionPoint, i, i + 1);
	}

	for (size_t * i = a; i != c; ++i) DEBUG(*i << ' ');
	DEBUG(std::endl);
}

template <typename F>
void test(std::vector<size_t> & numbers, size_t N, const char * kind, F && f) {
	std::mt19937 rng;
	for (size_t i = 0; i < 2 * N; ++i) numbers[i] = rng();
	std::sort(&numbers[0], &numbers[0] + N);
	std::sort(&numbers[0] + N, &numbers[0] + 2 * N);
	{
		timer t;
		f(&numbers[0], &numbers[N], &numbers[0] + 2*N);
		std::cout << N << "\t" << kind << "\t"
			<< t.elapsed() << std::endl;
	}
	for (size_t i = 1; i < 2*N; ++i) {
		if (numbers[i] < numbers[i - 1]) {
			std::cout << "failed " << i << std::endl;
		}
	}
}

void test_all(size_t sqrtN, size_t trials) {
	size_t N = sqrtN * sqrtN;
	std::vector<size_t> numbers(2 * N);
	for (size_t i = 0; i < trials; ++i) {
		test(numbers, N, "huang", inplace_merge);
		test(numbers, N, "std", std::inplace_merge<size_t *>);
		// test(numbers, N, "sort",
		// 	[](size_t * a, size_t *, size_t * c) {
		// 		std::sort(a, c);
		// 	});
	}
}

int main() {
	// std::vector<size_t> numbers(50);
	// test(numbers, 25, "huang", inplace_merge);
	test_all(2000, 5);
	return 0;
}
