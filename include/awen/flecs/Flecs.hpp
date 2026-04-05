// Wrapper header for flecs.h that suppresses third-party warnings.
//
// MSVC: #pragma warning(push, 0) silences all warnings in this header where
//       /external:W0 may not suppress template-instantiation diagnostics.
#pragma once

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif

#include <flecs.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif
