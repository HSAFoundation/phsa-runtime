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
#include "FinalizedProgram.hh"

#include <libelf.h>

namespace phsa {

FinalizedProgram::FinalizedProgram(char *ElfBlob, size_t ElfSize, hsa_isa_t ISA,
                                   hsa_machine_model_t MM, hsa_profile_t P,
                                   hsa_default_float_rounding_mode_t RM)
    : BinaryFileName(""), ELFBlob(ElfBlob), ELFSize(ElfSize), IsValid(true),
      ISA(ISA), MachineModel(MM), Profile(P), DefaultRoundingMode(RM) {

  // Parse only the function descriptors from the special ELF sections at
  // this stage.

  Elf64_Shdr *SectionHeader = nullptr;
  Elf_Scn *Section = nullptr;
  Elf_Data *DataDesc = nullptr;
  Elf *ELF = elf_memory(ElfBlob, ElfSize);

  Elf64_Ehdr *EHdr = elf64_getehdr(ELF);
  Version = EHdr->e_version;

  std::string KernelName;

  size_t StringSectionIndex = 0;
  elf_getshdrstrndx(ELF, &StringSectionIndex);

  while ((Section = elf_nextscn(ELF, Section)) != nullptr) {
    SectionHeader = elf64_getshdr(Section);
    std::string SectionName =
        elf_strptr(ELF, StringSectionIndex, SectionHeader->sh_name);

    if (SectionName.substr(0, std::strlen(PHSA_DESC_SECTION_PREFIX)) ==
        PHSA_DESC_SECTION_PREFIX) {
      std::string FuncName =
          SectionName.substr(std::strlen(PHSA_DESC_SECTION_PREFIX));

      phsa_descriptor *Descriptor =
          static_cast<phsa_descriptor *>(elf_rawdata(Section, nullptr)->d_buf);
      FuncDescriptors[FuncName] = Descriptor;
    }
  }
  free(ELF);
}

FinalizedProgram::~FinalizedProgram() {
  delete[](char *)ELFBlob;
  for (auto I = symbol_begin(), End = symbol_end(); I != End; ++I) {
    delete (*I);
  }
}

bool FinalizedProgram::loadAndCheckControlDirectives(
    hsa_ext_control_directives_t ControlDirectives) {

  for (auto Val : FuncDescriptors) {
    auto FuncName = Val.first;
    auto FuncDesc = *Val.second;
// The control_directives_mask is not properly specified (which bit maps
// to which field) and the runtime conformance suite doesn't use it.  So
// just assume a field is enabled in case it's nonzero.
#define HANDLE_CTRL_DIRECTIVE(FDFIELD, CHECK, CDFIELD)                         \
  if (ControlDirectives.CDFIELD > 0)                                           \
    if (FuncDesc.FDFIELD > 0 &&                                                \
        FuncDesc.FDFIELD CHECK ControlDirectives.CDFIELD) {                    \
      return false;                                                            \
    } else                                                                     \
  FuncDesc.FDFIELD = ControlDirectives.CDFIELD

    HANDLE_CTRL_DIRECTIVE(max_dynamic_group_size, <, max_dynamic_group_size);
    HANDLE_CTRL_DIRECTIVE(max_flat_grid_size, <, max_flat_grid_size);
    HANDLE_CTRL_DIRECTIVE(max_flat_workgroup_size, <, max_flat_workgroup_size);
    HANDLE_CTRL_DIRECTIVE(required_grid_size[0], !=, required_grid_size[0]);
    HANDLE_CTRL_DIRECTIVE(required_grid_size[1], !=, required_grid_size[1]);
    HANDLE_CTRL_DIRECTIVE(required_grid_size[2], !=, required_grid_size[2]);
    HANDLE_CTRL_DIRECTIVE(required_workgroup_size[0], !=,
                          required_workgroup_size.x);
    HANDLE_CTRL_DIRECTIVE(required_workgroup_size[1], !=,
                          required_workgroup_size.y);
    HANDLE_CTRL_DIRECTIVE(required_workgroup_size[2], !=,
                          required_workgroup_size.z);
    HANDLE_CTRL_DIRECTIVE(required_dim, !=, required_dim);
  }
  return true;
}

Symbol *FinalizedProgram::findSymbol(std::string const &SymbolName) {
  bool HasModule = SymbolName.find("::") != std::string::npos;
  std::string ActualSymbolName = SymbolName;
  // Replace "::":s with dots
  size_t pos = 0;
  while ((pos = ActualSymbolName.find("::", pos)) != std::string::npos) {
    ActualSymbolName.replace(pos, 2, ".");
    pos += 1;
  }

  // Remove all &:s except the first one
  ActualSymbolName.erase(
      std::remove(ActualSymbolName.begin() + 1, ActualSymbolName.end(), '&'),
      ActualSymbolName.end());

  // If the module is specified, GCC prefixes the name with "gccbrig"
  if (HasModule)
    ActualSymbolName.insert(1, "gccbrig.");

  for (auto I = symbol_begin(), End = symbol_end(); I != End; ++I) {
    if ((*I)->Name == ActualSymbolName) {
      return *I;
    }
  }
  
  return nullptr;
}

phsa_descriptor *
FinalizedProgram::findDescriptor(std::string const &SymbolName) {

  auto DescIter = FuncDescriptors.find(SymbolName);
  if (DescIter != FuncDescriptors.end())
    return (*DescIter).second;
  else
    return nullptr;
}
}
