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
#include "common/Logging.hh"
#include "Runtime.hh"
#include "Signal.hh"

using phsa::MemoryOrder;

hsa_status_t HSA_API hsa_signal_create(hsa_signal_value_t initial_value,
                                       uint32_t num_consumers,
                                       const hsa_agent_t *consumers,
                                       hsa_signal_t *signal) {
  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  if (signal == nullptr || (num_consumers > 0 && consumers == nullptr)) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  hsa_status_t status;
  std::tie(status, *signal) = phsa::Runtime::get().createSignal(initial_value);
  return status;
}

hsa_status_t HSA_API hsa_signal_destroy(hsa_signal_t signal) {
  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  if (signal.handle == 0) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  if (S == nullptr) {
    return HSA_STATUS_ERROR_INVALID_SIGNAL;
  }

  delete S;

  return HSA_STATUS_SUCCESS;
}

hsa_signal_value_t HSA_API hsa_signal_load_acquire(hsa_signal_t signal) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  return S->load(MemoryOrder::Acquire);
}

hsa_signal_value_t HSA_API hsa_signal_load_relaxed(hsa_signal_t signal) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  return S->load(MemoryOrder::Relaxed);
}

void HSA_API hsa_signal_store_relaxed(hsa_signal_t signal,
                                      hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  S->store(value, MemoryOrder::Relaxed);
}

void HSA_API hsa_signal_store_release(hsa_signal_t signal,
                                      hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  S->store(value, MemoryOrder::Release);
}

hsa_signal_value_t HSA_API
hsa_signal_exchange_acq_rel(hsa_signal_t signal, hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  return S->exchange(value, MemoryOrder::AcquireRelease);
}

hsa_signal_value_t HSA_API
hsa_signal_exchange_acquire(hsa_signal_t signal, hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  return S->exchange(value, MemoryOrder::Acquire);
}

hsa_signal_value_t HSA_API
hsa_signal_exchange_relaxed(hsa_signal_t signal, hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  return S->exchange(value, MemoryOrder::Relaxed);
}

hsa_signal_value_t HSA_API
hsa_signal_exchange_release(hsa_signal_t signal, hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  return S->exchange(value, MemoryOrder::Release);
}

hsa_signal_value_t HSA_API hsa_signal_cas_acq_rel(hsa_signal_t signal,
                                                  hsa_signal_value_t expected,
                                                  hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  return S->compareExchange(expected, value, MemoryOrder::AcquireRelease);
}

hsa_signal_value_t HSA_API hsa_signal_cas_acquire(hsa_signal_t signal,
                                                  hsa_signal_value_t expected,
                                                  hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  return S->compareExchange(expected, value, MemoryOrder::Acquire);
}

hsa_signal_value_t HSA_API hsa_signal_cas_relaxed(hsa_signal_t signal,
                                                  hsa_signal_value_t expected,
                                                  hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  return S->compareExchange(expected, value, MemoryOrder::Relaxed);
}

hsa_signal_value_t HSA_API hsa_signal_cas_release(hsa_signal_t signal,
                                                  hsa_signal_value_t expected,
                                                  hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  return S->compareExchange(expected, value, MemoryOrder::Release);
}

void HSA_API hsa_signal_add_acq_rel(hsa_signal_t signal,
                                    hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  S->add(value, MemoryOrder::AcquireRelease);
}

void HSA_API hsa_signal_add_acquire(hsa_signal_t signal,
                                    hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  S->add(value, MemoryOrder::Acquire);
}

void HSA_API hsa_signal_add_relaxed(hsa_signal_t signal,
                                    hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  S->add(value, MemoryOrder::Relaxed);
}

void HSA_API hsa_signal_add_release(hsa_signal_t signal,
                                    hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  S->add(value, MemoryOrder::Release);
}

void HSA_API hsa_signal_subtract_acq_rel(hsa_signal_t signal,
                                         hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  S->subtract(value, MemoryOrder::AcquireRelease);
}

void HSA_API hsa_signal_subtract_acquire(hsa_signal_t signal,
                                         hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  S->subtract(value, MemoryOrder::Acquire);
}

