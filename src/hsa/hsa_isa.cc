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

#include "common/Info.hh"
#include "hsa.h"
#include "ISA.hh"
#include "Runtime.hh"

using phsa::WriteField;

hsa_status_t HSA_API hsa_isa_get_info(hsa_isa_t isa, hsa_isa_info_t attribute,
                                      uint32_t index, void *value) {
  if (!phsa::Runtime::isInitialized())
    return HSA_STATUS_ERROR_NOT_INITIALIZED;

  if (isa.handle == 0) {
    return HSA_STATUS_ERROR_INVALID_ISA;
  }

  if (value == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  std::string I = ISA::fromHSAObject(isa);

  if (I.empty()) {
    return HSA_STATUS_ERROR_INVALID_ISA;
  }

  if (index >= ISA::callingConventionCount(I)) {
    return HSA_STATUS_ERROR_INVALID_INDEX;
  }

  switch (attribute) {
  case HSA_ISA_INFO_NAME_LENGTH: {
    WriteField(value, static_cast<uint32_t>(I.size()));
    break;
  }
  case HSA_ISA_INFO_NAME: {
    WriteField(value, I, I.size());
    break;
  }
  case HSA_ISA_INFO_CALL_CONVENTION_COUNT: {
    WriteField(value, ISA::callingConventionCount(I));
    break;
  }
  case HSA_ISA_INFO_CALL_CONVENTION_INFO_WAVEFRONT_SIZE: {
    WriteField(value, ISA::callingConventions(I)[index].wavefrontSize);
    break;
  }
  case HSA_ISA_INFO_CALL_CONVENTION_INFO_WAVEFRONTS_PER_COMPUTE_UNIT: {
    WriteField(value,
               ISA::callingConventions(I)[index].wavefrontsPerComputeUnit);
    break;
  }
  default:
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_isa_compatible(hsa_isa_t code_object_isa,
                                        hsa_isa_t agent_isa, bool *result) {
  if (code_object_isa.handle == 0 || agent_isa.handle == 0) {
    return HSA_STATUS_ERROR_INVALID_ISA;
  }

  if (result == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  return HSA_STATUS_SUCCESS;
}
