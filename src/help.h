#include <boost/intrusive_ptr.hpp>
#include <boost/atomic.hpp>
#include <random>
#include <thread>
#include <vector>


#include "ThreadPool.h"
#include "utils.h"

#define prec double



//int n = 0;
boost::atomic<int> n{0};
std::random_device rd;
std::mt19937 mt(rd());
std::uniform_int_distribution<int> dist(-1000, 1000);
auto rnd = std::bind(dist, mt);

std::vector < std::vector<int> > random_partition (size_t N, size_t npart) {
    assert ( N != 0 );
    std::vector < std::vector<int> > ret(npart);
    std::vector<int> rand_list = randomlist(N);
    size_t len = N / npart;
    size_t rest = N % npart;
    for(size_t idx = 0; idx < npart; ++idx) {
        auto mod_iter1 = rand_list.begin() + len * idx;
        auto mod_iter2 = rand_list.begin() + len * (idx + 1);
        ret[idx].assign(mod_iter1, mod_iter2);
    }
    for (size_t idx = 0; idx < rest; ++idx) {
        ret[idx].push_back (N - rest + idx);
    }
    return ret;
}
class FMC_MODULE {
public:
    FMC_MODULE () {
        fillcell = rand();
        macro = 0;
        cell = 0;
    }
    void clear() {
        fillcell = 0;
        macro = 0;
        cell = 0;
    }
    prec fillcell;
    prec macro;
    prec cell;
};
class bin_struct {
    public:
        bin_struct () {
            cell_area = 0;
            cell_area2 = 0;
        }
        prec cell_area;
        prec cell_area2;
};
