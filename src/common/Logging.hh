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
 *
 * @author tomi.aijo@parmance.com for General Processor Tech.
 */

#ifndef HSA_RUNTIME_LOGGING_HH
#define HSA_RUNTIME_LOGGING_HH

#ifndef NDEBUG
#include <iostream>
#include <cstring>

struct D {
  ~D() { std::cerr << std::endl; }
};
#define __FILENAME__                                                           \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define DEBUG                                                                  \
  (D(), std::cerr << __FILENAME__ << ":" << __FUNCTION__ << ":" << __LINE__    \
                  << ": ")
#else
#define DEBUG
#endif

#define PRINT_VAR(__VARIABLE__) DEBUG << #__VARIABLE__ << " == " << __VARIABLE__

#define ABORT_UNIMPLEMENTED                                                    \
  do {                                                                         \
    DEBUG << "Called an unimplemented API!";                                   \
    abort();                                                                   \
  } while (false)

#endif // HSA_RUNTIME_LOGGING_HH
