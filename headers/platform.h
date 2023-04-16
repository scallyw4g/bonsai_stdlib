#if POOF_PREPROCESSOR

// TODO(Jesse: immediate): Remove these ?  Certainly move them out of here
// because they're windows specific
#define __clrcall
#define __cdecl
#define __stdcall
#define __declspec(...)

// TODO(Jesse: immediate): Remove these
#define APIENTRY __stdcall
#define WINAPI
#define WINAPI_FAMILY 100
#define DECLSPEC_IMPORT

// TODO(Jesse): These forcibly skip some includes that are broken.  Some expose
// actual bugs in poof
#define _INC_WINAPIFAMILY
#define __INTRIN_H
#define _CHRONO_
#define _RANDOM_
#define _ALGORITHM_
#define _NUMERIC_
/* #define _STL_COMPILER_PREPROCESSOR 0 */

#define __clang__ 1
#define __clang_major__ 10
#define __clang_minor__ 0

#define _MSC_VER 2000

// TODO(Jesse): This is actually a preprocessor keyword.  We should implement it
// properly
#define __has_cpp_attribute(...) 0

#endif
//
// Required for FILE* type .. might want to rebuild the file API to use
// platform functions instead, but for now the CRT ones are good enough.
//
#include <stdio.h>


#define CACHE_LINE_SIZE (64)

// TODO(Jesse)(metaprogramming): This can certainly be metaprogrammed.  Probably
// we can do the BindKey[down|up] directly instead of having this helper macro.
#define BindToInput(Keysym, InputField, Boolean) \
  case Keysym: {                                 \
    Plat->Input.InputField.Clicked = Boolean; \
    Plat->Input.InputField.Pressed = Boolean;     \
  } break;

// TODO(Jesse)(metaprogramming): This can maybe be metaprogrammed?
#define BindKeydownToInput(Keysym, InputField) \
    BindToInput(Keysym, InputField, True)

// TODO(Jesse)(metaprogramming): This can maybe be metaprogrammed?
#define BindKeyupToInput(Keysym, InputField) \
    BindToInput(Keysym, InputField, False)

#if BONSAI_WIN32
#include <bonsai_stdlib/platform/win32_platform.h>

#elif BONSAI_LINUX
#include <bonsai_stdlib/platform/posix_platform.h>
#include <bonsai_stdlib/platform/unix_platform.h>

#elif BONSAI_EMCC
#include <bonsai_stdlib/platform/posix_platform.h>
#include <bonsai_stdlib/platform/wasm_platform.h>

#else
#error "Unknown Platform!"
#endif
