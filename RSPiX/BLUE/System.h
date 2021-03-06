///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2016 RWS Inc, All Rights Reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of version 2 of the GNU General Public License as published by
// the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
///////////////////////////////////////////////////////////////////////////////
//
//  System.h
//
// History:
//    07/19/96 JMI  Started.
//
//////////////////////////////////////////////////////////////////////////////
//
// This file decides what platform you are on and #includes the correct
// *System.h.  If you #include the platform specific *System.h instead of this
// one, you will get a compile error.
//
//////////////////////////////////////////////////////////////////////////////
//
//  UnixSystem.h
// 
// History:
//    06/01/04 RCG    Added.
//
////////////////////////////////////////////////////////////////////////////////
//
//  This file provides typedefs, macros, pragmas, etc. for Unix systems.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef SYSTEM_H
#define SYSTEM_H

// C++
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <cstdint>
#include <map>

// RSPiX
#include <CompileOptions.h>

////////////////////////////////////////////////////////////////////////////////
// Handy defines.
////////////////////////////////////////////////////////////////////////////////

#ifndef QUOTE
#define Q(x) #x
#define QUOTE(x) Q(x)
#endif

#ifndef TRUE
# define TRUE     1
#endif

#ifndef FALSE
# define FALSE    0
#endif

#if 1
//!defined(STRICT) && !defined(PENDANT)

# ifndef SUCCESS
#  define SUCCESS  0
# endif

# ifndef FAILURE
#  define FAILURE -1
# endif

typedef const char* c_string;
typedef int errcode_t;

#else

#undef SUCCESS
#undef FAILURE

enum errcode : int
{
  FAILURE = -1,
  SUCCESS,
};

constexpr errcode operator * (errcode c, int mult)
  { return errcode(reinterpret_cast<int>(c) * mult); }

struct errcode_t
{
  int code;
#if defined(PENDANT)
  constexpr errcode_t& operator =(errcode c)
  {
    code = c;
    return *this;
  }

  constexpr errcode_t& operator =(errcode_t c)
  {
    code = c.code;
    return *this;
  }
#else
  template<typename T>
  constexpr errcode_t& operator =(T c)
  {
    static_assert(sizeof(T) <= sizeof(errcode_t), "not large enough");
    code = c;
    return *this;
  }
#endif
  template<typename T>
  constexpr operator T(void)
  {
    static_assert(sizeof(T) >= sizeof(errcode_t), "data would truncate");
    return reinterpret_cast<T>(code);
  }
};

struct c_string
{
  const char* data;

  template<typename T>
  constexpr c_string& operator =(T other)
  {
    static_assert(sizeof(c_string) != sizeof(T), "did you mean to use nullptr?");
    data = other;
    return *this;
  }

  constexpr operator const char*(void)
  {
    return data;
  }
};

#endif

////////////////////////////////////////////////////////////////////////////////
// types, type limits, standard functions and endian detection
////////////////////////////////////////////////////////////////////////////////

#include <BLUE/portable_endian.h>
#include <sys/types.h>
#include <sys/stat.h>

// errors raised upon cataclysmic failure

static_assert(sizeof(uintptr_t) == sizeof(void*), "your compiler is broken!");

#if CHAR_BIT != 8
# error "unsupported char size"
#endif

////////////////////////////////////////////////////////////////////////////////
// POSIX specific fixes
////////////////////////////////////////////////////////////////////////////////

#if defined(_POSIX_VERSION) // || _POSIX_VERSION < 200112L
# include <unistd.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// compiler specific fixes
////////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER) // for all versions
# pragma message("I find your lack of standards compliance disturbing. ;)")
# include <BLUE/stdint_msvc.h>
# include <BaseTsd.h> // for SSIZE_T
# include <direct.h> // for mkdir

# define S_IRUSR    0400  // Read by owner.
# define S_IWUSR    0200  // Write by owner.
# define S_IXUSR    0100  // Execute by owner.
# define S_IRWXU    0700  // Read, write, and execute by owner.
# define S_IRGRP    0040  // Read by group.
# define S_IWGRP    0020  // Write by group.
# define S_IXGRP    0010  // Execute by group.
# define S_IRWXG    0070  // Read, write, and execute by group.
# define S_IROTH    0004  // Read by others.
# define S_IWOTH    0002  // Write by others.
# define S_IXOTH    0001  // Execute by others.
# define S_IRWXO    0007  // Read, write, and execute by others.
# define S_IFBLK    0060000  // Block device.
# define S_IFIFO    0010000  // FIFO.
# define S_IFLNK    0120000  // Symbolic link.
# define S_IFSOCK   0140000  // Socket.
# define S_ISTYPE(mode, mask)  (((mode) & 0170000) == (mask))
# define S_ISDIR(mode)         S_ISTYPE((mode), S_IFDIR)
# define S_ISCHR(mode)         S_ISTYPE((mode), S_IFCHR)
# define S_ISBLK(mode)         S_ISTYPE((mode), S_IFBLK)
# define S_ISREG(mode)         S_ISTYPE((mode), S_IFREG)

