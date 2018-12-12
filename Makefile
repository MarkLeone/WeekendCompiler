
CXXFLAGS := -Wall -g -std=c++14
DEPFLAGS = -MT $@ -MMD -MP -MF $*.d

# LLVM := /home/mleone/src/llvm-7.0.0
LLVM := c:/Users/mleone/src/llvm-7.0.0/
LLVM_INCLUDE := -I$(LLVM)/build/include -I$(LLVM)/include

# LLVM_FLAGS := -DWIN32 -D_WINDOWS -D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_NONSTDC_NO_WARNINGS -D_SCL_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS -DUNICODE -D_UNICODE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACRSO
LLVM_FLAGS := -fPIC -fvisibility-inlines-hidden -Werror=date-time -Wall -W -Wno-unused-parameter -Wwrite-strings -Wcast-qual -Wno-missing-field-initializers -pedantic -Wno-long-long -Wno-maybe-uninitialized -Wdelete-non-virtual-dtor -Wno-comment -ffunction-sections -fdata-sections -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS

CXXFLAGS += $(LLVM_INCLUDE) $(LLVM_FLAGS)

LLVM_LIBS := \
	LLVMLTO \
	LLVMPasses \
	LLVMObjCARCOpts \
	LLVMSymbolize \
	LLVMDebugInfoPDB \
	LLVMDebugInfoDWARF \
	LLVMMIRParser \
	LLVMFuzzMutate \
	LLVMCoverage \
	LLVMTableGen \
	LLVMDlltoolDriver \
	LLVMOrcJIT \
	LLVMXCoreDisassembler \
	LLVMXCoreCodeGen \
	LLVMXCoreDesc \
	LLVMXCoreInfo \
	LLVMXCoreAsmPrinter \
	LLVMSystemZDisassembler \
	LLVMSystemZCodeGen \
	LLVMSystemZAsmParser \
	LLVMSystemZDesc \
	LLVMSystemZInfo \
	LLVMSystemZAsmPrinter \
	LLVMAArch64Disassembler \
	LLVMAArch64CodeGen \
	LLVMAArch64AsmParser \
	LLVMAArch64Desc \
	LLVMAArch64Info \
	LLVMAArch64AsmPrinter \
	LLVMAArch64Utils \
	LLVMObjectYAML \
	LLVMLibDriver \
	LLVMOption \
	LLVMWindowsManifest \
	LLVMX86Disassembler \
	LLVMX86AsmParser \
	LLVMX86CodeGen \
	LLVMGlobalISel \
	LLVMSelectionDAG \
	LLVMAsmPrinter \
	LLVMX86Desc \
	LLVMMCDisassembler \
	LLVMX86Info \
	LLVMX86AsmPrinter \
	LLVMX86Utils \
	LLVMMCJIT \
	LLVMLineEditor \
	LLVMInterpreter \
	LLVMExecutionEngine \
	LLVMRuntimeDyld \
	LLVMCodeGen \
	LLVMTarget \
	LLVMCoroutines \
	LLVMipo \
	LLVMInstrumentation \
	LLVMVectorize \
	LLVMScalarOpts \
	LLVMLinker \
	LLVMIRReader \
	LLVMAsmParser \
	LLVMInstCombine \
	LLVMBitWriter \
	LLVMAggressiveInstCombine \
	LLVMTransformUtils \
	LLVMAnalysis \
	LLVMProfileData \
	LLVMObject \
	LLVMMCParser \
	LLVMMC \
	LLVMDebugInfoCodeView \
	LLVMDebugInfoMSF \
	LLVMBitReader \
	LLVMCore \
	LLVMBinaryFormat \
	LLVMSupport \
	LLVMDemangle

OTHER_LIBS := -lz -ltinfo -lpthread -ldl

LDFLAGS += -L$(LLVM)/build/lib $(LLVM_LIBS:%=-l%) $(OTHER_LIBS)

RE2C := re2c
RE2C_OPTS :=

SRCS := main.cpp Codegen.cpp Lexer.cpp Parser.cpp Printer.cpp Token.cpp Typechecker.cpp
OBJS := $(SRCS:%.cpp=%.o)
EXE := weekend

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.cpp: %.re
	$(RE2C) $(RE2C_OPTS) $< -o $@

%.o: %.cpp
	$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c $<

.PHONY: clean
clean:
	$(RM) $(OBJS) $(EXE) Lexer.cpp *.d

sinclude $(SRCS:%.cpp=%.d)
