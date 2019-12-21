CC=gcc-8
CXX=g++-8
build: debug release

.PHONY: test
test: debug
	./build/debug/tests/unittest

.PHONY: gdbtest
gdbtest: debug
	gdb ./build/debug/tests/unittest

.PHONY: memtest
memtest: debug
	valgrind --leak-check=full --show-leak-kinds=all ./build/debug/tests/unittest

.PHONY: run
run: debug
	echo "TODO: debug"

.PHONY: debug
debug: build/debug
	cd build/debug && ninja

.PHONY: release
release: build/release
	cd build/release && ninja

build/debug:
	mkdir -p build/debug
	cd build/debug && CC=$(CC) CXX=$(CXX) cmake -GNinja -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=Off ../..

build/release:
	mkdir -p build/release
	cd build/release && CC=$(CC) CXX=$(CXX) cmake -GNinja -DCMAKE_BUILD_TYPE=Release ../..

.PHONY: clean
clean:
	cd build/debug && ninja clean
	cd build/release && ninja clean

.PHONY: fullclean
fullclean:
	rm -rf build