void HSA_API hsa_signal_subtract_relaxed(hsa_signal_t signal,
                                         hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  S->subtract(value, MemoryOrder::Relaxed);
}

void HSA_API hsa_signal_subtract_release(hsa_signal_t signal,
                                         hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  S->subtract(value, MemoryOrder::Release);
}

void HSA_API hsa_signal_and_acq_rel(hsa_signal_t signal,
                                    hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  S->and_(value, MemoryOrder::AcquireRelease);
}

void HSA_API hsa_signal_and_acquire(hsa_signal_t signal,
                                    hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  S->and_(value, MemoryOrder::Acquire);
}

void HSA_API hsa_signal_and_relaxed(hsa_signal_t signal,
                                    hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  S->and_(value, MemoryOrder::Relaxed);
}

void HSA_API hsa_signal_and_release(hsa_signal_t signal,
                                    hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  S->and_(value, MemoryOrder::Release);
}

void HSA_API hsa_signal_or_acq_rel(hsa_signal_t signal,
                                   hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  S->or_(value, MemoryOrder::AcquireRelease);
}

void HSA_API hsa_signal_or_acquire(hsa_signal_t signal,
                                   hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  S->or_(value, MemoryOrder::Acquire);
}

void HSA_API hsa_signal_or_relaxed(hsa_signal_t signal,
                                   hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  S->or_(value, MemoryOrder::Relaxed);
}

void HSA_API hsa_signal_or_release(hsa_signal_t signal,
                                   hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  S->or_(value, MemoryOrder::Release);
}

void HSA_API hsa_signal_xor_acq_rel(hsa_signal_t signal,
                                    hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  S->xor_(value, MemoryOrder::AcquireRelease);
}

void HSA_API hsa_signal_xor_acquire(hsa_signal_t signal,
                                    hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  S->xor_(value, MemoryOrder::Acquire);
}

void HSA_API hsa_signal_xor_relaxed(hsa_signal_t signal,
                                    hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  S->xor_(value, MemoryOrder::Relaxed);
}

void HSA_API hsa_signal_xor_release(hsa_signal_t signal,
                                    hsa_signal_value_t value) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);
  S->xor_(value, MemoryOrder::Release);
}

namespace {
phsa::Signal::UnaryPredicate ToPredicate(hsa_signal_condition_t condition,
                                         hsa_signal_value_t compare_value) {
  switch (condition) {

  case HSA_SIGNAL_CONDITION_EQ:
    return [compare_value](hsa_signal_value_t Value) {
      return Value == compare_value;
    };
  case HSA_SIGNAL_CONDITION_NE:
    return [compare_value](hsa_signal_value_t Value) {
      return Value != compare_value;
    };
  case HSA_SIGNAL_CONDITION_LT:
    return [compare_value](hsa_signal_value_t Value) {
      return Value < compare_value;
    };
  case HSA_SIGNAL_CONDITION_GTE:
    return [compare_value](hsa_signal_value_t Value) {
      return Value >= compare_value;
    };
  }
}

std::chrono::high_resolution_clock::duration ToStdPeriod(uint64_t Timeout) {
  if (Timeout == std::numeric_limits<uint64_t>::max()) {
    return std::chrono::high_resolution_clock::duration::max();
  }

  return std::chrono::high_resolution_clock::duration(Timeout);
}
}

hsa_signal_value_t HSA_API
hsa_signal_wait_acquire(hsa_signal_t signal, hsa_signal_condition_t condition,
                        hsa_signal_value_t compare_value, uint64_t timeout_hint,
                        hsa_wait_state_t wait_state_hint) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);

  // TODO: wait_state
  return S->wait(ToPredicate(condition, compare_value),
                 ToStdPeriod(timeout_hint), MemoryOrder::Acquire);
}

hsa_signal_value_t HSA_API
hsa_signal_wait_relaxed(hsa_signal_t signal, hsa_signal_condition_t condition,
                        hsa_signal_value_t compare_value, uint64_t timeout_hint,
                        hsa_wait_state_t wait_state_hint) {
  phsa::Signal *S = phsa::Signal::fromHSAObject(signal);

  return S->wait(ToPredicate(condition, compare_value),
                 ToStdPeriod(timeout_hint), MemoryOrder::Relaxed);
}
