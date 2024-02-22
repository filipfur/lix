LIX_SOURCES=$(wildcard src/*.cpp impact/*.cpp ecs/*.cpp)
LIX_OBJECTS=$(LIX_SOURCES:.cpp=.o)
LIX_INCLUDE=-Iinclude -I. -Iglm/glm -Ijson -Igltf -Ittf -Iimpact -Iecs -I$(GLEW_HOME)/include -I$(SDL2_HOME)/include
LIX_EXT_LIB=-L$(GLEW_HOME)/lib -lglew -L$(SDL2_HOME)/lib -lsdl2 -framework OpenGL

TEST_GEN_SOURCES=$(wildcard gen/fonts/*.cpp gen/images/*.cpp gen/objects/*.cpp gen/shaders/*.cpp)
TEST_GEN_OBJECTS=$(TEST_GEN_SOURCES:.cpp=.o)

WFLAGS=-Wall -Wextra -Werror
DEPLOY=deploy
TARGET_TEST_ALL=$(DEPLOY)/test_all
LIB=$(DEPLOY)/liblix.a
COVFLAGS=--coverage
COVDIR=coverage

.PHONY: all
all: $(TARGET_TEST_ALL) $(LIB)
	./$(TARGET_TEST_ALL)

.PHONY: lib_static
lib_static: $(LIB)

.PHONY: coverage
coverage:
	$(MAKE) $(TARGET_TEST_ALL) EXTRAS=$(COVFLAGS)
	./$(TARGET_TEST_ALL)
	@mkdir -p $(COVDIR)/html
	@lcov --ignore-errors inconsistent,inconsistent --capture --directory . --rc branch_coverage=1 --no-external --output-file $(COVDIR)/coverage-initial.info
	@lcov --ignore-errors inconsistent,inconsistent --remove $(COVDIR)/coverage-initial.info "glm/*" "gen/*" --output-file $(COVDIR)/coverage.info
	@genhtml --ignore-errors inconsistent,inconsistent -o $(COVDIR)/html $(COVDIR)/coverage.info --ignore-errors inconsistent

%.o: %.cpp
	@clang++ -c -std=c++17 -O0 -g $(WFLAGS) $(EXTRAS) \
		$(LIX_INCLUDE) \
		$< \
		-o $@

$(DEPLOY)/%: test/%.cpp $(LIX_OBJECTS) $(TEST_GEN_OBJECTS) | $(DEPLOY)
	clang++ -std=c++17 $(EXTRAS) $^ -o $@ \
		$(LIX_INCLUDE) \
		$(LIX_EXT_LIB)

$(LIB): $(LIX_OBJECTS) | $(DEPLOY)
	$(AR) rcs $@ $^

$(DEPLOY):
	mkdir -p $(DEPLOY)

.PHONY: clean
clean:
	rm -f $(LIX_OBJECTS)
	rm -f $(TEST_GEN_OBJECTS)
	rm -rf $(DEPLOY)
	rm -rf $(COVDIR)
	rm -f *.gcov *.gcda *.gcno
	rm -f src/*.gcov src/*.gcda src/*.gcno gen/*/*.gcov gen/*/*.gcda gen/*/*.gcno

include asproc/gen_assets.mk