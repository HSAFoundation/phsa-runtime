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
#include "Agent.hh"
#include "hsa.h"
#include "Queue.hh"
#include "Runtime.hh"
#include "Signal.hh"
#include "MemoryRegion.hh"
#include "common/Logging.hh"

using phsa::MemoryOrder;

hsa_status_t HSA_API hsa_queue_create(
    hsa_agent_t agent, uint32_t size, hsa_queue_type_t type,
    void (*callback)(hsa_status_t status, hsa_queue_t *source, void *data),
    void *data, uint32_t private_segment_size, uint32_t group_segment_size,
    hsa_queue_t **queue) {

  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  // Check that size is non-zero and a power of two
  if (queue == nullptr || size == 0 || size & (size - 1) != 0) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  phsa::Agent *A = phsa::Agent::fromHSAObject(agent);

  if (A == nullptr) {
    return HSA_STATUS_ERROR_INVALID_AGENT;
  }

  if (A->getQueuesMax() <= A->getQueueCount()) {
    return HSA_STATUS_ERROR_OUT_OF_RESOURCES;
  }

  if (!A->IsSupportedQueueType(type)) {
    return HSA_STATUS_ERROR_INVALID_QUEUE_CREATION;
  }

  phsa::Queue *Q = A->createQueue(size, type, callback);

  if (Q == nullptr) {
    return HSA_STATUS_ERROR;
  }
  *queue = Q->getHSAQueue();

  (*queue)->id = (uint64_t)Q;

  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_soft_queue_create(hsa_region_t region, uint32_t size,
                                           hsa_queue_type_t type,
                                           uint32_t features,
                                           hsa_signal_t doorbell_signal,
                                           hsa_queue_t **queue) {
  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  // Check queue type
  if (type != HSA_QUEUE_TYPE_MULTI && type != HSA_QUEUE_TYPE_SINGLE) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  // Check that size is non-zero and a power of two
  if (queue == nullptr || size == 0 || size & (size - 1) != 0) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }
  phsa::MemoryRegion *MR = phsa::MemoryRegion::fromHSAObject(region);
  phsa::Signal *S = phsa::Signal::fromHSAObject(doorbell_signal);

  if (S == nullptr || MR == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  phsa::Queue *Q = phsa::Runtime::get().createSoftQueue(MR, size, type, S);

  if (Q == nullptr) {
    return HSA_STATUS_ERROR;
  }
  *queue = Q->getHSAQueue();

  (*queue)->id = (uint64_t)Q;
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_queue_destroy(hsa_queue_t *queue) {

  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  if (queue == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  phsa::Queue *Q = phsa::Queue::FindQueue(queue);

  if (Q == nullptr || Q->isDestroyed()) {
    return HSA_STATUS_ERROR_INVALID_QUEUE;
  }

  Q->setDestroyed();
  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_queue_inactivate(hsa_queue_t *queue) {
  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  if (queue == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  phsa::Queue *Q = phsa::Queue::FindQueue(queue);

  if (Q == nullptr || Q->isDestroyed()) {
    return HSA_STATUS_ERROR_INVALID_QUEUE;
  }

  Q->setInactivated();

  return HSA_STATUS_SUCCESS;
}

uint64_t HSA_API hsa_queue_load_read_index_acquire(const hsa_queue_t *queue) {

  phsa::Queue *Q = phsa::Queue::FindQueue(queue);
  return Q->loadReadIndex(MemoryOrder::Acquire);
}

uint64_t HSA_API hsa_queue_load_read_index_relaxed(const hsa_queue_t *queue) {
  phsa::Queue *Q = phsa::Queue::FindQueue(queue);
  return Q->loadReadIndex(MemoryOrder::Relaxed);
}

uint64_t HSA_API hsa_queue_load_write_index_acquire(const hsa_queue_t *queue) {
  phsa::Queue *Q = phsa::Queue::FindQueue(queue);
  return Q->loadWriteIndex(MemoryOrder::Acquire);
}

uint64_t HSA_API hsa_queue_load_write_index_relaxed(const hsa_queue_t *queue) {
  phsa::Queue *Q = phsa::Queue::FindQueue(queue);
  return Q->loadWriteIndex(MemoryOrder::Relaxed);
}

void HSA_API hsa_queue_store_write_index_relaxed(const hsa_queue_t *queue,
                                                 uint64_t value) {
  phsa::Queue *Q = phsa::Queue::FindQueue(queue);
  Q->storeWriteIndex(value, MemoryOrder::Relaxed);
}

void HSA_API hsa_queue_store_write_index_release(const hsa_queue_t *queue,
                                                 uint64_t value) {
  phsa::Queue *Q = phsa::Queue::FindQueue(queue);
  Q->storeWriteIndex(value, MemoryOrder::Release);
}

uint64_t HSA_API hsa_queue_cas_write_index_acq_rel(const hsa_queue_t *queue,
                                                   uint64_t expected,
                                                   uint64_t value) {
  phsa::Queue *Q = phsa::Queue::FindQueue(queue);
  return Q->compareExchangeWriteIndex(expected, value,
                                      MemoryOrder::AcquireRelease);
}

uint64_t HSA_API hsa_queue_cas_write_index_acquire(const hsa_queue_t *queue,
                                                   uint64_t expected,
                                                   uint64_t value) {
  phsa::Queue *Q = phsa::Queue::FindQueue(queue);
  return Q->compareExchangeWriteIndex(expected, value, MemoryOrder::Acquire);
}

uint64_t HSA_API hsa_queue_cas_write_index_relaxed(const hsa_queue_t *queue,
                                                   uint64_t expected,
                                                   uint64_t value) {
  phsa::Queue *Q = phsa::Queue::FindQueue(queue);
  return Q->compareExchangeWriteIndex(expected, value, MemoryOrder::Relaxed);
}

uint64_t HSA_API hsa_queue_cas_write_index_release(const hsa_queue_t *queue,
                                                   uint64_t expected,
                                                   uint64_t value) {
  phsa::Queue *Q = phsa::Queue::FindQueue(queue);
  return Q->compareExchangeWriteIndex(expected, value, MemoryOrder::Release);
}

uint64_t HSA_API hsa_queue_add_write_index_acq_rel(const hsa_queue_t *queue,
                                                   uint64_t value) {
  phsa::Queue *Q = phsa::Queue::FindQueue(queue);
  return Q->addWriteIndex(value, MemoryOrder::AcquireRelease);
}

uint64_t HSA_API hsa_queue_add_write_index_acquire(const hsa_queue_t *queue,
                                                   uint64_t value) {
  phsa::Queue *Q = phsa::Queue::FindQueue(queue);
  return Q->addWriteIndex(value, MemoryOrder::Acquire);
}

uint64_t HSA_API hsa_queue_add_write_index_relaxed(const hsa_queue_t *queue,
                                                   uint64_t value) {
  phsa::Queue *Q = phsa::Queue::FindQueue(queue);
  return Q->addWriteIndex(value, MemoryOrder::Relaxed);
}

uint64_t HSA_API hsa_queue_add_write_index_release(const hsa_queue_t *queue,
                                                   uint64_t value) {
  phsa::Queue *Q = phsa::Queue::FindQueue(queue);
  return Q->addWriteIndex(value, MemoryOrder::Release);
}

void HSA_API hsa_queue_store_read_index_relaxed(const hsa_queue_t *queue,
                                                uint64_t value) {
  phsa::Queue *Q = phsa::Queue::FindQueue(queue);
  Q->storeReadIndex(value, MemoryOrder::Relaxed);
}

void HSA_API hsa_queue_store_read_index_release(const hsa_queue_t *queue,
                                                uint64_t value) {
  phsa::Queue *Q = phsa::Queue::FindQueue(queue);
  Q->storeReadIndex(value, MemoryOrder::Release);
}
