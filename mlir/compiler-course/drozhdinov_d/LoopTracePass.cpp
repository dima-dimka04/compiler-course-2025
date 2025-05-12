#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
class LoopTracePass
    : public mlir::PassWrapper<LoopTracePass, mlir::OperationPass<mlir::ModuleOp>> {
public:
  mlir::StringRef getArgument() const final { return "LoopTracePass"; }

  mlir::StringRef getDescription() const final {
    return "Insert function calls in begin and end of the cycles";
  }

  void runOnOperation() override {
    mlir::ModuleOp module = getOperation();
    mlir::OpBuilder builder(module.getContext());

    auto declareTraceFunc = [&](mlir::StringRef name) {
      if (!module.lookupSymbol<mlir::func::FuncOp>(name)) {
        builder.setInsertionPointToStart(module.getBody());
        auto func = builder.create<mlir::func::FuncOp>(
            module.getLoc(), name, builder.getFunctionType({}, {}));
        func.setSymVisibility("private");
      }
    };

    declareTraceFunc("trace_loop_iter_begin");
    declareTraceFunc("trace_loop_iter_end");

    module.walk([&](mlir::Operation *op) {
      if (llvm::isa<mlir::scf::ForOp, mlir::scf::WhileOp,
                    mlir::scf::ParallelOp, mlir::affine::AffineForOp>(op)) {
        mlir::Location loc = op->getLoc();
        handleLoopOp(op, loc, builder);
      }
    });
  }
 private:
  static void createcalls(mlir::Block &block, mlir::Location loc,
                                        mlir::OpBuilder &builder) {
    builder.setInsertionPointToStart(&block);
    builder.create<mlir::func::CallOp>(loc, "trace_loop_iter_begin", mlir::TypeRange{}, mlir::ValueRange{});

    mlir::Operation *term = block.getTerminator();
    builder.setInsertionPoint(term);
    builder.create<mlir::func::CallOp>(loc, "trace_loop_iter_end", mlir::TypeRange{}, mlir::ValueRange{});
  }

  static void handleLoopOp(mlir::Operation *op, mlir::Location loc, mlir::OpBuilder &builder) {
    if (auto affineFor = mlir::dyn_cast<mlir::affine::AffineForOp>(op)) {
      createcalls(*affineFor.getBody(), loc, builder);
    } else if (auto scfFor = mlir::dyn_cast<mlir::scf::ForOp>(op)) {
      createcalls(*scfFor.getBody(), loc, builder);
    } else if (auto scfWhile = mlir::dyn_cast<mlir::scf::WhileOp>(op)) {
      createcalls(scfWhile.getAfter().front(), loc, builder);
    }
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(LoopTracePass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(LoopTracePass)

mlir::PassPluginLibraryInfo getTraceLoopPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "LoopTracePass", "1.0",
          []() { mlir::PassRegistration<LoopTracePass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getTraceLoopPassPluginInfo();
}
