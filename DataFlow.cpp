// 15-745 S13 Assignment 1: FunctionInfo.cpp
// 
// Based on code from Todd C. Mowry
// Modified by Arthur Peters
////////////////////////////////////////////////////////////////////////////////

#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"

#include "llvm/ADT/ValueMap.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/Support/CFG.h"



#include <ostream>
#include <fstream>
#include <iostream>

using namespace llvm;

namespace {
//namespace llvm {}

	class BasicBlockInfo {
		public:
			BitVector *gen;
			BitVector *kill;
			BitVector *in;
			BitVector *out;
			BasicBlock *BB;
			BasicBlockInfo(BasicBlock *BB, unsigned len) {
				this->BB = BB;
				gen = new BitVector(len);
				kill = new BitVector(len);
				in = new BitVector(len);
				out = new BitVector(len);
			}
	};

	template<class T>
	class DataFlow {
		public:
			//ValueMap<T, unsigned> DomainMap...
			//ValueMap<BasicBlock *, BasicBlockInfo> BBMap...

/*

		T a;
		*/
		DataFlow() {
		}
	
		/*
		DataFlow(std::vector<T> domain, bool forward) {

		}
		*/

		void analysis(std::vector<T> domain, Function& F, bool forward) {
			//Gen, Kill
			//Block in, out
			//Instruction in, out

			//Domain????
			//how many blocks....each block has a BasicBlockInfo class...
			//map the domain entry to index
			ValueMap<T, unsigned> domainToIdx;
			for (int i = 0; i < domain.size(); ++i) {
				domainToIdx[domain[i]] = i;
			}

			//Boundary Condition, and the top....
			//We don't need to calc top, since we have guarantee the meetOp....top meet any = any....

			ValueMap<BasicBlock *, BasicBlockInfo *> BBtoInfo;
			//...........generate the BBtoInfo............
			for (Function::iterator Bi = F.begin(), Be = F.end(); Bi != Be; ++Bi) {
				//init the BBtoInfo
				BBtoInfo[&*Bi] = new BasicBlockInfo(domain.size());
			}

			for (Function::iterator Bi = F.begin(), Be = F.end(); Bi != Be; ++Bi) {
				//init the BBtoInfo
				BBtoInfo[&*Bi]->in = initFlowValues(domain.size());
			}

			BBtoInfo[&(F.back())]->in = getBoundaryCondition(domain.size());

			//init the flow map
			//


			//init the gen and kill set
			//GenSet;
			//KillSet;
			initGenKill(domain.size(), F, domainToIdx, BBtoInfo);
			//for (Function::iterator Bi = F.begin(), Be = F.end(); Bi != Be; ++Bi) {}


			//first implement with the ischanged flag, then transform it into the WorkList form...Note: 1. set or list 2. bfs, guarantee to traverse all the node from bottom to top at least once.
			bool isChanged = true;
			while (isChanged) {
				isChanged = false;
				for (Function::iterator Bi = F.begin(), Be = F.end(); Bi != Be; ++Bi) {
					// for liveness: out[B] = U(in[B], mask[B][Succ]
					//first, we don't consider the mask...
					//we don't consider out and in combination yet....
					//ValueMap<
					std::vector<BitVector *> meetInput;
					for (succ_iterator succIt = succ_begin(Bi), succE = succ_end(Bi); succIt != succE; ++succIt) {
						BasicBlock *BB = *succIt;
						meetInput.push_back(BBtoInfo[BB]->in);
					}
					//delete BBtoInfo[&*Bi]->out???;
					BBtoInfo[&*Bi]->out = meetOp(meetInput);


					BitVector *oldOut = BBtoInfo[&*Bi]->in;

					//delete BBtoInfo[&*Bi]->in???;
					BBtoInfo[&*Bi]->in = transferFunc(BBtoInfo[&*Bi]->out, BBtoInfo[&*Bi]->gen, BBtoInfo[&*Bi]->kill); 

					BVprint(*oldOut);
					BVprint(*(BBtoInfo[&*Bi]->in));

					if (isChanged && *oldOut != *(BBtoInfo[&*Bi]->in)) {
						isChanged = true;
					}

				}
			}
		}

		//print the Bitvector
		void BVprint(BitVector BV) {
			for (int i = 0; i < BV.size(); ++i) {
				errs() << (BV[i]?"1":"0");
			}

			errs() << "\n";

		}




		//flowmap

		virtual void initGenKill(int len, Function &F, ValueMap<T, unsigned> domainToIdx, BasicBlockInfo *BBtoInfo) = 0;
		virtual BitVector* meetOp(std::vector<BitVector *> input) = 0;

		virtual BitVector* transferFunc(BitVector *input, BitVector *gen, BitVector *kill) = 0;

		//Notice: BoundatryCondition for the Reaching Def may vary...argu -> true; instruction -> false....initially
		virtual BitVector* getBoundaryCondition(int len) = 0;
		virtual BitVector* initFlowValues(int len) = 0;

		virtual void initGen() = 0;
		virtual void initKill() = 0;

	};
}
