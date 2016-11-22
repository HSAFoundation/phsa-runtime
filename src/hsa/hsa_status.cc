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
 * @author pekka.jaaskelainen@parmance.com and tomi.aijo@parmance.com
 *         for General Processor Tech.
 */

#include <sstream>
#include <cstring>
#include "hsa.h"
#include "hsa_ext_finalize.h"
#include "Runtime.hh"

hsa_status_t HSA_API hsa_status_string(hsa_status_t status,
                                       const char **status_string) {

  if (!phsa::Runtime::isInitialized())
    return HSA_STATUS_ERROR_NOT_INITIALIZED;

  if (status_string == nullptr)
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;

#define STATUS_CASE(X)                                                         \
  case X:                                                                      \
    *status_string = #X;                                                       \
    break
  switch (status) {
    STATUS_CASE(HSA_STATUS_SUCCESS);
    STATUS_CASE(HSA_STATUS_INFO_BREAK);
    STATUS_CASE(HSA_STATUS_ERROR);
    STATUS_CASE(HSA_STATUS_ERROR_INVALID_ARGUMENT);
    STATUS_CASE(HSA_STATUS_ERROR_INVALID_QUEUE_CREATION);
    STATUS_CASE(HSA_STATUS_ERROR_INVALID_ALLOCATION);
    STATUS_CASE(HSA_STATUS_ERROR_INVALID_AGENT);
    STATUS_CASE(HSA_STATUS_ERROR_INVALID_REGION);
    STATUS_CASE(HSA_STATUS_ERROR_INVALID_SIGNAL);
    STATUS_CASE(HSA_STATUS_ERROR_INVALID_QUEUE);
    STATUS_CASE(HSA_STATUS_ERROR_OUT_OF_RESOURCES);
    STATUS_CASE(HSA_STATUS_ERROR_INVALID_PACKET_FORMAT);
    STATUS_CASE(HSA_STATUS_ERROR_RESOURCE_FREE);
    STATUS_CASE(HSA_STATUS_ERROR_NOT_INITIALIZED);
    STATUS_CASE(HSA_STATUS_ERROR_REFCOUNT_OVERFLOW);
    STATUS_CASE(HSA_STATUS_ERROR_INCOMPATIBLE_ARGUMENTS);
    STATUS_CASE(HSA_STATUS_ERROR_INVALID_INDEX);
    STATUS_CASE(HSA_STATUS_ERROR_INVALID_ISA);
    STATUS_CASE(HSA_STATUS_ERROR_INVALID_ISA_NAME);
    STATUS_CASE(HSA_STATUS_ERROR_INVALID_CODE_OBJECT);
    STATUS_CASE(HSA_STATUS_ERROR_INVALID_EXECUTABLE);
    STATUS_CASE(HSA_STATUS_ERROR_FROZEN_EXECUTABLE);
    STATUS_CASE(HSA_STATUS_ERROR_INVALID_SYMBOL_NAME);
    STATUS_CASE(HSA_STATUS_ERROR_VARIABLE_ALREADY_DEFINED);
    STATUS_CASE(HSA_STATUS_ERROR_VARIABLE_UNDEFINED);
    STATUS_CASE(HSA_STATUS_ERROR_EXCEPTION);
    STATUS_CASE(HSA_EXT_STATUS_ERROR_INVALID_PROGRAM);
    STATUS_CASE(HSA_EXT_STATUS_ERROR_INVALID_MODULE);
    STATUS_CASE(HSA_EXT_STATUS_ERROR_INCOMPATIBLE_MODULE);
    STATUS_CASE(HSA_EXT_STATUS_ERROR_MODULE_ALREADY_INCLUDED);
    STATUS_CASE(HSA_EXT_STATUS_ERROR_SYMBOL_MISMATCH);
    STATUS_CASE(HSA_EXT_STATUS_ERROR_FINALIZATION_FAILED);
    STATUS_CASE(HSA_EXT_STATUS_ERROR_DIRECTIVE_MISMATCH);
  default:
    std::ostringstream str;
    str << "unknown status code 0x" << std::hex << status;
    *status_string = strdup(str.str().c_str());
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
    break;
  };
  return HSA_STATUS_SUCCESS;
}
