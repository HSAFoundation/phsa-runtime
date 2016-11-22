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

#include "Agent.hh"
#include "common/Info.hh"
#include "hsa.h"
#include "MemoryRegion.hh"
#include "Runtime.hh"

using phsa::WriteField;

hsa_status_t HSA_API hsa_region_get_info(hsa_region_t region,
                                         hsa_region_info_t attribute,
                                         void *value) {
  if (!phsa::Runtime::isInitialized())
    return HSA_STATUS_ERROR_NOT_INITIALIZED;

  if (value == nullptr || attribute < HSA_REGION_INFO_SEGMENT ||
      attribute > HSA_REGION_INFO_RUNTIME_ALLOC_ALIGNMENT) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  phsa::MemoryRegion *MR = phsa::MemoryRegion::fromHSAObject(region);

  if (MR == nullptr) {
    return HSA_STATUS_ERROR_INVALID_REGION;
  }

  switch (attribute) {
  case HSA_REGION_INFO_SEGMENT: {
    WriteField(value, MR->getRegion());
    break;
  }
  case HSA_REGION_INFO_GLOBAL_FLAGS: {
    WriteField(value, MR->getGlobalFlags());
    break;
  }
  case HSA_REGION_INFO_SIZE: {
    WriteField(value, MR->getSize());
    break;
  }
  case HSA_REGION_INFO_ALLOC_MAX_SIZE: {
    WriteField(value, MR->getMaxAllocSize());

    break;
  }
  case HSA_REGION_INFO_RUNTIME_ALLOC_ALLOWED: {
    WriteField(value, MR->getRuntimeAllocAllowed());

    break;
  }
  case HSA_REGION_INFO_RUNTIME_ALLOC_GRANULE: {
    WriteField(value, MR->getRuntimeAllocGranularity());

    break;
  }
  case HSA_REGION_INFO_RUNTIME_ALLOC_ALIGNMENT: {
    WriteField(value, MR->getRuntimeAllocAlignment());

    break;
  }
  }
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_agent_iterate_regions(
    hsa_agent_t agent,
    hsa_status_t (*callback)(hsa_region_t region, void *data), void *data) {

  if (!phsa::Runtime::isInitialized())
    return HSA_STATUS_ERROR_NOT_INITIALIZED;

  if (callback == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  phsa::Agent *A = phsa::Agent::fromHSAObject(agent);
  if (A == nullptr) {
    return HSA_STATUS_ERROR_INVALID_AGENT;
  }

  hsa_status_t Status = HSA_STATUS_SUCCESS;
  for (phsa::Agent::region_iterator I = A->region_begin(), E = A->region_end();
       I != E; ++I) {
    Status = callback((*I)->toHSAObject(), data);
    if (Status != HSA_STATUS_SUCCESS) {
      return Status;
    }
  }

  return HSA_STATUS_SUCCESS;
}
