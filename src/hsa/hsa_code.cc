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

#include "common/Info.hh"
#include "hsa.h"
#include "Finalizer/GCC/DLFinalizedProgram.hh"
#include "FinalizedProgram.hh"
#include "Runtime.hh"
#include "Executable.hh"

using phsa::WriteField;

hsa_status_t HSA_API hsa_code_object_serialize(
    hsa_code_object_t code_object,
    hsa_status_t (*alloc_callback)(size_t size, hsa_callback_data_t data,
                                   void **address),
    hsa_callback_data_t callback_data, const char *options,
    void **serialized_code_object, size_t *serialized_code_object_size) {

  if (!phsa::Runtime::isInitialized())
    return HSA_STATUS_ERROR_NOT_INITIALIZED;

  phsa::FinalizedProgram *FP =
      phsa::FinalizedProgram::fromHSAObject(code_object);

  if (FP == nullptr) {
    return HSA_STATUS_ERROR_INVALID_CODE_OBJECT;
  }

  if (alloc_callback == nullptr || serialized_code_object == nullptr ||
      serialized_code_object_size == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  std::size_t Size = FP->serializedSize();

  hsa_status_t Status =
      alloc_callback(Size, callback_data, serialized_code_object);

  if (Status != HSA_STATUS_SUCCESS) {
    return Status;
  }

  FP->serializeTo(reinterpret_cast<uint8_t *>(*serialized_code_object));

  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_code_object_deserialize(
    void *serialized_code_object, size_t serialized_code_object_size,
    const char *options, hsa_code_object_t *code_object) {
  if (!phsa::Runtime::isInitialized())
    return HSA_STATUS_ERROR_NOT_INITIALIZED;

  if (serialized_code_object == nullptr || code_object == nullptr ||
      serialized_code_object_size == 0) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  // TODO: For now, only support deserialization of DLFinalizedProgram
  phsa::DLFinalizedProgram *FP = phsa::DLFinalizedProgram::deserialize(
      reinterpret_cast<uint8_t *>(serialized_code_object));

  *code_object = FP->toHSAObject();

  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_code_object_destroy(hsa_code_object_t code_object) {

  // Specs say that hsa_code_object_destroy is undefined in case
  // the executable has not been destroyed. In turn, executable
  // should not be destroyed while kernels are in flight.
  // See:
  // http://www.hsafoundation.com/html/HSA_Library.htm#Runtime/
  // Topics/02_Core/hsa_executable_destroy.htm
  // Thus, it's safe to delete the code object and its possible runtime
  // program image here without any reference counting or similar.
  delete phsa::FinalizedProgram::fromHSAObject(code_object);

  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_code_object_get_info(hsa_code_object_t code_object,
                                              hsa_code_object_info_t attribute,
                                              void *value) {
  if (!phsa::Runtime::isInitialized())
    return HSA_STATUS_ERROR_NOT_INITIALIZED;

  if (value == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  phsa::FinalizedProgram *FP =
      phsa::FinalizedProgram::fromHSAObject(code_object);

  switch (attribute) {
  case HSA_CODE_OBJECT_INFO_VERSION: {
    WriteField(value, std::to_string(FP->version()), 64);
    break;
  }
  case HSA_CODE_OBJECT_INFO_TYPE: {
    WriteField(value, HSA_CODE_OBJECT_TYPE_PROGRAM);
    break;
  }
  case HSA_CODE_OBJECT_INFO_ISA: {
    WriteField(value, FP->getMachineModel());
    break;
  }
  case HSA_CODE_OBJECT_INFO_MACHINE_MODEL: {
    WriteField(value, FP->getMachineModel());
    break;
  }
  case HSA_CODE_OBJECT_INFO_PROFILE: {
    WriteField(value, FP->getProfile());
    break;
  }
  case HSA_CODE_OBJECT_INFO_DEFAULT_FLOAT_ROUNDING_MODE: {
    WriteField(value, FP->getProfile());
    break;
  }
  default:
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_code_object_get_symbol(hsa_code_object_t code_object,
                                                const char *symbol_name,
                                                hsa_code_symbol_t *symbol) {
  if (!phsa::Runtime::isInitialized())
    return HSA_STATUS_ERROR_NOT_INITIALIZED;

  phsa::FinalizedProgram *FP =
      phsa::FinalizedProgram::fromHSAObject(code_object);

  if (FP == nullptr) {
    return HSA_STATUS_ERROR_INVALID_CODE_OBJECT;
  }

  if (symbol == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  phsa::Symbol *S = FP->findSymbol(symbol_name);

  if (S == nullptr) {
    return HSA_STATUS_ERROR_INVALID_SYMBOL_NAME;
  }

  *symbol = hsa_code_symbol_t{S->toHSAObject().handle};

  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_code_symbol_get_info(hsa_code_symbol_t code_symbol,
                                              hsa_code_symbol_info_t attribute,
                                              void *value) {
  if (!phsa::Runtime::isInitialized())
    return HSA_STATUS_ERROR_NOT_INITIALIZED;

  if (code_symbol.handle == 0) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  if (value == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  phsa::Symbol *S = (phsa::Symbol *)code_symbol.handle;

  if (S == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  switch (attribute) {
  case HSA_CODE_SYMBOL_INFO_NAME_LENGTH: {
    WriteField(value, S->Name.size());
    break;
  }
  case HSA_CODE_SYMBOL_INFO_TYPE: {
    WriteField(value, S->Type);
    break;
  }
  case HSA_CODE_SYMBOL_INFO_NAME: {
    WriteField(value, S->Name, S->Name.size());
    break;
  }
  case HSA_CODE_SYMBOL_INFO_MODULE_NAME_LENGTH: {
    WriteField(value, S->ModuleName.size());
    break;
  }
  case HSA_CODE_SYMBOL_INFO_MODULE_NAME: {
    WriteField(value, S->ModuleName, S->ModuleName.size());
    break;
  }
  case HSA_CODE_SYMBOL_INFO_LINKAGE: {
    WriteField(value, S->Linkage);
    break;
  }
  case HSA_CODE_SYMBOL_INFO_KERNEL_KERNARG_SEGMENT_SIZE: {
    WriteField(value, ((phsa::Kernel *)S)->KernargSegmentSize);
    break;
  }
  case HSA_CODE_SYMBOL_INFO_KERNEL_KERNARG_SEGMENT_ALIGNMENT: {
    WriteField(value, ((phsa::Kernel *)S)->KernargSegmentAlignment);
    break;
  }
  case HSA_CODE_SYMBOL_INFO_KERNEL_GROUP_SEGMENT_SIZE: {
    WriteField(value, ((phsa::Kernel *)S)->GroupSegmentSize);
    break;
  }
  case HSA_CODE_SYMBOL_INFO_KERNEL_PRIVATE_SEGMENT_SIZE: {
    WriteField(value, ((phsa::Kernel *)S)->PrivateSegmentSize);
    break;
  }
  case HSA_CODE_SYMBOL_INFO_KERNEL_DYNAMIC_CALLSTACK: {
    WriteField(value, ((phsa::Kernel *)S)->DynamicCallStack);
    break;
  }
  case HSA_CODE_SYMBOL_INFO_VARIABLE_ALIGNMENT: {
    WriteField(value, ((phsa::Variable *)S)->Alignment);
    break;
  }
  case HSA_CODE_SYMBOL_INFO_IS_DEFINITION: {
    WriteField(value, S->IsDefinition);
    break;
  }
  case HSA_CODE_SYMBOL_INFO_VARIABLE_ALLOCATION: {
    WriteField(value, ((phsa::Variable *)S)->Allocation);
    break;
  }
  case HSA_CODE_SYMBOL_INFO_VARIABLE_SEGMENT: {
    WriteField(value, ((phsa::Variable *)S)->Segment);
    break;
  }
  case HSA_CODE_SYMBOL_INFO_VARIABLE_SIZE: {
    WriteField(value, ((phsa::Variable *)S)->Size);
    break;
  }
  case HSA_CODE_SYMBOL_INFO_VARIABLE_IS_CONST: {
    WriteField(value, ((phsa::Variable *)S)->IsConst);
    break;
  }
  case HSA_CODE_SYMBOL_INFO_INDIRECT_FUNCTION_CALL_CONVENTION: {
    WriteField(value,
               ((phsa::IndirectFunctionCall *)S)->FunctionCallConvention);
    break;
  }
  default:
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_code_object_iterate_symbols(
    hsa_code_object_t code_object,
    hsa_status_t (*callback)(hsa_code_object_t, hsa_code_symbol_t, void *),
    void *data) {

  phsa::FinalizedProgram *FP =
      phsa::FinalizedProgram::fromHSAObject(code_object);

  if (FP == nullptr) {
    return HSA_STATUS_ERROR_INVALID_CODE_OBJECT;
  }

  hsa_status_t Status = HSA_STATUS_SUCCESS;
  for (phsa::FinalizedProgram::symbol_iterator I = FP->symbol_begin(),
                                               E = FP->symbol_end();
       I != E; ++I) {
    phsa::Symbol *S = *I;
    hsa_code_symbol_t HSASymbol;
    HSASymbol.handle = (uint64_t)S;
    Status = callback(code_object, HSASymbol, data);
    if (Status != HSA_STATUS_SUCCESS) {
      return Status;
    }
  }

  return HSA_STATUS_SUCCESS;
}
