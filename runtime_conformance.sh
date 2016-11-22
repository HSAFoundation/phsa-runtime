#!/bin/sh

# Test suite bugs (to report):
# image_: Image support not implemented, tests assume otherwise
# hsa_system_get_extension_table: Assumes image extension is supported
# async_invalid_group_memory: Tests assumes that 2^32 allocation is bound to fail
# hsa_region_get_info: Invalid test, asserts that if region is not global, max_size must be 0

# Reported test suite bugs:
# hsa_executable_symbol_get_info: https://github.com/HSAFoundation/HSA-Runtime-Conformance/issues/4
# hsa_ext_program_finalize_directive_mismatch: https://github.com/HSAFoundation/HSA-Runtime-Conformance/issues/5

# TODO:
# code_define_global_program, code_module_scope_symbol, code_mixed_scope, code_define_readonly_agent: TODO


export CK_DEFAULT_TIMEOUT=1200
export CK_TIMEOUT_MULTIPLIER=100

ctest -E "image_*|hsa_system_get_extension_table|async_invalid_group_memory|\
hsa_region_get_info|hsa_executable_symbol_get_info|code_module_scope_symbol|code_mixed_scope|code_define_readonly_agent"

