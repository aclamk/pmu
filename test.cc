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
    val *= 3;
    val += 1;
  }
  write(0, &val, 0);
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
}

