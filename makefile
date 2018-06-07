MAKEFLAGS += --no-print-directory

CC	!= which clang-devel || which clang
CXX	!= which clang++-devel || which clang++
FORMAT	!= which clang-format-devel || which clang-format
SYSDBG	!= which lldb-devel || which lldb || which gdb
PROJECT	!= grep "^project" CMakeLists.txt | cut -c9- | cut -d " " -f1 | tr "[:upper:]" "[:lower:]"
SOURCES	!= find src -type f -name '*.hpp' -or -name '*.cpp'

CONFIG	:= -DVCPKG_TARGET_TRIPLET=${VCPKG_DEFAULT_TRIPLET}
CONFIG	+= -DCMAKE_TOOLCHAIN_FILE=${VCPKG}/scripts/buildsystems/vcpkg.cmake
CONFIG	+= -DCMAKE_INSTALL_PREFIX=$(PWD)
CONFIG	+= -DBUILD_APPLICATION=ON

all: debug

run: build/llvm/debug/CMakeCache.txt
	@cmake --build build/llvm/debug --target main && build/llvm/debug/$(PROJECT)

dbg: build/llvm/debug/CMakeCache.txt
	@cmake --build build/llvm/debug --target main && $(SYSDBG) build/llvm/debug/$(PROJECT)

test: build/llvm/debug/CMakeCache.txt
	@cmake --build build/llvm/debug --target tests && cd build/llvm/debug && ctest

bench: build/llvm/release/CMakeCache.txt
	@cmake --build build/llvm/release --target benchmark && build/llvm/release/benchmark

install: release
	@cmake --build build/llvm/release --target install

debug: build/llvm/debug/CMakeCache.txt $(SOURCES)
	@cmake --build build/llvm/debug

release: build/llvm/release/CMakeCache.txt $(SOURCES)
	@cmake --build build/llvm/release

build/llvm/debug/CMakeCache.txt: CMakeLists.txt build/llvm/debug
	@cd build/llvm/debug && CC=$(CC) CXX=$(CXX) cmake -GNinja \
	  -DCMAKE_BUILD_TYPE=Debug -DBUILD_BENCHMARK=OFF -DBUILD_TESTS=ON $(CONFIG) $(PWD)

build/llvm/release/CMakeCache.txt: CMakeLists.txt build/llvm/release
	@cd build/llvm/release && CC=$(CC) CXX=$(CXX) cmake -GNinja \
	  -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARK=ON -DBUILD_TESTS=OFF $(CONFIG) $(PWD)

build/llvm/debug:
	@mkdir -p build/llvm/debug

build/llvm/release:
	@mkdir -p build/llvm/release

format:
	@$(FORMAT) -i $(SOURCES)

clean:
	@rm -rf build/llvm

uninstall:
	@rm -rf bin include lib

.PHONY: all run dbg test bench install debug release format clean uninstall
