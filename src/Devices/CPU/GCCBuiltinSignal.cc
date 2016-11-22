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

#include "GCCBuiltinSignal.hh"
#include "MemoryRegion.hh"
#include "common/Logging.hh"
#include "common/Atomic.hh"

namespace phsa {

GCCBuiltinSignal::GCCBuiltinSignal(hsa_signal_value_t InitialValue,
                                   MemoryRegion &Region)
    : CurrentValue((hsa_signal_value_t *)Region.allocate(
          sizeof(hsa_signal_value_t), sizeof(hsa_signal_value_t))),
      SignalMemory(Region), Signal([](const Signal *Object) {
        const GCCBuiltinSignal *MS =
            static_cast<const GCCBuiltinSignal *>(Object);
        hsa_signal_t Ret;
        Ret.handle = reinterpret_cast<uint64_t>(MS->CurrentValue);
        return Ret;
      }, false) {
  *CurrentValue = InitialValue;
  // Now register the object after it has been completely constructed and
  // the custom mapping function that relied on the lower parts can be
  // reliably called.
  registerObject(this);
}

GCCBuiltinSignal::~GCCBuiltinSignal() {
  SignalMemory.free((void *)CurrentValue);
}

hsa_signal_value_t GCCBuiltinSignal::load(MemoryOrder MO) {
  switch (MO) {
  case MemoryOrder::Acquire:
    return loadAcquire();
  case MemoryOrder::Relaxed:
    return loadRelaxed();
  default:
    ABORT_UNIMPLEMENTED;
  }
}

hsa_signal_value_t GCCBuiltinSignal::loadAcquire() {
  hsa_signal_value_t Value = *CurrentValue;
  Runtime::get().memoryFence();
  return Value;
}

hsa_signal_value_t GCCBuiltinSignal::loadRelaxed() { return *CurrentValue; }

void GCCBuiltinSignal::store(hsa_signal_value_t Value, MemoryOrder MO) {
  switch (MO) {
  case MemoryOrder::Relaxed:
    storeRelaxed(Value);
    return;
  case MemoryOrder::Release:
    storeRelease(Value);
    return;
  default:
    ABORT_UNIMPLEMENTED;
  }
}

void GCCBuiltinSignal::storeRelaxed(hsa_signal_value_t Value) {
  *CurrentValue = Value;
}

void GCCBuiltinSignal::storeRelease(hsa_signal_value_t Value) {
  Runtime::get().memoryFence();
  *CurrentValue = Value;
  // Call just in case the Runtime has Agents requiring explicit
  // value updates. TOFIX: in that case there are cerntaily
  // data races in case the value is not "owned".
  Runtime::get().flush((void *)CurrentValue, sizeof(CurrentValue));
}

hsa_signal_value_t
GCCBuiltinSignal::wait(UnaryPredicate Condition,
                       std::chrono::high_resolution_clock::duration Timeout,
                       MemoryOrder MO) {

  std::function<std::chrono::high_resolution_clock::time_point(void)> Now =
      []() { return std::chrono::high_resolution_clock::now(); };
  std::chrono::high_resolution_clock::time_point StartTime = Now();

  hsa_signal_value_t Value;
  do {
    Value = load(MO);
    if (Now() - StartTime > Timeout)
      break;
  } while (!Condition(Value));

  return Value;
}

hsa_signal_value_t GCCBuiltinSignal::exchange(hsa_signal_value_t Value,
                                              MemoryOrder MO) {
  return Atomic::Exchange(Value, (hsa_signal_value_t *)CurrentValue, MO);
}

hsa_signal_value_t
GCCBuiltinSignal::compareExchange(hsa_signal_value_t Expected,
                                  hsa_signal_value_t Value, MemoryOrder MO) {
  Atomic::CompareExchange((hsa_signal_value_t *)CurrentValue, &Expected, Value,
                          MO);
  return Expected;
}

void GCCBuiltinSignal::subtract(hsa_signal_value_t value, MemoryOrder MO) {
  Atomic::FetchSub(value, (hsa_signal_value_t *)CurrentValue, MO);
}

void GCCBuiltinSignal::add(hsa_signal_value_t value, MemoryOrder MO) {
  Atomic::FetchAdd(value, (hsa_signal_value_t *)CurrentValue, MO);
}

void GCCBuiltinSignal::xor_(hsa_signal_value_t value, MemoryOrder MO) {
  Atomic::FetchXor(value, (hsa_signal_value_t *)CurrentValue, MO);
}

void GCCBuiltinSignal::and_(hsa_signal_value_t value, MemoryOrder MO) {
  Atomic::FetchAnd(value, (hsa_signal_value_t *)CurrentValue, MO);
}

void GCCBuiltinSignal::or_(hsa_signal_value_t value, MemoryOrder MO) {
  Atomic::FetchOr(value, (hsa_signal_value_t *)CurrentValue, MO);
}

} // namespace phsa

