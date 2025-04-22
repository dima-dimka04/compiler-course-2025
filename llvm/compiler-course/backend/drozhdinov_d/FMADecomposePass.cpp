#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
class FMADecomposePass : public MachineFunctionPass {
public:
  static char ID;
  FMADecomposePass() : MachineFunctionPass(ID) {}
  
  bool runOnMachineFunction(MachineFunction &MF) override {
  	const X86Subtarget &STI = MF.getSubtarget<X86Subtarget>();
  	const X86InstrInfo *TII = STI.getInstrInfo();
  	bool status = false;
  	
  	for (auto &MBB : MF) {
  		for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE;) {
  			MachineInstr &MI = *MII++;
  			
  			unsigned opcode = MI.getOpcode();
  			if (opcode == X86::VFMADD132PSr) { // FMA 128bit
  				Register Dst = MI.getOperand(0).getReg();
  				Register Mul1 = MI.getOperand(1).getReg();
  				Register Mul2 = MI.getOperand(2).getReg();
  				Register Add = MI.getOperand(3).getReg();
  				
  				Register Tmp = MF.getRegInfo().createVirtualRegister(&X86::VR128RegClass);
  				BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(X86::VMULPSrr), Tmp).addReg(Mul1).addReg(Mul2);
  				BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(X86::VMULPSrr), Dst).addReg(Add).addReg(Tmp);
  				
  				MI.eraseFromParent();
  				status = true;
  			}
  		}
  	}
  	return status;
  }
};

char FMADecomposePass::ID = 0;
} // namespace

static RegisterPass<FMADecomposePass> X("fma-decompose", "Decompose FMA into MUL + ADD", false,
                                   false);
