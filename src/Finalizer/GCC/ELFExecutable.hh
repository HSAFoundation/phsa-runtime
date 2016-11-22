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

#ifndef HSA_RUNTIME_ELFEXECUTABLE_HH
#define HSA_RUNTIME_ELFEXECUTABLE_HH

#include <cstring>
#include <libelf.h>
#include <iostream>

#include "Executable.hh"
#include "common/Logging.hh"

#include "gcc-phsa.h"

#define MIN_ALIGNMENT 16u

namespace phsa {

// Handles ELFs produced by the GCC BRIG frontend.
class ELFExecutable : public Executable {
public:
  ELFExecutable(hsa_profile_t Profile, bool IsFrozen)
      : Executable(IsFrozen), Profile(Profile) {}
  virtual ~ELFExecutable() {}

  virtual hsa_profile_t getProfile() const override { return Profile; }

  virtual HSAReturnValue<> LoadCodeObject(phsa::Agent *Agent,
                                          const hsa_code_object_t CodeObject,
                                          const char *Options) override;

private:
  hsa_profile_t Profile;
};

} // namespace phsa

#endif // HSA_RUNTIME_ELFEXECUTABLE_HH
