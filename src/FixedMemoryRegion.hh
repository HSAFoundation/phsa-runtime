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
 * @author pekka.jaaskelainen@parmance.com for General Processor Tech.
 */

#ifndef HSA_RUNTIME_FIXED_MEMORYREGION_HH
#define HSA_RUNTIME_FIXED_MEMORYREGION_HH

#include <cstdlib>
#include <map>

#include "MemoryRegion.hh"

namespace phsa {


// MemoryRegion implementation that allocates space from a fixed memory region.
class FixedMemoryRegion : public MemoryRegion {
public:
  FixedMemoryRegion(hsa_region_segment_t Segment, std::size_t RegionStart,
                    std::size_t Size)
      : RegionSegment(Segment), StartAddress(RegionStart),
        FreeSpaceStart(RegionStart), RegionSize(Size),
        RegionEnd(RegionStart + Size - 1) {}

  virtual void *allocate(std::size_t Size, std::size_t Align) override;
  virtual bool free(void *Ptr) override;

  virtual hsa_region_segment_t getRegion() const override {
    return RegionSegment;
  }

  virtual uint32_t getGlobalFlags() const override {
    return HSA_REGION_GLOBAL_FLAG_KERNARG | HSA_REGION_GLOBAL_FLAG_FINE_GRAINED;
  }

  virtual std::size_t getSize() const override { return RegionSize; }

  virtual std::size_t getMaxAllocSize() const override { return RegionSize; }

  virtual bool getRuntimeAllocAllowed() const override { return true; }

  virtual std::size_t getRuntimeAllocGranularity() const override { return 1; }

  virtual std::size_t getRuntimeAllocAlignment() const override { return 1; }

private:
  // All chunks of memory allocated so far. Key = address, value = size.
  std::map<std::size_t, std::size_t> Allocations;
  std::size_t StartAddress;
  // First free address at the end of region.
  std::size_t FreeSpaceStart;
  // Size of the whole region.
  std::size_t RegionSize;
  // The last byte address inside the region.
  std::size_t RegionEnd;
  hsa_region_segment_t RegionSegment;
};

} // namespace phsa

#endif // HSA_RUNTIME_FIXED_MEMORYREGION_HH
