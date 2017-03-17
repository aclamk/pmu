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
#include <sys/time.h>







namespace pmu
{

enum measurements {
  CPU_CYCLES              = 1 << 0,
  INSTRUCTIONS            = 1 << 1,
  CACHE_REFERENCES        = 1 << 2,
  CACHE_MISSES            = 1 << 3,
  BRANCH_INSTRUCTIONS     = 1 << 4,
  BRANCH_MISSES           = 1 << 5,
  BUS_CYCLES              = 1 << 6,
  STALLED_CYCLES_FRONTEND = 1 << 7,
  STALLED_CYCLES_BACKEND  = 1 << 8,
  REF_CPU_CYCLES          = 1 << 9,

  TIME                    = 1 << 30,
  COUNT                   = 1 << 31,

  CPU                     = CPU_CYCLES | INSTRUCTIONS,
  CACHE                   = CACHE_REFERENCES | CACHE_MISSES,
  BRANCH                  = BRANCH_INSTRUCTIONS | BRANCH_MISSES,
  BUS                     = BUS_CYCLES | STALLED_CYCLES_FRONTEND | STALLED_CYCLES_BACKEND,

  ALL                     = CPU | CACHE | BRANCH | BUS | TIME | COUNT | REF_CPU_CYCLES
};


#if 0
enum countersxx {
  CPU_CYCLES		= 0,
  INSTRUCTIONS		= 1,
  CACHE_REFERENCES		= 2,
  CACHE_MISSES		= 3,
  BRANCH_INSTRUCTIONS	= 4,
  BRANCH_MISSES		= 5,
  BUS_CYCLES		= 6,
  STALLED_CYCLES_FRONTEND	= 7,
  STALLED_CYCLES_BACKEND	= 8,
  REF_CPU_CYCLES		= 9
};
#endif

static constexpr uint32_t PMU_COUNTER_COUNT = 10;
extern const char* counters_names[PMU_COUNTER_COUNT];

//static constexpr uint32_t TIME_ELAPSED = 30;
//static constexpr uint32_t MEASUREMENT_COUNT = 31;

struct hw_counters {
  static constexpr int uninitialized = -2;
  hw_counters();
  ~hw_counters();
  int fd[PMU_COUNTER_COUNT];
  void init_counter(uint32_t counter);
  uint64_t get_counter(uint32_t counter);
};

struct hw_counters& get_hw_counters();

template <typename T, bool s> struct OptionalStorage {};
template<typename T> struct OptionalStorage<T, true> {
  OptionalStorage() : val() {}
  T val;
};

template <uint64_t val> struct sumbits {
  static constexpr uint8_t value = (val & 1) + sumbits< (val>>1) >::value;
};
template <> struct sumbits<0> {
  static constexpr uint8_t value = 0;
};


template <uint64_t _events> class counter
{

public:
  static constexpr uint64_t events = _events;
private:
  static constexpr uint8_t active_hw_counters = sumbits< events & ((2<<PMU_COUNTER_COUNT) - 1) >::value;
  static constexpr bool count_uses = (events & COUNT) != 0;
  static constexpr bool count_time = (events & TIME) != 0;

  std::atomic<uint64_t> hw_values[active_hw_counters];
  OptionalStorage<std::atomic<uint64_t>, count_uses> use_count;
  OptionalStorage<std::atomic<uint64_t>, count_time> time_elapsed;
  std::string name;

  template <uint64_t events, bool b> friend struct IncrementUses;
  template <uint64_t events, bool b> struct IncrementUses
  {
    static void inc(counter<events>&r) {}
  };
  template <uint64_t events> struct IncrementUses<events, true>
  {
    static void inc(counter<events>&r)
    {
      r.use_count.val += 1;
    }
  };

  template <uint64_t events, bool b> friend struct MeasureTime;
  template <uint64_t events, bool b> struct MeasureTime
  {
    static void start(counter<events>& r) {}
    static void stop(counter<events>& r) {}
  };
  template <uint64_t events> struct MeasureTime<events, true>
  {
    static uint64_t now_usec()
    {
      struct timeval tv;
      gettimeofday(&tv, nullptr);
      return tv.tv_sec*1000000 + tv.tv_usec;
    }
    static void start(counter<events>& r) { r.time_elapsed.val -= now_usec(); }
    static void stop(counter<events>& r) { r.time_elapsed.val += now_usec(); }
  };

  template <uint64_t events, bool b> friend struct TimePrinter;
  template <uint64_t events, bool b> struct TimePrinter
  {
    static void print(counter<events>& r) {}
  };
  template <uint64_t events> struct TimePrinter<events, true>
  {
    static void print(counter<events>& r)
    {
      printf("%25s: %luus\n","time elapsed", r.time_elapsed.val.load());
    }
  };

  template <uint64_t events, bool b> friend struct UsesPrinter;
  template <uint64_t events, bool b> struct UsesPrinter
  {
    static void print(counter<events>& r) {}
  };
  template <uint64_t events> struct UsesPrinter<events, true>
  {
    static void print(counter<events>& r)
    {
      printf("%25s: %lu\n","use count", r.use_count.val.load());
    }
  };



  void inc_use_count()
  {
    IncrementUses<events,count_uses>::inc(*this);
  }
  void time_start()
  {
    MeasureTime<events,count_time>::start(*this);
  }
  void time_stop()
  {
    MeasureTime<events,count_time>::stop(*this);
  }
  void print_time()
  {
    TimePrinter<events,count_time>::print(*this);
  }
  void print_uses()
  {
    UsesPrinter<events,count_uses>::print(*this);
  }


public:
  counter(const char* name): name(name)
  {
    using namespace std;
    struct hw_counters& counters = get_hw_counters();
    int pos=0;
    for (int i=0; i<PMU_COUNTER_COUNT;i++) {
      if ( (events & (1<<i)) != 0 ) {
        counters.init_counter(i);
        pos++;
      }
    }
    for(auto &h : hw_values)
      h = 0;
  }
  ~counter()
  {
    using namespace std;
    int pos=0;
    struct hw_counters& hw = get_hw_counters();
    printf("%s:\n",name.c_str());
    for (int i=0; i<PMU_COUNTER_COUNT;i++) {
      if ( (events & (1<<i)) != 0 ) {
        if (hw.fd[i] >= 0) {
          printf("%25s: %lu\n",counters_names[i], hw_values[pos].load());
        } else {
          printf("%25s: N/A\n",counters_names[i]);
        }
        pos++;
      }
    }
    print_time();
    print_uses();
    printf("\n");
  }

  template <uint64_t events> friend class scope;
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
    counters.time_start();
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
    counters.time_stop();
    counters.inc_use_count();
  }

private:
  counter<events>& counters;
};

//template <uint32_t x, typename... rest> mask





}


#endif /* PMU_H_ */