#define stat _stat
typedef uint32_t mode_t; // no sys/types.h
inline int _mkdir(const char *path, int) { return _mkdir(path); }
# define mkdir        _mkdir
# define strcasecmp   _stricmp
# define NOTE(x)      __pragma(message("NOTE: " x))
# define alignof(x)   __alignof(x) // C++11 patch
#  define noexcept
# if _MSC_VER < 1900
#  define constexpr   inline
#  define snprintf    _snprintf
#  define vsnprintf   _vsnprintf
#  define PATH_MAX    _MAX_PATH
typedef SSIZE_T ssize_t;
# endif
#elif defined(_WIN32) // everything for Windows that isn't MSVC
# include <unistd.h>
inline int mkdir(const char *path, int) { return mkdir(path); }
# define DO_PRAGMA(x) _Pragma (#x)
# define NOTE(x) DO_PRAGMA(message("NOTE: " x))
#endif

#if defined(__DJGPP__)
# if defined(__STRICT_ANSI__)
#  error You need to disable C++ standards complaince for DJGPP
# endif
# include <limits.h> // somehow not referenced by climits?
#endif

////////////////////////////////////////////////////////////////////////////////
// platform specific fixes
////////////////////////////////////////////////////////////////////////////////

#if !defined(_WIN32) // non-Windows compiler
# if defined(__GNUC__) && defined(__GNUC_MINOR__) // GCC based compiler
#  define __GNUC_PREREQ(maj, min) ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
# else // non-GCC based compiler
#  define __GNUC_PREREQ(maj, min) 0
# endif
# define DO_PRAGMA(x) _Pragma (#x)
# if !defined(__GNUC__) || __GNUC_PREREQ(4,4)
#  define NOTE(x) DO_PRAGMA(message("NOTE: " x))
# else
#  define NOTE(x) DO_PRAGMA(warning("NOTE: " x))
# endif
#endif

////////////////////////////////////////////////////////////////////////////////
// non-standard definitions
////////////////////////////////////////////////////////////////////////////////

/* Minimum of unsigned integral types.  */
# define UINT8_MIN  0
# define UINT16_MIN 0
# define UINT32_MIN 0
# define UINT64_MIN 0

// 128-bit got a little trickier...
#if BYTE_ORDER == LITTLE_ENDIAN
typedef struct { uint64_t lo;
                  int64_t hi; } int128_t;
typedef struct { uint64_t lo;
                 uint64_t hi; } uint128_t;
#elif BYTE_ORDER == BIG_ENDIAN
typedef struct {  int64_t hi;
                 uint64_t lo; } int128_t;
typedef struct { uint64_t hi;
                 uint64_t lo; } uint128_t;
#elif BYTE_ORDER == PDP_ENDIAN
# error Middle-endian is not implemented.
#elif defined(BYTE_ORDER)
# error Unknown endian format detected!
#else
# error The endianness of your machine could not be detected!
#endif

////////////////////////////////////////////////////////////////////////////////
// Macros to avoid warnings
////////////////////////////////////////////////////////////////////////////////

#define UNUSED1(a)                (void)(a)
#define UNUSED2(a,b)             UNUSED1(a),UNUSED1(b)
#define UNUSED3(a,b,c)           UNUSED1(a),UNUSED2(b,c)
#define UNUSED4(a,b,c,d)         UNUSED1(a),UNUSED3(b,c,d)
#define UNUSED5(a,b,c,d,e)       UNUSED1(a),UNUSED4(b,c,d,e)
#define UNUSED6(a,b,c,d,e,f)     UNUSED1(a),UNUSED5(b,c,d,e,f)
#define UNUSED7(a,b,c,d,e,f,g)   UNUSED1(a),UNUSED6(b,c,d,e,f,g)
#define UNUSED8(a,b,c,d,e,f,g,h) UNUSED1(a),UNUSED7(b,c,d,e,f,g,h)

#define VA_NUM_ARGS_IMPL(_1,_2,_3,_4,_5, _6, _7, _8, N,...) N
#define VA_NUM_ARGS(...) VA_NUM_ARGS_IMPL(__VA_ARGS__,8, 7, 6, 5, 4, 3, 2, 1)

#define UNUSED_IMPL_(nargs) UNUSED ## nargs
#define UNUSED_IMPL(nargs) UNUSED_IMPL_(nargs)
#define UNUSED(...) UNUSED_IMPL( VA_NUM_ARGS(__VA_ARGS__))(__VA_ARGS__ )

#if defined(RELEASE) || defined(NDEBUG) || defined(_NDEBUG)
#define UNHANDLED_SWITCH    default: break;
#else
#define UNHANDLED_SWITCH    default: {{ static bool displayed = false; if(!displayed) { displayed = true; TRACE("Unhandled switch\n"); } }} break
#endif

////////////////////////////////////////////////////////////////////////////////
// Aliases
////////////////////////////////////////////////////////////////////////////////

