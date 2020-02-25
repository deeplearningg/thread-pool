#include <sys/stat.h>

#include "utils.h"

std::chrono::high_resolution_clock::time_point tp_timer::base = std::chrono::high_resolution_clock::now();
std::chrono::time_point<std::chrono::system_clock> cpu_timer::base = std::chrono::system_clock::now();


std::vector<int> randomlist(size_t num,bool OutOrder){
    std::vector<int> result;
    result.reserve(num);
    for(int i = 0; i < num;i++)
        result.push_back(i);
    if(OutOrder){
        //srand( (unsigned int)time(nullptr) );
        srand(0);
        int p1,p2,temp;
        while(--num){
            p1 = num;
            p2 = rand()%num;
            temp = result[p1];
            result[p1] = result[p2];
            result[p2] = temp;
        }
    }

    return result;
}

ParallelMergeSort::ParallelMergeSort(size_t nparts,size_t arrSize){
    threadSize = nparts;
    Index.resize(2*nparts);
    for(size_t i = 0;i < nparts;i++){
        Index[2*i]   = (i)*arrSize/nparts;
        Index[2*i+1] = (i+1)*arrSize/nparts - 1;
    }
    srand(time(nullptr));
    for(size_t i = 0;i < arrSize;i++){
        ret.push_back(i);
        ref.push_back( rand()%10000 );
    }
}
ParallelMergeSort::ParallelMergeSort(size_t nparts,std::vector<prec> _arr){
    threadSize = nparts;
    Index.resize(2*nparts);
    size_t arrSize = _arr.size();
    for(size_t i = 0;i < arrSize;i++){
        ret.push_back(i);
    }
    for(size_t i = 0;i < nparts;i++){
        Index[2*i]   = (i)*arrSize/nparts;
        Index[2*i+1] = (i+1)*arrSize/nparts - 1;
    }
    ref = _arr;
}
ParallelMergeSort::~ParallelMergeSort(){
    std::vector< int >().swap(ret);
    std::vector< prec >().swap(ref);
}
void ParallelMergeSort::mergeSort(ParallelMergeSort *self,size_t part){
    // calculating low and high
    int low  = self->Index[part*2];
    int high = self->Index[part*2+1];

    // evaluating mid point
    int mid = low + (high - low) / 2;
    if (low < high) {
        self->merge_sort(low, mid);
        self->merge_sort(mid + 1, high);
        self->merge(low, mid, high);
    }

    pthread_mutex_lock(&self->lock);//lock
    self->done++;
    //std::cout << "thread " << part << ":\t" << "done && threadsize " \
                           << done << " && " << threadSize << std::endl;
    if(self->done ==self->threadSize){
       pthread_cond_signal(&self->cond_lock);
       //std::cout << "thread  " << part << "release\n";
    }
    pthread_mutex_unlock(&self->lock);//unlock

}
void ParallelMergeSort::merge_sort(int low, int high){
        // calculating mid point of array
    int mid = low + (high - low) / 2;
    if (low < high) {

        // calling first half
        merge_sort(low, mid);

        // calling second half
        merge_sort(mid + 1, high);

        // merging the two halves
        merge(low, mid, high);
    }
}
// merge function for merging two parts
void ParallelMergeSort::merge(int low, int mid, int high)
{
    int* left = new int[mid - low + 1];
    int* right = new int[high - mid];

    // n1 is size of left part and n2 is size
    // of right part
    int n1 = mid - low + 1, n2 = high - mid, i, j;

    // storing values in left part
    for (i = 0; i < n1; i++)
        left[i] = ret[i + low];

    // storing values in right part
    for (i = 0; i < n2; i++)
        right[i] = ret[i + mid + 1];

    int k = low;
    i = j = 0;

    // merge left and right in ascending order
    while (i < n1 && j < n2) {
        if (ref[ left[i] ] <= ref[ right[j] ])
            ret[k++] = left[i++];
        else
            ret[k++] = right[j++];
    }

    // insert remaining values from left
    while (i < n1) {
        ret[k++] = left[i++];
    }

    // insert remaining values from right
    while (j < n2) {
        ret[k++] = right[j++];
    }
}
std::vector<int> ParallelMergeSort::Sort(){
    ThreadPool pool(threadSize);
    pool.init();

    pthread_mutex_init(&lock,nullptr);
    cond_lock = PTHREAD_COND_INITIALIZER;

    done = 0;
    for(size_t i = 0;i < threadSize;i++)
        pool.submit(ParallelMergeSort::mergeSort,this,i);


    pthread_mutex_lock(&lock);
    if(done != threadSize)
        pthread_cond_wait(&cond_lock,&lock);
    pthread_mutex_unlock(&lock);
    ////  merge all threads

#if 0
    size_t arrSize = ret.size();
    merge(0, (arrSize / 2 - 1) / 2, arrSize / 2 - 1);
    merge(arrSize / 2, arrSize/2 + (arrSize-1-arrSize/2)/2, arrSize - 1);
    merge(0, (arrSize - 1)/2, arrSize - 1);
#else
    merge(Index[0], Index[1], Index[3]);
    merge(Index[4], Index[5], Index[7]);
    merge(Index[0], Index[3], Index[7]);
#endif
    pool.shutdown();

    return ret;
}

