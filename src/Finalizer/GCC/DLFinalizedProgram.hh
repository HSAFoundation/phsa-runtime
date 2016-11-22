/*
  Copyright (c) 2015-2016 General Processor Tech.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
/**
 * @author pekka.jaaskelainen@parmance.com for General Processor Tech.
 */

#ifndef PHSA_DLFINALIZEDPROGRAM_HH
#define PHSA_DLFINALIZEDPROGRAM_HH

#include <mutex>
#include <cassert>
#include <unordered_map>
#include <vector>
#include <dlfcn.h>
#include "FinalizedProgram.hh"
#include "hsa_ext_finalize.h"

namespace phsa {


// FinalizedProgram implementation for CPU programs finalized to dynamic
// libraries that are loaded to the current process via dlopen() for execution.
class DLFinalizedProgram : public FinalizedProgram {
public:
  DLFinalizedProgram(char *ElfBlob, size_t ElfSize, hsa_isa_t ISA,
                     hsa_machine_model_t MM, hsa_profile_t P,
                     hsa_default_float_rounding_mode_t RM);
  virtual ~DLFinalizedProgram();

  /// Returns the dlopen() handle for the finalized binary, if feasible.
  /// dlopens the .so in case not opened yet.
  void *dlhandle();

  void defineGlobalSymbolAddress(std::string SymbolName,
                                 uint64_t Addr) override;

  /// Returns the address of the symbol in memory. Returns the address in
  /// the current process memory for the CPU/dlopen case.
  uint64_t symbolAddress(std::string SymbolName,
                         Elf64_Sym *Symbol = nullptr) override;

  virtual std::size_t serializedSize() const override;
  virtual void serializeTo(uint8_t *Buffer) const override;
  static DLFinalizedProgram *deserialize(uint8_t *Buffer);

private:
  void *Dlhandle;
};

} // namespace phsa

#endif
