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

#ifndef HSA_RUNTIME_SIGNAL_HH
#define HSA_RUNTIME_SIGNAL_HH

#include <atomic>
#include <chrono>
#include <mutex>
#include <common/MemoryOrder.hh>

#include "hsa.h"
#include "HSAObjectMapping.hh"

namespace phsa {

// Signal is an interface to a HSA signal.
//
// For an implementation example, see `src/Devices/CPU/GccBuiltinSignal.[cc|hh]`.
class Signal : public HSAObjectMapping<Signal, hsa_signal_t> {
public:
  Signal() {}

  Signal(HSAObjectMapping<Signal, hsa_signal_t>::Mapper ObjectMapper,
         bool RegisterObject = true)
      : HSAObjectMapping<Signal, hsa_signal_t>(ObjectMapper, RegisterObject) {}

  using UnaryPredicate = std::function<bool(hsa_signal_value_t)>;

  // See "HSA Runtime Programmer’s Reference Manual" for detailed information
  // about the semantics and guarantees of the methods below.
  virtual void store(hsa_signal_value_t Value, MemoryOrder MO) = 0;

  virtual hsa_signal_value_t load(MemoryOrder MO) = 0;

  virtual hsa_signal_value_t
  wait(UnaryPredicate Condition,
       std::chrono::high_resolution_clock::duration Timeout,
       MemoryOrder MO) = 0;
  virtual hsa_signal_value_t exchange(hsa_signal_value_t Value,
                                      MemoryOrder MO) = 0;
  virtual void subtract(hsa_signal_value_t value, MemoryOrder MO) = 0;
  virtual void add(hsa_signal_value_t value, MemoryOrder MO) = 0;
  virtual void xor_(hsa_signal_value_t value, MemoryOrder MO) = 0;
  virtual void and_(hsa_signal_value_t value, MemoryOrder MO) = 0;
  virtual void or_(hsa_signal_value_t value, MemoryOrder MO) = 0;

  virtual hsa_signal_value_t compareExchange(hsa_signal_value_t Expected,
                                             hsa_signal_value_t Value,
                                             MemoryOrder MO) = 0;
};

} // namespace phsa

#endif // HSA_RUNTIME_SIGNAL_HH
