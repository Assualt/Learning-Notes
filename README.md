# Learning-Notes
Learning 是学习笔记,主要包含`c++` `python`,`java`等一些库的学习

## c++
其中主要根据`muduo` 实现了自己网络库的封装，并给出了具有一定功能的
`httpserver`，`httpclient`，`mailserver`，`rpcserver（待完善）`。
```shell
>>> tree -L  2 -I "build" c++
➜  c++ git:(master) ✗ tree  -I "third_party"     

.
├── base
│   ├── argparse
│   │   ├── cmdline.h
│   │   └── unitargparse.cpp
│   ├── Backtrace.cpp
│   ├── Backtrace.h
│   ├── checksum
│   │   ├── checksum.cpp
│   │   └── checksum.h
│   ├── CMakeLists.txt
│   ├── Condition.cpp
│   ├── Condition.h
│   ├── Configure.cpp
│   ├── Configure.h
│   ├── Conv.cpp
│   ├── Conv.h
│   ├── copyable.h
│   ├── CountDownLatch.cpp
│   ├── CountDownLatch.h
│   ├── crypto
│   │   ├── base64.cpp
│   │   └── base64.h
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
│   │   ├── CMakeLists.txt
│   │   ├── fsm_configuration.h
│   │   ├── fsm_context.h
│   │   ├── fsm_logger.h
│   │   ├── fsm_stateconfigure.h
│   │   ├── fsm_statemachine.h
│   │   └── tests
│   │       ├── CMakeLists.txt
│   │       └── test_fsm_state.cpp
│   ├── json
│   │   ├── json.cpp
│   │   └── json.h
│   ├── Likely.h
│   ├── Logging.cpp
│   ├── Logging.h
│   ├── LogHandle.h
│   ├── Mutex.cpp
│   ├── Mutex.h
│   ├── nonecopyable.h
│   ├── Object.h
│   ├── ObjPool.h
│   ├── ProcInfo.cpp
│   ├── ProcInfo.h
│   ├── Range.h
│   ├── stream
│   │   ├── BinaryStream.cpp
│   │   ├── BinaryStream.h
│   │   ├── CompressStream.h
│   │   ├── StreamBase.cpp
│   │   ├── StreamBase.h
│   │   └── ZlibStream.h
│   ├── System.h
│   ├── Thread.cpp
│   ├── Thread.h
│   ├── ThreadPool.cpp
│   ├── ThreadPool.h
│   ├── Timer.cpp
│   ├── Timer.h
│   ├── Timestamp.cpp
│   ├── Timestamp.h
│   ├── Unicode.cpp
│   └── Unicode.h
├── cmake
│   ├── bin
│   ├── cxx_compile_flags.cmake
│   └── project_init.cmake
├── CMakeLists.txt
├── config
├── db
│   ├── CMakeLists.txt
│   ├── DbHelper.h
│   ├── leveldb_client.cpp
│   ├── leveldb_client.h
│   ├── mysql_client.cpp
│   ├── mysql_client.h
│   ├── sqlite_client.cpp
│   └── sqlite_client.h
├── format_code
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
│   │   ├── client
│   │   │   ├── CMakeLists.txt
│   │   │   ├── HttpClient.cpp
│   │   │   ├── HttpClient.h
│   │   │   ├── HttpConnection.cpp
│   │   │   ├── HttpConnection.h
│   │   │   ├── HttpUrl.h
│   │   │   └── main_client.cpp
│   │   ├── CMakeLists.txt
│   │   ├── cpp12306
│   │   │   ├── CMakeLists.txt
│   │   │   └── unitTicketQuery.cpp
│   │   ├── HttpConfig.cpp
│   │   ├── HttpConfig.h
│   │   ├── HttpContext.cpp
│   │   ├── HttpContext.h
│   │   ├── HttpLog.cpp
│   │   ├── HttpLog.h
│   │   ├── HttpParttern.cpp
│   │   ├── HttpParttern.h
│   │   ├── HttpRequest.cpp
│   │   ├── HttpRequest.h
│   │   ├── HttpResponse.cpp
│   │   ├── HttpResponse.h
│   │   ├── HttpUtils.cpp
│   │   ├── HttpUtils.h
│   │   ├── resource
│   │   │   ├── cert.csr
│   │   │   ├── cert.pem
│   │   │   ├── dirHtml.tmpl
│   │   │   └── privkey.pem
│   │   ├── server
│   │   │   ├── CMakeLists.txt
│   │   │   ├── controller
│   │   │   │   ├── 40xController.cpp
│   │   │   │   ├── 40xController.h
│   │   │   │   ├── Controller_if.h
│   │   │   │   ├── DefaultController.cpp
│   │   │   │   ├── DefaultController.h
│   │   │   │   ├── lib
│   │   │   │   │   ├── CMakeLists.txt
│   │   │   │   │   ├── mycontroller1.cpp
│   │   │   │   │   ├── mycontroller1.h
│   │   │   │   │   ├── mycontroller.cpp
│   │   │   │   │   ├── mycontroller.h
│   │   │   │   │   ├── upload_ctl.cpp
│   │   │   │   │   └── upload_ctl.h
│   │   │   │   ├── UserController.cpp
│   │   │   │   ├── UserController.h
│   │   │   │   ├── WebSockController.cpp
│   │   │   │   └── WebSockController.h
│   │   │   ├── CtlScanner.cpp
│   │   │   ├── CtlScanner.h
│   │   │   ├── HttpServer.cpp
│   │   │   ├── HttpServer.h
│   │   │   ├── main_server.cpp
│   │   │   ├── WsServer.cpp
│   │   │   └── WsServer.h
│   │   └── tldextract
│   │       ├── CMakeLists.txt
│   │       ├── tldextract.cpp
│   │       ├── tldextract.h
│   │       └── unitTldExtract.cpp
│   ├── InetAddress.cpp
│   ├── InetAddress.h
│   ├── mail
│   │   ├── CMakeLists.txt
│   │   ├── conf
│   │   │   └── programs.cf
│   │   ├── lib
│   │   │   ├── libmuduo_base.a
│   │   │   └── libmuduo_net.a
│   │   ├── mail_common.h
│   │   ├── mail_context.h
│   │   ├── mail_env.cpp
│   │   ├── mail_env.h
│   │   ├── mail_process.cpp
│   │   ├── mail_process.h
│   │   ├── mailserver.cpp
│   │   ├── mailserver.h
│   │   ├── mail_utils.h
│   │   ├── Makefile
│   │   ├── tests
│   │   │   ├── em1.py
│   │   │   └── em.py
│   │   └── unitMailServer.cpp
│   ├── poller
│   │   ├── EPollPoller.cpp
│   │   ├── EPollPoller.h
│   │   ├── PollPoller.cpp
│   │   └── PollPoller.h
│   ├── Poller.cpp
│   ├── Poller.h
│   ├── rpc
│   │   ├── client
│   │   ├── CMakeLists.txt
│   │   └── server
│   │       ├── CMakeLists.txt
│   │       ├── main_server.cpp
│   │       ├── RpcRegisterManager.cpp
│   │       ├── RpcRegisterManager.h
│   │       ├── RpcRequest.cpp
│   │       ├── RpcRequest.h
│   │       ├── RpcResponse.cpp
│   │       ├── RpcResponse.h
│   │       ├── RpcServer.cpp
│   │       └── RpcServer.h
│   ├── Socket.cpp
│   ├── Socket.h
│   ├── SocketsOp.cpp
│   ├── SocketsOp.h
│   ├── TcpClient.cpp
│   ├── TcpClient.h
│   ├── TcpConnection.cpp
│   ├── TcpConnection.h
│   ├── TcpServer.cpp
│   ├── TcpServer.h
│   ├── Uri.cpp
│   └── Uri.h
├── test
│   ├── base
│   │   ├── CMakeLists.txt
│   │   ├── test_checksum.cpp
│   │   ├── test_crypto.cpp
│   │   ├── test_file.cpp
│   │   ├── test_logging.cpp
│   │   ├── test_range.cpp
│   │   ├── test_stream.cpp
│   │   ├── test_thread.cpp
│   │   ├── test_threadpool.cpp
│   │   ├── test_timer.cpp
│   │   └── test_timestamp.cpp
│   ├── CMakeLists.txt
│   ├── db
│   │   ├── CMakeLists.txt
│   │   ├── test_leveldb.cpp
│   │   ├── test_mysql.cpp
│   │   └── test_sqlite.cpp
│   ├── main.cpp
│   ├── mock
│   ├── net
│   │   ├── CMakeLists.txt
│   │   ├── test_poller.cpp
│   │   ├── test_tcpclient.cpp
│   │   ├── test_uri.cpp
│   │   └── test_zlib.cpp
│   └── unitmain.cpp
└── threadpool
    ├── threadpool.cpp
    └── threadpool.h

34 directories, 227 files
