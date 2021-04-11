CXXFLAGS += -Wall -Wextra -pthread -fopenmp -g -O3 -DNDEBUG

.phony: all wsp release

all: release

release: accessibility.c
	g++ accessibility.c -o accessibility $(CXXFLAGS)

clean:
	rm -f ./accessibility
	rm -f ./accessibility
