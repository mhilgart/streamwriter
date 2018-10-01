all:
	cmake3 --build build --target all -- -j 24

clean:
	rm -rf build

init:
	mkdir -p build
	cd build && cmake3 ..
