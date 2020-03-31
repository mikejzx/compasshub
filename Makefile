# Compilers
CXX = g++

# Project name
PROJECT = compasshub

OPTS = -std=c++17 -Wall -I./src -DPLATFORM_UNIX -D_GNU_SOURCE
LIBS = -lncurses -lssl -lcrypto

# SRCS = $(shell find src -name '*.c*' | grep -P '.*\.*(cpp|c)$$')
SRCS = $(shell find src -name '*.cpp' | grep -P '.*\.*cpp$$')
DIRS = $(shell find src -type d | sed 's/src/./g')
OBJS = $(patsubst src/%.cpp,bin/intermed/%.o,$(SRCS))
DEPS = $(patsubst src/%.cpp,bin/intermed/%.d,$(SRCS))

.PHONY: all clean

all: ./bin/$(PROJECT)

clean:
	$(RM) $(OBJS) $(DEPS) bin/$(PROJECT) bin/$(PROJECT).log bin/$(PROJECT).prefs src/pch.h.gch
	mkdir -p bin/intermed/logger

./bin/$(PROJECT): $(OBJS)
	$(CXX) $(OPTS) $^ -o $@ $(LIBS)

-include $(DEPS)

bin/intermed/%.o: src/%.cpp Makefile
	$(CXX) $(OPTS) $(LIBS) -MMD -MP -c $< -o $@

