/* incbin.h — embed binary files via assembler .incbin (ELF/GCC/Clang)
 * Adapted from Dale Weiler's incbin (github.com/graphitemaster/incbin). Unlicense.
 *
 * Usage (one translation unit — define):
 *   #define INCBIN_PREFIX g
 *   #define INCBIN_STYLE  INCBIN_STYLE_CAMEL
 *   #include "incbin.h"
 *   INCBIN(MyData, "/absolute/or/relative/path.bin");
 *
 * Usage (other translation units — declare):
 *   #include "incbin.h"
 *   INCBIN_EXTERN(MyData);
 *
 * Generates (with prefix g, CAMEL style):
 *   const unsigned char  gMyDataData[];   — pointer to raw bytes
 *   const unsigned char  gMyDataEnd[];    — one past last byte (+ NUL guard)
 *   const unsigned int   gMyDataSize;     — byte count
 */
#pragma once

#define INCBIN_STYLE_CAMEL 0
#define INCBIN_STYLE_SNAKE 1

#ifndef INCBIN_PREFIX
#  define INCBIN_PREFIX g
#endif

#ifndef INCBIN_STYLE
#  define INCBIN_STYLE INCBIN_STYLE_CAMEL
#endif

/* ── Internal helpers ────────────────────────────────────────────────────── */
#define INCBIN_STR_(x)   #x
#define INCBIN_STR(x)    INCBIN_STR_(x)
#define INCBIN_CAT_(a,b) a##b
#define INCBIN_CAT(a,b)  INCBIN_CAT_(a,b)
#define INCBIN_PFXD(n)   INCBIN_CAT(INCBIN_PREFIX, n)

#if INCBIN_STYLE == INCBIN_STYLE_CAMEL
#  define INCBIN_DATA_SYM(n) INCBIN_PFXD(INCBIN_CAT(n, Data))
#  define INCBIN_END_SYM(n)  INCBIN_PFXD(INCBIN_CAT(n, End))
#  define INCBIN_SIZE_SYM(n) INCBIN_PFXD(INCBIN_CAT(n, Size))
#else
#  define INCBIN_DATA_SYM(n) INCBIN_PFXD(INCBIN_CAT(n, _data))
#  define INCBIN_END_SYM(n)  INCBIN_PFXD(INCBIN_CAT(n, _end))
#  define INCBIN_SIZE_SYM(n) INCBIN_PFXD(INCBIN_CAT(n, _size))
#endif

#ifdef __cplusplus
#  define INCBIN_DECL_ extern "C"
#else
#  define INCBIN_DECL_ extern
#endif

/* ── INCBIN(Name, "file") ────────────────────────────────────────────────── *
 * Defines three global symbols in .rodata.  Use exactly once per binary file,
 * in a single .c / .cpp translation unit.                                    */
#define INCBIN(name, file)                                                       \
    __asm__(".section .rodata\n"                                                 \
            ".global " INCBIN_STR(INCBIN_DATA_SYM(name)) "\n"                   \
            ".type   " INCBIN_STR(INCBIN_DATA_SYM(name)) ", %object\n"          \
            ".align  8\n"                                                        \
            INCBIN_STR(INCBIN_DATA_SYM(name)) ":\n"                             \
            ".incbin \"" file "\"\n"                                             \
            ".global " INCBIN_STR(INCBIN_END_SYM(name))  "\n"                   \
            ".type   " INCBIN_STR(INCBIN_END_SYM(name))  ", %object\n"          \
            INCBIN_STR(INCBIN_END_SYM(name)) ":\n"                              \
            ".byte   0\n"                                                        \
            ".align  4\n"                                                        \
            ".global " INCBIN_STR(INCBIN_SIZE_SYM(name)) "\n"                   \
            ".type   " INCBIN_STR(INCBIN_SIZE_SYM(name)) ", %object\n"          \
            INCBIN_STR(INCBIN_SIZE_SYM(name)) ":\n"                             \
            ".int    " INCBIN_STR(INCBIN_END_SYM(name))                         \
                " - "  INCBIN_STR(INCBIN_DATA_SYM(name)) "\n"                   \
            ".align  4\n");                                                      \
    INCBIN_DECL_ const unsigned char INCBIN_DATA_SYM(name)[];                   \
    INCBIN_DECL_ const unsigned char INCBIN_END_SYM(name)[];                    \
    INCBIN_DECL_ const unsigned int  INCBIN_SIZE_SYM(name)

/* ── INCBIN_EXTERN(Name) ─────────────────────────────────────────────────── *
 * Forward-declares symbols produced by INCBIN in another translation unit.   */
#define INCBIN_EXTERN(name)                                                      \
    INCBIN_DECL_ const unsigned char INCBIN_DATA_SYM(name)[];                   \
    INCBIN_DECL_ const unsigned char INCBIN_END_SYM(name)[];                    \
    INCBIN_DECL_ const unsigned int  INCBIN_SIZE_SYM(name)
