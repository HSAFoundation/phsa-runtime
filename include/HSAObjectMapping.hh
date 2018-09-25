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

#ifndef HSA_RUNTIME_HSAOBJECTMAPPING_HH
#define HSA_RUNTIME_HSAOBJECTMAPPING_HH

#include <mutex>
#include <unordered_map>
#include <algorithm>
#include <functional>

#include "common/Logging.hh"

// HSAObjectMaping is an abstraction that provides mapping between
// the PHSA runtime object types and the HSA API counter parts. To
// utilize mapping, a class must be inherited from HSAObjectMapping and
// and both the class type and the HSA type must be given as template
// parameters, respectively.
//
// By default, the opaque handle is a pointer to the object instance.
// Alternatively, a custom mapping fuction may be given to the constructor.
template <class ObjectType, class HSAType> class HSAObjectMapping {
public:
  using Mapper = std::function<HSAType(const ObjectType *)>;

  HSAObjectMapping()
      : HSAObjectMapping([](const ObjectType *Object) {
          HSAType Ret;
          Ret.handle = reinterpret_cast<uint64_t>(Object);
          return Ret;
        }) {}

  HSAObjectMapping(std::function<HSAType(const ObjectType *)> OM,
                   bool RegisterObject = true)
      : ObjectMapper(OM) {
    if (RegisterObject)
      registerObject(static_cast<ObjectType *>(this));
  }

  virtual ~HSAObjectMapping() {
    std::lock_guard<std::recursive_mutex> Lock(RegistryLock);
    // Find the item manually because ObjectMapper might not work anymore
    // as we have partially destroyed the object.
    auto Item = std::find_if(
        Registry.begin(), Registry.end(),
        [this](const std::pair<const long unsigned int, ObjectType *> &val) {
          return val.second == this;
        });
    if (Item != Registry.end())
      Registry.erase(Item);
  }

  // Map from opaque handle to PHSA object type
  static ObjectType *fromHandle(uint64_t Handle) {
    std::lock_guard<std::recursive_mutex> Lock(RegistryLock);

    return Registry[Handle];
  }

  // Map from HSA opaque struct to PHSA object type
  static ObjectType *fromHSAObject(HSAType HSAObject) {
    std::lock_guard<std::recursive_mutex> Lock(RegistryLock);

    auto It = Registry.find(HSAObject.handle);

    if (It != Registry.end())
      return It->second;

    return nullptr;
  }

  // Map from PHSA object type to its HSA opaque struct counter part
  HSAType toHSAObject() const {
    return ObjectMapper(static_cast<const ObjectType *>(this));
  }

  /// Deletes all objects in the registry. Can be called
  /// in hsa_shut_down() in case it's known there are no
  /// dangling references to the objects.
  static void garbageCollect() {
    std::lock_guard<std::recursive_mutex> Lock(RegistryLock);

    auto i = Registry.begin(), e = Registry.end();
    while (i != e) {
      delete (*i).second;
      i = Registry.begin();
    }
    Registry.clear();
  }

  static void registerObject(ObjectType *Object) {
    HSAType HSAObject = Object->toHSAObject();

    std::lock_guard<std::recursive_mutex> Lock(RegistryLock);
    Registry[HSAObject.handle] = Object;
  }

  static void deregisterObject(HSAType HSAObject) {
    std::lock_guard<std::recursive_mutex> Lock(RegistryLock);
    Registry.erase(HSAObject.handle);
  }

private:
  const Mapper ObjectMapper;

  static std::unordered_map<uint64_t, ObjectType *> &Registry;
  static std::recursive_mutex& RegistryLock;
};

template <class ObjectType, class HSAType>
std::unordered_map<uint64_t, ObjectType *>&
HSAObjectMapping<ObjectType, HSAType>::Registry =
  *(new std::unordered_map<uint64_t, ObjectType *>);

template <class ObjectType, class HSAType>
std::recursive_mutex& HSAObjectMapping<ObjectType, HSAType>::RegistryLock =
  *(new std::recursive_mutex);

#endif // HSA_RUNTIME_HSAOBJECTMAPPING_HH
