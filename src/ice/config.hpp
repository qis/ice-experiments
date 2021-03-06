#pragma once
#include <ice/error.hpp>

#if defined(WIN32)
#define ICE_OS_WIN32 1
#elif defined(__linux__)
#define ICE_OS_LINUX 1
#elif defined(__FreeBSD__)
#define ICE_OS_FREEBSD 1
#else
#error Unsupported OS
#endif

#ifndef ICE_OS_WIN32
#define ICE_OS_WIN32 0
#endif

#ifndef ICE_OS_LINUX
#define ICE_OS_LINUX 0
#endif

#ifndef ICE_OS_FREEBSD
#define ICE_OS_FREEBSD 0
#endif

#ifndef ICE_EXCEPTIONS
#if (defined(_MSC_VER) && _HAS_EXCEPTIONS) || !defined(_MSC_VER) && __cpp_exceptions
#define ICE_EXCEPTIONS 1
#else
#define ICE_EXCEPTIONS 0
#endif
#endif

#ifndef ICE_NO_EXCEPTIONS
#define ICE_NO_EXCEPTIONS (!ICE_EXCEPTIONS)
#endif

#ifndef ICE_DEBUG
#ifdef NDEBUG
#define ICE_DEBUG 0
#else
#define ICE_DEBUG 1
#endif
#endif

#ifndef ICE_NO_DEBUG
#define ICE_NO_DEBUG (!ICE_DEBUG)
#endif
