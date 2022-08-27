#include "StreamBase.h"
#include <assert.h>
#include <memory>
namespace muduo {
namespace base {

const size_t        StreamStatus::nEOS        = static_cast<size_t>(0);
const size_t        StreamStatus::nERRORS     = static_cast<size_t>(-1);
const off_t         StreamStatus::nOutOfRange = static_cast<off_t>(-1);
static const size_t g_nMinCacheSize           = 4 * 1024;
static const size_t g_nMaxCacheSize           = 4 * 1024 * 1024;

ReadStreamBuffer::ReadStreamBuffer(ReadStream *pParent, size_t nCacheSize)
    : m_pParent(pParent)
    , m_nCacheSize(nCacheSize)
    , m_lpReadBegin(NULL)
    , m_lpReadEnd(NULL)
    , m_lpBufferBegin(NULL) {
    if (m_pParent)
        m_pParent->allocReadBuffer(*this, nCacheSize);
}

bool ReadStreamBuffer::init(ReadStream *pParent, size_t nCacheSize) {
    release();
    if (pParent) {
        m_pParent = pParent;
        pParent->allocReadBuffer(*this, nCacheSize);
        return true;
    }
    return false;
}

void ReadStreamBuffer::release(void) {
    if (m_pParent) {
        m_pParent->releaseReadBuffer(*this);
        m_lpReadBegin = m_lpReadEnd = m_lpBufferBegin = NULL;
        m_pParent                                     = NULL;
        m_nCacheSize                                  = 0;
    }
}

off_t ReadStreamBuffer::seekg(off_t nOffset, StreamStatus::seek_dir nSeekFrom) {
    assert(m_lpBufferBegin != NULL && m_pParent != NULL);

    if (nSeekFrom == StreamStatus::seek_begin)
        nOffset -= getRestCount();
    m_lpReadBegin = m_lpReadEnd;
    off_t result  = m_pParent->seekg(nOffset, nSeekFrom);
    getNext();
    return result;
}

void ReadStreamBuffer::swap(ReadStreamBuffer &buf) {
    if (this != &buf) {
        std::swap(m_pParent, buf.m_pParent);
        std::swap(m_nCacheSize, buf.m_nCacheSize);
        std::swap(m_lpReadBegin, buf.m_lpReadBegin);
        std::swap(m_lpReadEnd, buf.m_lpReadEnd);
        std::swap(m_lpBufferBegin, buf.m_lpBufferBegin);
    }
}

//////////////////////////////////////////////////////////////////////////

WriteStreamBuffer::WriteStreamBuffer(WriteStream *pParent, size_t nCacheSize)
    : m_pParent(pParent)
    , m_lpWriteBegin(NULL)
    , m_lpWriteEnd(NULL)
    , m_lpBufferBegin(NULL) {
    if (m_pParent)
        m_pParent->allocWriteBuffer(*this, nCacheSize);
}

bool WriteStreamBuffer::init(WriteStream *pParent, size_t nCacheSize) {
    release();
    if (pParent) {
        m_pParent = pParent;
        m_pParent->allocWriteBuffer(*this, nCacheSize);
    } // if (pParent)
    return m_lpWriteBegin != m_lpWriteEnd;
}

size_t WriteStreamBuffer::seekp(off_t nOffset, StreamStatus::seek_dir nSeekFrom) {
    getNext();
    return m_pParent->seekp(nOffset, nSeekFrom);
}

void WriteStreamBuffer::release(void) {
    if (m_lpBufferBegin) {
        m_pParent->releaseWriteBuffer(*this);
        m_lpBufferBegin = m_lpWriteBegin = m_lpWriteEnd = NULL;
        m_pParent                                       = NULL;
    }
}

size_t WriteStreamBuffer::flush(void) { return m_pParent->flushWriteBuffer(*this, true); }

void WriteStreamBuffer::swap(WriteStreamBuffer &buf) {
    if (this != &buf) {
        std::swap(m_pParent, buf.m_pParent);
        std::swap(m_lpWriteBegin, buf.m_lpWriteBegin);
        std::swap(m_lpWriteEnd, buf.m_lpWriteEnd);
        std::swap(m_lpBufferBegin, buf.m_lpBufferBegin);
    }
}

//////////////////////////////////////////////////////////////////////////

off_t ReadStream::tellg(void) { return nOutOfRange; }

off_t ReadStream::seekg(off_t /*nOffset*/, StreamStatus::seek_dir /*nSeekFrom*/) { return nOutOfRange; }

size_t ReadStream::loopRead(void *lpBuf, size_t nSize, size_t nHasReadSize) {
    size_t nRest = nSize - nHasReadSize;
    tbyte *tpBuf = static_cast<tbyte *>(lpBuf) + nHasReadSize;
    while (nRest > 0) {
        size_t nReadNum = read(tpBuf, nRest);
        if (!StreamStatus::resultIsOK(nReadNum))
            break;
        nRest -= nReadNum;
        tpBuf += nReadNum;
    }
    return nSize - nRest;
}

size_t ReadStream::fillReadBuffer(ReadStreamBuffer &buf, size_t nSuggestSize) {
    if (buf.getRestCount() >= nSuggestSize)
        return 0;

    const tbyte *pBuffer    = NULL;
    const tbyte *pReadBegin = NULL;
    const tbyte *pReadEnd   = NULL;
    size_t       nSize;
    getReadBufferImp(buf, pBuffer, nSize, pReadBegin, pReadEnd);

    size_t nRestDataNotRead = pReadEnd - pReadBegin;
    if (pReadBegin != pBuffer) {
        memmove(const_cast<tbyte *>(pBuffer), pReadBegin, nRestDataNotRead);
        pReadBegin = pBuffer;
        pReadEnd   = pBuffer + nRestDataNotRead;
    }

    tbyte *p         = const_cast<tbyte *>(pReadEnd);
    size_t nAddbytes = read(p, nSize - nRestDataNotRead);
    if (StreamStatus::resultIsOK(nAddbytes))
        pReadEnd += nAddbytes;

    setReadBufferImp(buf, pBuffer, nSize, pReadBegin, pReadEnd);

    return nAddbytes;
}

void ReadStream::releaseReadBuffer(ReadStreamBuffer &buf) {
    tbyte *tp = const_cast<tbyte *>(buf.getBuffer());
    delete[] tp;
}

void ReadStream::allocReadBuffer(ReadStreamBuffer &buf, size_t nSuggestSize) {
    tbyte *tp = new tbyte[ nSuggestSize ];
    //	setReadBufferImp(buf, tp, nSuggestSize, tp, tp + nSuggestSize);
    setReadBufferImp(buf, tp, nSuggestSize, tp, tp);
}

//-------------------------------------------------------------------------------------

off_t WriteStream::seekp(off_t nOffset, seek_dir nSeekFrom) { return nOutOfRange; }

off_t WriteStream::tellp(void) { return nOutOfRange; }

size_t WriteStream::loopWrite(const void *lpBuf, size_t nSize, size_t nHasWrite) {
    size_t       nRest = nSize - nHasWrite;
    const tbyte *tpBuf = static_cast<const tbyte *>(lpBuf) + nHasWrite;
    while (nRest > 0) {
        size_t nWriteNum = write(tpBuf, nRest);
        if (!StreamStatus::resultIsOK(nWriteNum))
            break;
        nRest -= nWriteNum;
        tpBuf += nWriteNum;
    }
    return nSize - nRest;
}

void WriteStream::allocWriteBuffer(WriteStreamBuffer &buf, size_t nSuggestSize) {
    tbyte *tp = new tbyte[ nSuggestSize ];
    setWriteBufferImp(buf, tp, tp + nSuggestSize, tp);
}

void WriteStream::releaseWriteBuffer(WriteStreamBuffer &buf) {
    tbyte *pBuffer    = NULL;
    tbyte *pBufferEnd = NULL;
    tbyte *pWritePos  = NULL;

    getWriteBufferImp(buf, pBuffer, pBufferEnd, pWritePos);

    size_t tLength = static_cast<size_t>(pWritePos - pBuffer);
    if (tLength)
        writeBlockException(pBuffer, tLength);
    delete[] pBuffer;
}

size_t WriteStream::flushWriteBuffer(WriteStreamBuffer &buf, bool /*boJustFlush*/) {
    tbyte *pBuffer    = NULL;
    tbyte *pBufferEnd = NULL;
    tbyte *pWritePos  = NULL;

    getWriteBufferImp(buf, pBuffer, pBufferEnd, pWritePos);
    size_t tLength = static_cast<size_t>(pWritePos - pBuffer);
    setWriteBufferImp(buf, pBuffer, pBufferEnd, pBuffer);
    if (tLength)
        writeBlockException(pBuffer, tLength);
    //	pWritePos = pBuffer;
    return tLength;
}

size_t WriteStream::write(const TWriteDataBlocks *pDatas, size_t n, size_t nOffset) {
    const TWriteDataBlocks *p          = pDatas;
    size_t                  nTotalSize = 0, i = 0, nRet = 0;
    if (nOffset > 0) {
        for (; i < n; ++i, ++p)
            if (nOffset < p->size)
                break;
            else
                nOffset -= p->size;
        if (nOffset > 0 && i < n) {
            nRet = this->write(static_cast<const char *>(p->data) + nOffset, p->size - nOffset);
            if (!StreamBase::resultIsOK(nRet))
                return nRet;
            else if (nRet < p->size - nOffset)
                return nRet;
            nTotalSize = nRet;
            ++i;
            ++p;
        }
    }
    for (; i < n; ++i, ++p) {
        nRet = this->write(p->data, p->size);
        if (StreamBase::resultIsError(nRet))
            return nRet;
        else {
            nTotalSize += nRet;
            if (nRet < p->size)
                break;
        }
    }
    return nTotalSize;
}

size_t WriteStream::writeBlocks(const TWriteDataBlocks *pDatas, size_t n, size_t nOffset) {
    const TWriteDataBlocks *p          = pDatas;
    size_t                  nTotalSize = 0, i = 0;
    while (i < n) {
        size_t nRet = write(p + i, n - i, nOffset);
        if (StreamBase::resultIsError(nRet))
            return nRet;
        else if (StreamBase::resultIsEndofStream(nRet))
            break;
        nTotalSize += nRet;
        nRet += nOffset;
        for (; i < n; ++i) {
            if (nRet >= (p + i)->size)
                nRet -= (p + i)->size;
            else
                break;
        }
        nOffset = nRet;
    }
    return nTotalSize;
}

void WriteStream::writeBlocksException(const TWriteDataBlocks *pDatas, size_t n, size_t nOffset) {
    const TWriteDataBlocks *p          = pDatas;
    size_t                  nTotalSize = 0;
    for (size_t i = 0; i < n; ++i, ++p)
        nTotalSize += p->size;
    if (nTotalSize <= nOffset)
        return;
    nTotalSize -= nOffset;
    if (writeBlocks(pDatas, n, nOffset) != nTotalSize)
        throw StreamException(FmtString("write data (%)byte(s) error!").arg(nTotalSize).str());
}

//////////////////////////////////////////////////////////////////////////

size_t StreamBase::copyStreamToStream(ReadStream *pSrc, WriteStream *pTar, size_t nCopyData, size_t nInputCacheSize) {
    auto                     tmpSize    = std::min(nInputCacheSize, nCopyData);
    size_t                   nCacheSize = std::min(g_nMaxCacheSize, std::max(g_nMinCacheSize, tmpSize));
    std::shared_ptr<tbyte[]> auBuf(new tbyte[ nCacheSize ]);
    tbyte                   *tp    = auBuf.get();
    size_t                   nSave = nCopyData;

    while (nCopyData) {
        size_t nReadNum = pSrc->read(tp, std::min(nCacheSize, nCopyData));
        if (StreamStatus::resultIsEndofStream(nReadNum))
            break;
        else if (StreamStatus::resultIsError(nReadNum))
            return nReadNum;
        else if (pTar->writeBlock(tp, nReadNum) != nReadNum)
            break;
        nCopyData -= nReadNum;
    } // while(nCopyData)
    return nSave - nCopyData;
}

size_t StreamBase::copyStreamToStream(ReadStreamBuffer &buf, WriteStream *pTar, size_t nCopyData) {
    size_t nSave = nCopyData;
    while (nCopyData && buf.getNext(nCopyData)) {
        size_t nWrite = pTar->writeBlock(buf.getBegin(), std::min(buf.getRestCount(), nCopyData));
        buf.moveNext(nWrite);
        nCopyData -= nWrite;
    } // while(nCopyData && buf.getNext(nCopyData))
    return nSave - nCopyData;
}

size_t StreamBase::copyStreamToStream(ReadStream *pSrc, WriteStreamBuffer &wbuf, size_t nCopyData) {
    size_t nSave = nCopyData;
    while (nCopyData) {
        size_t nFreeSize = wbuf.getFreeSize();
        if (nFreeSize == 0) {
            wbuf.getNext();
            nFreeSize = wbuf.getFreeSize();
        } // if ( nFreeSize == 0 )

        size_t nReadNum = pSrc->read(wbuf.getBegin(), std::min(nFreeSize, nCopyData));
        if (StreamStatus::resultIsEndofStream(nReadNum))
            break;
        else if (StreamStatus::resultIsError(nReadNum))
            return nReadNum;
        else {
            wbuf.moveNext(nReadNum);
            nCopyData -= nReadNum;
        }
    } // while(nCopyData && buf.getNext(nCopyData))
    return nSave - nCopyData;
}

size_t StreamBase::copyStreamToStreamBuffer(ReadStream *pSrc, WriteStream *pTar, size_t nCopyData, size_t nCachSize) {
    nCachSize = std::min(nCachSize, nCopyData);
    WriteStreamBuffer buf(pTar, nCachSize);
    size_t            nResult = copyStreamToStream(pSrc, buf, nCopyData);
    buf.flush();
    return nResult;
}

//------------------------------------------------------------------------------------------
} // namespace base
} // namespace muduo