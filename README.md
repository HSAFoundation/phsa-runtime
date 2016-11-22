Portable HSA Runtime
====================

Phsa-runtime is a generic implementation of HSA Runtime Specification 1.0.
It is designed to be used with the GCC's BRIG frontend for finalization support.
It has been tested on Debian based Linux systems, but should port easily
to other operating systems.

Phsa-runtime together with the GCC's BRIG frontend can be used to implement
a base profile HSA 1.0 software stack with HSAIL finalization support for
any GCC-supported target.

Phsa-runtime was developed by [Parmance](http://parmance.com)
for [General Processor Technologies](http://www.generalprocessortech.com/)
who published it as an open source project in November 2016.

# Status

Phsa-runtime fully passes the [HSA runtime conformance](https://github.com/HSAFoundation/HSA-Runtime-Conformance) 
test suite. There are a few HSA API methods that are not implemented yet, and 
a small number of implemented ones might be missing features. 

# Requirements

 * CMake 2.8+
 * A C++ compiler with C++11 support.
 * Boost Thread library (libboost-thread-dev)
 * Boost Filesystem library (libboost-filesystem-dev)
 * Boost System library (libboost-system-dev)
 * libelf (libelf-dev)
 * [GCC BRIG frontend](https://github.com/HSAFoundation/gccbrig/tree/gccbrig-gcc-master) with a backend for the desired device to be used as a kernel agent

# Installation

The familiar CMake build process is used:

```
mkdir build
cd build
cmake ..
make
make install
```

# License

The code base is licensed with the permissive
[MIT license](https://opensource.org/licenses/MIT) which
allows adopting the phsa-runtime for closed and open source purposes.

# Repository structure

Source tree is structured as follows:

```
.
├── include             Public headers
└── src
    ├── common          Common helpers
    ├── Devices         Agent implementations
    │   └── CPU         CPU agent that uses the host machine's cores for kernel execution
    ├── Finalizer       Finalizer implementations
    │   └── GCC         The default finalizer that uses the GCC BRIG frontend
    ├── hsa             HSA API method implementations
    └── Platform        Runtime implementations
        └── CPUOnly     Platform that only uses CPU agents for execution


```

# Code Base Internals

The code base of phsa-runtime is a relatively straightforward class hierarchy.
When porting phsa-runtime to a new platform, the relevant classes 
are Runtime, MemoryRegion, Agent, Queue and Signal which are covered in
the following.

## class Runtime ([Runtime.hh](include/Runtime.hh), [Runtime.cc](src/Runtime.cc))

A good spot to start when porting phsa-runtime to a new platform is the
Runtime class.
This class is a central point for holding objects that are used to control
an HSA enabled platform. A recommended practice is to derive a new Runtime
implementation for the new platform at hand, and create an instance of
that class in Runtime::get().

In the Runtime class for the new target, the constructor should
initialize all the objects used for controlling the resources of the
platform. For example, MemoryRegion objects are used to keep book of
allocations from different HSA memory regions, and Agent objects
are used to control kernel agents associated with the platform.
In addition, the extension list, which currently
supports only the finalizer extension, must be filled. Typically GCCFinalizer is
instantiated here as the finalizer implementation.

In addition to instantiating platform specific implementation of
HSA objects, the Runtime's interface has
methods for instantiating certain HSA objects. The subclass 
should override createSoftQueue() and createSignal() to return objects 
of the derived types.

An example implementation is the CPURuntime ([CPURuntime.hh](src/Platform/CPUOnly/CPURuntime.hh), 
[CPURuntime.cc](src/Platform/CPUOnly/CPURuntime.cc)) class. 
The implementation includes a single "CPU agent" that
utilizes GCCFinalizer for finalizing HSAIL programs for the host CPU.
CPU agents are simply kernel agents that are running in the same set of processor
cores the host program is running in. The basic assumption in the case of
platforms with only CPU agents is a fine grained coherent virtual
memory thanks to the shared memory hierarchy between the cores.

## class MemoryRegion ([MemoryRegion.hh](include/MemoryRegion.hh))

Used for keeping book of allocations from different HSA memory regions
in the platform.

FixedMemoryRegion ([FixedMemoryRegion.hh](src/FixedMemoryRegion.hh), 
[FixedMemoryRegion.cc](src/FixedMemoryRegion.cc)) is a MemoryRegion implementation
that returns chunks of memory from a fixed (virtual) address region with a 
straightforward (but fast) allocation algorithm. This is useful for platforms
with heterogeneous devices with their own local physical memories mapped
to certain physical address ranges that are in turn mapped to the process'
virtual memory via mmap() or a similar mechanism. The standard malloc()
cannot be used for allocation in that case as one must ensure chunks
are returned from that address range only.

Instead of using a custom allocator, CPUMemoryRegion 
([CPUMemoryRegion.hh](src/Devices/CPU/CPUMemoryRegion.hh), 
[CPUMemoryRegion.cc](src/Devices/CPU/CPUMemoryRegion.cc)) uses the standard 
malloc() of the host system. It can be used for platforms with only CPU agents.

## class Agent ([Agent.hh](include/Agent.hh))

A derived type of the KernelDispatchAgent ([Agent.hh](include/Agent.hh#L120)) interface should be implemented
for all the different types of devices in the platform that one wants to
run kernels on. A KernelDispatchAgent implementation controls an agent that
processes packets from user mode queues (the Queue interface) and
executes them.

An example implementation, CPUKernelAgent ([CPUKernelAgent.hh](src/Devices/CPU/CPUKernelAgent.hh), 
[CPUKernelAgent.cc](src/Devices/CPU/CPUKernelAgent.cc)), implements a kernel agent that
both processes queues and also executes kernel dispatch packets
in the host CPU. The kernel functions are assumed to be loaded to
the same process which are then simply called from a thread dedicated
to processing queues.

In case of heterogeneous platforms, KernelDispatchAgent implementations
orchestrate the execution with the agent device utilizing target specific
communication and synchronization mechanisms. The division of responsibilities
between processing the queues and executing kernels is target dependent.
In case the device supports processing user mode queues directly, the
implementation can be as simple as initializing the device in the constructor,
and finalizing it in shutDown(). In some cases a host thread can
process the user mode queues itself and only delegate single kernel packets to
the agent.

## class Queue ([Queue.hh](include/Queue.hh))

An interface for implementing user mode queues and soft queues. The
default implementation is UserModeQueue ([UserModeQueue.hh](src/Devices/CPU/UserModeQueue.hh), 
[UserModeQueue.cc](src/Devices/CPU/UserModeQueue.cc)) which should work for most
purposes. It allocates the actual queue from a specified MemoryRegion and
updates the different indices using gcc's __atomic_* builtins.

## class Signal ([Signal.hh](include/Signal.hh))

The concept of signals is opaque in HSA. This class implement the different
hsa_signal* APIs for atomic variables that work globally in the heterogeneous
platform at hand. The default implementation GCCBuiltinSignal ([GCCBuiltinSignal.hh](src/Devices/CPU/GCCBuiltinSignal.hh),
[GCCBuiltinSignal.cc](src/Devices/CPU/GCCBuiltinSignal.cc))
uses gcc's atomic builtins. This matches with the gcc BRIG frontend of
which runtime library also uses them for accessing signal values in HSAIL.

## class GCCFinalizer ([GCCFinalizer.hh](src/Finalizer/GCC/GCCFinalizer.hh), [GCCFinalizer.cc](src/Finalizer/GCC/GCCFinalizer.cc))

This class is an interface between phsa-runtime and the GCC's BRIG frontend.
It calls a BRIG enabled GCC via command line to finalize the incoming HSAIL
programs encoded as BRIG binaries. The default implementation links the
ELF produced by GCC to a dynamic library that can be loaded to the current
process via dlopen() for execution.

When porting the finalizer to a new GCC-supported device, this class should
be derived and adapted such that the returned ELF binary is loaded
correctly for the device at hand. The default CPU agent implementation
serves merely as an example in that case.

During porting or bug hunting, it might become useful to have the gcc's
intermediate files dumped from the compilation process for closer inspection.
This behavior can be enabled by setting the environment variable PHSA\_DEBUG\_MODE
to 1.



