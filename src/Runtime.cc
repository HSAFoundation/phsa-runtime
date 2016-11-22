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

#include "Runtime.hh"

#include "Agent.hh"
#include "common/Logging.hh"
#include "Finalizer/GCC/DLFinalizedProgram.hh"
#include "HSAILProgram.hh"
#include "MemoryRegion.hh"
#include "Platform/CPUOnly/CPURuntime.hh"
#include "Queue.hh"

namespace phsa {

int32_t Runtime::ReferenceCounter = 0;
std::mutex Runtime::ReferenceCounterMutex;
Runtime *Runtime::Instance = nullptr;
std::mutex Runtime::InstanceMutex;

Runtime::Runtime() {}

Runtime::~Runtime() {
  for (auto Agent : Agents) {
    Agent->shutDown();
    delete Agent;
  }
  Agents.clear();

  Queue::garbageCollect();
  phsa::DLFinalizedProgram::garbageCollect();
  phsa::HSAILProgram::garbageCollect();

  for (auto R : MemoryRegions) {
    delete R;
  }
  MemoryRegions.clear();
}

void Runtime::memoryFence() {
  // Assume the function call itself creates a fence.
}

void Runtime::flush(void *, size_t) {
  // Assume all memory is fine-grained coherent.
}

bool Runtime::initialize() {
  std::lock_guard<std::mutex> ReferenceCounterLock(ReferenceCounterMutex);
  if (ReferenceCounter == std::numeric_limits<int32_t>::max()) {
    return false;
  }

  if (ReferenceCounter == 0) {
    std::lock_guard<std::mutex> InstanceLock(InstanceMutex);
    Instance = new CPURuntime();
  }

  ++ReferenceCounter;

  return true;
}

bool Runtime::release() {
  std::lock_guard<std::mutex> ReferenceCounterLock(ReferenceCounterMutex);
  if (ReferenceCounter == 0) {
    return false;
  }

  if (ReferenceCounter == 1) {
    std::lock_guard<std::mutex> InstanceLock(InstanceMutex);
    delete Instance;
    Instance = nullptr;
  }

  --ReferenceCounter;

  return true;
}

bool Runtime::isInitialized() { return ReferenceCounter > 0; }

ExtensionRegistry &Runtime::getExtensionRegistry() { return ER; }

void Runtime::registerAgent(Agent *A) { Agents.push_back(A); }

void Runtime::freePointer(void *Ptr) {
  for (auto MemRegion : MemoryRegions) {
    if (MemRegion->free(Ptr))
      break;
  }
}

Runtime &Runtime::get() {
  assert(Instance != nullptr);
  return *Instance;
}

} // namespace phsa

