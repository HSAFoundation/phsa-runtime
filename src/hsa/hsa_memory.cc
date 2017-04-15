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

#include "hsa.h"
#include <cstring>
#include "MemoryRegion.hh"
#include "common/Logging.hh"
#include "Runtime.hh"

hsa_status_t HSA_API hsa_memory_allocate(hsa_region_t region, size_t size,
                                         void **ptr) {
  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  if (size == 0 || ptr == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  phsa::MemoryRegion *MR = phsa::MemoryRegion::fromHSAObject(region);

  if (MR == nullptr) {
    return HSA_STATUS_ERROR_INVALID_REGION;
  }

  if (size > MR->getMaxAllocSize() || !MR->getRuntimeAllocAllowed()) {
    return HSA_STATUS_ERROR_INVALID_ALLOCATION;
  }

  // Let's align at least to the pointer size like malloc does.
  *ptr = MR->allocate(size, sizeof(void *));
  if (*ptr == nullptr) {
    return HSA_STATUS_ERROR_OUT_OF_RESOURCES;
  } else {
    return HSA_STATUS_SUCCESS;
  }
}

hsa_status_t HSA_API hsa_memory_free(void *Ptr) {
  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  if (Ptr == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  phsa::Runtime &RT = phsa::Runtime::get();
  RT.freePointer(Ptr);

  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_memory_copy(void *dst, const void *src, size_t size) {
  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  if (dst == nullptr || src == nullptr || size == 0) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  memmove(dst, src, size);
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_memory_assign_agent(void *ptr, hsa_agent_t agent,
                                             hsa_access_permission_t access) {
  //ABORT_UNIMPLEMENTED;
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_memory_register(void *ptr, size_t size) {
  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  if (ptr == nullptr || size == 0) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_memory_deregister(void *ptr, size_t size) {
  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  if (ptr == nullptr || size == 0) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  return HSA_STATUS_SUCCESS;
}
