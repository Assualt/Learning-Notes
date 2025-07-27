
#include "archive_core.h"
#include "base/log.h"

using namespace ssp::crypto;
using namespace ssp::base;

bool ArchiveLib::Init(LIB_TYPE type)
{
    switch (type) {
        case TYPE_7ZIP:
            return Init7zipLib();

        case TYPE_CUSTOMER:
            return InitCustomerLib();

        // TODO: add other libs
        default:
            break;
    }
    return false;
}

void ArchiveLib::UnInit() {

}

bool ArchiveLib::Init7zipLib()
{
    auto ret = dllTool_.Open("7z.dll", base::LOAD_NOW);
    if (ret != 0) {
        logger.Error("7z.dll load failed, ret: %d", ret);
        return false;
    }

    createArchiveFunc_ = dllTool_.GetSymbol<CreateArchiveFunc>("CreateObject");
    getMethodPropertyFunc_ = dllTool_.GetSymbol<GetMethodPropertyFunc>("GetMethodProperty");
    getNumofMethodFunc_ = dllTool_.GetSymbol<GetNumofMethodFunc>("GetNumofMethod");

    return true;
}

bool ArchiveLib::InitCustomerLib()
{
    return true;
}

bool ArchiveLib::OpenArchive(const std::string &name, ArchiveStream &input, Archive **archive, bool isStandrd,
                             const std::string &passwd)
{
    if (!isInitialized_) {
        return false;
    }

    return true;
}

bool ArchiveLib::CloseArchive(ArchiveStream *archive)
{
    return archive ? archive->Close() : true;
}

bool ArchiveLib::Extract(Archive *archive, ArchiveStream &output, std::string_view path)
{
    return true;
}

bool ArchiveLib::Add(Archive *archive, ArchiveStream &input, std::string_view path) {
    return false;
}

ArchiveLib::~ArchiveLib() {
}
