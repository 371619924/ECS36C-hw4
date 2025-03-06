all: test_multimap cfs_sched

test_multimap: test_multimap.cc multimap.h
	g++ -Wall -Werror -std=c++11 -o test_multimap test_multimap.cc -pthread -lgtest

cfs_sched: cfs_sched.cc
	g++ -Wall -Werror -std=c++11 -o cfs_sched cfs_sched.cc

clean:
	rm -f test_multimap cfs_sched

lint:
	python3 ./cpplint multimap.h cfs_sched.cc test_multimap.cc
