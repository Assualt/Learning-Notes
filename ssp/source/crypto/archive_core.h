#ifndef SSP_TOOLS_ARCHIVE_CORE_H
#define SSP_TOOLS_ARCHIVE_CORE_H

#include "archive.h"
#include "archive_stream.h"
#include <string_view>
#include "base/dll.h"

#include "CPP/Common/MyGuidDef.h"
#include "CPP/Common/MyWindows.h"

namespace ssp::crypto {

enum LIB_TYPE {
    TYPE_CUSTOMER, // 自定义的lib库
    TYPE_7ZIP,     // 7z的lib库
    TYPE_BUTT
};

using CreateArchiveFunc = long (*)(const GUID *, const GUID *, void **);
using GetNumofMethodFunc = long (*)(uint32_t *);
using GetMethodPropertyFunc = long(*)(uint32_t, PROPID, PROPVARIANT *);

class ArchiveLib {
public:
    ~ArchiveLib();

    ArchiveLib() = default;

public:
    bool Init(LIB_TYPE type = TYPE_7ZIP);

    void UnInit();

    bool IsInitialized() const { return isInitialized_; }

    bool OpenArchive(const std::string &archiveName, ArchiveStream &input, Archive **archive, bool isStandrd = true,
                     const std::string &passwd = "");

    bool CloseArchive(ArchiveStream *archive);

    bool Extract(Archive *archive, ArchiveStream &output, std::string_view path);

    bool Add(Archive *archive, ArchiveStream &input, std::string_view path);

    bool Delete(Archive *archive, std::string_view path);

private:
    bool Init7zipLib();

    bool InitCustomerLib();

private:
    bool isInitialized_{false};
    CreateArchiveFunc createArchiveFunc_{nullptr};
    GetMethodPropertyFunc getMethodPropertyFunc_{nullptr};
    GetNumofMethodFunc getNumofMethodFunc_{nullptr};
    base::Dll dllTool_;
};

} // namespace ssp::crypto
#endif // SSP_TOOLS_ARCHIVE_CORE_H