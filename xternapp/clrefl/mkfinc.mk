
# mkf.inc

LLVM_BUILD_PATH = $(HOME)/clang/bin-install
LLVM_BIN_PATH = $(LLVM_BUILD_PATH)/bin

LLVM_LIBS=core mc
LLVM_CONFIG_CC_FLAGS = $(shell $(LLVM_BIN_PATH)/llvm-config --cxxflags)
LLVM_CONFIG_LD_FLAGS = $(shell $(LLVM_BIN_PATH)/llvm-config --ldflags --libs $(LLVM_LIBS))


CFLAGS := -fno-rtti -I$(TOPROOT)/src -I$(TOPROOT)/inc 
CFLAGS += -I$(LLVM_BUILD_PATH)/include
CFLAGS += $(LLVM_CONFIG_CC_FLAGS)

LDFLAGS := $(LLVM_CONFIG_LD_FLAGS)


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


.phony: allsub allsubexe cleansub

allsub: prelink $(OBJS:%=$(SRCOBJDIR)/%)

$(SRCOBJDIR)/%.o: %.cpp
	@echo CC: $<
	g++ $(CFLAGS) -c $< -o $@

allsubexe: prelink $(OBJS:%=$(SRCOBJDIR)/%)
	@echo EXE: $<
	g++ \
	        $(OBJS:%=$(SRCOBJDIR)/%) \
	        $(SCANLIBS)  \
	        $(CFLAGS) \
	        $(CLANGLIBS) \
	        $(LDFLAGS) \
	        -o $(SRCDIR:%=$(SRCOBJDIR)/%) \

cleansub:
	@echo RM: $(SRCOBJDIR)
	@-rm -r $(SRCOBJDIR)

prelink:
	@if [ ! -d $(SRCOBJDIR) ]; then mkdir -p $(SRCOBJDIR); echo MKDIR: $(SRCOBJDIR); fi

#for linking clReflectScan: clReflectCore clReflectCpp
SCANLIBS := $(wildcard $(OBJDIR)/clReflectCore/*.o)
SCANLIBS += $(wildcard $(OBJDIR)/clReflectCpp/*.o)
#for linking clReflectTest: clReflectUtil
SCANLIBS += $(wildcard $(OBJDIR)/clReflectUtil/*.o)

#from scan cmake file:
#CLANGLIBS := -lclangParse
#CLANGLIBS += -lclangFrontend
#CLANGLIBS += -lclangSema
#CLANGLIBS += -lclangAnalysis
#CLANGLIBS += -lclangLex
#CLANGLIBS += -lclangBasic
#CLANGLIBS += -lclangSerialization
#CLANGLIBS += -lclangDriver
#CLANGLIBS += -lclangAST
#CLANGLIBS += -lclangEdit
 
#from clang/tools/driver/makefile:

CLANGLIBS = \
  -lclangFrontendTool -lclangFrontend -lclangDriver \
  -lclangSerialization -lclangCodeGen -lclangParse \
  -lclangSema -lclangStaticAnalyzerFrontend \
  -lclangStaticAnalyzerCheckers -lclangStaticAnalyzerCore \
  -lclangAnalysis -lclangARCMigrate -lclangRewrite \
  -lclangEdit -lclangAST -lclangLex -lclangBasic

