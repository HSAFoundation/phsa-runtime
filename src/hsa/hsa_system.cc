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

#include "hsa.h"
#include "Runtime.hh"

#include <algorithm>
#include <chrono>
#include <limits>

hsa_status_t HSA_API hsa_system_get_info(hsa_system_info_t attribute,
                                         void *value) {

  if (value == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  switch (attribute) {

  case HSA_SYSTEM_INFO_VERSION_MAJOR: {
    *((uint16_t *)value) = HSA_VERSION_MAJOR;
    break;
  }
  case HSA_SYSTEM_INFO_VERSION_MINOR: {
    *((uint16_t *)value) = HSA_VERSION_MINOR;
    break;
  }
  case HSA_SYSTEM_INFO_TIMESTAMP: {
    *(uint64_t *)value = static_cast<uint64_t>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count());
    break;
  }
  case HSA_SYSTEM_INFO_TIMESTAMP_FREQUENCY: {
    *(uint64_t *)value =
        static_cast<uint64_t>(std::chrono::high_resolution_clock::period::den);
    break;
  }
  case HSA_SYSTEM_INFO_SIGNAL_MAX_WAIT: {
    *((uint64_t *)value) = std::numeric_limits<uint64_t>::max();
    break;
  }
  case HSA_SYSTEM_INFO_ENDIANNESS: {
#ifdef HSA_LITTLE_ENDIAN
    *((hsa_endianness_t *)value) = HSA_ENDIANNESS_LITTLE;
#else
    *((hsa_endianness_t *)value) = HSA_ENDIANNESS_BIG;
#endif

    break;
  }
  case HSA_SYSTEM_INFO_MACHINE_MODEL: {
#if defined(HSA_LARGE_MODEL)
    *((hsa_machine_model_t *)value) = HSA_MACHINE_MODEL_LARGE;
#else
    *((hsa_machine_model_t *)value) = HSA_MACHINE_MODEL_SMALL;
#endif
    break;
  }
  case HSA_SYSTEM_INFO_EXTENSIONS: {
    uint8_t *Ptr = static_cast<uint8_t *>(value);
    std::fill(Ptr, Ptr + sizeof(Ptr[0]) * 128, 0);
    ExtensionRegistry const &ER = phsa::Runtime::get().getExtensionRegistry();

    for (ExtensionRegistry::const_iterator I = ER.cbegin(), E = ER.cend();
         I != E; ++I) {
      Extension::Identifier Id = I->first;
      *((uint8_t *)value) |= 1 << Id;
    }
    break;
  }
  default:
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_system_extension_supported(uint16_t extension,
                                                    uint16_t version_major,
                                                    uint16_t version_minor,
                                                    bool *result) {
  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  if (result == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  // Valid extension IDs are between 0 and 127
  if (extension > 127) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  ExtensionRegistry &ER = phsa::Runtime::get().getExtensionRegistry();
  ExtensionRegistry::iterator First, End;
  std::tie(First, End) = ER.findExtensions(extension);

  *result = std::any_of(
      First, End, [&](const std::pair<Extension::Identifier, Extension *> E) {
        Version V = E.second->getVersion();
        return V.Major == version_major && V.Minor == version_minor;

      });

  return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_system_get_extension_table(uint16_t extension,
                                                    uint16_t version_major,
                                                    uint16_t version_minor,
                                                    void *table) {

  if (table == nullptr) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  if (!phsa::Runtime::isInitialized()) {
    return HSA_STATUS_ERROR_NOT_INITIALIZED;
  }

  ExtensionRegistry &ER = phsa::Runtime::get().getExtensionRegistry();
  ExtensionRegistry::iterator First, End, Item;
  std::tie(First, End) = ER.findExtensions(extension);

  Item = std::find_if(
      First, End, [&](const std::pair<Extension::Identifier, Extension *> E) {
        Version V = E.second->getVersion();
        return V.Major == version_major && V.Minor == version_minor;

      });

  if (Item == ER.end()) {
    return HSA_STATUS_ERROR_INVALID_ARGUMENT;
  }

  Item->second->fillExtensionTable(table);

  return HSA_STATUS_SUCCESS;
}
