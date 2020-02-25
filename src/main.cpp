#include <iostream>
#include <fstream>
#include <random>
#include <thread>
#include <vector>

//boost
#include <boost/intrusive_ptr.hpp>
#include <boost/atomic.hpp>

#include "ThreadPool.h"
#include "utils.h"

//int n = 0;
boost::atomic<int> n{0};
std::random_device rd;
std::mt19937 mt(rd());
std::uniform_int_distribution<int> dist(-1000, 1000);
auto rnd = std::bind(dist, mt);

int task_done = 0;
pthread_mutex_t lock;
pthread_cond_t  cond_lock;
int thread_num = 4;

void simulate_hard_computation() {
  std::this_thread::sleep_for(std::chrono::milliseconds(2000 + rnd()));
}

// Simple function that adds multiplies two numbers and prints the result
void multiply(const int a, const int b) {
  simulate_hard_computation();
  const int res = a * b;
  std::cout << a << " * " << b << " = " << res << std::endl;
}
void task(const int task_size,const int num,const int a, const int b) {
  simulate_hard_computation();
  const int res = a * b;
  std::cout << "task ->" << num << "\t" << a << " * " << b << " = " << res << std::endl;

 pthread_mutex_lock(&lock);//lock
 task_done++;
 if(task_done == task_size)
     pthread_cond_signal(&cond_lock);
 pthread_mutex_unlock(&lock);//unlock

}

// Same as before but now we have an output parameter
void multiply_output(int & out, const int a, const int b) {
  simulate_hard_computation();
  out = a * b;
  std::cout << a << " * " << b << " = " << out << std::endl;
}

// Same as before but now we have an output parameter
int multiply_return(const int a, const int b) {
  simulate_hard_computation();
  const int res = a * b;
  std::cout << a << " * " << b << " = " << res << std::endl;
  return res;
}
void NetsShuffle(std::vector< std::vector<int> >& nets_cluster, const int threadsize, const int netsize, const std::vector<int> randVector){
    nets_cluster.clear();
    nets_cluster.resize(threadsize);
    std::vector<int>::const_iterator iter = randVector.begin();
    for(int i = 0;i < threadsize;i++){
        if(i != threadsize - 1){
            nets_cluster[i].assign(iter + i*netsize,iter + i*netsize + netsize);
        }
        else{
            nets_cluster[i].assign(iter + i*netsize,randVector.end());
        }
    }
}
std::vector<int> randVector(size_t num,bool OutOrder){
    std::vector<int> result;
    result.clear();
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
void parallelAdd(size_t num){
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    pthread_mutex_lock(&lock);//lock
    task_done++;
    std::cout << task_done << std::endl;
    if(task_done == thread_num) {
        pthread_cond_signal(&cond_lock);
        std::cout << "releasing..." << std::endl;
    }
    pthread_mutex_unlock(&lock);//unlock

}

int main(int argc,char* argv[]) {
  // Create pool with 3 threads
  std::chrono::high_resolution_clock::time_point start;
  std::chrono::high_resolution_clock::time_point end;

  std::chrono::duration<double> elapse_time;
  if (0) {
    #undef TP
  }
#ifdef TP
  std::cout << "hello tp!" << std::endl;
#else
  std::cout << "tp gone!" << std::endl;
#endif

#if 1
  int task_num   = 8;
  int loop = 1;
  if(argc == 2){
      thread_num = atof(argv[1]);
  }

  if(argc == 4){
      thread_num = atof(argv[1]);
      task_num   = atof(argv[2]);
      loop = atof(argv[3]);
  }
  std::cout << "thread size " << thread_num << "\t" << "task size " << task_num << "\t" << "loop " << loop << std::endl;

  // lock

#endif
  ThreadPool pool(thread_num);

  // Initialize pool
  pool.init();
  task_done = 0;
  double tp_time = 0;
  double cpu_time = 0;
  clock_t real_time;

  start = std::chrono::high_resolution_clock::now();
  tp_timer::time_start (tp_time);
  cpu_timer::time_start (cpu_time);
  real_time = std::clock();

  size_t tot_num = 8;
  size_t mean_num = tot_num / thread_num;
  std::vector <size_t> list (thread_num);
  pthread_mutex_init(&lock,nullptr);
  cond_lock = PTHREAD_COND_INITIALIZER;
  for (int i = 0; i < thread_num; ++i) {
        //auto future = pool.submit(task,task_num,i,i, i+1);
        list[i] = i;
        std::cout << "idx: " << i << std::endl;
        pool.submit(parallelAdd, mean_num);
  }
  pthread_mutex_lock(&lock);
  if(task_done != thread_num) {
    pthread_cond_wait(&cond_lock,&lock);
    std::cout << "waiting..." << std::endl;
  }
  pthread_mutex_unlock(&lock);

  tp_timer::time_end (tp_time);
  cpu_timer::time_end (cpu_time);
  end = std::chrono::high_resolution_clock::now();
  real_time = std::clock() - real_time;
  double d_real_time = (double)( real_time / CLOCKS_PER_SEC );

  //getchar();
  pool.shutdown();

  elapse_time = std::chrono::duration_cast< std::chrono::duration<double> >( end - start );
  std::cout << "tot  time: " << elapse_time.count() << std::endl;
  std::cout << "cpu  time: " << cpu_time << std::endl;
  std::cout << "tp   time: " << tp_time << std::endl;
  //std::cout << "real time: " << (float)(real_time/CLOCKS_PER_SEC) << std::endl;
  //std::cout << "real time: " << d_real_time << std::endl;
  printf("real time: %f\n", d_real_time);

  double moment = 0.0;
  tp_timer::time_start (moment);
  tp_timer::time_end (moment);
  std::cout << "moment: " << moment << std::endl;
  return 0;
}
