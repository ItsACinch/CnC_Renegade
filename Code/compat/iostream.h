/*
 * iostream.h - Compatibility shim for old-style C++ header
 *
 * Old C++ code used <iostream.h>, modern C++ uses <iostream>
 * This file provides backwards compatibility.
 */

#ifndef COMPAT_IOSTREAM_H
#define COMPAT_IOSTREAM_H

#include <iostream>
#include <iomanip>
#include <fstream>

// Old-style iostream.h put these in the global namespace
using std::cin;
using std::cout;
using std::cerr;
using std::clog;
using std::endl;
using std::flush;
using std::ios;
using std::istream;
using std::ostream;
using std::iostream;
using std::ifstream;
using std::ofstream;
using std::fstream;
using std::streambuf;
using std::filebuf;

#endif // COMPAT_IOSTREAM_H
