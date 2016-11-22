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

#ifndef HSA_RUNTIME_EXTENSIONREGISTRY_HH
#define HSA_RUNTIME_EXTENSIONREGISTRY_HH

#include <unordered_map>

#include "Extension.hh"

// Runtime uses `ExtensionRegistry` to list and query the supported extensions.
class ExtensionRegistry {
public:
  ExtensionRegistry();
  ~ExtensionRegistry();

  using iterator =
      std::unordered_map<Extension::Identifier, Extension *>::iterator;
  using const_iterator =
      std::unordered_map<Extension::Identifier, Extension *>::const_iterator;

  void registerExtension(Extension::Identifier Id, Extension *E);
  std::pair<ExtensionRegistry::iterator, ExtensionRegistry::iterator>
  findExtensions(Extension::Identifier Id);

  iterator begin() { return Extensions.begin(); }
  iterator end() { return Extensions.end(); }
  const_iterator cbegin() const { return Extensions.cbegin(); }
  const_iterator cend() const { return Extensions.cend(); }

  ExtensionRegistry(ExtensionRegistry const &) = delete;
  ExtensionRegistry(ExtensionRegistry &&) = delete;
  ExtensionRegistry &operator=(ExtensionRegistry const &) = delete;
  ExtensionRegistry &operator=(ExtensionRegistry &&) = delete;

private:
  std::unordered_map<Extension::Identifier, Extension *> Extensions;
};

#endif // HSA_RUNTIME_EXTENSIONREGISTRY_HH
