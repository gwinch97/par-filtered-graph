# Determine the operating system
OS := $(shell uname -s)

# Compiler and linker flags
CC := g++
CCFLAGS := -O3 -Wall -std=c++17 -mcx16 # -DPROFILE -DPARLAY_SEQUENTIAL # -fcilkplus -DCILKP -DPARLAY_CILK

# Platform-specific linker flags
ifeq ($(OS), Linux)
    PLFLAGS := -ldl -pthread $(SDSLLF) # -fcilkplus
else
    PLFLAGS := -pthread $(SDSLLF) # -fcilkplus
endif

# Include directories
PARLAY_DIR := ../parlaylib/include
BOOST_ROOT := C:/boost_1_85_0/boost_1_85_0
INCLUDE := -I$(PARLAY_DIR) -I$(BOOST_ROOT)

# Source files and object files
SRC := tmfg.cpp partmfg.cpp partmfg_double.cpp
NNOBJECTS := $(SRC:.cpp=.o)

# Default target
tmfg: $(NNOBJECTS) Makefile
	$(CC) $(PLFLAGS) $(NNOBJECTS) -o $@

# Object file targets
tmfg.o: $(SRC) dbht.h linkage.h chain.h partmfg.h
	$(CC) $(INCLUDE) $(CCFLAGS) -c $(SRC)

# Debug target
debug: CCFLAGS = -g -Wall -std=c++17 -I$(PARLAY_DIR) -DDEBUG -mcx16
# -fcilkplus -DCILKP -DPARLAY_CILK -DPARLAY_USE_STD_ALLOC -DPARLAY_SEQUENTIAL
debug: tmfg

# Clean target
clean:
	rm -f tmfg *.o
