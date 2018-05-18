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

#include "ELFExecutable.hh"
#include "FinalizedProgram.hh"
#include <algorithm>
#include <assert.h>
#include <elf.h>

namespace phsa {

HSAReturnValue<>
ELFExecutable::LoadCodeObject(phsa::Agent *Agent,
                              const hsa_code_object_t CodeObject,
                              const char *Options) {

  phsa::FinalizedProgram *Program =
      phsa::FinalizedProgram::fromHSAObject(CodeObject);

  if (Program == nullptr) {
    return HSAReturn(HSA_STATUS_ERROR_INVALID_CODE_OBJECT);
  }

  for (auto s : DefinedSymbols) {
    std::string Name = s.first;
    uint64_t Addr = s.second;

    Program->defineGlobalSymbolAddress(Name, Addr);
  }

  Elf64_Shdr *SectionHeader = nullptr;
  Elf_Scn *Section = nullptr;
  Elf_Data *DataDesc = nullptr;
  Elf *ELF = elf_memory(Program->elfBlob(), Program->elfSize());
  Elf64_Ehdr *EHdr = elf64_getehdr(ELF);
  Program->setVersion(EHdr->e_version);

  Elf_Scn *SymbolSection = nullptr;
  std::string KernelName;

  size_t StringSectionIndex = 0;
  elf_getshdrstrndx(ELF, &StringSectionIndex);
  while ((Section = elf_nextscn(ELF, Section)) != nullptr) {
    SectionHeader = elf64_getshdr(Section);
    if (SectionHeader->sh_type == SHT_SYMTAB) {
      SymbolSection = Section;
    }
  }

  SectionHeader = elf64_getshdr(SymbolSection);

  DataDesc = elf_getdata(SymbolSection, DataDesc);
  Elf64_Sym *Symbols = static_cast<Elf64_Sym *>(DataDesc->d_buf);
  std::size_t SymbolCount = SectionHeader->sh_size / SectionHeader->sh_entsize;

  for (std::size_t I = 0; I < SymbolCount; ++I) {
    Elf64_Sym Symbol = Symbols[I];
    std::string SymbolName =
        elf_strptr(ELF, SectionHeader->sh_link, Symbol.st_name);

#define PHSA_KERNEL_PREFIX "phsa_kernel."

    // PHSA kernels are always currently single WI kernels which do not
    // use any local memory. They are detected from a known function name
    // prefix.
    bool IsPHSAKernel =
      SymbolName.size() > strlen(PHSA_KERNEL_PREFIX) &&
      SymbolName.substr(0, strlen(PHSA_KERNEL_PREFIX)) == PHSA_KERNEL_PREFIX;

    // Hide some uninteresting / compiler internal symbols. TODO:
    // most of these are likely external symbols which can be skipped by
    // checking for it.
    if (SymbolName.size() == 0 ||
        (!IsPHSAKernel && SymbolName.size() > 7 &&
	 SymbolName.substr(0, 8) != "gccbrig." &&
         SymbolName.find(".") != std::string::npos) ||
        SymbolName == "frame_dummy" ||
        SymbolName == "__do_global_dtors_aux_fini_array_entry" ||
        SymbolName == "__frame_dummy_init_array_entry" ||
        SymbolName == "__FRAME_END__" ||
        SymbolName == "__dso_handle" ||
        SymbolName == "_DYNAMIC" ||
        SymbolName == "__TMC_END__" ||
        SymbolName == "_GLOBAL_OFFSET_TABLE_" ||
        SymbolName == "register_tm_clones" ||
        SymbolName == "deregister_tm_clones")
      continue;

    phsa_descriptor *Descriptor = Program->findDescriptor(SymbolName);
    bool IsKernel = Descriptor != nullptr && Descriptor->is_kernel;

    if (IsKernel) {
      Kernel *K = new Kernel;
      K->Name = std::string("&") + SymbolName;
      K->Type = HSA_SYMBOL_KIND_KERNEL;
      K->ModuleName = "";
      K->Agent = NULL;
      K->Linkage = HSA_SYMBOL_LINKAGE_PROGRAM;
      K->IsDefinition = true;

      K->Object = reinterpret_cast<uint64_t>(K);
      K->Address = (void *)Program->symbolAddress(SymbolName, &Symbol);
      K->KernargSegmentSize = Descriptor->kernarg_segment_size;
      K->KernargSegmentAlignment =
          std::max((uint16_t)MIN_ALIGNMENT, Descriptor->kernarg_max_align);
      K->GroupSegmentSize = Descriptor->group_segment_size;
      K->PrivateSegmentSize = Descriptor->private_segment_size;
      K->DynamicCallStack = false; // TODO
      K->ImplementationData = Program;

      Program->addSymbol(K);
      registerSymbol(K);
    } else if (IsPHSAKernel) {
      Kernel *K = new Kernel;
      K->Name = std::string("&") + SymbolName;
      K->Type = HSA_SYMBOL_KIND_KERNEL;
      K->ModuleName = "";
      K->Agent = NULL;
      K->Linkage = HSA_SYMBOL_LINKAGE_PROGRAM;
      K->IsDefinition = true;

      K->Object = reinterpret_cast<uint64_t>(K);
      K->Address = (void *)Program->symbolAddress(SymbolName, &Symbol);
      K->KernargSegmentSize = 2048;
      K->KernargSegmentAlignment = 1;

      K->GroupSegmentSize = 0;
      K->PrivateSegmentSize = 0;
      K->DynamicCallStack = false;
      K->ImplementationData = Program;

      Program->addSymbol(K);
      registerSymbol(K);
    } else if ((Symbol.st_info & 0x0f) == STT_OBJECT) {
      Variable *V = new Variable;
      V->Name = std::string("&") + SymbolName;
      V->Type = HSA_SYMBOL_KIND_VARIABLE;
      V->ModuleName = "";
      // TODO: Support agent-scope variables properly.
      V->Agent = NULL;
      V->Linkage = HSA_SYMBOL_LINKAGE_PROGRAM;
      V->IsDefinition = true;
      V->Address = (void *)Program->symbolAddress(SymbolName, &Symbol);
      Program->addSymbol(V);
      registerSymbol(V);
    } else {
      // TODO: other program/module scope symbol types.
    }
  }
  Program->setExecutable(*this);

  free(ELF);

  return HSAReturn(HSA_STATUS_SUCCESS);
}

} // namespace phsa

