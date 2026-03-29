// Wrapper header for flecs.h that suppresses third-party warnings.
//
// MSVC: #pragma warning(push, 0) silences all warnings in the global-module
//       fragment where /external:W0 does not apply to C++ module includes.
// Clang/GCC: #pragma GCC system_header marks this translation unit as a system
//             header, which causes the compiler to suppress all warnings from
//             headers included below.
#pragma once

#ifdef _MSC_VER
#pragma warning(push, 0)
#else
#pragma GCC system_header
#endif

#include <flecs.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif
