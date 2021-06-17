namespace socket {

using SOCKET = int;

class Socket {
private:
    SOCKET _sock;

public:
    Socket();
    ~Socket();
};

Socket::Socket() {
}

Socket::~Socket() {
}

} // namespace socket
