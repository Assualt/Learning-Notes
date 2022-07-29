# Learning-Notes
Learning 是学习笔记,主要包含`c++` `python`,`java`等一些库的学习

## c++
其中主要根据`muduo` 实现了自己网络库的封装，并给出了具有一定功能的
`httpserver`，`httpclient`，`mailserver`，`rpcserver（待完善）`。
```shell
>>> tree -L  2 -I "build" c++

  c++ git:(master) ✗ tree -L  2 -I "build"
.
├── base
│   ├── argparse
│   ├── Backtrace.cpp
│   ├── Backtrace.h
│   ├── checksum
│   ├── CMakeLists.txt
│   ├── Condition.cpp
│   ├── Condition.h
│   ├── Configure.cpp
│   ├── Configure.h
│   ├── copyable.h
│   ├── CountDownLatch.cpp
│   ├── CountDownLatch.h
│   ├── crypto
│   ├── Date.cpp
│   ├── Date.h
│   ├── DirScanner.cpp
│   ├── DirScanner.h
│   ├── Dll.cpp
│   ├── Dll.h
│   ├── Exception.h
│   ├── File.cpp
│   ├── File.h
│   ├── Format.h
│   ├── fsm
│   ├── json
│   ├── Logging.cpp
│   ├── Logging.h
│   ├── LogHandle.h
│   ├── Mutex.cpp
│   ├── Mutex.h
│   ├── nonecopyable.h
│   ├── Object.h
│   ├── ObjPool.h
│   ├── Range.h
│   ├── stream
│   ├── System.h
│   ├── Thread.cpp
│   ├── Thread.h
│   ├── ThreadPool.cpp
│   ├── ThreadPool.h
│   ├── Timestamp.cpp
│   └── Timestamp.h
├── cmake
│   ├── cxx_compile_flags.cmake
│   └── project_init.cmake
├── CMakeLists.txt
├── db
│   ├── CMakeLists.txt
│   ├── DbHelper.h
│   ├── leveldbclient.cpp
│   └── sqliteclient.cpp
├── format_code
├── httpclient
│   ├── base64.o
│   ├── gurl
│   ├── httpclient
│   ├── httpclient.hpp
│   ├── liburlparse
│   ├── logging.o
│   ├── Makefile
│   ├── unitHttpClient.cpp
│   ├── unitHttpClient.o
│   └── url -> ../../../liburlparse/
├── httpserver
│   ├── bin
│   ├── conf
│   ├── html
│   ├── logs
│   ├── Makefile
│   ├── src
│   └── test
├── net
│   ├── Acceptor.cpp
│   ├── Acceptor.h
│   ├── Buffer.cpp
│   ├── Buffer.h
│   ├── Callback.h
│   ├── Channel.cpp
│   ├── Channel.h
│   ├── CMakeLists.txt
│   ├── Connection.h
│   ├── CurrentThread.cpp
│   ├── CurrentThread.h
│   ├── Endian.h
│   ├── EventLoop.cpp
│   ├── EventLoop.h
│   ├── EventLoopThread.cpp
│   ├── EventLoopThread.h
│   ├── EventLoopThreadPool.cpp
│   ├── EventLoopThreadPool.h
│   ├── http
│   ├── InetAddress.cpp
│   ├── InetAddress.h
│   ├── mail
│   ├── poller
│   ├── Poller.cpp
│   ├── Poller.h
│   ├── rpc
│   ├── Socket.cpp
│   ├── Socket.h
│   ├── SocketsOp.cpp
│   ├── SocketsOp.h
│   ├── TcpConnection.cpp
│   ├── TcpConnection.h
│   ├── TcpServer.cpp
│   ├── TcpServer.h
│   └── ZlibStream.h
├── test
│   ├── base
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── mock
│   ├── net
│   └── unitmain.cpp
├── third_party
│   ├── googletest
│   ├── gtest
│   └── url -> /home/xhou/work/git/liburlparse
├── threadpool
│   ├── threadpool.cpp
│   └── threadpool.h
└── tldextract
    ├── Makefile
    ├── mutithread.cpp
    ├── test_gzip.cpp
    ├── tldextract.cpp
    ├── tldextract.h
    └── unitTldExtract.cpp

44 directories, 119 files



```

