//===-- JITMemoryManager.h - Interface JIT uses to Allocate Mem -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the JITMemoryManagerInterface
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_EXECUTION_ENGINE_JIT_MEMMANAGER_H
#define LLVM_EXECUTION_ENGINE_JIT_MEMMANAGER_H

#include "llvm/Support/DataTypes.h"

namespace llvm {
  class Function;

/// JITMemoryManager - This interface is used by the JIT to allocate and manage
/// memory for the code generated by the JIT.  This can be reimplemented by
/// clients that have a strong desire to control how the layout of JIT'd memory
/// works.
class JITMemoryManager {
protected:
  bool HasGOT;
  bool SizeRequired;
public:

  JITMemoryManager() : HasGOT(false), SizeRequired(false) {}
  virtual ~JITMemoryManager();
  
  /// CreateDefaultMemManager - This is used to create the default
  /// JIT Memory Manager if the client does not provide one to the JIT.
  static JITMemoryManager *CreateDefaultMemManager();
  
  /// setMemoryWritable - When code generation is in progress,
  /// the code pages may need permissions changed.
  virtual void setMemoryWritable(void) = 0;

  /// setMemoryExecutable - When code generation is done and we're ready to
  /// start execution, the code pages may need permissions changed.
  virtual void setMemoryExecutable(void) = 0;

  /// setPoisonMemory - Setting this flag to true makes the memory manager
  /// garbage values over freed memory.  This is useful for testing and
  /// debugging, and is be turned on by default in debug mode.
  virtual void setPoisonMemory(bool poison) = 0;

  //===--------------------------------------------------------------------===//
  // Global Offset Table Management
  //===--------------------------------------------------------------------===//

  /// AllocateGOT - If the current table requires a Global Offset Table, this
  /// method is invoked to allocate it.  This method is required to set HasGOT
  /// to true.
  virtual void AllocateGOT() = 0;
  
  /// isManagingGOT - Return true if the AllocateGOT method is called.
  ///
  bool isManagingGOT() const {
    return HasGOT;
  }
  
  /// getGOTBase - If this is managing a Global Offset Table, this method should
  /// return a pointer to its base.
  virtual uint8_t *getGOTBase() const = 0;
  
  /// SetDlsymTable - If the JIT must be able to relocate stubs after they have
  /// been emitted, potentially because they are being copied to a process
  /// where external symbols live at different addresses than in the JITing
  ///  process, allocate a table with sufficient information to do so.
  virtual void SetDlsymTable(void *ptr) = 0;
  
  /// getDlsymTable - If this is managing a table of entries so that stubs to
  /// external symbols can be later relocated, this method should return a
  /// pointer to it.
  virtual void *getDlsymTable() const = 0;
  
  /// NeedsExactSize - If the memory manager requires to know the size of the
  /// objects to be emitted
  bool NeedsExactSize() const {
    return SizeRequired;
  }

  //===--------------------------------------------------------------------===//
  // Main Allocation Functions
  //===--------------------------------------------------------------------===//
  
  /// startFunctionBody - When we start JITing a function, the JIT calls this 
  /// method to allocate a block of free RWX memory, which returns a pointer to
  /// it.  The JIT doesn't know ahead of time how much space it will need to
  /// emit the function, so it doesn't pass in the size.  Instead, this method
  /// is required to pass back a "valid size".  The JIT will be careful to not
  /// write more than the returned ActualSize bytes of memory. 
  virtual uint8_t *startFunctionBody(const Function *F, 
                                     uintptr_t &ActualSize) = 0;
  
  /// allocateStub - This method is called by the JIT to allocate space for a
  /// function stub (used to handle limited branch displacements) while it is
  /// JIT compiling a function.  For example, if foo calls bar, and if bar
  /// either needs to be lazily compiled or is a native function that exists too
  /// far away from the call site to work, this method will be used to make a
  /// thunk for it.  The stub should be "close" to the current function body,
  /// but should not be included in the 'actualsize' returned by
  /// startFunctionBody.
  virtual uint8_t *allocateStub(const GlobalValue* F, unsigned StubSize,
                                unsigned Alignment) = 0;
  
  /// endFunctionBody - This method is called when the JIT is done codegen'ing
  /// the specified function.  At this point we know the size of the JIT
  /// compiled function.  This passes in FunctionStart (which was returned by
  /// the startFunctionBody method) and FunctionEnd which is a pointer to the 
  /// actual end of the function.  This method should mark the space allocated
  /// and remember where it is in case the client wants to deallocate it.
  virtual void endFunctionBody(const Function *F, uint8_t *FunctionStart,
                               uint8_t *FunctionEnd) = 0;

  /// allocateSpace - Allocate a memory block of the given size.
  virtual uint8_t *allocateSpace(intptr_t Size, unsigned Alignment) = 0;

  /// allocateGlobal - Allocate memory for a global.
  virtual uint8_t *allocateGlobal(uintptr_t Size, unsigned Alignment) = 0;

  /// deallocateMemForFunction - Free JIT memory for the specified function.
  /// This is never called when the JIT is currently emitting a function.
  virtual void deallocateMemForFunction(const Function *F) = 0;
  
  /// startExceptionTable - When we finished JITing the function, if exception
  /// handling is set, we emit the exception table.
  virtual uint8_t* startExceptionTable(const Function* F,
                                       uintptr_t &ActualSize) = 0;
  
  /// endExceptionTable - This method is called when the JIT is done emitting
  /// the exception table.
  virtual void endExceptionTable(const Function *F, uint8_t *TableStart,
                                 uint8_t *TableEnd, uint8_t* FrameRegister) = 0;
};

} // end namespace llvm.

#endif
