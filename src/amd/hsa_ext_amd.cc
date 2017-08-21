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
 * Simple implementations of the AMD extensions used by HCC.
 *
 * @todo This might be better moved to hcc-phsa as a default
 * implementation that is implemented using the HSA full profile
 * runtime API.
 *
 * @author pekka.jaaskelainen@parmance.com and
 *         tomi.aijo@parmance.com for General Processor Tech.
 */

#include "hsa_ext_amd.h"

#include <memory>
#include <vector>
#include <map>

#include "common/Logging.hh"

hsa_status_t
initialize_memory_pools(hsa_region_t region, void *data);

#define HSA_CHECK_STATUS(__STATUS__, __MSG__)                           \
  hsa_status_t status_temp_XXXX = __STATUS__;                           \
  do { if (status_temp_XXXX != HSA_STATUS_SUCCESS) {                    \
      std::cerr << "phsa-runtime (hsa_ext_amd wrapper) ERROR (status "  \
                << std::hex << status_temp_XXXX << std::dec << "): "    \
                << __MSG__ << std::endl;                                \
      abort();                                                          \
    } } while(false)

class AgentData {
public:
  AgentData(hsa_agent_t agent) {
    HSA_CHECK_STATUS(
      hsa_agent_iterate_regions(agent, initialize_memory_pools, this),
      "Initializing memory pools failed.");
  }
  ~AgentData() {
    for (auto mp : mem_pools)
      delete mp;
  }
  std::vector<hsa_amd_memory_pool_t*> mem_pools;
};


hsa_status_t
initialize_memory_pools(hsa_region_t region, void *data) {
  AgentData *agentData = (AgentData*)data;
  hsa_amd_memory_pool_t *mem_pool = new hsa_amd_memory_pool_t;
  mem_pool->handle = region.handle;
  agentData->mem_pools.push_back(mem_pool);
  return HSA_STATUS_SUCCESS;
}

// TODO: this leaks currently, which might be just fine.
std::map<uint64_t, AgentData*> agents;

static AgentData&
get_agent_data(hsa_agent_t agent) {
  if (agents.find(agent.handle) == agents.end())
    agents[agent.handle] = new AgentData(agent);
  return *agents[agent.handle];
}

