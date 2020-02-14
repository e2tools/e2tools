/** \file compiler-definitions.h
 * \brief Compiler specific definitions.
 */

#ifndef E2TOOLS_COMPILER_DEFINITIONS_H
#define E2TOOLS_COMPILER_DEFINITIONS_H


#include "e2tools-autoconfig.h"


/** Size of struct member
 *
 * Cf. http://stackoverflow.com/a/2129531/182675
 */
#define SIZEOF_MEMBER(structname, membername) \
  sizeof(((structname *)NULL)->membername)


/** Compile time assertion, to be used within a function */
#define COMPILE_TIME_ASSERT(CONDITION) \
  switch (0) {            \
  case 0:                 \
  case (CONDITION):       \
    break;                \
  }


/** Helper for BARE_COMPILE_TIME_ASSERT() macro
 *
 * This is ugly C preprocessor macro hacking to get the value of the
 * macro __LINE__ to be used in the function name. You need not
 * understand how this works - just accept that it does.
 */
#define MAKE_BARE_COMPILE_TIME_ASSERT_NAME \
  MAKE_BARE_COMPILE_TIME_ASSERT_NAME1(COMPILE_TIME_ASSERT_fails_in_line, __LINE__)


/** Helper for BARE_COMPILE_TIME_ASSERT() macro
 *
 * This is ugly C preprocessor macro hacking to get the value of the
 * macro __LINE__ to be used in the function name. You need not
 * understand how this works - just accept that it does.
 */
#define MAKE_BARE_COMPILE_TIME_ASSERT_NAME1(BASE, PARAM) \
  MAKE_BARE_COMPILE_TIME_ASSERT_NAME2(BASE, PARAM)


/** Helper for BARE_COMPILE_TIME_ASSERT() macro
 *
 * This is ugly C preprocessor macro hacking to get the value of the
 * macro __LINE__ to be used in the function name. You need not
 * understand how this works - just accept that it does.
 */
#define MAKE_BARE_COMPILE_TIME_ASSERT_NAME2(BASE, PARAM)        \
  BASE ## _ ## PARAM


/** Compile time assertion, to be used outside a function
 *
 * The generated function is generated "naked", so that it does not
 * actually produce any code in the output. Thus the compile time
 * assertion does not end up producing any actual code. The
 * impacto on the object file and the binary should be mainly an
 * entry in the symbol table which is acceptable.
 */
#define BARE_COMPILE_TIME_ASSERT(CONDITION)               \
  static                                                  \
  void MAKE_BARE_COMPILE_TIME_ASSERT_NAME(void)           \
    FUNCTION_ATTRIBUTE_NAKED_IF_POSSIBLE                  \
    __attribute__(( used ));                              \
  static                                                  \
  void MAKE_BARE_COMPILE_TIME_ASSERT_NAME(void)           \
  {                                                       \
    COMPILE_TIME_ASSERT(CONDITION);                       \
  }


#endif /* !defined(E2TOOLS_COMPILER_DEFINITIONS_H) */
