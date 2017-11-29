INCLUDE(TestBigEndian)

TEST_BIG_ENDIAN(HSA_BIG_ENDIAN)
if (NOT ${HSA_BIG_ENDIAN})
    add_definitions(-DHSA_LITTLE_ENDIAN)
endif()

set(BITNESS_STR "")
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(BITNESS_STR "64")
endif()

add_definitions(-DHSA_VERSION_MAJOR=1)
add_definitions(-DHSA_VERSION_MINOR=0)

set(LIBRARY_NAME "phsa-runtime${BITNESS_STR}")
set (HSA_SOURCE_FILES hsa/hsa_runtime.cc hsa/hsa_agent.cc hsa/hsa_signal.cc
        hsa/hsa_queue.cc hsa/hsa_region.cc hsa/hsa_memory.cc hsa/hsa_isa.cc
        hsa/hsa_code.cc hsa/hsa_executable.cc hsa/hsa_system.cc hsa/hsa_status.cc
        hsa/hsa_finalize.cc hsa/hsa_image.cc)

set (HSA_AMD_SOURCE_FILES amd/hsa_ext_amd.cc)

set (CPU_DEVICE_SOURCE_FILES FixedMemoryRegion.cc
        Devices/CPU/CPUMemoryRegion.cc Devices/CPU/UserModeQueue.cc Devices/CPU/StdAtomicSignal.cc
        Devices/CPU/GCCBuiltinSignal.cc Devices/CPU/CPUKernelAgent.cc)

set (CPUONLY_PLATFORM_SOURCE_FILES Platform/CPUOnly/CPURuntime.cc)

set(GCC_FINALIZER_SOURCE_FILES Finalizer/GCC/ELFExecutable.cc Finalizer/GCC/GCCFinalizer.cc
        Finalizer/GCC/DLFinalizedProgram.cc)

set(SOURCE_FILES
        ExtensionRegistry.cc MemoryRegion.cc Agent.cc common/Info.cc common/Debug.cc
        Signal.cc Queue.cc FinalizedProgram.cc HSAILProgram.cc Finalizer.cc
        common/MemoryOrder.cc common/Atomic.cc ISA.cc Runtime.cc)

add_library(${LIBRARY_NAME} SHARED ${SOURCE_FILES} ${HSA_SOURCE_FILES} ${HSA_AMD_SOURCE_FILES}
        ${CPU_DEVICE_SOURCE_FILES} ${GCC_FINALIZER_SOURCE_FILES} ${CPUONLY_PLATFORM_SOURCE_FILES})

set_target_properties(${LIBRARY_NAME} PROPERTIES SOVERSION 1)

set(Boost_USE_STATIC_LIBS OFF)
find_package(Boost 1.54.0 COMPONENTS filesystem system thread)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(${LIBRARY_NAME} ${Boost_LIBRARIES})
else ()
    message (FATAL_ERROR "Boost filesystem required.")
endif()

target_link_libraries(${LIBRARY_NAME} dl)
target_link_libraries(${LIBRARY_NAME} elf)

install(TARGETS ${LIBRARY_NAME}
        LIBRARY DESTINATION "${PHSA_INSTALL_PUBLIC_LIBDIR}")
