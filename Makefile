## 
## HSM lib test Makefile
## -------------------------------------------------------
#  $Id: Makefile 187 2012-01-27 15:57:50Z mlange $
#  -------------------------------------------------------
export LD_LIBRARY_PATH=/usr/local/lib/
APP=shuffle
ORIGINALAPP=test
LIBAPP=libshuffle.so
VUVUZELA_APP=vuvuzela_shuffle
VUVUZELA_SERVER_APP=shuffle_server
VUVUZELA_CLIENT_APP=shuffle_client

# options
DATE  = $(shell /bin/date +%d.%m.%y)
_DATE = $(shell /bin/date +%d_%m_%y)

RD    = "\033[0;31m"
NC    = "\033[0m"


#directories
PRJDIR        = $(shell pwd)
OBJDIR        = obj
LCOVDIR       = lcov


INC_DIR   = src
SRC_DIR   = src


VPATH = $(SRC_DIR)

OPTIMIZE_FLAGS=-O2 -flto
# OPTIMIZE_FLAGS=-O0

# tools 
CC = gcc -std=c++0x -fPIC
CXX = g++ -std=c++0x -fPIC

# compiler / linker flags
CFLAGS= \
        $(OPTIMIZE_FLAGS) \
        -I $(INC_DIR)\
		-Wall -g\
		-fopenmp
CXXFLAGS=\
	$(OPTIMIZE_FLAGS) \
        -I $(INC_DIR)\
		-Wall -g\
		-fopenmp

LDFLAGS= $(OPTIMIZE_FLAGS)

# libraries to link against
LIBS +=  -L/usr/local/lib/ -lntl -lgmp -lboost_system  -lboost_filesystem -lpthread -lboost_regex -lboost_thread -lboost_context -lgomp

# source and header files
ORIGINAL = \
	test\
	Cipher_elg\
	CurvePoint\
	ElGammal\
	FakeZZ\
	func_pro\
	func_ver\
	Functions\
	G_q\
	Mod_p\
	multi_expo\
	Pedersen\
	Permutation\
	Prover_toom\
	SchnorrProof\
	Verifier_toom\
	CipherTable \
	Utils \
	sha256\
	NIZKProof\
	RemoteShuffler\
	VerifierClient

TESTFILES = \
	Cipher_elg\
	CurvePoint\
	ElGammal\
	FakeZZ\
	func_pro\
	func_ver\
	Functions\
	G_q\
	Mod_p\
	multi_expo\
	Pedersen\
	Permutation\
	Prover_toom\
	SchnorrProof\
	Verifier_toom\
	CipherTable \
	Utils \
	sha256\
	NIZKProof\
	RemoteShuffler\
	VerifierClient

TESTOBJECTS = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(TESTFILES)))
ORIGINALOBJECTS = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(ORIGINAL)))
LIBOBJECTS = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(LIBFILES)))
TESTDEPENDS = $(addprefix $(OBJDIR)/, $(addsuffix .d, $(TESTFILES)))
ORIGINALDEPENDS = $(addprefix $(OBJDIR)/, $(addsuffix .d, $(ORIGINAL)))
LIBDEPENDS = $(addprefix $(OBJDIR)/, $(addsuffix .d, $(LIBFILES)))

#--- how to create object and dependencie files from sources ---
$(OBJDIR)/%.o:%.cpp
	@echo $(RD)"    compiling $<"$(NC)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo $(RD)"    creating dependencies for $<"$(NC)
	$(CXX) $(CXXFLAGS) -MM -MT $(OBJDIR)/$*.o $<  >$(OBJDIR)/$*.d

# all
## make all  - creates software
# -------------------------------------------------------
#all: $(OBJDIR) $(OBJECTS) 
#	@echo $(RD)"    linking object files"$(NC)
#	$(CXX) $(LDFLAGS) -o $(APP) $(OBJECTS) $(LIBS)
#
#-include $(DEPENDS)

lib: $(OBJDIR) $(TESTOBJECTS) 
	@echo $(RD)"    linking object files"$(NC)
	$(CXX) $(LDFLAGS) -shared -o $(LIBAPP) $(TESTOBJECTS) $(LIBS)
	cp src/Utils.h ./Utils.h
	cp src/keys.h ./keys.h
-include $(TESTDEPENDS)

test: $(OBJDIR) $(ORIGINALOBJECTS) 
	@echo $(RD)"    linking object files"$(NC)
	$(CXX) $(LDFLAGS) -o $(ORIGINALAPP) $(ORIGINALOBJECTS) $(LIBS)

-include $(ORIGINALDEPENDS)

# create directories
$(OBJDIR):
	@echo $(RD)"    create $@"$(NC)
	@mkdir -p $@
	@chmod 775 $@


.PHONY: clean
clean:
	@rm -rf $(OBJDIR) $(LCOVDIR) $(APP)

# help
## make help - print this help
# -------------------------------------------------------
.PHONY: help
help:
	@sed -n '/^##/s/## //p' Makefile
