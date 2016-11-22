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

#include <boost/filesystem.hpp>
#include <mutex>
#include <dlfcn.h>
#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <Brig.h>

#include "common/Debug.hh"
#include "common/Logging.hh"
#include "HSAILProgram.hh"

namespace phsa {

HSAILProgram::HSAILProgram(hsa_machine_model_t MM, hsa_profile_t P,
                           hsa_default_float_rounding_mode_t RM)
    : MachineModel(MM), Profile(P), DefaultRoundingMode(RM) {}

HSAILProgram::~HSAILProgram() {
  if (!IsDebugMode()) {
    for (std::string FName : TempFiles) {
      unlink(FName.c_str());
    }
  }
}

bool HSAILProgram::hasModule(hsa_ext_module_t module) const {
  return std::find(BRIGs.begin(), BRIGs.end(), module) != BRIGs.end();
}

// Dump all BRIGs to temporary files.
bool HSAILProgram::dumpBRIGS(std::string TempDir,
                             std::vector<std::string> &Paths) {

  const bool DEBUG_MODE = phsa::IsDebugMode();

  for (size_t i = 0; i < moduleCount(); ++i) {

    const BrigModuleHeader *Header = (const BrigModuleHeader *)module(i);
    std::string BrigPath;
    BrigPath = (boost::filesystem::unique_path(std::string(TempDir.c_str()) +
                                               "/%%%%%%%%%%%%.brig"))
                   .string();
    std::ofstream Outfile(BrigPath.c_str(), std::ofstream::binary);

    if (!Outfile.is_open())
      return false;

    // Dump the in-memory BRIG to a temp file, trust that its size header is OK
    // for now. What else can we do here?
    Outfile.write((char *)Header, Header->byteCount);
    Outfile.close();

    addTempFile(BrigPath);
    Paths.push_back(BrigPath);
  }
  return true;
}

} // namespace phsa

