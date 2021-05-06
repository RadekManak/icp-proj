.PHONY:all build clean run doxygen

all: build
build:
	mkdir -p build
	cd build/ && cmake .. && make all --no-print-directory
clean:
	rm -rf build/
run:
	./build/projekt

doxygen:
	doxygen Doxyfile
