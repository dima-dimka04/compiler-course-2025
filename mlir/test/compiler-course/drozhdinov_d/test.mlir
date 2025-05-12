// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/LoopTracePass_DrozhdinovD_FIIT1_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(LoopTracePass)" %s | FileCheck %s

module {
  // CHECK: func.func private @trace_loop_iter_end()
  // CHECK-NEXT: func.func private @trace_loop_iter_begin()

  // CHECK-LABEL: func.func @affine_for()
  // CHECK: func.call @trace_loop_iter_begin() : () -> ()
  // CHECK-NEXT: %1 = arith.index_cast %arg0 : index to i32
  // CHECK-NEXT: %2 = arith.addi %arg1, %1 : i32
  // CHECK-NEXT: func.call @trace_loop_iter_end() : () -> ()
  func.func @affine_for() -> i32 {
    %sum_init = arith.constant 0 : i32
    %result = affine.for %i = 0 to 10 iter_args(%sum_iter = %sum_init) -> i32 {
      %i_i32 = arith.index_cast %i : index to i32
      %new_sum = arith.addi %sum_iter, %i_i32 : i32
      affine.yield %new_sum : i32
    }
    return %result : i32
  }
  
  /// CHECK-LABEL: func.func @scf_while(%arg0: index)
  func.func @scf_while(%arg0: index) {
    // CHECK:   func.call @trace_loop_iter_begin() : () -> ()
    // CHECK:   func.call @trace_loop_iter_end() : () -> ()
    %one   = arith.constant 1 : index
    %res = scf.while (%i = %arg0) : (index) -> (index) {
      %limit = arith.constant 10 : index
      %cond  = arith.cmpi slt, %i, %limit : index
      scf.condition(%cond) %i : index
    } do {
      ^bb0(%i_curr: index):
        %v   = arith.constant 5 : i32 
        %inc = arith.addi %i_curr, %one : index
        scf.yield %inc : index 
    }
    return
  }
  
}
