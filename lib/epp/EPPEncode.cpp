#define DEBUG_TYPE "epp_encode"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/CFGPrinter.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"

#include "AuxGraph.h"
#include "EPPEncode.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <set>
#include <vector>

using namespace llvm;
using namespace epp;
using namespace std;

extern cl::opt<bool> dumpGraphs;

bool EPPEncode::doInitialization(Module &m) { return false; }
bool EPPEncode::doFinalization(Module &m) { return false; }

namespace {

void printCFG(Function &F) {
    legacy::FunctionPassManager FPM(F.getParent());
    FPM.add(llvm::createCFGPrinterLegacyPassPass());
    FPM.doInitialization();
    FPM.run(F);
    FPM.doFinalization();
}

void dumpDotGraph(StringRef filename, const AuxGraph &AG) {
    error_code EC;
    raw_fd_ostream out(filename, EC, sys::fs::F_Text);
    AG.dot(out);
    out.close();
}
}

bool EPPEncode::runOnFunction(Function &F) {
    LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    encode(F);
    return false;
}

void EPPEncode::releaseMemory() {
    LI = nullptr;
    numPaths.clear();
    AG.clear();
}

DenseSet<pair<const BasicBlock *, const BasicBlock *>>
getBackEdges(BasicBlock *StartBB) {
    SmallVector<std::pair<const BasicBlock *, const BasicBlock *>, 8>
        BackEdgesVec;
    FindFunctionBackedges(*StartBB->getParent(), BackEdgesVec);
    DenseSet<pair<const BasicBlock *, const BasicBlock *>> BackEdges;

    for (auto &BE : BackEdgesVec) {
        BackEdges.insert(BE);
    }
    return BackEdges;
}

void EPPEncode::encode(Function &F) {
    DEBUG(errs() << "Called Encode on " << F.getName() << "\n");

    AG.init(F);

    auto *Entry    = &F.getEntryBlock();
    auto BackEdges = getBackEdges(Entry);

    SetVector<std::pair<const BasicBlock *, const BasicBlock *>> SegmentEdges;

    for (auto &BB : AG.nodes()) {
        for (auto S = succ_begin(BB), E = succ_end(BB); S != E; S++) {
            if (BackEdges.count(make_pair(BB, *S)) ||
                LI->getLoopFor(BB) != LI->getLoopFor(*S)) {
                SegmentEdges.insert({BB, *S});
            }
        }
    }

    if (dumpGraphs) {
        dumpDotGraph("auxgraph-1.dot", AG);
    }

    AG.segment(SegmentEdges);

    if (dumpGraphs) {
        dumpDotGraph("auxgraph-2.dot", AG);
    }

    for (auto &B : AG.nodes()) {
        APInt pathCount(64, 0, true);

        auto Succs = AG.succs(B);
        if (Succs.empty()) {
            pathCount = 1;
            assert(
                B->getName().startswith("fake.exit") &&
                "The only block without a successor should be the fake exit");
        } else {
            for (auto &SE : Succs) {
                AG[SE]  = pathCount;
                auto *S = SE->tgt;
                if (numPaths.count(S) == 0)
                    numPaths.insert(make_pair(S, APInt(64, 0, true)));

                // This is the only place we need to check for overflow.
                // If there is an overflow, indicate this by saving 0 as the
                // number of paths from the entry block. This is impossible for
                // a regular CFG where the numpaths from entry would atleast be
                // 1
                // if the entry block is also the exit block.
                bool Ov   = false;
                pathCount = pathCount.sadd_ov(numPaths[S], Ov);
                if (Ov) {
                    numPaths.clear();
                    numPaths.insert(make_pair(Entry, APInt(64, 0, true)));
                    DEBUG(errs()
                          << "Integer Overflow in function " << F.getName());
                    return;
                }
            }
        }

        numPaths.insert({B, pathCount});
    }

    if (dumpGraphs) {
        dumpDotGraph("auxgraph-3.dot", AG);
    }
}

char EPPEncode::ID = 0;
static RegisterPass<EPPEncode> X("", "EPPEncode");
