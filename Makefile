# This makefile is only used to build unit tests - the actual library
# is header only.

# For an optimized, stripped build, use:
#
#   $ make OPTIMIZE=-O3 SYMBOLS=""
#
# For a C++14 build, use:
#
#   $ make COMPILER=g++-5 STDLIB=c++14
#
# Optional command line arguments:
# see http://stackoverflow.com/a/24264930/43839
#

OPTIMIZE ?= -O0
STDLIB ?= c++11
SYMBOLS ?= -g

#
# Compilation variables.
#
CODE_GENERATION = $(OPTIMIZE) $(SYMBOLS) -std=$(STDLIB) -pthread
DEPENDENCIES = -MMD -MP -MF
INCLUDES = -Isrc
LIBRARIES = -lm -lstdc++
WARNINGS = -Wall -Wextra -Wno-strict-aliasing -Wpedantic

DEFINES = -DDEBUG

CXXFLAGS_BASE +=     \
  $(CODE_GENERATION) \
  $(DEFINES)         \
  $(INCLUDES)        \
  $(LIBRARIES)       \
  $(WARNINGS)        \
  $(DEPENDENCIES)

CXXFLAGS = $(CXXFLAGS_BASE) $(DEPENDENCIES)
CXXFLAGS_TEST = $(CXXFLAGS_BASE)

BINARIES = build/utf8_test
OBJ = build/obj
DIRECTORIES = build $(OBJ) build/.deps

#
# Build rules
#

.PHONY: all binaries
.SUFFIXES:
.SECONDARY:

all: binaries

pre-build:
	mkdir -p $(DIRECTORIES)

binaries: pre-build
	@$(MAKE) --no-print-directory $(BINARIES)

build/%: src/swirly/%.cpp
	$(CXX) -o $@ $< $(CXXFLAGS) build/.deps/$*.d

clean:
	rm -Rf $(DIRECTORIES)

-include build/.deps/*.d
