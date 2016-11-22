/*
    Copyright (c) 2016 General Processor Tech.
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
    deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:
    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/
/**
 * @author tomi.aijo@parmance.com for General Processor Tech.
 */

#include "hsa.h"

#include "Agent.hh"
#include "common/Info.hh"
#include "Finalizer/GCC/ELFExecutable.hh"
#include "Runtime.hh"

using phsa::WriteField;

hsa_status_t HSA_API hsa_executable_create(
    hsa_profile_t profile, hsa_executable_state_t executable_state,
    const char *options, hsa_executable_t *executable) {
  if (!phsa::Runtime::isInitialized())
    return HSA_STATUS_ERROR_NOT_INITIALIZED;

  if (executable == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  phsa::Executable *E = new phsa::ELFExecutable(
      profile, executable_state == HSA_EXECUTABLE_STATE_FROZEN);
  *executable = E->toHSAObject();
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_executable_destroy(hsa_executable_t executable) {
  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  phsa::Executable *E = phsa::Executable::fromHSAObject(executable);
  if (E == nullptr) {
    return HSA_STATUS_ERROR_INVALID_EXECUTABLE;
  }

  delete E;

  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_executable_load_code_object(
    hsa_executable_t executable, hsa_agent_t agent,
    hsa_code_object_t code_object, const char *options) {

  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  phsa::Executable *E = phsa::Executable::fromHSAObject(executable);

  if (E == nullptr) {
    return HSA_STATUS_ERROR_INVALID_EXECUTABLE;
  }

  if (E->isFrozen()) {
    return HSA_STATUS_ERROR_FROZEN_EXECUTABLE;
  }

  phsa::Agent *Agent = phsa::Agent::fromHSAObject(agent);
  if (Agent == nullptr) {
    return HSA_STATUS_ERROR_INVALID_AGENT;
  }

  if (code_object.handle == 0) {
    return HSA_STATUS_ERROR_INVALID_CODE_OBJECT;
  }

  hsa_status_t Status;
  std::tie(Status) = E->LoadCodeObject(Agent, code_object, options);

  return Status;
}

hsa_status_t HSA_API hsa_executable_freeze(hsa_executable_t executable,
                                           const char *options) {
  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  // TODO: Validate executable.
  phsa::Executable *E = phsa::Executable::fromHSAObject(executable);

  if (E == nullptr) {
    return HSA_STATUS_ERROR_INVALID_EXECUTABLE;
  }

  if (E->isFrozen()) {
    return HSA_STATUS_ERROR_FROZEN_EXECUTABLE;
  }

  E->freeze();
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_executable_get_info(hsa_executable_t executable,
                                             hsa_executable_info_t attribute,
                                             void *value) {
  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  if (value == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  phsa::Executable *E = phsa::Executable::fromHSAObject(executable);

  if (E == nullptr) {
    return HSA_STATUS_ERROR_INVALID_EXECUTABLE;
  }

  switch (attribute) {
  case HSA_EXECUTABLE_INFO_PROFILE: {
    WriteField(value, E->getProfile());
    break;
  }
  case HSA_EXECUTABLE_INFO_STATE: {
    WriteField(value, E->getExecutableState());
    break;
  }
  default:
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_executable_global_variable_define(
    hsa_executable_t executable, const char *variable_name, void *address) {

  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  phsa::Executable *E = phsa::Executable::fromHSAObject(executable);

  if (E == nullptr) {
    return HSA_STATUS_ERROR_INVALID_EXECUTABLE;
  }

  if (variable_name == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  E->defineGlobalSymbolAddress(variable_name,
                               reinterpret_cast<uint64_t>(address));

  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_executable_agent_global_variable_define(
    hsa_executable_t executable, hsa_agent_t agent, const char *variable_name,
    void *address) {
  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  phsa::Executable *E = phsa::Executable::fromHSAObject(executable);

  if (E == nullptr) {
    return HSA_STATUS_ERROR_INVALID_EXECUTABLE;
  }

  if (variable_name == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  E->defineGlobalSymbolAddress(variable_name,
                               reinterpret_cast<uint64_t>(address));

  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_executable_readonly_variable_define(
    hsa_executable_t executable, hsa_agent_t agent, const char *variable_name,
    void *address) {
  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  phsa::Executable *E = phsa::Executable::fromHSAObject(executable);

  if (E == nullptr) {
    return HSA_STATUS_ERROR_INVALID_EXECUTABLE;
  }

  if (variable_name == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  E->defineGlobalSymbolAddress(variable_name,
                               reinterpret_cast<uint64_t>(address));

  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_executable_validate(hsa_executable_t executable,
                                             uint32_t *result) {
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_executable_get_symbol(
    hsa_executable_t executable, const char *module_name,
    const char *symbol_name, hsa_agent_t agent, int32_t call_convention,
    hsa_executable_symbol_t *symbol) {

  std::string SymbolName = symbol_name;
  if (module_name != nullptr && std::strlen(module_name) > 0) {
    SymbolName.insert(1, std::string(module_name).substr(1) + ".");
    SymbolName.insert(1, "gccbrig.");
  }

  phsa::Executable *E = phsa::Executable::fromHSAObject(executable);
  const phsa::Symbol *S = E->getSymbol(SymbolName);
  if (S == nullptr) {
    return HSA_STATUS_ERROR_INVALID_SYMBOL_NAME;
  }

  *symbol = S->toHSAObject();
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_executable_symbol_get_info(
    hsa_executable_symbol_t executable_symbol,
    hsa_executable_symbol_info_t attribute, void *value) {

  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  if (value == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  phsa::Symbol *S = phsa::Symbol::fromHSAObject(executable_symbol);

  if (S == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  phsa::Variable *V = reinterpret_cast<phsa::Variable *>(S);
  phsa::Kernel *K = reinterpret_cast<phsa::Kernel *>(S);
  phsa::IndirectFunctionCall *IFC =
      reinterpret_cast<phsa::IndirectFunctionCall *>(S);
  switch (attribute) {

  case HSA_EXECUTABLE_SYMBOL_INFO_TYPE: {
    WriteField(value, S->Type);
    break;
  }

  case HSA_EXECUTABLE_SYMBOL_INFO_NAME_LENGTH: {
    WriteField(value, static_cast<uint32_t>(S->Name.size()));
    break;
  }
  case HSA_EXECUTABLE_SYMBOL_INFO_NAME: {
    WriteField(value, S->Name, S->Name.size());
    break;
  }
  case HSA_EXECUTABLE_SYMBOL_INFO_MODULE_NAME_LENGTH: {
    WriteField(value, static_cast<uint32_t>(S->ModuleName.size()));
    break;
  }
  case HSA_EXECUTABLE_SYMBOL_INFO_MODULE_NAME: {
    WriteField(value, S->ModuleName, S->ModuleName.size());
    break;
  }
  case HSA_EXECUTABLE_SYMBOL_INFO_AGENT: {
    WriteField(value, S->Agent);
    break;
  }
  case HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_ADDRESS: {
    if (V != nullptr) {
      WriteField(value, V->Address);
    }
    break;
  }
  case HSA_EXECUTABLE_SYMBOL_INFO_LINKAGE: {
    WriteField(value, S->Linkage);
    break;
  }
  case HSA_EXECUTABLE_SYMBOL_INFO_IS_DEFINITION: {
    WriteField(value, S->IsDefinition);
    break;
  }
  case HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_ALLOCATION: {
    if (V != nullptr) {
      WriteField(value, V->Allocation);
    }
    break;
  }
  case HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_SEGMENT: {
    if (V != nullptr) {
      WriteField(value, V->Segment);
    }
    break;
  }
  case HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_ALIGNMENT: {
    if (V != nullptr) {
      WriteField(value, V->Alignment);
    }
    break;
  }
  case HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_SIZE: {
    if (V != nullptr) {
      WriteField(value, V->Size);
    }
    break;
  }
  case HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_IS_CONST: {
    if (V != nullptr) {
      WriteField(value, V->IsConst);
    }
    break;
  }
  case HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_OBJECT: {
    if (K != nullptr) {
      WriteField(value, K->Object);
    }
    break;
  }
  case HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_KERNARG_SEGMENT_SIZE: {
    if (K != nullptr) {
      WriteField(value, K->KernargSegmentSize);
    }
    break;
  }
  case HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_KERNARG_SEGMENT_ALIGNMENT: {
    if (K != nullptr) {
      WriteField(value, K->KernargSegmentAlignment);
    }
    break;
  }
  case HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_GROUP_SEGMENT_SIZE: {
    if (K != nullptr) {
      WriteField(value, K->GroupSegmentSize);
    }
    break;
  }
  case HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_PRIVATE_SEGMENT_SIZE: {
    if (K != nullptr) {
      WriteField(value, K->PrivateSegmentSize);
    }
    break;
  }
  case HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_DYNAMIC_CALLSTACK: {
    if (K != nullptr) {
      WriteField(value, K->DynamicCallStack);
    }
    break;
  }
  case HSA_EXECUTABLE_SYMBOL_INFO_INDIRECT_FUNCTION_OBJECT: {
    if (IFC != nullptr) {
      WriteField(value, IFC->FunctionObject);
    }
    break;
  }
  case HSA_EXECUTABLE_SYMBOL_INFO_INDIRECT_FUNCTION_CALL_CONVENTION: {
    if (IFC != nullptr) {
      WriteField(value, IFC->FunctionCallConvention);
    }
    break;
  }
  default: { return HSA_STATUS_ERROR_INVALID_ARGUMENT; }
  }

  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_executable_iterate_symbols(
    hsa_executable_t executable,
    hsa_status_t (*callback)(hsa_executable_t executable,
                             hsa_executable_symbol_t symbol, void *data),
    void *data) {

  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  phsa::Executable *E = phsa::Executable::fromHSAObject(executable);

  hsa_status_t Status = HSA_STATUS_SUCCESS;
  for (phsa::Executable::symbol_iterator I = E->symbol_begin(),
                                         End = E->symbol_end();
       I != End; ++I) {
    Status = callback(executable, (*I)->toHSAObject(), data);
    if (Status != HSA_STATUS_SUCCESS) {
      return Status;
    }
  }

  return HSA_STATUS_SUCCESS;
}
