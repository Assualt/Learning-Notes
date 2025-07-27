//
// Created by rick on 2022/12/28.
//

#ifndef SSP_TOOLS_ARCHIVE_STREAM_H
#define SSP_TOOLS_ARCHIVE_STREAM_H

#include <string_view>

namespace ssp::crypto {

class ArchiveStream {
public:
    ArchiveStream() = default;

    virtual ~ArchiveStream() = default;

    virtual bool Open(const std::string &path, const std::string &passwd) = 0;

    virtual bool Close() = 0;

    virtual bool Read(void *data, size_t size) = 0;

    virtual bool Write(const void *data, size_t size) = 0;
};

class ArchiveFileStream : public ArchiveStream {
public:
    ArchiveFileStream();

    ~ArchiveFileStream() override;

    bool Open(const std::string &path, const std::string &passwd) override;

    bool Close() override;

    bool Read(void *data, size_t size) override;

    bool Write(const void *data, size_t size) override;
};

class ArchiveMemoryStream : public ArchiveStream {
public:
    ArchiveMemoryStream() = default;
};

} // namespace ssp::crypto

#endif // SSP_TOOLS_ARCHIVE_STREAM_H