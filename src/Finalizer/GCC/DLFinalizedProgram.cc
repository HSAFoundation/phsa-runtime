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
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <unistd.h>
#include "DLFinalizedProgram.hh"
#include "common/Debug.hh"
#include "ISA.hh"

namespace phsa {

DLFinalizedProgram::DLFinalizedProgram(char *ElfBlob, size_t ElfSize,
                                       hsa_isa_t ISA, hsa_machine_model_t MM,
                                       hsa_profile_t P,
                                       hsa_default_float_rounding_mode_t RM)
    : FinalizedProgram(ElfBlob, ElfSize, ISA, MM, P, RM), Dlhandle(nullptr) {}

DLFinalizedProgram::~DLFinalizedProgram() {
  if (Dlhandle != nullptr)
    dlclose(Dlhandle);
  deregisterObject(this->toHSAObject());

  if (!phsa::IsDebugMode())
    unlink(BinaryFileName.c_str());
}

void DLFinalizedProgram::defineGlobalSymbolAddress(std::string SymbolName,
                                                   uint64_t Addr) {

  dlhandle();

  // GCC mangling does not contain & characters
  if (SymbolName.at(0) == '&')
    SymbolName = SymbolName.substr(1);

  std::string HostDefSymName =
      std::string(PHSA_HOST_DEF_PTR_PREFIX) + SymbolName;

  void *SymbolAddress = dlsym(Dlhandle, HostDefSymName.c_str());
  char *Error = dlerror();
  if (Error != NULL) {
    // Whole program optimizations might have removed the host def
    // in case it was used by a function not called by a kernel.
    // Exit silently here, in case it was needed by a function,
    // the actual error should occur already at when loading the .so.
    return;
  }
  *(char **)SymbolAddress = (char *)Addr;
}

void *DLFinalizedProgram::dlhandle() {

  if (Dlhandle != nullptr)
    return Dlhandle;
  assert(BinaryFileName != "");
  dlerror();
  Dlhandle = dlopen(BinaryFileName.c_str(), RTLD_NOW | RTLD_LOCAL);
  char *DlErrorStr = dlerror();
  if (DlErrorStr != nullptr) {
    std::cerr << "dlopen() error: " << DlErrorStr << std::endl;
    return nullptr;
  }
  return Dlhandle;
}

uint64_t DLFinalizedProgram::symbolAddress(std::string symbolName,
                                           Elf64_Sym *) {

  void *dlh = dlhandle();
  if (dlh == nullptr)
    return 0;
  dlerror();

  assert(symbolName.size() != 0);

  if (symbolName[0] == '&')
    symbolName = std::string(symbolName, 1, symbolName.size() - 1);

  void *symbolAddress = dlsym(dlh, symbolName.c_str());
  char *error = dlerror();
  if (error != NULL) {
    std::cerr << "dlsym() error: '" << error << "'" << std::endl;
    abort();
  }
  return (uint64_t)symbolAddress;
}

std::size_t DLFinalizedProgram::serializedSize() const {
  std::size_t Size = 0;
  Size += sizeof(std::size_t);
  Size += elfSize();
  Size += sizeof(ISA);
  Size += sizeof(DefaultRoundingMode);
  Size += sizeof(Profile);
  Size += sizeof(MachineModel);

  return Size;
}

void DLFinalizedProgram::serializeTo(uint8_t *Buffer) const {
  std::size_t ES = elfSize();
  memcpy(Buffer, &ES, sizeof(ES));
  Buffer += sizeof(ES);
  memcpy(Buffer, elfBlob(), elfSize());
  Buffer += elfSize();
  memcpy(Buffer, &ISA, sizeof(ISA));
  Buffer += sizeof(ISA);
  memcpy(Buffer, &DefaultRoundingMode, sizeof(DefaultRoundingMode));
  Buffer += sizeof(DefaultRoundingMode);
  memcpy(Buffer, &Profile, sizeof(Profile));
  Buffer += sizeof(Profile);
  memcpy(Buffer, &MachineModel, sizeof(MachineModel));
}

DLFinalizedProgram *DLFinalizedProgram::deserialize(uint8_t *Buffer) {
  std::size_t ES = 0;
  memcpy(&ES, Buffer, sizeof(ES));
  Buffer += sizeof(ES);

  uint8_t *ElfBlob = new uint8_t[ES];
  memcpy(ElfBlob, Buffer, ES);
  Buffer += ES;

  hsa_isa_t ISA;
  memcpy(&ISA, Buffer, sizeof(ISA));
  Buffer += sizeof(ISA);
  hsa_default_float_rounding_mode_t DefaultRoundingMode;
  memcpy(&DefaultRoundingMode, Buffer, sizeof(DefaultRoundingMode));
  Buffer += sizeof(DefaultRoundingMode);
  hsa_profile_t Profile;
  memcpy(&Profile, Buffer, sizeof(Profile));
  Buffer += sizeof(Profile);
  hsa_machine_model_t MachineModel;
  memcpy(&MachineModel, Buffer, sizeof(MachineModel));

  // Dump binary back to disk
  char *tempDirC = nullptr;
  char *dirTemplate = strdup("/tmp/phsa-finalized-program-XXXXXX");
  assert((tempDirC = mkdtemp(dirTemplate)) != nullptr);
  boost::filesystem::path tempDir(tempDirC);
  boost::filesystem::create_directories(tempDir);
  boost::filesystem::path outPath(tempDir / "temp.elf");
  std::ofstream outfile(outPath.string(), std::ofstream::binary);
  outfile.write(reinterpret_cast<char *>(ElfBlob), ES);

  auto Ret = new DLFinalizedProgram(reinterpret_cast<char *>(ElfBlob), ES, ISA,
                                    MachineModel, Profile, DefaultRoundingMode);

  Ret->setBinFileName(outPath.string());

  return Ret;
}
}
