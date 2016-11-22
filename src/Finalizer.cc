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
 * @author tomi.aijo@parmance.com and pekka.jaaskelainen@parmance.com
 *         for General Processor Tech.
 */

#include "Finalizer.hh"
#include "HSAILProgram.hh"

HSAReturnValue<hsa_ext_program_s>
Finalizer::createProgram(hsa_machine_model_t MM, hsa_profile_t P,
                         hsa_default_float_rounding_mode_t RM) {
  phsa::HSAILProgram *hsailProgram = new phsa::HSAILProgram(MM, P, RM);
  return HSAReturn(HSA_STATUS_SUCCESS, hsailProgram->toHSAObject());
}

void Finalizer::destroyProgram(hsa_ext_program_t Program) {
  phsa::HSAILProgram *HsailProgram = phsa::HSAILProgram::fromHSAObject(Program);
  delete HsailProgram;
}

void Finalizer::addModule(hsa_ext_program_t Program, hsa_ext_module_t Module) {
  phsa::HSAILProgram::fromHSAObject(Program)->addModule(Module);
}
