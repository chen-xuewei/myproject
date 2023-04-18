all:procctl shmemory

procctl:procctl.cpp
	g++ procctl.cpp -o procctl

shmemory:
	g++ shmemory.cpp -std=c++11 -o shmemory

clean:
	rm -f procctl
	rm -f shmemory