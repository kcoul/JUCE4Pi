# BinaryToCArray.cmake
# Converts a binary file to a C source file defining three symbols that match
# what incbin.h's INCBIN() macro would produce, for use on compilers that do
# not support GAS inline assembly (e.g. MSVC).
#
# Invoke at build time via add_custom_command:
#   cmake -DINPUT=<path> -DOUTPUT=<path> -DSYMBOL=<Name> -DPREFIX=<prefix>
#         -P /path/to/BinaryToCArray.cmake
#
# Example — INCBIN(SileroVadOnnx, ...) with PREFIX=g produces:
#   const unsigned char gSileroVadOnnxData[];
#   const unsigned int  gSileroVadOnnxSize;
#   (gSileroVadOnnxEnd is declared extern in the header but never used; omitted)

cmake_minimum_required(VERSION 3.22)

if(NOT DEFINED INPUT OR NOT DEFINED OUTPUT OR NOT DEFINED SYMBOL OR NOT DEFINED PREFIX)
    message(FATAL_ERROR "BinaryToCArray.cmake requires -DINPUT -DOUTPUT -DSYMBOL -DPREFIX")
endif()

file(READ "${INPUT}" _hex HEX)
string(LENGTH "${_hex}" _hexlen)
math(EXPR _size "${_hexlen} / 2")

# Turn "deadbeef..." into "0xde,0xad,0xbe,0xef,..." (trailing comma is valid C99)
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," _arr "${_hex}")

file(WRITE "${OUTPUT}" "\
/* Auto-generated from ${INPUT} — do not edit. */
#include <stddef.h>

#ifdef __cplusplus
extern \"C\" {
#endif

const unsigned char ${PREFIX}${SYMBOL}Data[] = {
    ${_arr}
};
const unsigned int ${PREFIX}${SYMBOL}Size = ${_size}U;

#ifdef __cplusplus
}
#endif
")
