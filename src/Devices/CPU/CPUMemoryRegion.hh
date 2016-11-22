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

#ifndef HSA_RUNTIME_CPUMEMORYREGION_HH
#define HSA_RUNTIME_CPUMEMORYREGION_HH

#include <cstdlib>

#include "MemoryRegion.hh"

namespace phsa {

class CPUMemoryRegion : public MemoryRegion {

public:
  CPUMemoryRegion(hsa_region_segment_t Segment);

  virtual void *allocate(std::size_t Size, std::size_t Align) override;

  virtual bool free(void *Ptr) override {
    std::free(Ptr);
    return true;
  }

  virtual hsa_region_segment_t getRegion() const override {
    return RegionSegment;
  }

  virtual uint32_t getGlobalFlags() const override {
    return HSA_REGION_GLOBAL_FLAG_KERNARG |
      HSA_REGION_GLOBAL_FLAG_FINE_GRAINED;
  }

  virtual std::size_t getSize() const override {
    // TODO: probe using libhwloc like pocl does.
    return 1024 * 1024 * 1024;
  }

  virtual std::size_t getMaxAllocSize() const override {
    // TODO: probe using libhwloc like pocl does.
    return 512 * 1024 * 1024;
  }

  virtual bool getRuntimeAllocAllowed() const override { return true; }

  virtual std::size_t getRuntimeAllocGranularity() const override { return 1; }

  virtual std::size_t getRuntimeAllocAlignment() const override { return 1; }

private:
  hsa_region_segment_t RegionSegment;
};

} // namespace phsa

#endif // HSA_RUNTIME_CPUMEMORYREGION_HH
