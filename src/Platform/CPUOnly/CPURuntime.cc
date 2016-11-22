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

#include "CPURuntime.hh"

#include "Devices/CPU/CPUKernelAgent.hh"
#include "Devices/CPU/CPUMemoryRegion.hh"
#include "Devices/CPU/UserModeQueue.hh"
#include "Devices/CPU/GCCBuiltinSignal.hh"
#include "Finalizer/GCC/GCCFinalizer.hh"
#include "ISA.hh"

namespace phsa {

CPURuntime::CPURuntime() {
  CPUMemoryRegion *GlobalMemRegion =
      new CPUMemoryRegion(HSA_REGION_SEGMENT_GLOBAL);
  CPUKernelAgent *CPUAgent = new CPUKernelAgent(*GlobalMemRegion);
  CPUAgent->registerMemoryRegion(GlobalMemRegion);

  CPUMemoryRegion *GroupMemRegion =
      new CPUMemoryRegion(HSA_REGION_SEGMENT_GROUP);
  CPUAgent->registerMemoryRegion(GroupMemRegion);
  CPUAgent->setGroupMemoryRegion(GroupMemRegion);

  registerMemoryRegion(GlobalMemRegion);
  registerMemoryRegion(GroupMemRegion);

  registerAgent(CPUAgent);
  getExtensionRegistry().registerExtension(HSA_EXTENSION_FINALIZER,
                                           new GCCFinalizer);
}

Queue *CPURuntime::createSoftQueue(MemoryRegion *Region, uint32_t Size,
                                hsa_queue_type_t Type, Signal *Doorbell) {
  UserModeQueue *Q = new UserModeQueue(
      Size, Type, *Region, Queue::QueueCallback(), nullptr, Doorbell);
  return Q;
}

HSAReturnValue<hsa_signal_t>
CPURuntime::createSignal(hsa_signal_value_t InitialValue) {
  Signal *Sign = new GCCBuiltinSignal(InitialValue, **MemoryRegions.begin());
  return HSAReturn((hsa_status_t)::HSA_STATUS_SUCCESS, Sign->toHSAObject());
}

} // namespace phsa
