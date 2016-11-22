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

#ifndef HSA_RUNTIME_STDATOMICSIGNAL_HH
#define HSA_RUNTIME_STDATOMICSIGNAL_HH

#include "common/Atomic.hh"
#include "HSAObjectMapping.hh"
#include "Signal.hh"

namespace phsa {

/**
 * A signal based on an opaque std::atomic object.
 *
 * This type cannot be utilized in heterogeneous setups where the std::signal
 * data layout can vary per device and target compiler. However, it's useful
 * for homogeneous CPU platforms as it's using the C++11 standard for
 * implementation.
 *
 * @note gccbrig assumes the signal value to be in the beginning
 * of the signal object, thus this implementation won't work if that is not
 * the case.
 */
class StdAtomicSignal : public Signal {
public:
  /**
   * @param InitialValue The value the signal should be initialized to.
   */
  StdAtomicSignal(hsa_signal_value_t InitialValue)
      : Signal([](const Signal *Object) {
          const StdAtomicSignal *MS =
              static_cast<const StdAtomicSignal *>(Object);
          hsa_signal_t Ret;
          Ret.handle = reinterpret_cast<uint64_t>(&MS->CurrentValue);
          return Ret;
        }) {
    CurrentValue.store(InitialValue);
  }

  virtual hsa_signal_value_t load(MemoryOrder MO) override {
    return CurrentValue.load(ToStdMemoryOrder(MO));
  }

  virtual void store(hsa_signal_value_t Value, MemoryOrder MO) override {
    CurrentValue.store(Value, ToStdMemoryOrder(MO));
  }

  virtual hsa_signal_value_t
  wait(UnaryPredicate Condition,
       std::chrono::high_resolution_clock::duration Timeout,
       MemoryOrder MO) override {
    // TODO: optimize this with a lock free queue and a condition variable
    return wait(Condition, Timeout, ToStdMemoryOrder(MO));
  }

  virtual hsa_signal_value_t exchange(hsa_signal_value_t Value,
                                      MemoryOrder MO) override {
    return CurrentValue.exchange(Value, ToStdMemoryOrder(MO));
  }

  virtual hsa_signal_value_t compareExchange(hsa_signal_value_t Expected,
                                             hsa_signal_value_t Value,
                                             MemoryOrder MO) override {
    return CurrentValue.compare_exchange_strong(
        Expected, Value, ToStdMemoryOrder(MO),
        ToStdMemoryOrder(Atomic::ToCompareExchangeFailure(MO)));
  }

  virtual void add(hsa_signal_value_t value, MemoryOrder MO) override {
    CurrentValue.fetch_add(value, ToStdMemoryOrder(MO));
  }

  virtual void subtract(hsa_signal_value_t value, MemoryOrder MO) override {
    CurrentValue.fetch_sub(value, ToStdMemoryOrder(MO));
  }

  virtual void xor_(hsa_signal_value_t value, MemoryOrder MO) override {
    CurrentValue.fetch_xor(value, ToStdMemoryOrder(MO));
  }

  virtual void and_(hsa_signal_value_t value, MemoryOrder MO) override {
    CurrentValue.fetch_and(value, ToStdMemoryOrder(MO));
  }

  virtual void or_(hsa_signal_value_t value, MemoryOrder MO) override {
    CurrentValue.fetch_or(value, ToStdMemoryOrder(MO));
  }

private:
  // FIXME: this is not guaranteed to catch all changes,
  // in case there is an update that makes the condition
  // true later updated to one that makes it false (the ABA problem).
  hsa_signal_value_t wait(UnaryPredicate Condition,
                          std::chrono::high_resolution_clock::duration Timeout,
                          std::memory_order MemoryOrder) {

    std::function<std::chrono::high_resolution_clock::time_point(void)> Now =
        []() { return std::chrono::high_resolution_clock::now(); };
    std::chrono::high_resolution_clock::time_point StartTime = Now();

    hsa_signal_value_t Value;
    do {
      Value = CurrentValue.load(MemoryOrder);

      if (Now() - StartTime > Timeout)
        break;
    } while (!Condition(Value));

    return Value;
  }
  std::atomic<hsa_signal_value_t> CurrentValue;
};

} // namespace phsa

#endif // HSA_RUNTIME_MEMORYSIGNAL_HH
