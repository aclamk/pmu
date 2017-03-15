/*
 * pmu.cc
 *
 *  Created on: Mar 14, 2017
 *      Author: adam
 */

#include "pmu.h"


#include <unistd.h>
#include <iostream>
#include <mutex>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>


int perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
    int cpu, int group_fd, unsigned long flags)
{
  int ret;

  ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
      group_fd, flags);
  return ret;
}








using namespace std;
using namespace pmu;
bool check_perf()
{
	int res = access("/proc/sys/kernel/perf_event_paranoid",F_OK);
	return res == 0;
}

hw_counters::hw_counters()
{
  for (int i=0; i<PMU_COUNTER_COUNT;i++)
  {
    fd[i] = -1;
  }
}

hw_counters::~hw_counters()
{
  for (int i=0; i<PMU_COUNTER_COUNT;i++)
  {
    if (fd[i] != -1) {
      close(fd[i]);
      fd[i] = -1;
    }
  }
}
struct hw_counters& pmu::get_hw_counters()
{
  thread_local struct hw_counters hw;
  cout << "hw = " << &hw << endl;
  return hw;
}

void hw_counters::init_counter(uint32_t counter)
{
  static std::mutex m;
  m.lock();
  cout << "counterX " << counter << " = " << fd[counter] << endl;

  if (fd[counter] == -1)
  {
    struct perf_event_attr pe;

    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type = PERF_TYPE_HARDWARE;
    pe.size = sizeof(struct perf_event_attr);
    pe.config = PERF_COUNT_HW_INSTRUCTIONS;
    pe.disabled = 0;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;

    fd[counter] = perf_event_open(&pe, 0, -1, -1, 0);
    cout << "counter " << counter << " = " << fd[counter] << endl;
  }
  m.unlock();
}

uint64_t hw_counters::get_counter(uint32_t counter)
{
  int f = fd[counter];
  long long count = 0;
  read(f, &count, sizeof(long long));
  return count;
}

int main(int argc, char** argv)
{
  pmu::counter<15> cnt;
  pmu::scope<cnt.events> b(cnt);
  cout << "du*a" << endl;

  //for(int i = 0; i<1000; i++)
   // cout << "du*a" << i << endl;


}
