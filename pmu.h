/*
 * pmu.h
 *
 *  Created on: Mar 14, 2017
 *      Author: adam
 */

#ifndef PMU_H_
#define PMU_H_
#include <type_traits>
#include <stdint.h>
#include <atomic>

#include <unistd.h>
#include <iostream>
#include <mutex>


template <bool s> class Storage;

template<> class Storage<false> {};

template<> class Storage<true> {
  int fd;
};

template <typename T, bool s> class OptionalStorage;

template<typename T> class OptionalStorage<T, false> {};

template<typename T> class OptionalStorage<T, true>
{
  T val;
};


//((M>>0) & 1) | ((M>>1) & 1) | ((M>>2) & 1) | ((M>>3) & 1) |


template <uint64_t val> struct sumbits {
  static constexpr uint8_t value = (val & 1) + sumbits< (val>>1) >::value;
};

template <> struct sumbits<0> {
  static constexpr uint8_t value = 0;
};

template <int M> class X {


  static constexpr uint8_t bits = sumbits<M>::value;
public:
  int fds[bits];
  Storage< M & (1<<0) > s0;

  X(){
    for(int i=0; i<100;i++) {
      if(M & (1<<i)) {

      }
    }
    //static_assert(I==M,"");
  }
  //static_assert(I==M,"x");
};

void xxxx() {
  X<1> x;
  //close(x.fd);

}



namespace pmu
{

enum counters {
  CPU_CYCLES		= 0,
  INSTRUCTIONS		= 1,
  CACHE_REFERENCES		= 2,
  CACHE_MISSES		= 3,
  BRANCH_INSTRUCTIONS	= 4,
  BRANCH_MISSES		= 5,
  BUS_CYCLES		= 6,
  STALLED_CYCLES_FRONTEND	= 7,
  STALLED_CYCLES_BACKEND	= 8,
  REF_CPU_CYCLES		= 9,
  PMU_COUNTER_COUNT = 10,
  TIME_ELAPSED = 30,
  MEASUREMENT_COUNT = 31,
};

struct hw_counters {
  hw_counters();
  ~hw_counters();
  int fd[PMU_COUNTER_COUNT];
  void init_counter(uint32_t counter);
  uint64_t get_counter(uint32_t counter);
};

struct hw_counters& get_hw_counters();

template <uint64_t _events> class counter
{
public:
  static constexpr uint64_t events = _events;
  static constexpr uint8_t active_hw_counters = sumbits< events & ((2<<PMU_COUNTER_COUNT) - 1) >::value;

  static constexpr bool count_uses = (events & (1 << MEASUREMENT_COUNT)) != 0;
  static constexpr bool count_time = (events & (1 << TIME_ELAPSED)) != 0;
  std::atomic<uint64_t> hw_values[active_hw_counters];
  OptionalStorage<std::atomic<uint64_t>, count_uses> use_count;
  OptionalStorage<std::atomic<uint64_t>, count_time> time_elapsed;

public:
  counter()
  {
    using namespace std;
    struct hw_counters& counters = get_hw_counters();
    int pos=0;
    for (int i=0; i<PMU_COUNTER_COUNT;i++) {
      if ( (events & (1<<i)) != 0 ) {
        cout << "i=" << i << endl;
        counters.init_counter(i);
        pos++;
      }
    }
  }
  ~counter()
  {
    using namespace std;
    int pos=0;
    for (int i=0; i<PMU_COUNTER_COUNT;i++) {
      if ( (events & (1<<i)) != 0 ) {
        cout << "result " << i << "X "<< hw_values[pos]<< endl;
        pos++;
      }
    }
    //do printing
  }



};

template <uint64_t events> class scope {
public:
  scope(counter<events>& counters): counters(counters)
  {
    struct hw_counters& hw = get_hw_counters();
    int pos=0;
    for (int i=0; i<PMU_COUNTER_COUNT; i++) {
      if ( (events & (1<<i)) != 0 ) {
        counters.hw_values[pos] -= hw.get_counter(i);
        pos++;
      }
    }
  };
  ~scope()
  {
    struct hw_counters& hw = get_hw_counters();
    int pos=0;
    for (int i=0; i<PMU_COUNTER_COUNT; i++) {
      if ( (events & (1<<i)) != 0 ) {
        counters.hw_values[pos] += hw.get_counter(i);
        pos++;
      }
    }
  }

private:
  counter<events>& counters;
};

}



pmu::counter<111> cnt;

#endif /* PMU_H_ */
