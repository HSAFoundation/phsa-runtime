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

#ifndef HSA_RUNTIME_QUEUE_HH
#define HSA_RUNTIME_QUEUE_HH

#include <atomic>
#include <cinttypes>
#include <unordered_map>
#include <mutex>
#include <boost/thread/shared_mutex.hpp>

#include "common/Atomic.hh"
#include "hsa.h"
#include "phsa-queue.h"
#include <iostream>

namespace phsa {

class Agent;

// Queue class is an interface for implementing user mode queues and soft queues.
//
// For an implementation example, see `src/Devices/CPU/UserModeQueue.[cc|hh]`.
class Queue {
public:
  using QueueCallback =
      std::function<void(hsa_status_t, hsa_queue_t *, void *)>;

  Queue(hsa_queue_t *Queue, Agent *Owner = nullptr)
      : Owner(Owner), Destroyed(false), Inactivated(false) {
    registerQueue(this, Queue);
  }

  virtual ~Queue() {}

  Agent *ownerAgent() { return Owner; }

  virtual uint64_t loadWriteIndex(MemoryOrder MO) = 0;
  virtual void storeWriteIndex(uint64_t Value, MemoryOrder MO) = 0;
  virtual uint64_t addWriteIndex(uint64_t Increment, MemoryOrder MO) = 0;
  virtual uint64_t compareExchangeWriteIndex(uint64_t ExpectedValue,
                                             uint64_t Value,
                                             MemoryOrder MO) = 0;

  virtual uint64_t loadReadIndex(MemoryOrder MO) = 0;
  virtual void storeReadIndex(uint64_t Value, MemoryOrder MO) = 0;

  static Queue *FindQueue(const hsa_queue_t *HSAQueue);

  virtual void ExecuteCallback(hsa_status_t Status) = 0;

  void setDestroyed() { Destroyed = true; }
  bool isDestroyed() const { return Destroyed; }
  void setInactivated();
  bool isInactivated() const { return Inactivated; }

  static void garbageCollect();

  virtual hsa_queue_t *getHSAQueue() {
    return reinterpret_cast<hsa_queue_t *>(&HSAQueue);
  }

  // Used to internally keep book of packets that have been processed.
  void SetPacketProcessed(size_t QI, bool Flag)
    { if (PacketIsProcessed.size() == 0)
        PacketIsProcessed.resize(HSAQueue.hsa_queue.size, false);
      PacketIsProcessed[QI] = Flag; }
  bool IsPacketProcessed(size_t QI) const {
    if (PacketIsProcessed.size() == 0) return false;
    return PacketIsProcessed[QI]; }

protected:
  uint64_t getNextId() const { return ++QueueCount; }

  static std::unordered_map<const hsa_queue_t *, Queue *> Registry;
  static boost::shared_mutex RegistryLock;

private:
  static void registerQueue(Queue *Q, const hsa_queue_t *HSAQueue) {
    boost::lock_guard<boost::shared_mutex> L(RegistryLock);
    Registry[HSAQueue] = Q;
  }

  static void deregisterQueue(Queue *Q) {
    boost::lock_guard<boost::shared_mutex> L(RegistryLock);

    Registry.erase(Q->getHSAQueue());
  }

  static std::atomic<uint64_t> QueueCount;
  Agent *Owner;
  bool Destroyed;
  bool Inactivated;
  std::vector<bool> PacketIsProcessed;

protected:
  phsa_queue HSAQueue;
};

} // namespace phsa

#endif // HSA_RUNTIME_QUEUE_HH
