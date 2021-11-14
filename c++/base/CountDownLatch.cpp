#include "CountDownLatch.h"

namespace muduo {
namespace base {

CountDownLatch::CountDownLatch(int count)
    : m_nCount(count) {
}

void CountDownLatch::wait() {
    std::lock_guard<std::mutex> guard(m_mutex);
    while (m_nCount > 0) {
        
    }
}

int CountDownLatch::getCount() {
}

} // namespace base
} // namespace muduo
