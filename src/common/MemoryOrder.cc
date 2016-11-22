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

#include "MemoryOrder.hh"

namespace phsa {

std::memory_order ToStdMemoryOrder(MemoryOrder MO) {
  switch (MO) {
  case MemoryOrder::Relaxed:
    return std::memory_order::memory_order_relaxed;
  case MemoryOrder::Consume:
    return std::memory_order::memory_order_consume;
  case MemoryOrder::Acquire:
    return std::memory_order::memory_order_acquire;
  case MemoryOrder::Release:
    return std::memory_order::memory_order_release;
  case MemoryOrder::AcquireRelease:
    return std::memory_order::memory_order_acq_rel;
  case MemoryOrder::SequentiallyConsistent:
    return std::memory_order::memory_order_seq_cst;
  }
}

#ifdef __GNUC__

int ToGCCMemoryOrder(MemoryOrder MO) {
  switch (MO) {
  case MemoryOrder::Relaxed:
    return __ATOMIC_RELAXED;
  case MemoryOrder::Consume:
    return __ATOMIC_CONSUME;
  case MemoryOrder::Acquire:
    return __ATOMIC_ACQUIRE;
  case MemoryOrder::Release:
    return __ATOMIC_RELEASE;
  case MemoryOrder::AcquireRelease:
    return __ATOMIC_ACQ_REL;
  case MemoryOrder::SequentiallyConsistent:
    return __ATOMIC_SEQ_CST;
  }
}

}

#endif
