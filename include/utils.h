#ifndef __UTILS__
#define __UTILS__

#include <vector>
#include <chrono>
#include <ctime>
#include <bits/types.h>
#include <sys/resource.h>
#include <sys/time.h>


#include "ThreadPool.h"
typedef double prec;


std::vector<int> randomlist(size_t num,bool OutOrder = true);
size_t get_min_pos( const std::vector<prec>& sum);

class ParallelMergeSort{
    public:
        ParallelMergeSort(size_t nparts,size_t arrSize);
        ParallelMergeSort(size_t nparts,std::vector<prec> _arr);
        ~ParallelMergeSort();
        std::vector<int> Sort();
        std::vector< int > ret;
        std::vector< prec > ref;

    private:
        pthread_mutex_t lock;
        pthread_cond_t  cond_lock;
        size_t threadSize;
        size_t done;

        std::vector< int > Index;

        static void mergeSort(ParallelMergeSort *self,size_t part);
        void merge_sort(int low, int high);
        void merge(int low, int mid, int high);
};

class tp_timer {
    public:
        static void time_start(double& time);
        static void time_end(double& time);

        static std::chrono::high_resolution_clock::time_point base;
};
class cpu_timer {
    public:
        static double seconds();
        static void time_start (double& time);
        static void time_end (double& time);

        //static double base;
        //std::chrono::time_point base;
        static std::chrono::time_point<std::chrono::system_clock> base;
};
#endif
