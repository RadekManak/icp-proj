.PHONY:all build clean run sim doxygen pack

all: build

build:
	cd src && make build

clean:
	cd src && make clean

run: build
	cd src && make run

sim: build
	cd src && make sim

doxygen:
	cd src && make doxygen

pack:
	make clean
	zip -r 1-xmanak20-xbreza01.zip Makefile src README.md
