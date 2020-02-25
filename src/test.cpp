#include <iostream>
#include <fstream>

#include "help.h"

int task_done = 0;
pthread_mutex_t lock;
pthread_cond_t  cond_lock;
int thread_num = 4;
size_t tot_bin_cnt = 262144;


std::vector < std::vector<FMC_MODULE> > bin_area_share;
std::vector< std::vector< std::pair<double,double> > > bin_area_share2;
std::vector < std::vector <int> > bin_partition_by_random;
std::vector < bin_struct > bin_area;

void merge (void* args) {
    int* pidx = (int*)args;
    int idx = *pidx;

    auto bin_partition = bin_partition_by_random[idx];

    for (auto binId : bin_partition) {
        auto curbin = &bin_area[binId];
        curbin->cell_area2 = 0;
        curbin->cell_area  = 0;
        for (auto idx = 0; idx < thread_num; ++idx) {
            curbin->cell_area2 += bin_area_share[idx][binId].fillcell;
            curbin->cell_area  += bin_area_share[idx][binId].macro;
            //curbin->cell_area  += bin_area_share[idx][binId].cell;
            bin_area_share[idx][binId].clear();
        }
    }
    pthread_mutex_lock(&lock);//lock
    task_done++;
    if(task_done == thread_num) {
        pthread_cond_signal(&cond_lock);
        //std::cout << "releasing..." << std::endl;
    }
    pthread_mutex_unlock(&lock);//unlock
}
void merge2 (void* args) {
    int* pidx = (int*)args;
    int idx = *pidx;

    auto bin_partition = bin_partition_by_random[idx];

    for (auto binId : bin_partition) {
        auto curbin = &bin_area[binId];
        curbin->cell_area2 = 0;
        curbin->cell_area  = 0;
        for (auto idx = 0; idx < thread_num; ++idx) {
            auto& p = bin_area_share2[binId][idx];
            curbin->cell_area2 += p.first;
            curbin->cell_area  += p.second;
            //curbin->cell_area  += bin_area_share[idx][binId].cell;
            p.first = 0;
            p.second = 0;
        }
    }
    pthread_mutex_lock(&lock);//lock
    task_done++;
    if(task_done == thread_num) {
        pthread_cond_signal(&cond_lock);
        //std::cout << "releasing..." << std::endl;
    }
    pthread_mutex_unlock(&lock);//unlock
}
void parallelAdd(size_t num){
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    pthread_mutex_lock(&lock);//lock
    task_done++;
    std::cout << task_done << std::endl;
    if(task_done == thread_num) {
        pthread_cond_signal(&cond_lock);
        //std::cout << "releasing..." << std::endl;
    }
    pthread_mutex_unlock(&lock);//unlock

}
int main(int argc,char* argv[]) {

  size_t loopcount = 1;
  if(argc == 3){
    thread_num = atof(argv[1]);
    loopcount = atof(argv[2]);
  }
  std::cout << "thread_num: " << thread_num << "\tloop: " << loopcount << std::endl;


  bin_partition_by_random = random_partition (tot_bin_cnt, thread_num);
  bin_area_share.resize (thread_num);
  bin_area.resize (tot_bin_cnt);
  for (auto idx = 0; idx < thread_num; ++idx) {
      std::vector<FMC_MODULE> temp(tot_bin_cnt);
      bin_area_share[idx] = temp;
  }
  std::vector< std::pair<double, double> > temp (thread_num, std::make_pair (0, 0));
  bin_area_share2.resize (tot_bin_cnt, temp);

  // Initialize pool
  ThreadPool pool(thread_num);
  pool.init();

  double tp_time = 0;
  {
    tp_timer::time_start (tp_time);
    pthread_mutex_init(&lock,nullptr);
    cond_lock = PTHREAD_COND_INITIALIZER;
    for (size_t nloop = 0; nloop < loopcount; ++nloop) {

        task_done = 0;
        std::vector <int> count (thread_num);
        for (int i = 0; i < thread_num; ++i) {
            count[i] = i;
            //std::cout << count[i] << std::endl;
            pool.submit(merge, &count[i]);
        }

        pthread_mutex_lock(&lock);
        if(task_done != thread_num) {
            pthread_cond_wait(&cond_lock,&lock);
            //std::cout << "waiting..." << std::endl;
        }
        pthread_mutex_unlock(&lock);
    }
   tp_timer::time_end (tp_time);
   std::cout << "merge1: " << tp_time << std::endl;
  }
  {
    tp_timer::time_start (tp_time);
    pthread_mutex_init(&lock,nullptr);
    cond_lock = PTHREAD_COND_INITIALIZER;
    for (size_t nloop = 0; nloop < loopcount; ++nloop) {

        task_done = 0;
        std::vector <int> count (thread_num);
        for (int i = 0; i < thread_num; ++i) {
            count[i] = i;
            //std::cout << count[i] << std::endl;
            pool.submit(merge2, &count[i]);
        }

        pthread_mutex_lock(&lock);
        if(task_done != thread_num) {
            pthread_cond_wait(&cond_lock,&lock);
            //std::cout << "waiting..." << std::endl;
        }
        pthread_mutex_unlock(&lock);
    }
   tp_timer::time_end (tp_time);
   std::cout << "merge2: " << tp_time << std::endl;
  }


  pool.shutdown();
  return 0;
}
