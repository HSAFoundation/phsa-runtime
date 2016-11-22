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

#ifndef PHSA_HSAIL_PROGRAM_HH
#define PHSA_HSAIL_PROGRAM_HH

#include <mutex>
#include <cassert>
#include <unordered_map>
#include <vector>
#include <dlfcn.h>
#include <libelf.h>

#include "HSAObjectMapping.hh"
#include "hsa_ext_finalize.h"

namespace phsa {

// HSAILProgram is a container for BRIG modules that can be finalized. It
// also holds some necessary parameters for the compilation.
class HSAILProgram : public HSAObjectMapping<HSAILProgram, hsa_ext_program_t> {
public:
  HSAILProgram(hsa_machine_model_t MM, hsa_profile_t P,
               hsa_default_float_rounding_mode_t RM);
  virtual ~HSAILProgram();

  bool hasModules() const { return BRIGs.size() > 0; }
  bool hasModule(hsa_ext_module_t module) const;
  void addModule(hsa_ext_module_t module) { BRIGs.push_back(module); }
  size_t moduleCount() { return BRIGs.size(); }
  hsa_ext_module_t module(size_t i) { return BRIGs[i]; }

  hsa_machine_model_t getMachineModel() { return MachineModel; }
  hsa_profile_t getProfile() { return Profile; }
  hsa_default_float_rounding_mode_t getDefaultRoundingMode() {
    return DefaultRoundingMode;
  }

  void addTempFile(std::string TempFile) { TempFiles.push_back(TempFile); }

  bool dumpBRIGS(std::string TempDir, std::vector<std::string> &Paths);

protected:
  std::vector<hsa_ext_module_t> BRIGs;
  std::vector<std::string> TempFiles;

  hsa_machine_model_t MachineModel;
  hsa_profile_t Profile;
  hsa_default_float_rounding_mode_t DefaultRoundingMode;
};
}

#define PHSA_HOST_DEF_PTR_PREFIX "__phsa.host_def."

#endif
