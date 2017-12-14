#ifndef EPPPROFILE_H
#define EPPPROFILE_H
#include "llvm/ADT/DenseMap.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

#include "EPPEncode.h"

namespace epp {
struct EPPProfile : public llvm::ModulePass {
    static char ID;

    llvm::LoopInfo *LI;
    llvm::DenseMap<llvm::Function *, uint64_t> FunctionIds;

    EPPProfile() : llvm::ModulePass(ID), LI(nullptr) {}

    virtual void getAnalysisUsage(llvm::AnalysisUsage &au) const override {
        // au.addRequired<llvm::LoopInfoWrapperPass>();
        au.addRequired<EPPEncode>();
    }

    virtual bool runOnModule(llvm::Module &m) override;
    void instrument(llvm::Function &F, EPPEncode &E);
    void addCtorsAndDtors(llvm::Module &Mod);

    bool doInitialization(llvm::Module &m) override;
    bool doFinalization(llvm::Module &m) override;
    llvm::StringRef getPassName() const override { return "EPPProfile"; }
};
} // namespace epp

#endif
