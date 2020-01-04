#ifndef _X_FILE_H_2018_12_18
#define _X_FILE_H_2018_12_18

#include "xmtdef.h"
#include "base/xstring.h"

#if defined(XMT_WIN_32)
#include <fcntl.h>
#include <io.h>
#include <sys\stat.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

NAMESPACE_BEGIN

#if defined(CMX_WIN32_VER)
#define DEFAULT_OPEN_MODE (_S_IREAD | _S_IWRITE)
#define DEFAULT_DIR_MODE (_S_IREAD | _S_IWRITE)
#else
#define DEFAULT_OPEN_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define DEFAULT_DIR_MODE (S_IRWXG | S_IRWXU | S_IRWXO | S_IXOTH)
#endif

class TFile {};
NAMESPACE_END
#endif