hsa_status_t HSA_API hsa_amd_coherency_get_type(hsa_agent_t agent,
                                                hsa_amd_coherency_type_t* type) {
  ABORT_UNIMPLEMENTED;
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_coherency_set_type(hsa_agent_t agent,
                                                hsa_amd_coherency_type_t type) {
  ABORT_UNIMPLEMENTED;
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_profiling_set_profiler_enabled(hsa_queue_t* queue,
                                                            int enable) {
  // Unimplemented, but should not be required.
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_profiling_async_copy_enable(bool enable) {
  ABORT_UNIMPLEMENTED;
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_profiling_get_dispatch_time(
        hsa_agent_t agent, hsa_signal_t signal,
        hsa_amd_profiling_dispatch_time_t* time) {
  // A dummy implementation here for compatibility purposes.
  // There is no clean way to produce accurate profiling data
  // in the pure HSA API.
  time->start = 1;
  time->end = 2;
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_profiling_get_async_copy_time(
        hsa_signal_t signal, hsa_amd_profiling_async_copy_time_t* time) {
  ABORT_UNIMPLEMENTED;
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_profiling_convert_tick_to_system_domain(hsa_agent_t agent,
                                                uint64_t agent_tick,
                                                uint64_t* system_tick) {
  ABORT_UNIMPLEMENTED;
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_signal_async_handler(hsa_signal_t signal,
                             hsa_signal_condition_t cond,
                             hsa_signal_value_t value,
                             hsa_amd_signal_handler handler, void* arg) {
  ABORT_UNIMPLEMENTED;
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_async_function(void (*callback)(void* arg), void* arg) {
  ABORT_UNIMPLEMENTED;
  return HSA_STATUS_SUCCESS;
}

uint32_t HSA_API
hsa_amd_signal_wait_any(uint32_t signal_count, hsa_signal_t* signals,
                        hsa_signal_condition_t* conds,
                        hsa_signal_value_t* values, uint64_t timeout_hint,
                        hsa_wait_state_t wait_hint,
                        hsa_signal_value_t* satisfying_value) {
  ABORT_UNIMPLEMENTED;
  return 0;
}

hsa_status_t HSA_API hsa_amd_image_get_info_max_dim(hsa_agent_t agent,
                                                    hsa_agent_info_t attribute,
                                                    void* value) {
  ABORT_UNIMPLEMENTED;
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_queue_cu_set_mask(const hsa_queue_t* queue,
                                               uint32_t num_cu_mask_count,
                                               const uint32_t* cu_mask) {
  ABORT_UNIMPLEMENTED;
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API
hsa_amd_memory_pool_get_info(hsa_amd_memory_pool_t memory_pool,
                             hsa_amd_memory_pool_info_t attribute,
                             void* value) {
  hsa_region_t region;
  region.handle = memory_pool.handle;

  switch (attribute) {
  case HSA_AMD_MEMORY_POOL_INFO_SEGMENT: {
    hsa_region_segment_t region_segment;
    hsa_amd_segment_t *segment = (hsa_amd_segment_t*)value;

    HSA_CHECK_STATUS(hsa_region_get_info(region, HSA_REGION_INFO_SEGMENT,
                                         &region_segment),
                     "hsa_region_get_info() failed.");

    if (region_segment == HSA_REGION_SEGMENT_GLOBAL)
      *segment = HSA_AMD_SEGMENT_GLOBAL;
    else if (region_segment == HSA_REGION_SEGMENT_GROUP)
      *segment = HSA_AMD_SEGMENT_GROUP;
    else
      ABORT_UNIMPLEMENTED;
    break;
  }
  case HSA_AMD_MEMORY_POOL_INFO_GLOBAL_FLAGS: {
    hsa_amd_memory_pool_global_flag_t *flags =
      (hsa_amd_memory_pool_global_flag_t*)value;
    *flags = (hsa_amd_memory_pool_global_flag_t)
      ((int)HSA_AMD_MEMORY_POOL_GLOBAL_FLAG_FINE_GRAINED
       | (int)HSA_AMD_MEMORY_POOL_GLOBAL_FLAG_KERNARG_INIT);
    break;
  }
  case HSA_AMD_MEMORY_POOL_INFO_SIZE: {
    size_t *size = (size_t*)value;
    HSA_CHECK_STATUS(hsa_region_get_info(region, HSA_REGION_INFO_SIZE, size),
                     "hsa_region_get_info() failed.");
    break;
  }
  default:
    ABORT_UNIMPLEMENTED;
    break;
  }
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_agent_iterate_memory_pools(
        hsa_agent_t agent,
        hsa_status_t (*callback)(hsa_amd_memory_pool_t memory_pool, void* data),
        void* data) {
  AgentData &agentData = get_agent_data(agent);
  for (auto mp : agentData.mem_pools)
    callback(*mp, data);
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API
hsa_amd_memory_pool_allocate(hsa_amd_memory_pool_t memory_pool, size_t size,
                             uint32_t flags, void** ptr) {
  hsa_region_t region;
  region.handle = memory_pool.handle;
  return hsa_memory_allocate(region, size, ptr);
}

hsa_status_t HSA_API hsa_amd_memory_pool_free(void* ptr) {
  return hsa_memory_free(ptr);
}

hsa_status_t HSA_API
hsa_amd_memory_async_copy(void* dst, hsa_agent_t dst_agent, const void* src,
                          hsa_agent_t src_agent, size_t size,
                          uint32_t num_dep_signals,
                          const hsa_signal_t* dep_signals,
                          hsa_signal_t completion_signal) {
  for (uint32_t i = 0; i < num_dep_signals; ++i)
    hsa_signal_wait_acquire(dep_signals[i], HSA_SIGNAL_CONDITION_EQ,
                            0, UINT64_MAX, HSA_WAIT_STATE_ACTIVE);
  hsa_memory_copy(dst, src, size);
  hsa_signal_store_relaxed(completion_signal, 0);
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_agent_memory_pool_get_info(
        hsa_agent_t agent, hsa_amd_memory_pool_t memory_pool,
        hsa_amd_agent_memory_pool_info_t attribute, void* value) {
  switch (attribute) {
  case HSA_AMD_AGENT_MEMORY_POOL_INFO_ACCESS: {
    *(hsa_amd_memory_pool_access_t*)value =
      HSA_AMD_MEMORY_POOL_ACCESS_ALLOWED_BY_DEFAULT;
    break;
  }
  default:
    ABORT_UNIMPLEMENTED;
    break;
  }
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API
hsa_amd_agents_allow_access(uint32_t num_agents, const hsa_agent_t* agents,
                            const uint32_t* flags, const void* ptr) {
  // Assume all Agents have access to all regions by default.
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API
hsa_amd_memory_pool_can_migrate(hsa_amd_memory_pool_t src_memory_pool,
                                hsa_amd_memory_pool_t dst_memory_pool,
                                bool* result) {
  *result = true;
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_memory_migrate(const void* ptr,
                                            hsa_amd_memory_pool_t memory_pool,
                                            uint32_t flags) {
  ABORT_UNIMPLEMENTED;
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_memory_lock(void* host_ptr, size_t size,
                                         hsa_agent_t* agents, int num_agent,
                                         void** agent_ptr) {
  *agent_ptr = host_ptr;
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_memory_unlock(void* host_ptr) {
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API
hsa_amd_memory_fill(void* ptr, uint32_t value, size_t count) {
  ABORT_UNIMPLEMENTED;
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_interop_map_buffer(uint32_t num_agents,
                                                hsa_agent_t* agents,
                                                int interop_handle,
                                                uint32_t flags,
                                                size_t* size,
                                                void** ptr,
                                                size_t* metadata_size,
                                                const void** metadata) {
  ABORT_UNIMPLEMENTED;
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_interop_unmap_buffer(void* ptr) {
  ABORT_UNIMPLEMENTED;
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_image_create(
        hsa_agent_t agent,
        const hsa_ext_image_descriptor_t *image_descriptor,
        const hsa_amd_image_descriptor_t *image_layout,
        const void *image_data,
        hsa_access_permission_t access_permission,
        hsa_ext_image_t *image
) {
  ABORT_UNIMPLEMENTED;
  return HSA_STATUS_SUCCESS;
}
