// RubberBandSample.h --- A Win32 application                  -*- C++ -*-
//////////////////////////////////////////////////////////////////////////////

#ifndef RUBBERBANDSAMPLE_H_
#define RUBBERBANDSAMPLE_H_

//////////////////////////////////////////////////////////////////////////////

// resource IDs
#include "resource.h"

// tstring
#ifdef UNICODE
    typedef std::wstring tstring;
#else
    typedef std::string tstring;
#endif

// NOTE: Digital Mars C/C++ Compiler doesn't define INT_PTR type likely.
#ifdef __DMC__
    #define INT_PTR BOOL
#endif

#include "WindowBase.hpp"

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef RUBBERBANDSAMPLE_H_

//////////////////////////////////////////////////////////////////////////////
