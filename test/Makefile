LIX_ENGINE=../engine
ASPROC_HOME=../asproc
LIX_SOURCES=$(wildcard $(LIX_ENGINE)/*/*.cpp)
LIX_OBJECTS=$(LIX_SOURCES:.cpp=.o)
LIX_INCLUDE=-I. -I../glm -I../json -I$(ASPROC_HOME)/public -I$(LIX_ENGINE)/core -I$(LIX_ENGINE)/render -I$(LIX_ENGINE)/format -I$(LIX_ENGINE)/impact -I$(LIX_ENGINE)/ecs
LIX_EXT_LIBS=-L$(GLEW_HOME)/lib -lglew -L$(SDL2_HOME)/lib -lsdl2 -framework OpenGL

TEST_GEN_SOURCES=$(wildcard gen/fonts/*.cpp gen/images/*.cpp gen/objects/*.cpp gen/shaders/*.cpp)
TEST_GEN_OBJECTS=$(TEST_GEN_SOURCES:.cpp=.o)

WFLAGS=-Wall -Wextra -Werror
DEPLOY=deploy
TARGET_TEST_ALL=$(DEPLOY)/test_all
LIX_LIB=$(DEPLOY)/liblix.a
LIX_LIB_WASM=$(DEPLOY)/liblix_wasm.a
COVFLAGS=--coverage
COVDIR=coverage

.PHONY: all
all: $(TARGET_TEST_ALL) $(LIX_LIB)
	./$(TARGET_TEST_ALL)

.PHONY: unit_test
unit_test: $(DEPLOY)/unit_ecs $(DEPLOY)/unit_impact
	@for unit in $^; do \
		./$$unit ; \
	done

.PHONY: check_leaks
check_leaks:
	leaks --atExit -- $(binary)

# -s ALLOW_MEMORY_GROWTH=1 -s STACK_SIZE=5mb

.PHONY: wasm
wasm:
	mkdir -p $(DEPLOY)/wasm
	$(MAKE) gen_shaders GLSL_VERSION="300 es"
	$(MAKE) gen_assets
	emcc --std=c++17 -Wall -Wextra -O3 --memoryprofiler \
		-s FULL_ES3=1 -s USE_WEBGL2=1 -s MIN_WEBGL_VERSION=2 -s USE_SDL=2 \
		$(LIX_INCLUDE) \
		test_all/test.cpp $(TEST_GEN_SOURCES) $(LIX_SOURCES) \
		-o $(DEPLOY)/wasm/app.html
	cd $(DEPLOY) && node ../server.js

.PHONY: lib_static
lib_static: $(LIX_LIB)

.PHONY: lib_wasm
lib_wasm: $(LIX_LIB_WASM)


#@lcov --ignore-errors inconsistent,inconsistent --remove $(COVDIR)/coverage-initial.info "gen/*" --output-file $(COVDIR)/coverage.info

.PHONY: coverage
coverage:
	$(MAKE) $(TARGET_TEST_ALL) EXTRAS=$(COVFLAGS)
	./$(TARGET_TEST_ALL)
	@mkdir -p $(COVDIR)/html
	@lcov --ignore-errors inconsistent,inconsistent --capture --directory . --directory $(LIX_ENGINE)/core --directory $(LIX_ENGINE)/render --directory $(LIX_ENGINE)/impact --directory $(LIX_ENGINE)/format --rc branch_coverage=1 --no-external --output-file $(COVDIR)/coverage.info
	@genhtml --ignore-errors inconsistent,inconsistent -o $(COVDIR)/html $(COVDIR)/coverage.info --ignore-errors inconsistent

%.o: %.cpp
	@clang++ -c -std=c++17 -O0 -g $(WFLAGS) $(EXTRAS) \
		$(LIX_INCLUDE) -I$(GLEW_HOME)/include -I$(SDL2_HOME)/include \
		$< \
		-o $@

$(DEPLOY)/%: %/test.cpp $(LIX_OBJECTS) $(TEST_GEN_OBJECTS)
	mkdir -p $(DEPLOY)
	clang++ -std=c++17 $(EXTRAS) $^ -o $@ \
		$(LIX_INCLUDE) -I$(GLEW_HOME)/include -I$(SDL2_HOME)/include \
		$(LIX_EXT_LIBS)

$(LIX_LIB): $(LIX_OBJECTS)
	mkdir -p $(DEPLOY)
	$(AR) rcs $@ $^

$(LIX_LIB_WASM): $(LIX_SOURCES)
	mkdir -p $(DEPLOY)
	emcc --std=c++17 -Wall -Wextra --memoryprofiler -O3 \
		-Iglm/glm -Iinclude -Ijson -Igltf -Ittf $(LIX_SOURCES) \
		-o $@

.PHONY: clean
clean:
	rm -f $(LIX_OBJECTS)
	rm -f $(TEST_GEN_OBJECTS)
	rm -rf $(DEPLOY)
	rm -rf $(COVDIR)
	rm -f *.gcov *.gcda *.gcno
	rm -f $(LIX_ENGINE)/*/*.gcov $(LIX_ENGINE)/*/*.gcda $(LIX_ENGINE)/*/*.gcno \
		gen/*/*.gcov gen/*/*.gcda gen/*/*.gcno

include $(ASPROC_HOME)/gen_assets.mk