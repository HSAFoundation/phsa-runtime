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

#ifndef HSA_RUNTIME_AGENT_HH
#define HSA_RUNTIME_AGENT_HH

#include <array>
#include <mutex>
#include <list>
#include <string>

#include "hsa.h"
#include "HSAObjectMapping.hh"
#include "ISA.hh"
#include "Version.hh"
#include "Queue.hh"

namespace phsa {

class MemoryRegion;
class Signal;

class Agent : public HSAObjectMapping<Agent, hsa_agent_t> {
public:
  virtual Queue *createQueue(uint32_t Size, hsa_queue_type_t Type,
                             Queue::QueueCallback CB) = 0;

  using queue_iterator = std::list<Queue *>::iterator;
  using const_queue_iterator = std::list<Queue *>::const_iterator;
  queue_iterator queue_begin() { return Queues.begin(); }
  queue_iterator queue_end() { return Queues.end(); }

  const_queue_iterator queue_cbegin() const { return Queues.cbegin(); }
  const_queue_iterator queue_cend() const { return Queues.cend(); }
  std::size_t getQueueCount() const { return Queues.size(); }

  void registerMemoryRegion(MemoryRegion *MR);
  void setKernargMemoryRegion(MemoryRegion *MR) { KernargRegion = MR; }
  void setUMQMemoryRegion(MemoryRegion *MR) { UMQRegion = MR; }
  void setGroupMemoryRegion(MemoryRegion *MR) { GroupMemoryRegion = MR; }

  using region_iterator = std::list<MemoryRegion *>::iterator;
  using const_region_iterator = std::list<MemoryRegion *>::const_iterator;
  region_iterator region_begin() { return MemoryRegions.begin(); }
  region_iterator region_end() { return MemoryRegions.end(); }

  const_region_iterator region_cbegin() const { return MemoryRegions.cbegin(); }
  const_region_iterator region_cend() const { return MemoryRegions.cend(); }

  virtual std::string getName() const = 0;
  virtual std::string getVendor() const = 0;
  virtual hsa_default_float_rounding_mode_t getFloatRoundingMode() const = 0;
  virtual hsa_profile_t getProfile() const = 0;
  virtual uint32_t getQueuesMax() const = 0;
  virtual uint32_t getQueueMinSize() const = 0;
  virtual uint32_t getQueueMaxSize() const = 0;
  virtual hsa_queue_type_t getQueueType() const = 0;
  virtual bool IsSupportedQueueType(hsa_queue_type_t t) const { return false; }
  virtual uint32_t getNUMAId() const = 0;
  virtual hsa_device_type_t getDeviceType() const = 0;
  virtual std::array<uint32_t, 4> getCacheSize() const = 0;
  virtual uint32_t getComputeUnitCount() const { return 1; };
  virtual const std::string getISA() const = 0;
  virtual Version getVersion() const = 0;

  // Terminates the execution of the given queue, in case it's being executed.
  virtual void terminateQueue(Queue *Q) = 0;

  /// Shut down the agent so it's safe to delete the instance.
  /// Blocks.
  virtual void shutDown() = 0;

protected:
  void registerQueue(Queue *Q) {
    boost::lock_guard<boost::shared_mutex> L(QueueLock);
    Queues.push_back(Q);
  }

  std::list<Queue *> getQueues() {
    boost::shared_lock<boost::shared_mutex> L(QueueLock);
    return Queues;
  }

protected:
  /// The region from which to allocate kernel arguments.
  MemoryRegion *KernargRegion = nullptr;
  /// The region from which to allocate the user mode queue entries.
  MemoryRegion *UMQRegion = nullptr;
  /// The region from which to allocate the group memory.
  MemoryRegion *GroupMemoryRegion = nullptr;

private:
  std::list<MemoryRegion *> MemoryRegions;
  std::list<Queue *> Queues;
  boost::shared_mutex QueueLock;
};

class AgentDispatchAgent : public Agent {
public:
};

class KernelDispatchAgent : public Agent {
public:
  KernelDispatchAgent() {}
  virtual bool hasFastF16Operation() const = 0;
  virtual uint32_t getWavefrontSize() const = 0;
  virtual std::array<uint16_t, 3> getWorkGroupMaxDim() const = 0;
  virtual uint32_t getWorkGroupMaxSize() const = 0;
  virtual hsa_dim3_t getGridMaxDim() const = 0;
  virtual uint32_t getGridMaxSize() const = 0;
  virtual uint32_t getFBarrierMaxSize() const = 0;
};

} // namespace phsa

#endif // HSA_RUNTIME_AGENT_HH
