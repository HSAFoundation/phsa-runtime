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
 *
 * @author tomi.aijo@parmance.com for General Processor Tech.
 */

#ifndef HSA_RUNTIME_GCCFINALIZER_HH
#define HSA_RUNTIME_GCCFINALIZER_HH

#include <string>

#include "Finalizer.hh"

#define PHSA_RT_LIBNAME "phsa-rt"
#define RT_SOURCES 0

namespace phsa {

class GCCFinalizer : public Finalizer {
public:
  GCCFinalizer(std::string GCCBinaryName = "gccbrig")
      : GCCBinary(GCCBinaryName) {}

  virtual HSAReturnValue<hsa_code_object_s>
  finalizeProgram(hsa_ext_program_t Program,
                  hsa_ext_control_directives_t ControlDirectives,
                  hsa_isa_t VendorCompilerOptions, const char *string) override;

private:
  std::string GCCBinary;
};

}

#endif // HSA_RUNTIME_GCCFINALIZER_HH
