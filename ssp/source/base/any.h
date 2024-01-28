//
// Created by 侯鑫 on 2024/1/28.
//

#ifndef SSP_TOOLS_ANY_H
#define SSP_TOOLS_ANY_H

namespace ssp::base {

template <class T>
class Any {
public:
    Any(const T& val)
        : val_(val)
    {
    }

    T Get() const
    {
        return val_;
    }

private:
    T val_;
};

}

#endif //SSP_TOOLS_ANY_H
