
CXXFLAGS := -Wall -g -std=c++14
DEPFLAGS = -MT $@ -MMD -MP -MF $*.d

LLVM_BUILD := /home/mleone/clang
LLVM_INCLUDE := -I$(LLVM_BUILD)/include

LLVM_FLAGS := -DWIN32 -D_WINDOWS -D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_NONSTDC_NO_WARNINGS -D_SCL_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS -DUNICODE -D_UNICODE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACRSO
CXXFLAGS += $(LLVM_INCLUDE) $(LLVM_FLAGS)

LLVM_LIBS := LLVMLTO LLVMObjCARCOpts LLVMSymbolize LLVMDebugInfoPDB LLVMDebugInfoDWARF LLVMXCoreDisassembler LLVMXCoreCodeGen LLVMXCoreDesc LLVMXCoreInfo LLVMXCoreAsmPrinter LLVMSystemZDisassembler LLVMSystemZCodeGen LLVMSystemZAsmParser LLVMSystemZDesc LLVMSystemZInfo LLVMSystemZAsmPrinter LLVMSparcDisassembler LLVMSparcCodeGen LLVMSparcAsmParser LLVMSparcDesc LLVMSparcInfo LLVMSparcAsmPrinter LLVMPowerPCDisassembler LLVMPowerPCCodeGen LLVMPowerPCAsmParser LLVMPowerPCDesc LLVMPowerPCInfo LLVMPowerPCAsmPrinter LLVMNVPTXCodeGen LLVMNVPTXDesc LLVMNVPTXInfo LLVMNVPTXAsmPrinter LLVMMSP430CodeGen LLVMMSP430Desc LLVMMSP430Info LLVMMSP430AsmPrinter LLVMMipsDisassembler LLVMMipsCodeGen LLVMMipsAsmParser LLVMMipsDesc LLVMMipsInfo LLVMMipsAsmPrinter LLVMHexagonDisassembler LLVMHexagonCodeGen LLVMHexagonAsmParser LLVMHexagonDesc LLVMHexagonInfo LLVMCppBackendCodeGen LLVMCppBackendInfo LLVMBPFCodeGen LLVMBPFDesc LLVMBPFInfo LLVMBPFAsmPrinter LLVMARMDisassembler LLVMARMCodeGen LLVMARMAsmParser LLVMARMDesc LLVMARMInfo LLVMARMAsmPrinter LLVMAMDGPUCodeGen LLVMAMDGPUAsmParser LLVMAMDGPUDesc LLVMAMDGPUUtils LLVMAMDGPUInfo LLVMAMDGPUAsmPrinter LLVMAArch64Disassembler LLVMAArch64CodeGen LLVMAArch64AsmParser LLVMAArch64Desc LLVMAArch64Info LLVMAArch64AsmPrinter LLVMAArch64Utils LLVMMIRParser LLVMLibDriver LLVMOption LLVMTableGen LLVMLineEditor LLVMX86Disassembler LLVMX86AsmParser LLVMX86CodeGen LLVMSelectionDAG LLVMAsmPrinter LLVMX86Desc LLVMMCDisassembler LLVMX86Info LLVMX86AsmPrinter LLVMX86Utils LLVMMCJIT LLVMPasses LLVMipo LLVMVectorize LLVMLinker LLVMIRReader LLVMAsmParser LLVMDebugInfoCodeView LLVMInterpreter LLVMCodeGen LLVMScalarOpts LLVMInstCombine LLVMInstrumentation LLVMProfileData LLVMBitWriter LLVMOrcJIT LLVMTransformUtils LLVMExecutionEngine LLVMTarget LLVMAnalysis LLVMRuntimeDyld LLVMObject LLVMMCParser LLVMBitReader LLVMMC LLVMCore LLVMSupport

OTHER_LIBS := -lpthread -ldl

LDFLAGS += -L$(LLVM_BUILD)/lib $(LLVM_LIBS:%=-l%) $(OTHER_LIBS)


RE2C := re2c
RE2C_OPTS :=

SRCS := main.cpp Codegen.cpp Lexer.cpp Parser.cpp Printer.cpp Token.cpp Typechecker.cpp
OBJS := $(SRCS:%.cpp=%.o)
EXE := tutorial

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
