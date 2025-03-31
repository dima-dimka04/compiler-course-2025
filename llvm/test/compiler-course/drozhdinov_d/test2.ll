; RUN: opt -load-pass-plugin %llvmshlibdir/CallFunctionPass_DrozhdinovD_FIIT1_LLVM_IR%pluginext\
; RUN: -passes=callfunc -S %s | FileCheck %s

; CHECK: define double @dadd(double %a, double %b)
; CHECK: %result = fadd double %a, %b
; CHECK: ret double %result

; CHECK: define i32 @foo1(i32 %x, i32 %y)
; CHECK: %sum = add i32 %x, %y
; CHECK: ret i32 %sum

; CHECK-NOT: add i32

define double @dadd(double %a, double %b) {
entry:
  %result = fadd double %a, %b
  ret double %result
}

define i32 @foo1(i32 %x, i32 %y) {
entry:
  %sum = add i32 %x, %y
  ret i32 %sum
}
