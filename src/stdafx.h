// stdafx.h -- precompiled header                               -*- C++ -*-
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Windows-related

#ifdef _WIN32
    // We must do it strictly!!!
    #define STRICT 1

    // the target versioning header
    #include "targetver.h"

    #if defined(_MSC_VER) && !defined(NDEBUG)
        // for detecting memory leak (MSVC only)
        #define _CRTDBG_MAP_ALLOC
        #include <crtdbg.h>
    #endif

    // Windows headers
    #include <windows.h>
    #include <windowsx.h>
    #include <tchar.h>
#endif

//////////////////////////////////////////////////////////////////////////////
// C runtime headers

#include <cstdlib>
#include <cstring>
#include <cassert>

//////////////////////////////////////////////////////////////////////////////
// C++ headers

#include <string>
#include <vector>
#include <new>

//////////////////////////////////////////////////////////////////////////////
// private headers

#include "config.h"
#include "resource.h"
#include "RubberBandSample.hpp"

//////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER) && !defined(NDEBUG)
    // for detecting memory leak (MSVC only)
    #define new ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

//////////////////////////////////////////////////////////////////////////////
