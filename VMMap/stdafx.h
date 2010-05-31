//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: New BSD License (BSD).
//------------------------------------------------------------------------------

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

// TODO: reference additional headers your program requires here
#include <string>
#include <list>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include <windows.h>
#define DPSAPI_VERSION 1
#include <psapi.h>
#include <tlhelp32.h>
#pragma comment(lib, "psapi.lib")

// From MSDN sample. Eww.
#define MAX_NAME 256

#ifdef UNICODE
#define tstring wstring
#define tcout wcout
#define tcerr wcerr
#define tstrlen wcslen
#else
#define tstring string
#define tcout cout
#define tcerr cerr
#define tstrlen strlen
#endif
