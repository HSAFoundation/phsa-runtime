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

#include <cassert>
#include "FixedMemoryRegion.hh"

#include "common/Logging.hh"

namespace phsa {

/**
 * Extremely simple memory allocation strategy:
 * If there's room after all the other allocations, allocate from the end,
 * otherwise, find (via linear search starting from the lowest chunk address)
 * the first fragment that fits the size and return that, fail otherwise.
 * Thus, doesn't do anything to avoid fragmentation or to minimize the highest
 * address allocated.
 */
void *FixedMemoryRegion::allocate(std::size_t Size, std::size_t Align) {

  size_t Addr = 0;

  assert(Align > 0);

  if (Allocations.size() > 0) {
    auto i = Allocations.rbegin();
    size_t ChunkStart = (*i).first;
    size_t ChunkSize = (*i).second;
    FreeSpaceStart = ChunkStart + ChunkSize;
  }

  if (FreeSpaceStart + Size +
          (FreeSpaceStart % Align > 0 ? (Align - FreeSpaceStart % Align) : 0) <=
      RegionEnd) {
    Addr = FreeSpaceStart +
           (FreeSpaceStart % Align > 0 ? (Align - FreeSpaceStart % Align) : 0);
    FreeSpaceStart = Addr + Size;
  }

  if (Addr == 0 && Allocations.size() > 0) {
    auto i = Allocations.begin();
    auto Next = i;
    Next++;
    for (; Next != Allocations.end(); ++i, ++Next) {
      size_t ChunkStart = (*i).first;
      size_t ChunkSize = (*i).second;
      size_t ChunkEnd = ChunkStart + ChunkSize;

      size_t NextStart = (*Next).first;
      size_t NextChunkSize = (*Next).second;

      size_t AlignAdd = ChunkEnd % Align > 0 ? (Align - ChunkEnd % Align) : 0;
      if (ChunkEnd + Size + AlignAdd <= NextStart) {
        Addr = ChunkEnd + AlignAdd;
        break;
      }
    }
  }

  if (Addr == 0) {
    DEBUG << "### Failed to allocate: " << Size << " bytes (align " << Align
          << ")";
    std::cout << "### Allocations: " << std::endl;
    for (auto Allocation : Allocations) {
      size_t ChunkStart = Allocation.first;
      size_t ChunkSize = Allocation.second;
      std::cout << std::dec << ChunkSize << " bytes at 0x" << std::hex
                << ChunkStart << std::endl;
    }
    PRINT_VAR(FreeSpaceStart);
    abort();
    return NULL;
  }

  assert(Addr % Align == 0);

#ifdef DEBUG_ALLOCATION
  static int count = 0;
  ++count;
  DEBUG << "### Allocated: " << Size << " bytes (align " << Align << ") at "
        << std::hex << Addr;
  std::cout << "### Allocations: " << std::endl;
  for (auto Allocation : Allocations) {
    size_t ChunkStart = Allocation.first;
    size_t ChunkSize = Allocation.second;
    std::cout << std::dec << ChunkSize << " bytes at 0x" << std::hex
              << ChunkStart << std::endl;
  }
  PRINT_VAR(FreeSpaceStart);
#endif
  Allocations[Addr] = Size;
  return (void *)Addr;
}

bool FixedMemoryRegion::free(void *Ptr) {
  size_t OldSize = Allocations.size();
  Allocations.erase((size_t)(Ptr));
  return (OldSize > Allocations.size());
}

} // namespace phsa

