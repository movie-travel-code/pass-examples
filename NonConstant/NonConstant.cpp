// Description:
//  Visits all function in a module, collect the calls which accepts format-
//  string routine. If it is, retrieve the first parameter and check whether
//  it is a non constant global. If not, raise an alert.
//
// This file borrowed from 2019 LLVM Developers’ Meeting: A. Warzynski “Writing
// an LLVM Pass: 101”.
//
// Strictly speaking, this is an analysis pass (i.e. the functions are not
// modified).

#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

// No need to expose the internal of the pass to the outside world - keep
// everything in an anonymous namespace.
namespace {
// New pass manager implementation
struct NonConstant : PassInfoMixin<NonConstant> {
  // Main entry point, takes IR unit to run the pass on (&F) and the
  // corresponding pass manager (to be queried if need be)
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    for (auto &BB : F) {
      runOnBasicBlock(BB);
    }
    return PreservedAnalyses::all();
  }

  void runOnBasicBlock(BasicBlock &BB) {
    for (auto Inst = BB.begin(), IE = BB.end(); Inst != IE; ++Inst) {
      // skip non-call instructions.
      auto *inst = dyn_cast<CallInst>(Inst);
      if (!inst)
        continue;
      if (inst->getCalledFunction()->getName() != "printf" &&
          inst->getCalledFunction()->getName() != "printf")
        continue;
      auto *op = dyn_cast<GlobalValue>(inst->getOperand(0));
      if (!op) {
        errs() << "Function: " << inst->getCalledFunction()->getName()
               << " has non constant format string"
               << "\n";
      }
    }
  }
};

//------------------------------------------------------------------------------
// New PM Registration
//------------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getNonConstantPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "NonConstant", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "non-constant") {
                    FPM.addPass(NonConstant());
                    return true;
                  }
                  return false;
                });
          }};
}

// This is the core interface for pass plugins - with this 'opt' will be able
// to recoganize NonConstant when added to the pass pipeline on the command
// line, i.e. via '-pass=hello-world'
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getNonConstantPluginInfo();
}
} // namespace