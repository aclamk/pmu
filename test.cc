/*
 * test.cc
 *
 *  Created on: Mar 16, 2017
 *      Author: adam
 */



#include "pmu.h"
#include <iostream>

using namespace std;


void billion_instructions()
{
  pmu::counter<pmu::ALL> cnt("loop with circa billion instructions");
  pmu::scope<cnt.events> b(cnt);
  uint64_t val = 0;
  for (int i = 0; i<200*1000*1000; i++) {
    val *= 111;
    val += 1;
  }
  0 == write(0, &val, 0);
}

void branch_prediction_quarter()
{
  pmu::counter<pmu::ALL> cnt("circa 1/4 prediction branch misses");
  pmu::scope<cnt.events> b(cnt);
  uint64_t val = 0;
  for (int i = 0; i<200*1000*1000; i++)
  {
    if (rand() & 1)
    {
      val *= 113;
      val += 1;
    }
    else
    {
      val *= 117;
      val += 7;
    }
  }
  0 == write(0, &val, 0);
}


void test_rand()
{
  pmu::counter<pmu::ALL> cnt("measure rand() function");
  pmu::scope<cnt.events> b(cnt);
  for (int i = 0; i<200*1000*1000; i++)
  {
    rand();
  }
}


void cache_hits()
{
  pmu::counter<pmu::ALL> cnt("multiple cache hits");
  pmu::scope<cnt.events> b(cnt);
  int size=1024*1024*100;
  char* data = (char*)malloc(size);
  for (int i = 0; i<200*1000*1000; i++)
  {
    data[rand() % size]++;
  }
  free(data);
}

void test_sleeping_in_loop()
{
  pmu::counter<pmu::ALL> cnt("sleeping does not cost");
  pmu::scope<cnt.events> b(cnt);
  sleep(1);
}

void test_counting_10000_measurements()
{
  pmu::counter<pmu::ALL> cnt("10000 measurements");
  for (int i = 0; i<10*1000; i++)
  {
    pmu::scope<cnt.events> b(cnt);
  }
}


int main(int argc, char** argv)
{
  pmu::counter<pmu::ALL> cnt("counter for entire main()");
  pmu::scope<cnt.events> b(cnt);

  billion_instructions();
  branch_prediction_quarter();
  test_rand();
  cache_hits();
  test_sleeping_in_loop();
  test_counting_10000_measurements();


}

