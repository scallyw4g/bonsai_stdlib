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
  case Keysym: {                               \
    Plat->Input.InputField.Clicked = True;  \
    Plat->Input.InputField.Pressed = True;  \
  } break;

// TODO(Jesse)(metaprogramming): This can maybe be metaprogrammed?
#define BindKeyupToInput(Keysym, InputField) \
  case Keysym: {                             \
    Plat->Input.InputField.Clicked = False;  \
    Plat->Input.InputField.Pressed = False;  \
    Plat->Input.InputField.Released = True;  \
  } break;


#if BONSAI_WIN32
#include <bonsai_stdlib/src/platform/win32/win32_platform.h>

#elif BONSAI_LINUX
#include <bonsai_stdlib/src/platform/posix_platform.h>
#include <bonsai_stdlib/src/platform/linux/linux_platform.h>

#elif BONSAI_EMCC
#include <bonsai_stdlib/src/platform/posix_platform.h>
#include <bonsai_stdlib/src/platform/wasm_platform.h>

#else
#error "Unknown Platform!"
#endif

