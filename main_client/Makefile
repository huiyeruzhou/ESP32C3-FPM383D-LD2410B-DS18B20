

# Path to the nanopb root directory
NANOPB_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))../components/myrpc/nanopb)
# Path to erpc root directory
ERPC_ROOT := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))../components/myrpc)
# C source code files that are required
NAME = sensor
CRC  += client.cpp $(NAME).pb.cpp ${NANOPB_CORE}            # The main program
#########################################################
# leave following lines unchanged
#########################################################
# Files for the nanopb core
NANOPB_CORE = $(NANOPB_DIR)/pb_encode.c $(NANOPB_DIR)/pb_decode.c $(NANOPB_DIR)/pb_common.c
# Check if we are running on Windows
ifdef windir
WINDOWS = 1
endif
ifdef WINDIR
WINDOWS = 1
endif
# Check whether to use binary version of nanopb_generator or the
# system-supplied python interpreter.
ifneq "$(wildcard $(NANOPB_DIR)/generator-bin)" ""
	# Binary package
	PROTOC = $(NANOPB_DIR)/generator-bin/protoc
	PROTOC_OPTS =
else
	# Source only or git checkout
	PROTOC_OPTS =
	ifdef WINDOWS
	    PROTOC = python $(NANOPB_DIR)/generator/protoc
	else
	    PROTOC = $(NANOPB_DIR)/generator/protoc
	endif
endif

#########################################################
# end
#########################################################

# Rule for building .pb.c and .pb.h
%.pb.cpp %.pb.hpp: %.proto %.options
	$(PROTOC) $(PROTOC_OPTS) --nanopb_out=. $<

%.pb.cpp %.pb.hpp: %.proto
	$(PROTOC) $(PROTOC_OPTS) --nanopb_out=. $<

# Compiler flags to enable all warnings & debug info
CFLAGS = -Wall -Werror -g -O0
CXXFLAGS += "-I$(NANOPB_DIR)" -I$(ERPC_ROOT)/include -g -Wall -Werror -lpthread
# please build the library first
ERPC_LIB=$(ERPC_ROOT)/Debug/Linux/erpc/lib/liberpc.a

.PHONY: all clean
all: client
clean:
	rm -f server client *.pb.cpp *.pb.hpp
# Build rule for the main program
server: $(SRC) $(ERPC_LIB)
	$(CXX) $(CXXFLAGS) -osimple $^

# Build rule for the main program
client: $(CRC) $(ERPC_LIB) 
	$(CXX) $(CXXFLAGS) -osimpleclient $^
