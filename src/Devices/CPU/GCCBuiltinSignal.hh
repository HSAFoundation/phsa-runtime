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
 * @author pekka.jaaskelainen@parmance.com for General Processor Tech.
 */

#ifndef HSA_RUNTIME_GCCBUILTINSIGNAL_HH
#define HSA_RUNTIME_GCCBUILTINSIGNAL_HH

#include "HSAObjectMapping.hh"
#include "Signal.hh"
#include "Runtime.hh"

namespace phsa {

class MemoryRegion;

/**
 * A transparent signal of which value is updated with gcc atomic built-ins,
 * assuming the value is in a fine-grained coherent memory.
 *
 * This implementation works in case the gcc atomic built-ins are supported by
 * all of the target compilers and if the built-ins implement the necessary
 * visibility across the devices accessing the signal. Assumes aligned loads
 * and stores are atomic.

 * See: https://gcc.gnu.org/onlinedocs/gcc-4.4.3/gcc/Atomic-Builtins.html
 */
class GCCBuiltinSignal : public Signal {
public:
  /**
   * @param InitialValue The value the signal should be initialized to.
   * @param Region The MemoryRegion from which the value should be allocated.
   */
  GCCBuiltinSignal(hsa_signal_value_t InitialValue, MemoryRegion &Region);
  ~GCCBuiltinSignal();

  virtual hsa_signal_value_t load(MemoryOrder MO) override;

  virtual void store(hsa_signal_value_t Value, MemoryOrder MO) override;

  virtual hsa_signal_value_t
  wait(UnaryPredicate Condition,
       std::chrono::high_resolution_clock::duration Timeout,
       MemoryOrder MO) override;

  virtual void subtract(hsa_signal_value_t value, MemoryOrder MO) override;
  virtual void add(hsa_signal_value_t value, MemoryOrder MO) override;
  virtual void xor_(hsa_signal_value_t value, MemoryOrder MO) override;
  virtual void and_(hsa_signal_value_t value, MemoryOrder MO) override;
  virtual void or_(hsa_signal_value_t value, MemoryOrder MO) override;

  virtual hsa_signal_value_t exchange(hsa_signal_value_t Value,
                                      MemoryOrder MO) override;
  virtual hsa_signal_value_t compareExchange(hsa_signal_value_t Expected,
                                             hsa_signal_value_t Value,
                                             MemoryOrder MO) override;

private:
  void storeRelease(hsa_signal_value_t Value);
  void storeRelaxed(hsa_signal_value_t Value);

  hsa_signal_value_t loadAcquire();
  hsa_signal_value_t loadRelaxed();
  // Points to the value in the shared fine-grained coherent memory.
  hsa_signal_value_t volatile *CurrentValue;
  // The memory region from where the signal's value is allocated.
  MemoryRegion &SignalMemory;
};

} // namespace phsa

#endif // HSA_RUNTIME_MEMORYSIGNAL_HH
