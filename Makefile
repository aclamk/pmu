SRCS = test.cc pmu.cc
INCS = pmu.h
pmu-test: $(SRCS) $(INCS)
	g++ -o $@ $(SRCS) --std=c++11 -O3 -lpthread
