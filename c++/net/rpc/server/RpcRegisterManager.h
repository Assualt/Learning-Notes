#include <map>
namespace rpc {
class RcpRegisterManager final {
public:
    static RcpRegisterManager &Instance() {
        static RcpRegisterManager rpcManager;

        return rpcManager;
    }

private:
    RcpRegisterManager() = default;
};

} // namespace rpc