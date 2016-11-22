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

#ifndef HSA_RUNTIME_ATOMIC_HH
#define HSA_RUNTIME_ATOMIC_HH

#include "MemoryOrder.hh"

namespace Atomic {

using phsa::ToGCCMemoryOrder;
using phsa::MemoryOrder;

MemoryOrder ToCompareExchangeFailure(MemoryOrder Success);

template <class T> void Store(T Value, T *Ptr, MemoryOrder MO) {
  __atomic_store_n(Ptr, Value, ToGCCMemoryOrder(MO));
}

template <class T> T FetchAdd(T Increment, T *Ptr, MemoryOrder MO) {
  return __atomic_fetch_add(Ptr, Increment, ToGCCMemoryOrder(MO));
}

template <class T> T FetchSub(T Decrement, T *Ptr, MemoryOrder MO) {
  return __atomic_fetch_sub(Ptr, Decrement, ToGCCMemoryOrder(MO));
}

template <class T> T FetchXor(T Value, T *Ptr, MemoryOrder MO) {
  return __atomic_fetch_xor(Ptr, Value, ToGCCMemoryOrder(MO));
}

template <class T> T FetchOr(T Value, T *Ptr, MemoryOrder MO) {
  return __atomic_fetch_or(Ptr, Value, ToGCCMemoryOrder(MO));
}

template <class T> T FetchAnd(T Value, T *Ptr, MemoryOrder MO) {
  return __atomic_fetch_and(Ptr, Value, ToGCCMemoryOrder(MO));
}

template <class T> T Load(T *Ptr, MemoryOrder MO) {
  return __atomic_load_n(Ptr, ToGCCMemoryOrder(MO));
}

template <class T> T Exchange(T Value, T *Ptr, MemoryOrder MO) {
  return __atomic_exchange_n(Ptr, Value, ToGCCMemoryOrder(MO));
}

template <class T>
bool CompareExchange(T *Value, T *ExpectedValue, T DesiredValue,
                     MemoryOrder Success) {
  return __atomic_compare_exchange_n(
      Value, ExpectedValue, DesiredValue, false, ToGCCMemoryOrder(Success),
      ToGCCMemoryOrder(ToCompareExchangeFailure(Success)));
}

} // namespace phsa

#endif // HSA_RUNTIME_ATOMIC_HH
