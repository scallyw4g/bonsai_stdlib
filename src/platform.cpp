
#if BONSAI_LINUX || BONSAI_EMCC
#include <bonsai_stdlib/src/platform/posix_platform.cpp>
#endif

#if BONSAI_WIN32
#include <bonsai_stdlib/src/platform/win32_platform.cpp>
#elif BONSAI_LINUX
#include <bonsai_stdlib/src/platform/unix_platform.cpp>
#elif BONSAI_EMCC
#include <bonsai_stdlib/src/platform/wasm_platform.cpp>
#else
#error "Unsupported Platform"
#endif
