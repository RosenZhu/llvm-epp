#ifndef EPPENCODE_H
#define EPPENCODE_H

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"

#include <map>
#include <unordered_map>

#include "AuxGraph.h"

namespace epp {

struct EPPEncode : public llvm::FunctionPass {

    static char ID;

    llvm::LoopInfo *LI;
    llvm::DenseMap<llvm::BasicBlock *, llvm::APInt> NumPaths;
    AuxGraph AG;

    EPPEncode() : llvm::FunctionPass(ID), LI(nullptr) {}

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
        AU.addRequired<llvm::LoopInfoWrapperPass>();
        AU.setPreservesAll();
    }

    virtual bool runOnFunction(llvm::Function &F) override;
    void encode(llvm::Function &F);
    bool doInitialization(llvm::Module &M) override;
    bool doFinalization(llvm::Module &M) override;
    void releaseMemory() override;
    llvm::StringRef getPassName() const override { return "EPPEncode"; }
};
} // namespace epp
#endif
