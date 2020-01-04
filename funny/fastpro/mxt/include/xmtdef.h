#ifndef _X_NAMESPACE_2018_12_14
#define _X_NAMESPACE_2018_12_14

#ifndef NOT_USE_NAME_SPACE
//! The begining of the namespace
#define NAMESPACE_BEGIN namespace xmt {
//! The namespace's name
#define NAMESPACE_NAME xmt
//! The end of the namespace
#define NAMESPACE_END }
#else
//! The begining of the namespace
#define NAMESPACE_BEGIN
//! The namespace's name
#define NAMESPACE_NAME
//! The end of the namespace
#define NAMESPACE_END
#endif

#ifndef __GNUC__
#pragma warning(disable : 4290)
#pragma warning(disable : 4786)
#pragma warning(disable : 4291)
#pragma warning(disable : 4355)
#pragma warning(disable : 4996)
#pragma warning(disable : 4503)
#pragma warning(disable : 4250)
#endif

// judge the os type

#if defined(_WIN32) || defined(__WIN32__) || defined(WINDOWS)
#define XMT_WIN_32
#include <windows.h>
#else
#define XMT_UNIC
#endif

enum VAL_TYPE {
    TYPE_CHAR,
    TYPE_SHORT,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_LONG,
    TYPE_LONGLONG,
    TYPE_STRING
};
// Data defined
typedef unsigned long long ulonglong;

// contain some necessary .h
#endif
