#include "StreamBase.h"

namespace muduo {
namespace base {

const size_t StreamStatus::nEOS = static_cast<size_t>(0);
const size_t StreamStatus::nERRORS = INFINITE_VALUE;
const off_t StreamStatus::nOutOfRange = static_cast<off_t>(-1);
static const size_t g_nMinCacheSize = 4 * 1024;

StreamStatus::StreamStatus(void)
    : m_nState(STATE_CLOSE) {
}

} // namespace base
} // namespace muduo