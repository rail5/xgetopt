# XGetOpt test suite (GNU make)

CXX ?= g++
CXXFLAGS ?= -Wall -Wextra -Wpedantic -O0 -g

# Ensure we ALWAYS compile with gnu++20 standard
CXXFLAGS += -std=gnu++20

# Optional: enable with `make SAN=1`
ifeq ($(SAN),1)
CXXFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
LDFLAGS  += -fsanitize=address,undefined
endif

TARGET := xgetopt_tests
SOURCES := test_main.cpp

.PHONY: all clean run check

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -I.. $(SOURCES) -o $@ $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

check: run

clean:
	rm -f $(TARGET)
