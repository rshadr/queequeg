# Customize below to fit your system

CXX      =  g++
CXXFLAGS =
CXXFLAGS += -std=c++20
CXXFLAGS += -Wall -Wextra -pedantic -O2
CXXFLAGS += -ggdb3
CXXFLAGS += -march=native -ftree-vectorize
CXXFLAGS += $(shell pkg-config --cflags libgrapheme)
# CXXFLAGS += --coverage
# LDFLAGS = --coverage -lgcov
# CXXFLAGS += -pg
# LDFLAGS = -pg

# don't even care about this
# CFLAGS_GRAPHEME =
LIBS_GRAPHEME = -lgrapheme
LIBS_INFRA = -linfra
LIBS = $(LIBS_INFRA) $(LIBS_GRAPHEME)

