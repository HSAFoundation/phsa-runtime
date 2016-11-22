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

#ifndef HSA_RUNTIME_RUNTIME_HH
#define HSA_RUNTIME_RUNTIME_HH

#include <cinttypes>
#include <mutex>
#include <list>
#include "hsa.h"

#include "HSAReturnValue.hh"
#include "ExtensionRegistry.hh"

namespace phsa {

class Agent;
class MemoryRegion;
class Signal;
class Queue;

class Runtime {
public:
  static Runtime &get();
  static bool initialize();
  static bool release();
  static bool isInitialized();

  Runtime();
  virtual ~Runtime();

  Runtime(Runtime const &) = delete;
  Runtime(Runtime &&) = delete;
  Runtime &operator=(Runtime const &) = delete;
  Runtime &operator=(Runtime &&) = delete;

  ExtensionRegistry &getExtensionRegistry();

  void registerAgent(Agent *A);
  virtual void registerMemoryRegion(MemoryRegion *M) {
    MemoryRegions.push_back(M);
  }

  using agent_iterator = std::list<Agent *>::iterator;
  using const_agent_iterator = std::list<Agent *>::const_iterator;
  agent_iterator agent_begin() { return Agents.begin(); }
  agent_iterator agent_end() { return Agents.end(); }

  const_agent_iterator agent_cbegin() const { return Agents.cbegin(); }
  const_agent_iterator agent_cend() const { return Agents.cend(); }

  virtual Queue *createSoftQueue(MemoryRegion *Region, uint32_t Size,
                                 hsa_queue_type_t Type, Signal *Doorbell) = 0;

  virtual void freePointer(void *Ptr);

  virtual HSAReturnValue<hsa_signal_t>
  createSignal(hsa_signal_value_t initial_value) = 0;

  // Issue a memory fence.
  virtual void memoryFence();

  // Ensure all Agents see the updates to the given address of the
  // given size by the executing core.
  virtual void flush(void *address, size_t size);

protected:
  static int32_t ReferenceCounter;
  static std::mutex ReferenceCounterMutex;
  static Runtime *Instance;
  static std::mutex InstanceMutex;
  std::list<Agent *> Agents;
  ExtensionRegistry ER;
  std::list<MemoryRegion *> MemoryRegions;
};

} // namespace phsa

#endif // HSA_RUNTIME_RUNTIME_HH