size_t get_min_pos(const std::vector<prec>& sum) {
    prec min = sum[0];
    size_t idx = 0;
    for(size_t i = 1;i < sum.size();i++){
        if(sum[i] < min){
            idx = i;
            min = sum[i];
        }
    }
    return idx;
}
void tp_timer::time_start ( double& time ) {
    auto now = std::chrono::high_resolution_clock::now();
    auto delta = std::chrono::duration_cast< std::chrono::duration<double> > ( now - base);
    time = delta.count();
}
void tp_timer::time_end ( double& time ) {
    auto now = std::chrono::high_resolution_clock::now();
    auto delta = std::chrono::duration_cast< std::chrono::duration<double> > ( now - base);
    time = delta.count() - time;
}

double cpu_timer::seconds() {
#if 0
    rusage time;
    getrusage(RUSAGE_SELF,&time);
    //return (double)(1.0*time.ru_utime.tv_sec+0.000001*time.ru_utime.tv_usec);	// user time

    return (double)(1.0*time.ru_utime.tv_sec + 0.000001*time.ru_utime.tv_usec + // user time +
                    1.0*time.ru_stime.tv_sec + 0.000001*time.ru_stime.tv_usec); // system time
#else
    // clock() loop is about 72min. (or 4320 sec)
    return double(clock())/CLOCKS_PER_SEC;
#endif
}

void cpu_timer::time_start (double& time) {
    //time = cpu_timer::seconds();
    auto now = std::chrono::system_clock::now();
    auto delta = std::chrono::duration_cast < std::chrono::duration<double> > (now - base);
    time = delta.count();
}
void cpu_timer::time_end (double& time) {
    //double now = cpu_timer::seconds();
    //time = now - time;
    auto now = std::chrono::system_clock::now();
    auto delta = std::chrono::duration_cast < std::chrono::duration<double> > (now - base);
    time = delta.count() - time;
}

void time_start(double *time_cost) {
    struct timeval time_val;
    time_t time_secs;
    suseconds_t time_micro;
    gettimeofday(&time_val, NULL);
    time_micro = time_val.tv_usec;
    time_secs = time_val.tv_sec;
    *time_cost = (double)time_micro / 1000000 + time_secs;
    return;
}

void time_end(double *time_cost) {
    struct timeval time_val;
    time_t time_secs;
    suseconds_t time_micro;
    gettimeofday(&time_val, NULL);
    time_micro = time_val.tv_usec;
    time_secs = time_val.tv_sec;
    *time_cost = (double)time_micro / 1000000 + time_secs - *time_cost;
    return;
}
