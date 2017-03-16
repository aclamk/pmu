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
  write(0, &val, 0);
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
  write(0, &val, 0);
}

void cache_hits()
{
  pmu::counter<pmu::ALL> cnt("multiple cache hits");
  pmu::scope<cnt.events> b(cnt);
  int size=1024*1024*10;
  char* data = (char*)malloc(size);
  for (int i = 0; i<200*1000*1000; i++)
  {
    data[rand() % size]++;
  }

}

int main(int argc, char** argv)
{
  pmu::counter<pmu::ALL> cnt("aname");

  for(int i = 0; i<1000; i++)
  {
    pmu::scope<cnt.events> b(cnt);

    cout << "du*a" << i << endl;
  }
  billion_instructions();
  branch_prediction_quarter();
  cache_hits();


}

