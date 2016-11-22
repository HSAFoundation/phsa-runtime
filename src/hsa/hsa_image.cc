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

#include "hsa_ext_image.h"

hsa_status_t HSA_API hsa_ext_image_get_capability(
    hsa_agent_t agent, hsa_ext_image_geometry_t geometry,
    const hsa_ext_image_format_t *image_format, uint32_t *capability_mask) {
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_ext_image_data_get_info(
    hsa_agent_t agent, const hsa_ext_image_descriptor_t *image_descriptor,
    hsa_access_permission_t access_permission,
    hsa_ext_image_data_info_t *image_data_info) {
  return HSA_STATUS_SUCCESS;
}
hsa_status_t HSA_API hsa_ext_image_create(
    hsa_agent_t agent, const hsa_ext_image_descriptor_t *image_descriptor,
    const void *image_data, hsa_access_permission_t access_permission,
    hsa_ext_image_t *image) {
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_ext_image_destroy(hsa_agent_t agent,
                                           hsa_ext_image_t image) {
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_ext_image_copy(hsa_agent_t agent,
                                        hsa_ext_image_t src_image,
                                        const hsa_dim3_t *src_offset,
                                        hsa_ext_image_t dst_image,
                                        const hsa_dim3_t *dst_offset,
                                        const hsa_dim3_t *range) {
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_ext_image_import(
    hsa_agent_t agent, const void *src_memory, size_t src_row_pitch,
    size_t src_slice_pitch, hsa_ext_image_t dst_image,
    const hsa_ext_image_region_t *image_region) {
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_ext_image_export(
    hsa_agent_t agent, hsa_ext_image_t src_image, void *dst_memory,
    size_t dst_row_pitch, size_t dst_slice_pitch,
    const hsa_ext_image_region_t *image_region) {
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API
hsa_ext_image_clear(hsa_agent_t agent, hsa_ext_image_t image, const void *data,
                    const hsa_ext_image_region_t *image_region) {
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_ext_sampler_create(
    hsa_agent_t agent, const hsa_ext_sampler_descriptor_t *sampler_descriptor,
    hsa_ext_sampler_t *sampler) {
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_ext_sampler_destroy(hsa_agent_t agent,
                                             hsa_ext_sampler_t sampler) {
  return HSA_STATUS_SUCCESS;
}
