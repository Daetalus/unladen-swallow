; RUN: llvm-as < %s | llc 
; RUN: llvm-as < %s | llc -fast-isel
; RUN: llvm-as < %s | llc -march=x86-64
; RUN: llvm-as < %s | llc -fast-isel -march=x86-64
; PR4466

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:128:128"
target triple = "i386-apple-darwin9.7"

define i32 @main() nounwind {
entry:
	%0 = fcmp oeq float undef, 0x7FF0000000000000		; <i1> [#uses=1]
	%1 = zext i1 %0 to i32		; <i32> [#uses=1]
	store i32 %1, i32* undef, align 4
	ret i32 undef
}
