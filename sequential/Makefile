CXXFLAGS += -Wall -Wextra -pthread -fopenmp -g -O3 -DNDEBUG

.phony: all wsp release

all: release

release: accessibility.c
	g++ -I/afs/andrew.cmu.edu/usr2/mhur/private/418/parallel-accessibility/sequential/libxml2-2.7.3/include -lxml2 accessibility.c -o accessibility $(CXXFLAGS)

clean:
	rm -f ./accessibility
	rm -f ./accessibility
