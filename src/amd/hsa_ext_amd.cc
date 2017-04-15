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

#include "hsa_ext_amd.h"

#include <memory>

hsa_status_t HSA_API hsa_amd_coherency_get_type(hsa_agent_t agent,
                                                hsa_amd_coherency_type_t* type) {
    return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_coherency_set_type(hsa_agent_t agent,
                                                hsa_amd_coherency_type_t type) {
    return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_profiling_set_profiler_enabled(hsa_queue_t* queue, int enable) {
    return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_profiling_async_copy_enable(bool enable) {
    return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_profiling_get_dispatch_time(
        hsa_agent_t agent, hsa_signal_t signal,
        hsa_amd_profiling_dispatch_time_t* time) {
    return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_profiling_get_async_copy_time(
        hsa_signal_t signal, hsa_amd_profiling_async_copy_time_t* time) {
    return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_profiling_convert_tick_to_system_domain(hsa_agent_t agent,
                                                uint64_t agent_tick,
                                                uint64_t* system_tick) {
    return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_signal_async_handler(hsa_signal_t signal,
                             hsa_signal_condition_t cond,
                             hsa_signal_value_t value,
                             hsa_amd_signal_handler handler, void* arg) {
    return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_async_function(void (*callback)(void* arg), void* arg) {
    return HSA_STATUS_SUCCESS;
}

uint32_t HSA_API
hsa_amd_signal_wait_any(uint32_t signal_count, hsa_signal_t* signals,
                        hsa_signal_condition_t* conds,
                        hsa_signal_value_t* values, uint64_t timeout_hint,
                        hsa_wait_state_t wait_hint,
                        hsa_signal_value_t* satisfying_value) {
    return 0;
}

hsa_status_t HSA_API hsa_amd_image_get_info_max_dim(hsa_agent_t agent,
                                                    hsa_agent_info_t attribute,
                                                    void* value) {
    return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_queue_cu_set_mask(const hsa_queue_t* queue,
                                               uint32_t num_cu_mask_count,
                                               const uint32_t* cu_mask) {
    return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API
hsa_amd_memory_pool_get_info(hsa_amd_memory_pool_t memory_pool,
                             hsa_amd_memory_pool_info_t attribute,
                             void* value) {
    return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_agent_iterate_memory_pools(
        hsa_agent_t agent,
        hsa_status_t (*callback)(hsa_amd_memory_pool_t memory_pool, void* data),
        void* data) {
    return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API
hsa_amd_memory_pool_allocate(hsa_amd_memory_pool_t memory_pool, size_t size,
                             uint32_t flags, void** ptr) {

    *ptr = std::malloc(size);
    return HSA_STATUS_SUCCESS;

}

hsa_status_t HSA_API hsa_amd_memory_pool_free(void* ptr) {
    return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API
hsa_amd_memory_async_copy(void* dst, hsa_agent_t dst_agent, const void* src,
                          hsa_agent_t src_agent, size_t size,
                          uint32_t num_dep_signals,
                          const hsa_signal_t* dep_signals,
                          hsa_signal_t completion_signal) {
    return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_agent_memory_pool_get_info(
        hsa_agent_t agent, hsa_amd_memory_pool_t memory_pool,
        hsa_amd_agent_memory_pool_info_t attribute, void* value) {
    return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API
hsa_amd_agents_allow_access(uint32_t num_agents, const hsa_agent_t* agents,
                            const uint32_t* flags, const void* ptr) {
    return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API
hsa_amd_memory_pool_can_migrate(hsa_amd_memory_pool_t src_memory_pool,
                                hsa_amd_memory_pool_t dst_memory_pool,
                                bool* result) {
    return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_memory_migrate(const void* ptr,
                                            hsa_amd_memory_pool_t memory_pool,
                                            uint32_t flags) {
    return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_memory_lock(void* host_ptr, size_t size,
                                         hsa_agent_t* agents, int num_agent,
                                         void** agent_ptr) {
    return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_memory_unlock(void* host_ptr) {
    return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API
hsa_amd_memory_fill(void* ptr, uint32_t value, size_t count) {
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
    return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_amd_interop_unmap_buffer(void* ptr) {
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
    return HSA_STATUS_SUCCESS;
}