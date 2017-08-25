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
#include "hsa_ext_amd.h"

#include "Agent.hh"
#include "common/Info.hh"
#include "common/Logging.hh"
#include "Runtime.hh"

using phsa::WriteField;

#define NAME_MAX_LENGTH 64

hsa_status_t HSA_API hsa_agent_get_info(hsa_agent_t agent,
                                        hsa_agent_info_t attribute,
                                        void *value) {

  phsa::Agent *A = phsa::Agent::fromHSAObject(agent);

  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  if (A == nullptr) {
    return HSA_STATUS_ERROR_INVALID_AGENT;
  }

  if (value == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  phsa::KernelDispatchAgent *KA = dynamic_cast<phsa::KernelDispatchAgent *>(A);
  phsa::AgentDispatchAgent *AA = dynamic_cast<phsa::AgentDispatchAgent *>(A);

  switch (attribute) {

  case HSA_AGENT_INFO_NAME: {
    WriteField(value, A->getName(), NAME_MAX_LENGTH);
    break;
  }
  case HSA_AGENT_INFO_VENDOR_NAME: {
    WriteField(value, A->getVendor(), NAME_MAX_LENGTH);
    break;
  }
  case HSA_AGENT_INFO_FEATURE: {
    if (KA != nullptr) {
      WriteField(value, HSA_AGENT_FEATURE_KERNEL_DISPATCH);
    } else {
      WriteField(value, HSA_AGENT_FEATURE_AGENT_DISPATCH);
    }
    break;
  }
  case HSA_AGENT_INFO_MACHINE_MODEL: {
#if defined(HSA_LARGE_MODEL)
    WriteField(value, HSA_MACHINE_MODEL_LARGE);
#else
    WriteField(value, HSA_MACHINE_MODEL_SMALL);
#endif
    break;
  }
  case HSA_AGENT_INFO_PROFILE: {
    WriteField(value, A->getProfile());
    break;
  }
  case HSA_AGENT_INFO_DEFAULT_FLOAT_ROUNDING_MODE:
  case HSA_AGENT_INFO_BASE_PROFILE_DEFAULT_FLOAT_ROUNDING_MODES: {
    WriteField(value, A->getFloatRoundingMode());
    break;
  }
  case HSA_AGENT_INFO_FAST_F16_OPERATION: {
    if (KA != nullptr) {
      WriteField(value, KA->hasFastF16Operation());
    }
    break;
  }
  case HSA_AGENT_INFO_WAVEFRONT_SIZE: {
    if (KA != nullptr) {
      WriteField(value, KA->getWavefrontSize());
    }
    break;
  }
  case HSA_AGENT_INFO_WORKGROUP_MAX_DIM: {
    if (KA != nullptr) {
      WriteField(value, KA->getWorkGroupMaxDim());
    }
    break;
  }
  case HSA_AGENT_INFO_WORKGROUP_MAX_SIZE: {
    if (KA != nullptr) {
      WriteField(value, KA->getWorkGroupMaxSize());
    }
    break;
  }
  case HSA_AGENT_INFO_GRID_MAX_DIM: {
    if (KA != nullptr) {
      WriteField(value, KA->getGridMaxDim());
    }
    break;
  }
  case HSA_AGENT_INFO_GRID_MAX_SIZE: {
    if (KA != nullptr) {
      WriteField(value, KA->getGridMaxSize());
    }
    break;
  }
  case HSA_AGENT_INFO_FBARRIER_MAX_SIZE: {
    if (KA != nullptr) {
      WriteField(value, KA->getFBarrierMaxSize());
    }
    break;
  }
  case HSA_AGENT_INFO_QUEUES_MAX: {
    WriteField(value, A->getQueuesMax());
    break;
  }
  case HSA_AGENT_INFO_QUEUE_MIN_SIZE: {
    WriteField(value, A->getQueueMinSize());
    break;
  }
  case HSA_AGENT_INFO_QUEUE_MAX_SIZE: {
    WriteField(value, A->getQueueMaxSize());
    break;
  }
  case HSA_AGENT_INFO_QUEUE_TYPE: {
    WriteField(value, A->getQueueType());
    break;
  }
  case HSA_AGENT_INFO_NODE: {
    WriteField(value, A->getNUMAId());
    break;
  }
  case HSA_AGENT_INFO_DEVICE: {
    WriteField(value, A->getDeviceType());
    break;
  }
  case HSA_AGENT_INFO_CACHE_SIZE: {
    WriteField(value, A->getCacheSize());
    break;
  }
  case HSA_AGENT_INFO_ISA: {
    WriteField(value, ISA::toHSAObject(A->getISA()));

    break;
  }
  case HSA_AGENT_INFO_EXTENSIONS: {
    // TODO
    break;
  }
  case HSA_AGENT_INFO_VERSION_MAJOR: {
    WriteField(value, A->getVersion().Major);
    break;
  }
  case HSA_AGENT_INFO_VERSION_MINOR: {
    WriteField(value, A->getVersion().Minor);
    break;
  }
  case HSA_AMD_AGENT_INFO_COMPUTE_UNIT_COUNT: {
    WriteField(value, A->getComputeUnitCount());
    break;
  }
  default:
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_iterate_agents(
    hsa_status_t (*callback)(hsa_agent_t agent, void *data), void *data) {

  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  if (callback == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  phsa::Runtime &R = phsa::Runtime::get();

  hsa_status_t Status = HSA_STATUS_SUCCESS;
  for (phsa::Runtime::agent_iterator I = R.agent_begin(), E = R.agent_end();
       I != E; ++I) {
    Status = callback((*I)->toHSAObject(), data);
    if (Status != HSA_STATUS_SUCCESS) {
      return Status;
    }
  }

  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_agent_get_exception_policies(hsa_agent_t agent,
                                                      hsa_profile_t profile,
                                                      uint16_t *mask) {
  phsa::Agent *A = phsa::Agent::fromHSAObject(agent);

  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  if (A == nullptr) {
    return HSA_STATUS_ERROR_INVALID_AGENT;
  }

  if (mask == nullptr ||
      (profile != HSA_PROFILE_BASE && profile != HSA_PROFILE_FULL)) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  // TODO

  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_agent_extension_supported(uint16_t extension,
                                                   hsa_agent_t agent,
                                                   uint16_t version_major,
                                                   uint16_t version_minor,
                                                   bool *result) {

  phsa::Agent *A = phsa::Agent::fromHSAObject(agent);

  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  // Valid extension IDs are between 0 and 127
  if (extension > 127) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  if (A == nullptr) {
    return HSA_STATUS_ERROR_INVALID_AGENT;
  }

  if (result == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  // TODO

  return HSA_STATUS_SUCCESS;
}
