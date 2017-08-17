/*
    Copyright (c) 2016 General Processor Tech.
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
    deal in the Software without restriction, including without limitation the
    rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
    sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/
/**
 * @author tomi.aijo@parmance.com for General Processor Tech.
 */

#ifndef HSA_RUNTIME_EXECUTABLE_HH
#define HSA_RUNTIME_EXECUTABLE_HH

#include <list>
#include <string>
#include <unordered_map>

#include "hsa.h"
#include "HSAObjectMapping.hh"
#include "HSAReturnValue.hh"

namespace phsa {

class Agent;

// Symbol hierarchy models the HSA symbol classes and their properties,
// provides mapping for their HSA counterparts.
struct Symbol : public HSAObjectMapping<Symbol, hsa_executable_symbol_t> {
  std::string Name;
  hsa_symbol_kind_t Type;
  std::string ModuleName;
  phsa::Agent *Agent;
  hsa_symbol_linkage_t Linkage;
  bool IsDefinition;
};

struct Variable : public Symbol {
  void *Address;
  hsa_variable_allocation_t Allocation;
  hsa_variable_segment_t Segment;
  uint32_t Alignment;
  uint32_t Size;
  bool IsConst;
};

struct Kernel : public Symbol {
  uint64_t Object;
  void *Address;
  uint32_t KernargSegmentSize;
  uint32_t KernargSegmentAlignment;
  uint32_t GroupSegmentSize;
  uint32_t PrivateSegmentSize;
  bool DynamicCallStack;
  void *ImplementationData; /* Implementation specific data. */
};

struct IndirectFunctionCall : public Symbol {
#ifdef HSA_LARGE_MODEL
  uint64_t FunctionObject;
#else
  uint32_t FunctionObject;
#endif
  uint32_t FunctionCallConvention;
};

// Executable is a base class that holds the code objects and defined symbols.
// Subclasses must implement the code object loading. In addition, the symbols
// of the loaded code objects must be registered using the protected
// interface. Sub classes may use the `DefinedSymbols` index to query
// for the defined symbol addresses.
//
// See `src/Finalizer/GCC/ELFExecutable.[cc|hh] for an example implementation.
class Executable : public HSAObjectMapping<Executable, hsa_executable_t> {
public:
  Executable(bool IsFrozen) : IsFrozen(IsFrozen) {}

  virtual hsa_profile_t getProfile() const = 0;
  virtual hsa_executable_state_t getExecutableState() const {
    return isFrozen() ? HSA_EXECUTABLE_STATE_FROZEN
                      : HSA_EXECUTABLE_STATE_UNFROZEN;
  }

  virtual HSAReturnValue<> LoadCodeObject(phsa::Agent *Agent,
                                          const hsa_code_object_t CodeObject,
                                          const char *Options) = 0;

  virtual Symbol const *getSymbol(std::string const &Name) const {
    auto It = SymbolsByName.find(Name);
    if (It != SymbolsByName.end()) {
      return It->second;
    }

    return nullptr;
  };

  virtual void freeze() { IsFrozen = true; }

  virtual bool isFrozen() const { return IsFrozen; }

  using symbol_iterator = std::list<Symbol *>::iterator;
  using const_symbol_iterator = std::list<Symbol *>::const_iterator;
  symbol_iterator symbol_begin() { return Symbols.begin(); }
  symbol_iterator symbol_end() { return Symbols.end(); }

  const_symbol_iterator symbol_cbegin() const { return Symbols.cbegin(); }
  const_symbol_iterator symbol_cend() const { return Symbols.cend(); }

  /// Sets the address of a global symbol declaration that is external
  /// in the kernel binary.
  virtual void defineGlobalSymbolAddress(std::string SymbolName,
                                         uint64_t Addr) {
    DefinedSymbols[SymbolName] = Addr;
  }

protected:
  void registerSymbol(Symbol *S) {
    Symbols.push_back(S);
    SymbolsByName[S->Name] = S;
  }

  typedef std::unordered_map<std::string, uint64_t> SymbolAddressIndex;
  // The host defined symbols that should be set when the program
  // is loaded.
  SymbolAddressIndex DefinedSymbols;

private:
  std::list<Symbol *> Symbols;
  std::unordered_map<std::string, Symbol *> SymbolsByName;
  bool IsFrozen;
};

} // namespace phsa

#endif // HSA_RUNTIME_EXECUTABLE_HH
