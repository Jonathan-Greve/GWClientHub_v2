// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H


// add headers that you want to pre-compile here
#pragma warning(push)
#pragma warning(disable : 4244 4200 4101)
#include <sw/redis++/redis++.h>
#pragma warning(pop)

#include "framework.h"
#include <cstdint>
#include <cstdlib>

// Additional dependencies our project requires to build
#pragma comment(lib, "Ws2_32.lib")

#endif //PCH_H
