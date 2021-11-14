// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#pragma once
#include "base/Mutex.h"
#include "base/nonecopyable.h"
#include <pthread.h>
namespace muduo {
namespace base {

class CountDownLatch : nonecopyable {
public:
    explicit CountDownLatch(int count);

    void wait();

    void countDown();

    int getCount() const;

private:
    mutable std::mutex m_mutex;
    int                m_nCount;
};
} // namespace base
} // namespace muduo
