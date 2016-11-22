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

#ifndef HSA_RUNTIME_ISA_HH
#define HSA_RUNTIME_ISA_HH

#include <cinttypes>

#include <string>
#include <unordered_map>
#include <vector>

#include "CallingConventions.hh"
#include "hsa.h"
#include "HSAObjectMapping.hh"

// ISA is a static class that acts as a registry for supported instruction
// set architectures. All supported ISAs must be registered with
// `registerISA` function.
class ISA {
public:
  using CallingConventionList = std::vector<CallingConvention>;
  static hsa_isa_t registerISA(std::string Name, CallingConventionList CCs);
  static std::string fromHSAObject(hsa_isa_t HSAObject);
  static hsa_isa_t toHSAObject(std::string const &Name);
  static uint32_t callingConventionCount(std::string const &Name);
  static CallingConventionList callingConventions(std::string const &Name);

private:
  static std::unordered_map<std::string, CallingConventionList> ISAMap;
  static std::unordered_map<uint64_t, std::string> HashToISAMap;
  static std::recursive_mutex RegistryLock;
};

#endif // HSA_RUNTIME_ISA_HH
