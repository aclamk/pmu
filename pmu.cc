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










using namespace std;
using namespace pmu;




const char* pmu::counters_names[PMU_COUNTER_COUNT] =
{
    "cpu cycles",
    "instructions",
    "cache references",
    "cache misses",
    "branch instructions",
    "branch misses",
    "bus cycles",
    "stalled cycles frontend",
    "stalled cycles backend",
    "ref cpu cycles"
};

int perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
    int cpu, int group_fd, unsigned long flags)
{
  int ret;
  ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
      group_fd, flags);
  return ret;
}


bool check_perf()
{
	int res = access("/proc/sys/kernel/perf_event_paranoid",F_OK);
	return res == 0;
}

int open_hw_counter_on(uint32_t counter)
{
  struct perf_event_attr pe;

  memset(&pe, 0, sizeof(struct perf_event_attr));
  pe.type = PERF_TYPE_HARDWARE;
  pe.size = sizeof(struct perf_event_attr);
  pe.config = counter;//PERF_COUNT_HW_INSTRUCTIONS;
  pe.disabled = 0;
  pe.exclude_kernel = 1;
  pe.exclude_hv = 1;

  int fd;
  fd = perf_event_open(&pe, 0, -1, -1, 0);
  return fd;
}

int open_hw_counter_off(uint32_t counter)
{
  return -1;
}

extern "C"
{
static int (*resolve_open_hw_counter ()) (uint32_t)
{
  if(check_perf) {
    return open_hw_counter_on;
  } else {
    return open_hw_counter_off;
  }
}
}

int open_hw_counter(uint32_t counter) __attribute__ ((ifunc ("resolve_open_hw_counter")));


hw_counters::hw_counters()
{
  for (int i=0; i<PMU_COUNTER_COUNT;i++)
  {
    fd[i] = uninitialized;
  }
}

hw_counters::~hw_counters()
{
  for (int i=0; i<PMU_COUNTER_COUNT;i++)
  {
    if (fd[i] >= 0)
    {
      close(fd[i]);
    }
    fd[i] = uninitialized;
  }
}
struct hw_counters& pmu::get_hw_counters()
{
  thread_local struct hw_counters hw;
  return hw;
}

void hw_counters::init_counter(uint32_t counter)
{
  static std::mutex m;
  m.lock();
  if (fd[counter] == uninitialized)
  {
    fd[counter] = open_hw_counter(counter);
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

int main1(int argc, char** argv)
{
  pmu::counter<pmu::ALL> cnt("aname");

  for(int i = 0; i<1000; i++)
  {
    pmu::scope<cnt.events> b(cnt);

    cout << "du*a" << i << endl;
  }

}
