#include "archive_stream.h"

using namespace ssp::crypto;

ArchiveFileStream::ArchiveFileStream()
    : ArchiveStream()
{
}

ArchiveFileStream::~ArchiveFileStream()
{
    Close();
}

bool ArchiveFileStream::Open(const std::string &filename, const std::string &password)
{
    // TODO
    return false;
}

bool ArchiveFileStream::Close()
{
    // TODO
    return false;
}

bool ArchiveFileStream::Read(void *buffer, size_t size)
{
    // TODO
    return false;
}

bool ArchiveFileStream::Write(const void *buffer, size_t size)
{
    // TODO
    return false;
}