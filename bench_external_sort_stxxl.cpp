// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
#include <stxxl/mng>
#include <stxxl/sort>
#include <stxxl/vector>

#include <sys/time.h>

#include <limits>

#include <boost/random/linear_congruential.hpp>

typedef int type;

double operator - (const timeval & a, const timeval & b)
{
	long sec = a.tv_sec - b.tv_sec;
	long usec = a.tv_usec - b.tv_usec;
	if (usec < 0) {
		usec += 1000000;
		sec -= 1;
	}
	return double(sec) + double(usec) / 1000000.;
}

struct cmp : public std::less<type>
{
    type min_value() const
    {
        return std::numeric_limits<type>::min();
    }
    type max_value() const
    {
        return std::numeric_limits<type>::max();
    }
};

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " bytes [seed]" << std::endl;
		return 1;
	}

	const size_t n_records = atol(argv[1]) / sizeof(type);
	int seed = getpid();
	if (argc > 2)
		seed = atoi(argv[2]);

	boost::rand48 prng(seed);

#if STXXL_PARALLEL_MULTIWAY_MERGE
    STXXL_MSG("STXXL_PARALLEL_MULTIWAY_MERGE");
#endif
    unsigned long long memory_to_use = 4096L * 1024 * 1024;
    typedef stxxl::vector<type> vector_type;

    vector_type v(n_records);

    STXXL_MSG("Filling vector..., input size = " << v.size() << " elements (" << ((v.size() * sizeof(type)) >> 20) << " MiB)");
    for (vector_type::size_type i = 0; i < v.size(); i++)
        v[i] = prng();

    STXXL_MSG("Sorting (using " << (memory_to_use >> 20) << " MiB of memory)...");
	timeval before;
	gettimeofday(&before, 0);
    stxxl::sort(v.begin(), v.end(), cmp(), memory_to_use);
	timeval after;
	gettimeofday(&after, 0);

	double seconds = after - before;
	double MiB = double(n_records * sizeof(type)) / double(1 << 20);

	STXXL_MSG("Sorted " << n_records << " elements of size " << sizeof(type) << " (total " << (MiB / 1024) << " GiB) in " << seconds << " seconds, " << (double(n_records) / seconds) << " items/sec " << (MiB / seconds) << " MiB/s");

    return 0;
}
