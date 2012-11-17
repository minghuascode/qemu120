
# mkf.inc

LLVM_BUILD_PATH = $(HOME)/clang/bin-install
LLVM_BIN_PATH = $(LLVM_BUILD_PATH)/bin

LLVM_LIBS=core mc
LLVM_CONFIG_FLAGS = $(shell $(LLVM_BIN_PATH)/llvm-config --cxxflags --ldflags \
                                        --libs $(LLVM_LIBS))


CFLAGS := -fno-rtti -I$(TOPROOT)/src -I$(TOPROOT)/inc 
CFLAGS += -I$(LLVM_BUILD_PATH)/include
CFLAGS += $(LLVM_CONFIG_FLAGS)


OBJDIR := $(TOPROOT)/mkobj-dir
SRCOBJDIR := $(OBJDIR)/$(SRCDIR)


#variables available:
# SRCDIR  : from toplevel makefile , on sub make command line
# TOPROOT : from toplevel makefile at the beginning of file
#
# SRCS  : from the sub makefile before including this file

#variables provided by this inc file:
# OBJDIR      top/mkobj-dir
# SRCOBJDIR   top/mkobj-dir/SRCDIR


OBJS := $(SRCS:%.cpp=%.o)


.phony: allsub cleansub

allsub: prelink $(OBJS:%=$(SRCOBJDIR)/%)

$(SRCOBJDIR)/%.o: %.cpp
	@echo CC: $<
	g++ $(CFLAGS) -c $< -o $@

cleansub:
	@echo RM: $(SRCOBJDIR)
	@-rm -r $(SRCOBJDIR)

prelink:
	@if [ ! -d $(SRCOBJDIR) ]; then mkdir -p $(SRCOBJDIR); echo MKDIR: $(SRCOBJDIR); fi

