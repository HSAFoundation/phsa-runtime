/*
    Copyright (c) 2016-2018 General Processor Tech.

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
 * @author tomi.aijo@parmance.com and
 *         pekka.jaaskelainen@parmance.com for General Processor Tech.
 */

#include "CPUKernelAgent.hh"

#include <cstring>
#include <limits>
#include <phsa-rt.h>
#include <setjmp.h>
#include <pthread.h>
#include <signal.h>

#include "common/Logging.hh"
#include "Executable.hh"
#include "phsa-rt.h"
#include "UserModeQueue.hh"
#include "Finalizer/GCC/DLFinalizedProgram.hh"
#include "Signal.hh"
#include "MemoryRegion.hh"

namespace phsa {

std::atomic<bool> ShutDown(false);

CPUKernelAgent::CPUKernelAgent(MemoryRegion &QueueMemRegion)
    : Worker(&CPUKernelAgent::Execute, this), QueueRegion(QueueMemRegion),
      AgentISA("host-isa") {
  ISA::registerISA("host-isa", {CallingConvention{"SystemV", 1, 1}});
}

Queue *CPUKernelAgent::createQueue(uint32_t Size, hsa_queue_type_t Type,
                                   Queue::QueueCallback CB) {
  UserModeQueue *Q = new UserModeQueue(Size, Type, QueueRegion, CB, this);
  registerQueue(Q);
  return Q;
}

void CPUKernelAgent::shutDown() {
  ShutDown = true;
  if (Worker.joinable()) {
    Worker.join();
  }
}

CPUKernelAgent::~CPUKernelAgent() {}

bool CPUKernelAgent::AreDimensionsvalid(
    hsa_kernel_dispatch_packet_t &KernelPacket) {
  int Dimensions = ((1 << (HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS +
                           HSA_KERNEL_DISPATCH_PACKET_SETUP_WIDTH_DIMENSIONS)) -
                        1 &
                    KernelPacket.setup) >>
                   HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;

  bool AreOthersOne = true;
  switch (Dimensions) {
  case 1: {
    AreOthersOne &= KernelPacket.grid_size_y == 1 &&
                    KernelPacket.grid_size_z == 1 &&
                    KernelPacket.workgroup_size_y == 1 &&
                    KernelPacket.workgroup_size_z == 1;
    break;
  }
  case 2: {
    AreOthersOne &=
        KernelPacket.grid_size_z == 1 && KernelPacket.workgroup_size_z == 1;
    break;
  }
  case 3: {
    break;
  }
  default: { return false; }
  }

#if 0
  // PRM specs has a concept of "partial WGs" and lots of PRM
  // conformance tests have WG dimensions greater than grid
  // dimensions.
  bool AreGridSizesGreaterThanWGs =
    KernelPacket.grid_size_x >= KernelPacket.workgroup_size_x &&
    KernelPacket.grid_size_y >= KernelPacket.workgroup_size_y &&
    KernelPacket.grid_size_z >= KernelPacket.workgroup_size_z;
#endif
  return AreOthersOne; // && AreGridSizesGreaterThanWGs;
}

bool CPUKernelAgent::IsPacketTypeValid(uint16_t Header) {
  const uint8_t PacketType = Header >> HSA_PACKET_HEADER_TYPE;
  return PacketType > HSA_PACKET_TYPE_INVALID &&
         PacketType <= HSA_PACKET_TYPE_BARRIER_OR;
}

// Used to interrupt the execution of a kernel when a queue
// is invalidated via hsa_queue_inactivate(). This defines
// a safe spot for the kernel agent to resume at.
jmp_buf InterruptedQueueLandingSpot;

// Use a user signal for handling the case when a running queue must
// be interrupted due to it being invalidated via hsa_queue_inactivate().
// The signal handler will be invoked in case the queue to be invalidated
// is currently running. The handler will return back to
// InterruptedQueueLandingSpot via siglongjmp() to safely proceed
// executing possible other queues.
void QueueInterruptionHandler(int) {
  siglongjmp(InterruptedQueueLandingSpot, 1);
}

void CPUKernelAgent::terminateQueue(Queue *Q) {

  // Now we either know the queue is running or the Execute
  // knows it has been inactivated.  Let's interrupt it. There
  // is now a race with interrupting the correct queue as Execute
  // might finish the Queue and start executing another one, thus
  // we must signal the interruption process to avoid Execute
  // proceeding to the next queue.
  InterruptingTheQueue = true;

  if (Q == RunningQueue) {
    // Now either the Execute is still running the Q or has finished
    // it and waits in the InterruptingTheQueue forever loop.
    // It should be now safe to signal the thread so it can interrupt
    // the execution and jump back to the safe spot.
    pthread_kill(Worker.native_handle(), SIGUSR1);
    while (Q == RunningQueue) {
    }
  }

  InterruptingTheQueue = false;
}

void CPUKernelAgent::Execute() {

  struct sigaction SigHandler;

  SigHandler.sa_handler = &QueueInterruptionHandler;
  sigemptyset(&SigHandler.sa_mask);
  SigHandler.sa_flags = 0;

  sigaction(SIGUSR1, &SigHandler, NULL);

  // Resume execution here in case the currently executed queue is
  // inactivate during its execution.
  sigsetjmp(InterruptedQueueLandingSpot, 1);

  RunningQueue = nullptr;
  InterruptingTheQueue = false;

  while (!ShutDown) {

    std::list<Queue *> Queues = getQueues();
    RunningQueue = nullptr;
    for (Agent::queue_iterator It = Queues.begin(), End = Queues.end();
         It != End; ++It) {

      Queue *Q = *It;
      hsa_queue_t *HSAQueue = Q->getHSAQueue();

      RunningQueue = Q;
      if (InterruptingTheQueue)
        while (true) {
        }

      if (Q->isInactivated() || Q->isDestroyed()) {
        continue;
      }

      // For multi producer queues, the doorbell signal might not monotonically
      // increase: The packet updates from multiple producers might not
      // end up to the queue in the write index order. Therefore, we must
      // monitor the doorbell signal for any changes to avoid missing packets
      // that are added for older write index than the largest we have already
      // processed. The read index update can be done only after all packets
      // until that read index have been processed.

      // TO CHECK: Can this actually lead unoptimal queue processing or
      // even starvation as writers with lower write indices can hold back queue
      // writing (as the read index cannot be updated) even though there are
      // free slots _after_ the lower write index due to them having been
      // processed earlier/faster?

      hsa_signal_value_t DoorBell =
        hsa_signal_load_acquire(HSAQueue->doorbell_signal);

      if (DoorBell == std::numeric_limits<hsa_signal_value_t>::max() ||
          DoorBell == Q->getLastHandledDoorBell()) {
        continue;
      }
      Q->setLastHandledDoorBell(DoorBell);

      uint64_t CurrentReadIndex = Q->loadReadIndex(MemoryOrder::Relaxed);
      uint64_t CurrentWriteIndex = Q->loadWriteIndex(MemoryOrder::Relaxed);

      uint64_t QueueSize = HSAQueue->size;

      // In multiple producer queue we have to always check all packets
      // in the queue up to the write index, because we cannot be sure we
      // have received the highest write index from the door bell due to
      // multiple updaters of the write index.

      // In single producer queues, we execute packets in order as we know
      // the new packets are added by a single producer to a single write
      // position which are handed out in monotonically increasing order.

      int PacketsToCheck =
        HSAQueue->type == HSA_QUEUE_TYPE_SINGLE ?
        (Q->getLastHandledDoorBell() - CurrentReadIndex) : QueueSize;
      for (uint64_t CurrentIndex = CurrentReadIndex,
             LastIndex = CurrentReadIndex + PacketsToCheck;
           CurrentIndex <= LastIndex; ++CurrentIndex) {

        int PacketIndex = CurrentIndex % QueueSize;

        if (CurrentIndex >= CurrentWriteIndex)
          break;

        AQLPacket *PacketBuffer =
            static_cast<AQLPacket *>(HSAQueue->base_address);
        AQLPacket &Packet = PacketBuffer[PacketIndex];

        Signal *CompletionSignal = nullptr;

        uint16_t PacketType =
          (Packet.AgentDispatch.header >> HSA_PACKET_HEADER_TYPE) & 0xff;

        // INVALID marks either a packet that is not in use (has already
        // being processed) or is being updated by the producer.
        if (PacketType == HSA_PACKET_TYPE_INVALID) {
          // If it's a processed packet, we might be able to update the
          // read index, to free space in the ring buffer.
          if (Q->IsPacketProcessed(PacketIndex)) {
            // We can only update read index if all the packets until this
            // packet have been processed.
            if (CurrentReadIndex == CurrentIndex) {
              CurrentReadIndex = CurrentIndex + 1;
              Q->storeReadIndex(CurrentReadIndex, MemoryOrder::Relaxed);
              Q->SetPacketProcessed(PacketIndex, false);
            }
          }
          continue;
        }

        if (PacketType == HSA_PACKET_TYPE_BARRIER_AND) {

          hsa_barrier_and_packet_t &BarrierAndPacket = Packet.BarrierAnd;
          // Check the barrier condition.
          bool BarrierConditionOK = true;
          for (int i = 0; i < 5; ++i) {
            hsa_signal_t Dep = BarrierAndPacket.dep_signal[i];
            if (Dep.handle == 0)
              continue;
            hsa_signal_value_t Val = hsa_signal_load_acquire(Dep);
            if (Val != 0) {
              BarrierConditionOK = false;
              break;
            }
          }
          // Unsatisfied barrier. Skip to the next packet in the next queue.
          if (!BarrierConditionOK)
            break;

          CompletionSignal =
              Signal::fromHSAObject(BarrierAndPacket.completion_signal);

        } else if (PacketType == HSA_PACKET_TYPE_BARRIER_OR) {

          hsa_barrier_or_packet_t &BarrierOrPacket = Packet.BarrierOr;
          // Check the barrier condition.
          bool BarrierConditionOK = false;
          for (int i = 0; i < 5; ++i) {
            hsa_signal_t Dep = BarrierOrPacket.dep_signal[i];
            if (Dep.handle == 0) {
              continue;
            }
            hsa_signal_value_t Val = hsa_signal_load_acquire(Dep);
            if (Val == 0) {
              BarrierConditionOK = true;
              break;
            }
          }
          // Unsatisfied barrier. Skip to the next packet in the next queue.
          if (!BarrierConditionOK)
            break;

          CompletionSignal =
              Signal::fromHSAObject(BarrierOrPacket.completion_signal);

        } else if (PacketType == HSA_PACKET_TYPE_KERNEL_DISPATCH) {

          hsa_kernel_dispatch_packet_t &KernelPacket = Packet.KernelDispatch;

          CompletionSignal =
              Signal::fromHSAObject(KernelPacket.completion_signal);

          bool HasIterations = !(KernelPacket.workgroup_size_x == 0 ||
                                 KernelPacket.workgroup_size_y == 0 ||
                                 KernelPacket.workgroup_size_z == 0);

          bool ValidDimensions = AreDimensionsvalid(KernelPacket);
          bool ValidType = IsPacketTypeValid(KernelPacket.header);
          void *GroupMemory = KernelPacket.group_segment_size != 0
                                  ? GroupMemoryRegion->allocate(
                                        KernelPacket.group_segment_size, 16)
                                  : nullptr;
          bool ValidGroupMemory =
              GroupMemory != nullptr || KernelPacket.group_segment_size == 0;

          Kernel *K = dynamic_cast<Kernel *>(
              Symbol::fromHandle(KernelPacket.kernel_object));

          if (K != nullptr && HasIterations && ValidDimensions && ValidType &&
              ValidGroupMemory) {

            GCCBrigKernel KernelFunction =
                reinterpret_cast<GCCBrigKernelSignature *>(K->Address);

            PHSAKernelLaunchData LaunchData;
            LaunchData.dp = &KernelPacket;
            LaunchData.packet_id = CurrentIndex;

            bool RelocatedKernargs;
            if ((uint64_t)KernelPacket.kernarg_address %
                    K->KernargSegmentAlignment ==
                0) {
              LaunchData.kernarg_addr = KernelPacket.kernarg_address;
              RelocatedKernargs = false;
            } else {
              if (posix_memalign(&LaunchData.kernarg_addr,
                                 K->KernargSegmentAlignment,
                                 K->KernargSegmentSize) != 0)
                abort();

              std::memcpy(LaunchData.kernarg_addr, KernelPacket.kernarg_address,
                          K->KernargSegmentSize);
              RelocatedKernargs = true;
            }
            KernelFunction(&LaunchData, GroupMemory, LaunchData.kernarg_addr);

            if (RelocatedKernargs) {
              free(LaunchData.kernarg_addr);
            }

          } else if (!ValidType) {
            Q->ExecuteCallback(HSA_STATUS_ERROR_INVALID_PACKET_FORMAT);
          } else if (!ValidDimensions) {
            Q->ExecuteCallback(HSA_STATUS_ERROR_INCOMPATIBLE_ARGUMENTS);
          } else if (!ValidGroupMemory) {
            Q->ExecuteCallback(HSA_STATUS_ERROR_INVALID_ALLOCATION);
          } else {
            Q->ExecuteCallback(HSA_STATUS_ERROR_INVALID_CODE_OBJECT);
          }

          if (GroupMemory != nullptr) {
            GroupMemoryRegion->free(GroupMemory);
          }
        } else {
          PRINT_VAR(PacketType);
          ABORT_UNIMPLEMENTED;
        }

        Packet.AgentDispatch.header = HSA_PACKET_TYPE_INVALID;
        if (CurrentReadIndex == CurrentIndex) {
          CurrentReadIndex = CurrentIndex + 1;
          Q->storeReadIndex(CurrentReadIndex, MemoryOrder::Relaxed);
          Q->SetPacketProcessed(PacketIndex, false);
        } else {
          // There are unprocessed packets before this index, we cannot
          // update the read index yet, but have to wait until the earlier
          // ones have been processed.
          Q->SetPacketProcessed(PacketIndex, true);
        }

        if (CompletionSignal != nullptr) {
          CompletionSignal->store(0, MemoryOrder::Relaxed);
        }
      }
    }
  }
  ShutDown = false;
}

#ifdef __GNUC__
// Stops the executor thread gracefully when dlclose() is called for
// the runtime lib.
__attribute__((destructor)) static void StopExecutorThread() {
  ShutDown = true;
}
#endif
}