#define MILLISECONDS_MAX  INT32_MAX
#define MICROSECONDS_MAX UINT64_MAX

typedef  int32_t milliseconds_t;
typedef uint64_t microseconds_t;

typedef uint8_t volume_t;
typedef uint8_t channel_t;
typedef uint16_t palindex_t;

////////////////////////////////////////////////////////////////////////////////
// Palette/Pixel stuff
////////////////////////////////////////////////////////////////////////////////
/*
namespace palette
{
  static const uint16_t size = 0x0100;
}

struct color24_t
{
  channel_t blue;
  channel_t green;
  channel_t red;
};
static_assert(sizeof(color24_t) == 3, "compiler fail!");

struct color32_t
{
  channel_t blue;
  channel_t green;
  channel_t red;
  channel_t alpha;
};
static_assert(sizeof(color32_t) == 4, "compiler fail!");


// These pixel types take the endian order of the system into account.
typedef uint8_t RPixel;
typedef uint16_t RPixel16;

struct pixel24_t
{
  channel_t red;
  channel_t green;
  channel_t blue;

  constexpr bool operator==(const pixel24_t& other) const
  {
    return blue  == other.blue  &&
           green == other.green &&
           red   == other.red;
  }
};
static_assert(sizeof(pixel24_t) == 3, "compiler fail!");

struct pixel32_t
{
  channel_t alpha;
  channel_t red;
  channel_t green;
  channel_t blue;

  constexpr bool operator==(const pixel32_t& other) const
  {
    return blue  == other.blue  &&
           green == other.green &&
           red   == other.red   &&
           alpha == other.alpha;
  }
};
static_assert(sizeof(pixel32_t) == 4, "compiler fail!");
*/
////////////////////////////////////////////////////////////////////////////////
// Usefull Templates
////////////////////////////////////////////////////////////////////////////////

#undef MIN
template <class T>
constexpr T MIN(T a,T b) { return (a < b) ? a : b; }

#undef MAX
template <class T>
constexpr T MAX(T a,T b) { return (a > b) ? a : b; }

#undef SWAP // Swaps two identical typed variables
template <class T>
inline void SWAP(T &a,T &b) { T temp = a; a = b; b = temp; }

#undef SQR // squares a number
template <class T>
constexpr T SQR(T x) { return x * x; }

#undef ABS // returns the absolute value of a parameter
template <class T>
constexpr T ABS(T x) { return (x < 0) ? -x : x; }

template <class T> // returns the square of the absolute value
constexpr T ABS2(T x,T y) { return SQR(x) + SQR(y); }

template <class T> // returns the square of the absolute value
constexpr T ABS2(T x,T y,T z) { return SQR(x) + SQR(y) + SQR(z); }

#undef SGN // returns a binary sign (+1 or -1)
template <class T>
constexpr T SGN(T x) { return (x < 0) ? (T)-1 : (T)1; }

#undef SGN3 // returns a trinary sign (+1, 0, or -1)
template <class T>
constexpr T SGN3(T x) { return (x == 0) ? (T)0 : ((x < 0) ? (T)-1 : (T)1); }


////////////////////////////////////////////////////////////////////////////////
// Debug API
//
// Define the TRACE, STRACE, and ASSERT macros.  If _DEBUG or TRACENASSERT are
// defined, then these macros are usefull debugging aids.  Otherwise, it is
// assumed that the program is being compiled in "release" mode, and all three
// of the macros are changed such that no code nor data results from their
// use, thereby eliminating all traces of them from the program.
//
// TRACE is like printf, but sends the output to the debug window.  Note that
// it slips in the file and line number information before printing whatever
// the user requested.
//
// STRACE is like TRACE, except that it doesn't display the file and line
// number information.
//
// ASSERT is used to assert that an expression is true.  If it is, in fact,
// true, then ASSERT does nothing.  If not, then it emits a signal and halts
// execution.
//
////////////////////////////////////////////////////////////////////////////////

extern void rspTrace(const char* szFrmt, ...);

#if defined(RELEASE) || defined(NDEBUG) || defined(_NDEBUG)
#define STRACE    1 ? (void)0 : rspTrace
#define TRACE     STRACE
#define ASSERT(x)
#elif defined(_DEBUG) || defined(TRACENASSERT)
# include <cassert>
# define STRACE(...)  rspTrace(__VA_ARGS__)
# define TRACE(...)   STRACE("%s(%d):", __FILE__, __LINE__),STRACE(__VA_ARGS__)
# if defined(__DOS__)
#  define ASSERT(x)  if(!(x)) { TRACE("ASSERT failed! testing: %s", QUOTE(x)); assert(x); }
# else
# define ASSERT(x)  assert(x)
# endif
#else
# include <cassert>
# define STRACE(...)  rspTrace(__VA_ARGS__)
# define TRACE(...)   STRACE(__VA_ARGS__)
# define ASSERT(x)    assert(x)
#endif

#endif // SYSTEM_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
