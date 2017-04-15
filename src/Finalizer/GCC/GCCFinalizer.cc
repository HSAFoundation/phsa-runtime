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
 * @author tomi.aijo@parmance.com and pekka.jaaskelainen@parmance.com
 *         for General Processor Tech.
 */

#include <boost/filesystem.hpp>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>

#include "HSAILProgram.hh"
#include "DLFinalizedProgram.hh"
#include "Brig.h"
#include "common/Logging.hh"
#include "common/Debug.hh"
#include "GCCFinalizer.hh"

namespace phsa {

HSAReturnValue<hsa_code_object_s> GCCFinalizer::finalizeProgram(
    hsa_ext_program_t Program, hsa_ext_control_directives_t ControlDirectives,
    hsa_isa_t ISA, const char *VendorCompilerOptions) {

  // Call gcc's BRIG FE from the command line. There seems not to be a clean
  // library interface for frontend services in gcc which could be used
  // to feed in the in memory BRIG directly. Also, this should avoid force
  // feeding GPL3 to this file (I'm not a lawyer though).
  using namespace boost::filesystem;
  path brigFrontendBin;

  char *brigFrontendBuildDirEnv = std::getenv("PHSA_GCCBRIG_BUILD_DIR");
  if (brigFrontendBuildDirEnv == nullptr) {
    // Assume gccbrig is in PATH.
    brigFrontendBin = GCCBinary;
  } else {
    brigFrontendBin =
        std::string(brigFrontendBuildDirEnv) + "/gcc/" + GCCBinary;
    if (!exists(brigFrontendBin)) {
      std::cerr << "phsa-finalizer: gccbrig binary not found in "
                << brigFrontendBin << "." << std::endl;
      return HSAReturn((hsa_status_t)::HSA_EXT_STATUS_ERROR_FINALIZATION_FAILED,
                       hsa_code_object_t());
    }
  }

  std::string phsaRTFlags;
  phsa::HSAILProgram &prog = *phsa::HSAILProgram::fromHSAObject(Program);

  if (!prog.hasModules()) {
    return HSAReturn((hsa_status_t)::HSA_EXT_STATUS_ERROR_INVALID_PROGRAM,
                     hsa_code_object_t());
  }

  char *compilerTempEnv = std::getenv("PHSA_COMPILER_TEMP_DIR");
  path tempDir;
  if (compilerTempEnv != nullptr)
    tempDir = path(compilerTempEnv);
  else {
    char *tempDirC = nullptr;
    char *dirTemplate = strdup("/tmp/phsa-finalizer-XXXXXX");
    if ((tempDirC = mkdtemp(dirTemplate)) == nullptr) {
      std::cerr << "phsa-finalizer: could not create a compiler temp directory."
                << std::endl;
      return HSAReturn((hsa_status_t)::HSA_EXT_STATUS_ERROR_FINALIZATION_FAILED,
                       hsa_code_object_t());
    }
    tempDir = path(tempDirC);
  }

  create_directories(tempDir);
  std::string brigPaths;

  const bool DEBUG_MODE = phsa::IsDebugMode();

  std::vector<std::string> BRIGPaths;
  if (!prog.dumpBRIGS(tempDir.string(), BRIGPaths))
    return HSAReturn((hsa_status_t)::HSA_EXT_STATUS_ERROR_FINALIZATION_FAILED,
                     hsa_code_object_t());

  std::string LastBrigFile;
  std::string BrigFiles;
  for (std::string BrigPath : BRIGPaths) {
    BrigFiles += BrigPath + " ";
    LastBrigFile = BrigPath;
  }

  std::string dynObjectPath = LastBrigFile + ".so";
  std::string compileCmd = brigFrontendBin.string() +
                           " -frounding-math -fno-use-linker-plugin -lm -fPIC " +
                           BrigFiles + " " + phsaRTFlags +
                           " -shared -fPIC -o " + dynObjectPath + " ";

  if (DEBUG_MODE) {
    compileCmd += " -v ";
  }

  char *ldflags = std::getenv("LDFLAGS");
  if (ldflags != nullptr)
    compileCmd += std::string(ldflags) + " ";

  if (RT_SOURCES) {
    compileCmd +=
        std::string(" -I") + brigFrontendBuildDirEnv + "/gcc/include" + " ";
    compileCmd += std::string(" -I") + brigFrontendBuildDirEnv +
                  "/gcc/include-fixed" + " ";

    char *phsaRTIncDir = std::getenv("PHSA_RUNTIME_INC_DIR");
    if (phsaRTIncDir != nullptr) {
      compileCmd += std::string(" -I") + std::string(phsaRTIncDir) + " ";
    }
    compileCmd += "-flto ";
  }

  char *additionalFlags = std::getenv("PHSA_COMPILER_FLAGS");
  if (additionalFlags != nullptr)
    compileCmd += std::string(additionalFlags);

  if (DEBUG_MODE) {
    compileCmd += " -fdump-tree-all -save-temps ";
    std::cout << "phsa-finalizer: running: " << compileCmd << std::endl;
  }
  int status = std::system(compileCmd.c_str());
  if (status == -1 || WEXITSTATUS(status) != 0) {
    std::cerr << "phsa-finalizer: command failed: " << compileCmd << std::endl;
    return HSAReturn((hsa_status_t)::HSA_EXT_STATUS_ERROR_FINALIZATION_FAILED,
                     hsa_code_object_t());
  }

  // Read in the produced ELF .so. The rest is taken care of by
  // Portable-HSA-Runtime.
  std::ifstream elf;
  elf.open(dynObjectPath.c_str(), std::ios::binary);

  // Get length of the file.
  elf.seekg(0, std::ios::end);
  size_t length = elf.tellg();
  elf.seekg(0, std::ios::beg);

  char *elfBlob = new char[length];
  elf.read(elfBlob, length);
  elf.close();

  phsa::DLFinalizedProgram *finalizedProg = new phsa::DLFinalizedProgram(
      elfBlob, length, ISA, prog.getMachineModel(), prog.getProfile(),
      prog.getDefaultRoundingMode());

  finalizedProg->setBinFileName(dynObjectPath);

  if (!finalizedProg->loadAndCheckControlDirectives(ControlDirectives)) {
    delete finalizedProg;
    return HSAReturn((hsa_status_t)::HSA_EXT_STATUS_ERROR_DIRECTIVE_MISMATCH,
                     hsa_code_object_t());
  }

  return HSAReturn((hsa_status_t)::HSA_STATUS_SUCCESS,
                   finalizedProg->toHSAObject());
}

} // namespace phsa
