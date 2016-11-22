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

#ifndef HSA_RUNTIME_MEMORYREGION_HH
#define HSA_RUNTIME_MEMORYREGION_HH

#include <cstddef>

#include "hsa.h"
#include "HSAObjectMapping.hh"

namespace phsa {

// MemoryRegion keeps book of memory allocation of a specific region.
//
// For an implementation example, see `src/FixedMemoryRegion.[cc|hh]`.
class MemoryRegion : public HSAObjectMapping<MemoryRegion, hsa_region_t> {
public:
  virtual void *allocate(std::size_t Size, std::size_t Align) = 0;

  // Free the given pointer, if in this region. Returns true if it was.
  virtual bool free(void *Ptr) = 0;
  virtual hsa_region_segment_t getRegion() const = 0;
  virtual uint32_t getGlobalFlags() const = 0;
  virtual std::size_t getSize() const = 0;
  virtual std::size_t getMaxAllocSize() const = 0;
  virtual bool getRuntimeAllocAllowed() const = 0;
  virtual std::size_t getRuntimeAllocGranularity() const = 0;
  virtual std::size_t getRuntimeAllocAlignment() const = 0;
};

} // namespace phsa

#endif // HSA_RUNTIME_MEMORYREGION_HH
