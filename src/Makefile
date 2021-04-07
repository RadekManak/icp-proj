.PHONY:all build clean run

all: build
build:
	mkdir -p build
	cd build/ && cmake .. && make all --no-print-directory
clean:
	rm -rf build/
run:
	./build/projekt