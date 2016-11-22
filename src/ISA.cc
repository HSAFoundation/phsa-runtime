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

#include "ISA.hh"

std::unordered_map<std::string, ISA::CallingConventionList> ISA::ISAMap;
std::unordered_map<uint64_t, std::string> ISA::HashToISAMap;
std::recursive_mutex ISA::RegistryLock;

hsa_isa_t ISA::registerISA(std::string Name, CallingConventionList CCs) {
  std::lock_guard<std::recursive_mutex> Lock(RegistryLock);

  hsa_isa_t HSAObject = toHSAObject(Name);
  ;
  ISAMap[Name] = CCs;
  HashToISAMap[HSAObject.handle] = Name;

  return HSAObject;
}

std::string ISA::fromHSAObject(hsa_isa_t HSAObject) {
  return HashToISAMap[HSAObject.handle];
}

hsa_isa_t ISA::toHSAObject(std::string const &Name) {
  std::lock_guard<std::recursive_mutex> Lock(RegistryLock);

  return hsa_isa_t{std::hash<std::string>{}(Name)};
}

uint32_t ISA::callingConventionCount(std::string const &Name) {
  std::lock_guard<std::recursive_mutex> Lock(RegistryLock);

  return static_cast<uint32_t>(ISAMap[Name].size());
}

ISA::CallingConventionList ISA::callingConventions(std::string const &Name) {
  std::lock_guard<std::recursive_mutex> Lock(RegistryLock);

  return ISAMap[Name];
}
