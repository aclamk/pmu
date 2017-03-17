pmu-test: test.cc pmu.cc
	g++ -o $@ $^ --std=c++11 -O3
