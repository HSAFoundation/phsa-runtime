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

#ifndef HSA_RUNTIME_CPUAGENT_HH
#define HSA_RUNTIME_CPUAGENT_HH

#include <atomic>
#include <cassert>
#include <thread>
#include <cfenv>

#include "Agent.hh"
#include "Queue.hh"

namespace phsa {

class CPUKernelAgent : public KernelDispatchAgent {
public:
  CPUKernelAgent(MemoryRegion &QueueMemRegion);
  ~CPUKernelAgent();

  virtual Queue *createQueue(uint32_t Size, hsa_queue_type_t Type,
                             Queue::QueueCallback CB) override;

  virtual std::string getName() const override {
    return "phsa generic CPU agent";
  }

  virtual std::string getVendor() const override { return "UNKNOWN"; }

  virtual hsa_default_float_rounding_mode_t
  getFloatRoundingMode() const override {
    switch (fegetround()) {
    case FE_TONEAREST: {
      return HSA_DEFAULT_FLOAT_ROUNDING_MODE_NEAR;
    }
    case FE_TOWARDZERO: {
      return HSA_DEFAULT_FLOAT_ROUNDING_MODE_ZERO;
    }
    case FE_UPWARD:
    case FE_DOWNWARD:
    default: { break; }
    }

    assert(false && "Unsupported float rounding mode");
  }

  virtual hsa_profile_t getProfile() const override { return HSA_PROFILE_FULL; }

  virtual uint32_t getQueuesMax() const override { return 1024; }

  virtual uint32_t getQueueMinSize() const override { return 1; }

  virtual uint32_t getQueueMaxSize() const override { return 16; }
  virtual bool IsSupportedQueueType(hsa_queue_type_t t) const { return true; }

  virtual hsa_queue_type_t getQueueType() const override {
    return HSA_QUEUE_TYPE_MULTI;
  }

  virtual uint32_t getNUMAId() const override {
    // TODO: probe using libhwloc or similar.
    return 0;
  }

  virtual hsa_device_type_t getDeviceType() const override {
    return HSA_DEVICE_TYPE_CPU;
  }

  virtual std::array<uint32_t, 4> getCacheSize() const override {
    // TODO: probe using libhwloc or similar.
    return {16 * 1024, 0, 0, 0};
  }

  virtual const std::string getISA() const override { return AgentISA; }

  virtual Version getVersion() const override { return {1, 0}; }

  virtual bool hasFastF16Operation() const override {
    // TODO: validate if this is true
    return false;
  }

  virtual uint32_t getWavefrontSize() const override { return 1; }

  virtual std::array<uint16_t, 3> getWorkGroupMaxDim() const override {
    // hcc assumes at least 512 local size can be used. Otherwise
    // we would use the minimum maximum of 256 here to hint small
    // local sizes (with possibly many work-groups) are preferable
    // for CPU/DSP Agents.
    return {1024, 1024, 1024};
  }

  virtual uint32_t getWorkGroupMaxSize() const override { return 1024; }

  virtual hsa_dim3_t getGridMaxDim() const override {
    return {std::numeric_limits<uint32_t>::max(),
            std::numeric_limits<uint32_t>::max(),
            std::numeric_limits<uint32_t>::max()};
  }

  virtual uint32_t getGridMaxSize() const override {
    return std::numeric_limits<uint32_t>::max();
  }

  virtual uint32_t getFBarrierMaxSize() const override { return 32; }

  virtual void terminateQueue(Queue *Q) override;

  virtual void shutDown() override;

private:
  using GCCBrigKernelSignature = void(void *, void *);
  using GCCBrigKernel = std::function<GCCBrigKernelSignature>;

  bool AreDimensionsvalid(hsa_kernel_dispatch_packet_t &KernelPacket);
  bool IsPacketTypeValid(uint16_t Header);
  void Execute();
  std::thread Worker;
  MemoryRegion &QueueRegion;
  std::string AgentISA;
  // The currently executed queue.
  std::atomic<Queue *> RunningQueue;
  // Set to true in case the agent is being interrupted by the client program.
  std::atomic<bool> InterruptingTheQueue;
};

} // namespace phsa

#endif // HSA_RUNTIME_CPUAGENT_HH
