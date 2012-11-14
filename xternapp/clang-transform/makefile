
CXX = g++
CFLAGS = -fno-rtti

LLVM_SRC_PATH = ___PATH-TO-LLVM-SOURCE-DIR___
LLVM_BUILD_PATH = ___PATH-TO-LLVM-BUILD-DIR___

LLVM_BIN_PATH = $(LLVM_BUILD_PATH)/Debug+Asserts/bin
LLVM_LIBS=core mc
LLVM_CONFIG_COMMAND = $(LLVM_BIN_PATH)/llvm-config --cxxflags --ldflags \
                                        --libs $(LLVM_LIBS)
CLANG_BUILD_FLAGS = -I$(LLVM_SRC_PATH)/tools/clang/include \
                                      -I$(LLVM_BUILD_PATH)/tools/clang/include

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
              $(CLANG_BUILD_FLAGS) $(CLANGLIBS) $(LLVM_CONFIG_COMMAND)

clean:
      rm -rf *.o *.ll rewritersample

