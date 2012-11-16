
CXX = g++
CFLAGS = -fno-rtti

LLVM_BUILD_PATH = $(HOME)/clang/bin-install
LLVM_BIN_PATH = $(LLVM_BUILD_PATH)/bin

LLVM_LIBS=core mc
LLVM_CONFIG_FLAGS = $(shell $(LLVM_BIN_PATH)/llvm-config --cxxflags --ldflags \
                                        --libs $(LLVM_LIBS))

CLANG_BUILD_FLAGS = -I$(LLVM_BUILD_PATH)/include 

CLANGLIBS = \
  -lclangFrontendTool -lclangFrontend -lclangDriver \
  -lclangSerialization -lclangCodeGen -lclangParse \
  -lclangSema -lclangStaticAnalyzerFrontend \
  -lclangStaticAnalyzerCheckers -lclangStaticAnalyzerCore \
  -lclangAnalysis -lclangARCMigrate -lclangRewrite \
  -lclangEdit -lclangAST -lclangLex -lclangBasic

all: rewritersample

rewritersample: rewritersample.cpp
	$(CXX) rewritersample.cpp $(CFLAGS) -o rewritersample \
              $(CLANG_BUILD_FLAGS) $(CLANGLIBS) \
              $(LLVM_CONFIG_FLAGS)

clean:
	rm -rf *.o *.ll rewritersample

