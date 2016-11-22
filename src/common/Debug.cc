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

#include <cstdlib>
#include <cstring>
#include "Debug.hh"

namespace phsa {

static int DebugMode = 0;

bool IsDebugMode() {
  if (DebugMode == 1)
    return true;
  else if (DebugMode == 0) {
    char *DevelModeEnv = std::getenv("PHSA_DEBUG_MODE");
    if (DevelModeEnv != nullptr && std::strlen(DevelModeEnv) == 1 &&
        DevelModeEnv[0] == '1') {
      DebugMode = 1;
      return true;
    } else {
      DebugMode = 2;
      return false;
    }
  }
  return false;
}

} // namespace phsa

