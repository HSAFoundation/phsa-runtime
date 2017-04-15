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

#include <common/Info.hh>
#include "hsa_ext_finalize.h"

#include "Finalizer.hh"
#include "HSAObjectMapping.hh"
#include "ISA.hh"
#include "Runtime.hh"
#include "HSAILProgram.hh"
#include "Brig.h"
#include "FinalizedProgram.hh"

using phsa::WriteField;

namespace {

Finalizer *getFinalizer() {
  ExtensionRegistry &ER = phsa::Runtime::get().getExtensionRegistry();

  for (auto &E : ER) {
    if (E.first == HSA_EXTENSION_FINALIZER)
      return dynamic_cast<Finalizer *>(E.second);
  }

  return nullptr;
}

}

hsa_status_t HSA_API hsa_ext_program_create(
    hsa_machine_model_t machine_model, hsa_profile_t profile,
    hsa_default_float_rounding_mode_t default_float_rounding_mode,
    const char *options, hsa_ext_program_t *program) {

  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  if (machine_model != HSA_MACHINE_MODEL_SMALL &&
      machine_model != HSA_MACHINE_MODEL_LARGE) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  if (program == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  if (profile != HSA_PROFILE_BASE && profile != HSA_PROFILE_FULL) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  if (default_float_rounding_mode < HSA_DEFAULT_FLOAT_ROUNDING_MODE_DEFAULT ||
      default_float_rounding_mode > HSA_DEFAULT_FLOAT_ROUNDING_MODE_NEAR) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  Finalizer *F = getFinalizer();
  hsa_status_t Ret;

  std::tie(Ret, *program) =
      F->createProgram(machine_model, profile, default_float_rounding_mode);
  return Ret;
}

hsa_status_t HSA_API hsa_ext_program_destroy(hsa_ext_program_t program) {
  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  phsa::HSAILProgram *P = phsa::HSAILProgram::fromHSAObject(program);
  if (P == nullptr) {
    return static_cast<hsa_status_t>(HSA_EXT_STATUS_ERROR_INVALID_PROGRAM);
  }

  delete P;

  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_ext_program_add_module(hsa_ext_program_t program,
                                                hsa_ext_module_t module) {
  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  if (module == nullptr)
    return (hsa_status_t)HSA_EXT_STATUS_ERROR_INVALID_MODULE;

  // Basic validity/compatibility checking for the BRIG.
  const BrigModuleHeader *ModuleHeader = (const BrigModuleHeader *)module;
  const char *Blob = (char *)ModuleHeader;
  const char *CodeSection = nullptr;
  size_t CodeSectionSize = 0;

  //if (strncmp(ModuleHeader->identification, "HSA BRIG", 8) != 0)
  //  return (hsa_status_t)HSA_EXT_STATUS_ERROR_INVALID_MODULE;

  // Find the code section.
  for (uint32_t SectionIndex = 0; SectionIndex < ModuleHeader->sectionCount;
       ++SectionIndex) {
    uint64_t Offset =
        ((const uint64_t *)(Blob + ModuleHeader->sectionIndex))[SectionIndex];

    const BrigSectionHeader *SectionHeader =
        (const BrigSectionHeader *)(Blob + Offset);

    std::string SectionName((const char *)(&SectionHeader->name),
                            SectionHeader->nameLength);
    if (SectionIndex == BRIG_SECTION_INDEX_CODE && SectionName == "hsa_code") {
      CodeSection = (const char *)SectionHeader;
      CodeSectionSize = SectionHeader->byteCount;
      break;
    }
  }
  if (CodeSection == nullptr || CodeSectionSize == 0) {
    return (hsa_status_t)HSA_EXT_STATUS_ERROR_INVALID_MODULE;
  }

  const BrigSectionHeader *CodeSectionHeader =
      (const BrigSectionHeader *)CodeSection;
  BrigDirectiveModule *ModuleDirective = nullptr;

  // Find the module directive.
  for (size_t B = CodeSectionHeader->headerByteCount; B < CodeSectionSize;) {
    const BrigBase *Entry = (const BrigBase *)(CodeSection + B);
    if (Entry->kind == BRIG_KIND_DIRECTIVE_MODULE) {
      ModuleDirective = (BrigDirectiveModule *)Entry;
      break;
    }
    B += Entry->byteCount;
  }

  if (ModuleDirective == nullptr) {
    return (hsa_status_t)HSA_EXT_STATUS_ERROR_INVALID_MODULE;
  }

  phsa::HSAILProgram *Prog = phsa::HSAILProgram::fromHSAObject(program);

  if (ModuleDirective->profile != Prog->getProfile() ||
      ModuleDirective->machineModel != Prog->getMachineModel()) {
    return (hsa_status_t)HSA_EXT_STATUS_ERROR_INCOMPATIBLE_MODULE;
  }

  if (Prog->hasModule(module))
    return (hsa_status_t)HSA_EXT_STATUS_ERROR_MODULE_ALREADY_INCLUDED;

  Finalizer *F = getFinalizer();
  F->addModule(program, module);
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_ext_program_iterate_modules(
    hsa_ext_program_t program,
    hsa_status_t (*callback)(hsa_ext_program_t program, hsa_ext_module_t module,
                             void *data),
    void *data) {

  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  phsa::HSAILProgram *P = phsa::HSAILProgram::fromHSAObject(program);

  for (size_t I = 0; I < P->moduleCount(); ++I) {
    hsa_status_t Status = callback(program, P->module(I), data);
    if (Status != HSA_STATUS_SUCCESS) {
      return Status;
    }
  }

  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_ext_program_get_info(hsa_ext_program_t program,
                                              hsa_ext_program_info_t attribute,
                                              void *value) {
  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  phsa::HSAILProgram *P = phsa::HSAILProgram::fromHSAObject(program);

  if (P == nullptr) {
    return static_cast<hsa_status_t>(HSA_EXT_STATUS_ERROR_INVALID_PROGRAM);
  }

  if (value == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  switch (attribute) {

  case HSA_EXT_PROGRAM_INFO_MACHINE_MODEL: {
    WriteField(value, P->getMachineModel());
    break;
  }

  case HSA_EXT_PROGRAM_INFO_PROFILE: {
    WriteField(value, P->getProfile());
    break;
  }
  case HSA_EXT_PROGRAM_INFO_DEFAULT_FLOAT_ROUNDING_MODE: {
    WriteField(value, P->getDefaultRoundingMode());
    break;
  }
  default: { return HSA_STATUS_ERROR_INVALID_ARGUMENT; }
  }

  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_ext_program_finalize(
    hsa_ext_program_t program, hsa_isa_t isa, int32_t call_convention,
    hsa_ext_control_directives_t control_directives, const char *options,
    hsa_code_object_type_t code_object_type, hsa_code_object_t *code_object) {

  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  std::string I = ISA::fromHSAObject(isa);

  if (I.empty()) {
    return HSA_STATUS_ERROR_INVALID_ISA;
  }

  if (program.handle == 0) {
    return static_cast<hsa_status_t>(HSA_EXT_STATUS_ERROR_INVALID_PROGRAM);
  }

  Finalizer *F = getFinalizer();
  hsa_status_t Ret;

  std::tie(Ret, *code_object) =
      F->finalizeProgram(program, control_directives, isa, options);
  return Ret;
}
