#include "BinaryStream.h"

#include <exception>
#include <memory>
#include <stdexcept>
#include <stdio.h>
using std::out_of_range;
using std::runtime_error;

using namespace muduo::base;

BinaryStream::BinaryStream(int nAutoGroupUpSize)
    : StreamBase(StreamStatus::STATE_OPEN)
    , m_lpBuffer(NULL)
    , m_nBufLen(0)
    , m_nBufMaxLen(0)
    , m_nCurPos(0)
    , m_nAutoGroupUp(nAutoGroupUpSize) {
    setExtStateBit(BUF_TYPE_AUTODELETE);
}

BinaryStream::BinaryStream(void *lpszBuffer, size_t nSize)
    : StreamBase(StreamStatus::STATE_OPEN)
    , m_lpBuffer(reinterpret_cast<tbyte *>(lpszBuffer))
    , m_nBufLen(nSize)
    , m_nBufMaxLen(nSize)
    , m_nCurPos(0)
    , m_nAutoGroupUp(0) {
    setGoodState();
}

BinaryStream::~BinaryStream(void) {
    clear();
}

void BinaryStream::clear(void) {
    if (m_lpBuffer != NULL && !isAttach())
        delete[] m_lpBuffer;
    setState(StreamStatus::STATE_OPEN);
    setExtStateBit(BUF_TYPE_AUTODELETE);
    m_nBufMaxLen = 0;
    m_lpBuffer   = NULL;
    m_nBufLen    = 0;
    m_nCurPos    = 0;
}

int BinaryStream::resetAutoGroupUpSize(int nNewVal) {
    int nOldVal    = m_nAutoGroupUp;
    m_nAutoGroupUp = nNewVal;
    return nOldVal;
}

inline void BinaryStream::setGoodState(void) {
    clearStateBit(StreamStatus::STATE_ERR);
    if (m_nBufLen == static_cast<size_t>(m_nCurPos))
        setStateBit(StreamStatus::STATE_EOF);
    else
        clearStateBit(StreamStatus::STATE_EOF);
}

void BinaryStream::clearAndReserve(size_t nSize) {
    if (isAttach())
        throw runtime_error("Memory is Attach!");
    if (nSize <= m_nBufMaxLen && nSize > 0 && m_lpBuffer != NULL) {
        m_nBufLen = m_nCurPos = 0;
    } else {
        clear();
        reserve(nSize);
    }
    setGoodState();
}

void BinaryStream::resetSize(size_t nSize, tbyte bFillter) {
    if (nSize > m_nBufMaxLen)
        reserve(nSize);
    if (nSize > m_nBufLen)
        memset(m_lpBuffer + m_nBufLen, bFillter, nSize - m_nBufLen);
    m_nBufLen = nSize;
}

void BinaryStream::reserve(size_t nSize) {
    if (isAttach())
        throw runtime_error("Memory is Attach!");
    if (nSize > m_nBufMaxLen) {
        std::unique_ptr<tbyte[]> _au(new tbyte[ nSize ]);
        if (m_lpBuffer != NULL) {
            memcpy(_au.get(), m_lpBuffer, m_nBufLen);
            delete[] m_lpBuffer;
        }
        m_nBufMaxLen = nSize;
        m_lpBuffer   = _au.release();
        setGoodState();
    }
}

void BinaryStream::resetSizeWithoutFill(size_t nSize) {
    if (nSize > m_nBufMaxLen)
        reserve(nSize);
    m_nBufLen = nSize;
}

off_t BinaryStream::tellg(void) {
    return m_nCurPos;
}

off_t BinaryStream::seekg(off_t nPos, StreamStatus::seek_dir whence) {
    off_t newpos;
    if (whence == seek_cur)
        newpos = static_cast<off_t>(m_nCurPos + nPos);
    else if (whence == seek_end)
        newpos = static_cast<off_t>(m_nBufLen + nPos);
    else
        newpos = nPos;
    if (newpos < 0 || newpos > static_cast<off_t>(m_nBufLen))
        return StreamStatus::nOutOfRange;
    m_nCurPos = newpos;
    setGoodState();
    return m_nCurPos;
}

size_t BinaryStream::read(void *buf, size_t nSize) {
    size_t realsize = m_nBufLen - m_nCurPos;
    if (realsize == 0) {
        setStateBit(StreamStatus::STATE_EOF);
        return StreamStatus::nEOS;
    } else if (realsize < nSize)
        nSize = realsize;
    memcpy(buf, m_lpBuffer + m_nCurPos, nSize);
    m_nCurPos += static_cast<off_t>(nSize);
    // setGoodState();
    return nSize;
}

