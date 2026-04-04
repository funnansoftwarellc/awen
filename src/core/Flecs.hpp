// Wrapper header for flecs.h that suppresses third-party warnings.
//
// MSVC: #pragma warning(push, 0) silences all warnings in the global-module
//       fragment where /external:W0 does not apply to C++ module includes.
#pragma once

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif

#include <flecs.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif