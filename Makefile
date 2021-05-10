.PHONY:all build clean run sim doxygen pack

all: build

build:
	mkdir -p build
	cmake -B build
	make -C build

clean:
	rm -rf *build*/
	rm -rf doc

run: build
	cd build && ./mqtt-explorer-fit

sim: build
	cd build && ./trafficSimulator

doxygen:
	doxygen Doxyfile
	doxygen simDoxyfile

pack:
	make clean
	zip -r 1-xmanak20-xbreza01.zip src sim simDoxyfile Makefile Doxyfile CMakeLists.txt README.md