size_t BinaryStream::write(const void *buf, size_t nSize) {
    if (nSize == 0)
        return 0;
    size_t realsize = m_nBufMaxLen - m_nCurPos;
    if (realsize < nSize) {
        if (!isAttach()) {
            if (m_nAutoGroupUp == -1)
                reserve(m_nCurPos + nSize);
            else if (m_nAutoGroupUp != 0) {
                size_t nNewSize = ((m_nCurPos + nSize + 1) / m_nAutoGroupUp + 1) * m_nAutoGroupUp;
                reserve(nNewSize);
            }
            realsize = m_nBufMaxLen - m_nCurPos;
        }
        if (realsize == 0) {
            setStateBit(StreamStatus::STATE_ERR);
            return StreamStatus::nERRORS;
        }
    }
    if (realsize < nSize)
        nSize = realsize;
    memcpy(m_lpBuffer + m_nCurPos, buf, nSize);
    m_nCurPos += static_cast<off_t>(nSize);
    if (static_cast<off_t>(m_nBufLen) < m_nCurPos)
        m_nBufLen = m_nCurPos;
    setGoodState();
    return nSize;
}

void BinaryStream::attach(void *lpBuffer, size_t nLength, TAutoDeleteFlag boAutoDel) {
    clear();
    m_lpBuffer   = reinterpret_cast<tbyte *>(lpBuffer);
    m_nCurPos    = 0;
    m_nBufMaxLen = m_nBufLen = nLength;
    if (boAutoDel == ADF_NOT_AUTO_DELETE)
        clearExtStateBit(BUF_TYPE_AUTODELETE);
    else
        setExtStateBit(BUF_TYPE_AUTODELETE);
}

void *BinaryStream::detach(void) {
    void *res    = m_lpBuffer;
    m_lpBuffer   = NULL;
    m_nBufMaxLen = m_nBufLen = 0;
    m_nCurPos                = 0;
    clear();
    return res;
}

tbyte &BinaryStream::operator[](size_t index) {
    if (m_lpBuffer == NULL || index >= m_nBufLen)
        throw out_of_range("Out of BinaryStream Bounce");
    return m_lpBuffer[ index ];
}

tbyte BinaryStream::operator[](size_t index) const {
    if (m_lpBuffer == NULL || index >= m_nBufLen)
        throw out_of_range("Out of BinaryStream Bounce");
    return m_lpBuffer[ index ];
}

bool BinaryStream::truncate(off_t nSize) {
    if (static_cast<off_t>(m_nBufLen) > nSize)
        m_nBufLen = static_cast<size_t>(nSize);
    if (m_nCurPos > static_cast<off_t>(nSize))
        m_nCurPos = static_cast<off_t>(nSize);
    setGoodState();
    return true;
}

void BinaryStream::allocReadBuffer(ReadStreamBuffer &buf, size_t nSuggestSize) {
    ReadStream::setReadBufferImp(buf, m_lpBuffer, m_nBufLen, m_lpBuffer + m_nCurPos, m_lpBuffer + m_nBufLen);
    m_nCurPos = m_nBufLen;
    setGoodState();
}

void BinaryStream::releaseReadBuffer(ReadStreamBuffer &) {
}

size_t BinaryStream::fillReadBuffer(ReadStreamBuffer &, size_t) {
    return 0;
}

void BinaryStream::allocWriteBuffer(WriteStreamBuffer &buf, size_t nSuggestSize) {
    reserve(m_nCurPos + nSuggestSize);
    WriteStream::setWriteBufferImp(buf, m_lpBuffer + m_nCurPos, m_lpBuffer + m_nBufMaxLen, m_lpBuffer + m_nCurPos);
}

void BinaryStream::releaseWriteBuffer(WriteStreamBuffer &buf) {
    m_nCurPos += buf.getDataInBuffer();
    if (static_cast<size_t>(m_nCurPos) > m_nBufLen)
        m_nBufLen = m_nCurPos;
}

size_t BinaryStream::flushWriteBuffer(WriteStreamBuffer &buf, bool boJustFlush) {
    size_t nFlushSize = buf.getDataInBuffer();
    m_nCurPos += nFlushSize;
    if (static_cast<size_t>(m_nCurPos) > m_nBufLen)
        m_nBufLen = m_nCurPos;
    if (!boJustFlush && buf.getCacheSize() > m_nBufMaxLen - m_nCurPos)
        reserve(m_nBufMaxLen + (m_nBufMaxLen >> 1) + 10);
    WriteStream::setWriteBufferImp(buf, m_lpBuffer + m_nCurPos, m_lpBuffer + m_nBufMaxLen, m_lpBuffer + m_nCurPos);

    return nFlushSize;
}
