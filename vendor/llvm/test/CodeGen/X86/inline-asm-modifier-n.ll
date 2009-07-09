; RUN: llvm-as < %s | llc -march=x86 | grep { 37}
; rdar://7008959

define void @bork() nounwind {
entry:
	tail call void asm sideeffect "BORK ${0:n}", "i,~{dirflag},~{fpsr},~{flags}"(i32 -37) nounwind
	ret void
}
