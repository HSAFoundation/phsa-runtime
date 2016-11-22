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

#ifndef HSA_RUNTIME_USERMODEQUEUE_HH
#define HSA_RUNTIME_USERMODEQUEUE_HH

#include "AQLPacket.hh"
#include "Queue.hh"

namespace phsa {

class MemoryRegion;
class Agent;
class Signal;

class UserModeQueue : public Queue {
public:
  UserModeQueue(uint32_t Size, hsa_queue_type_t Type,
                MemoryRegion &TargetRegion, QueueCallback CB,
                Agent *Owner = nullptr, Signal *Doorbell = nullptr);

  ~UserModeQueue();

  void ExecuteCallback(hsa_status_t Status);

  virtual uint64_t loadWriteIndex(MemoryOrder MO) override {
    return Atomic::Load(&HSAQueue.write_index, MO);
  }

  virtual void storeWriteIndex(uint64_t Value, MemoryOrder MO) override {
    Atomic::Store(Value, &HSAQueue.write_index, MO);
  }

  virtual uint64_t addWriteIndex(uint64_t Increment, MemoryOrder MO) override {
    return Atomic::FetchAdd(Increment, &HSAQueue.write_index, MO);
  }

  virtual uint64_t compareExchangeWriteIndex(uint64_t ExpectedValue,
                                             uint64_t Value,
                                             MemoryOrder MO) override {
    Atomic::CompareExchange(&HSAQueue.write_index, &ExpectedValue, Value, MO);

    return ExpectedValue;
  }

  virtual uint64_t loadReadIndex(MemoryOrder MO) override {
    return Atomic::Load(&HSAQueue.read_index, MO);
  }

  virtual void storeReadIndex(uint64_t Value, MemoryOrder MO) override {
    Atomic::Store(Value, &HSAQueue.read_index, MO);
  }

private:
  MemoryRegion &Region;
  QueueCallback CB;
  // Set to true in case the queue has created the doorbell signal, and
  // thus should delete it. This is the case with non soft-queues.
  bool IsDoorbellOwned = false;
};

} // namespace phsa

#endif // HSA_RUNTIME_USERMODEQUEUE_HH
