GEN_SOURCES=$(wildcard test/gen/*/*.cpp)
WFLAGS=-Wall -Wextra -Werror -Wdeprecated -Woverloaded-virtual -Wconversion -Wold-style-cast -Wcast-align -Wmissing-include-dirs -Wnull-dereference -Wdouble-promotion -Wformat=2 -Wundef -Wnon-virtual-dtor -Wpedantic -Wshadow

.PHONY: all
all: bin/asproc test/bin/test

.PHONY: compile
compile:
	touch src/asproc.cpp
	$(MAKE) bin/asproc

.PHONY: clean
clean:
	rm -f bin/asproc
	rm -rf test/gen
	rm -rf test/bin

bin/asproc: src/asproc.cpp src/common.cpp src/shaderproc.cpp src/imageproc.cpp src/objectproc.cpp src/fontproc.cpp ../json/json.cpp
	mkdir -p bin
	clang++ $(WFLAGS) -std=c++17 -g -Iinclude -I../glm/glm -I../gltf -I../json -I../ttf $^ -o $@
	./$@ --version-override "300 es" -s test/shaders test/gen/shaders
	./$@ -o test/objects test/gen/objects
	./$@ -i test/images test/gen/images
	./$@ -f test/fonts test/gen/fonts

test/bin/test: test/test.cpp ${GEN_SOURCES}
	mkdir -p test/bin
	g++ -std=c++17 -Iinclude -I../glm/glm -I../gltf -I../ttf -Itest $^ -o $@
	./$@