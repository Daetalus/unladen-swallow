// This is the header for a file that contains C code that we want to
// be inlinable in the global LLVM module.  Right now, it only
// contains definitions specifically for LLVM, but eventually we'll
// compile large parts of the runtime into here.  We compile the
// relevant C code with clang into .bc files.  For now there's only
// one of these, but when there are more than one we'll use llvm-link
// to combine them all.  Then we use llc -march=cpp to make C++ code
// that reconstructs the combined .bc file as IR in the global Module.
//
// We assume that clang+llc saves the name of the type corresponding to
//   typedef struct _object { ... } PyObject;
// as "struct._object".  If clang stops doing that in the future, Nick
// suggests that we declare a function:
//   extern void structObject(struct _object);
// in llvm_inline_functions.c, then look up that function and pull the
// object type out of the function's type.

#include "llvm/CallingConv.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Instructions.h"
#include "llvm/Module.h"

#include "llvm/ADT/APInt.h"

using namespace llvm;


// Include code generated by llc -march=cpp.
#include "initial_llvm_module_body.cc"
