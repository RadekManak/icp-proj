.PHONY:all build clean run sim doxygen

all: build

build:
	mkdir -p build
	cmake -B build
	make -C build

clean:
	rm -rf build/

run: build
	cd build && ./mqtt-explorer-fit

sim: build
	cd build && ./trafficSimulator

doxygen:
	doxygen Doxyfile
	doxygen simDoxyfile
