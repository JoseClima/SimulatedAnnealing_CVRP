# ============================================================
# Makefile - CVRP Simulated Annealing
# Local: scripts/Makefile
# ============================================================

ifeq ($(OS),Windows_NT)
  SHELL := /bin/sh
endif

SCRIPT_DIR := $(patsubst %/,%,$(dir $(realpath $(lastword $(MAKEFILE_LIST)))))
ROOT       := $(realpath $(SCRIPT_DIR)/..)

ifeq ($(OS),Windows_NT)
  NPROC := $(NUMBER_OF_PROCESSORS)
else
  NPROC := $(shell nproc)
endif

CXX      := g++
CXXFLAGS := -O2
TARGET   := $(ROOT)/vrp_teste

SRCS := $(wildcard $(ROOT)/src/*.cpp) $(ROOT)/app/Main.cpp
OBJS := $(SRCS:.cpp=.o)

TESTS_DIR  ?= tests/A
TESTS_ROOT ?= tests

.PHONY: all clean run-all run-stats run-all-folders run-stats-folders run-stats-parallel help

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run-all: $(TARGET)
	@echo "== Rodando em $(TESTS_DIR) =="
	@cd $(ROOT) && find $(TESTS_DIR) -name "*.vrp" | sort | while read -r f; do \
		echo ""; \
		echo ">>> $$f"; \
		./vrp_teste $$f; \
	done

run-stats: $(TARGET)
	@echo "== Rodando com --statistics em $(TESTS_DIR) =="
	@cd $(ROOT) && find $(TESTS_DIR) -name "*.vrp" | sort | while read -r f; do \
		echo ">>> $$f"; \
		./vrp_teste $$f --statistics; \
	done

run-all-folders: $(TARGET)
	@echo "== Rodando em TODOS os .vrp dentro de $(TESTS_ROOT)/ =="
	@cd $(ROOT) && find $(TESTS_ROOT) -name "*.vrp" | sort | while read -r f; do \
		echo ""; \
		echo ">>> $$f"; \
		./vrp_teste $$f; \
	done

run-stats-folders: $(TARGET)
	@echo "== Rodando --statistics em TODOS os .vrp dentro de $(TESTS_ROOT)/ =="
	@cd $(ROOT) && find $(TESTS_ROOT) -name "*.vrp" | sort | while read -r f; do \
		echo ">>> $$f"; \
		./vrp_teste $$f --statistics; \
	done

run-stats-parallel: $(TARGET)
	@echo "== Rodando --statistics em $(TESTS_DIR) (paralelo, $(NPROC) nucleos) =="
	@cd $(ROOT) && find $(TESTS_DIR) -name "*.vrp" | sort | \
		xargs -I{} -P$(NPROC) sh -c 'echo ">>> {}"; ./vrp_teste "{}" --statistics'

clean:
	rm -f $(OBJS) $(TARGET)

help:
	@echo "Alvos:"
	@echo "  all                 - compila o vrp_teste (padrao)"
	@echo "  run-all             - roda $(TESTS_DIR) uma vez cada"
	@echo "  run-stats           - roda $(TESTS_DIR) com --statistics"
	@echo "  run-all-folders     - roda todos .vrp dentro de $(TESTS_ROOT)/"
	@echo "  run-stats-folders   - idem, com --statistics"
	@echo "  run-stats-parallel  - run-stats em paralelo ($(NPROC) nucleos)"
	@echo "  clean               - apaga .o e o binario"