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

#include "UserModeQueue.hh"
#include "MemoryRegion.hh"
#include "GCCBuiltinSignal.hh"

namespace phsa {

UserModeQueue::UserModeQueue(uint32_t Size, hsa_queue_type_t Type,
                             MemoryRegion &TargetRegion, QueueCallback CB,
                             Agent *Owner, Signal *Doorbell)
    : Queue(reinterpret_cast<hsa_queue_t *>(&HSAQueue), Owner),
      Region(TargetRegion), CB(CB) {
  HSAQueue.hsa_queue.base_address =
      Region.allocate(Size * sizeof(AQLPacket), sizeof(AQLPacket));
  AQLPacket *Buffer = static_cast<AQLPacket *>(HSAQueue.hsa_queue.base_address);

  // All packets must be initialized to HSA_PACKET_TYPE_INVALID
  for (std::size_t I = 0; I < Size; ++I) {
    AQLPacket &P = Buffer[I];
    P.AgentDispatch.header = HSA_PACKET_TYPE_INVALID;
  }
  HSAQueue.hsa_queue.size = Size;
  HSAQueue.hsa_queue.type = Type;
  HSAQueue.hsa_queue.reserved1 = 0;
  HSAQueue.read_index = 0;
  HSAQueue.write_index = 0;
  HSAQueue.hsa_queue.features = HSA_AGENT_FEATURE_KERNEL_DISPATCH;
  // In case this is used as a soft queue, doorbell signal is provided to the
  // constructor
  Signal *S =
      Doorbell == nullptr
          ? new GCCBuiltinSignal(std::numeric_limits<hsa_signal_value_t>::max(),
                                 TargetRegion)
          : Doorbell;
  HSAQueue.hsa_queue.doorbell_signal = S->toHSAObject();

  // We must not destroy doorbell signal in case of soft queue
  IsDoorbellOwned = Doorbell == nullptr;
}

UserModeQueue::~UserModeQueue() {
  Region.free(HSAQueue.hsa_queue.base_address);
  if (IsDoorbellOwned)
    delete Signal::fromHSAObject(HSAQueue.hsa_queue.doorbell_signal);
}

void UserModeQueue::ExecuteCallback(hsa_status_t Status) {
  if (CB)
    CB(Status, reinterpret_cast<hsa_queue_t *>(&HSAQueue), nullptr);
}

} // namespace phsa

