//
// Created by 侯鑫 on 2023/12/26.
//

#include "base/object.h"
#include "source/base/object_pool.h"
#include <gtest/gtest.h>

using namespace ssp::base;
TEST(object, show_it)
{
    Object a;
    std::cout << a.Hashcode() << std::endl;
    std::cout << a.ToString() << std::endl;
}

class TmpObject : public Object {
public:
    [[nodiscard]] const char * GetObjectName() const override { return "TmpObject"; }

    TmpObject()
    {
        std::cout << "run TmpObject" << std::endl;
    }

    ~TmpObject() override
    {
        std::cout << "run ~TmpObject" << std::endl;
    }
};

REG_OBJECT(TmpObject)

TEST(pool, test)
{
    auto obj = ObjectPool::Instance().Query<TmpObject>();
    ASSERT_EQ(obj, nullptr);

    ObjectPool::Instance().PreInit();
    ObjectPool::Instance().PostInit();

    obj = ObjectPool::Instance().Query<TmpObject>();
    ASSERT_NE(obj, nullptr);

    ASSERT_EQ(ObjectPool::Instance().Size(), 1);

    std::cout << obj->ToString() << std::endl;
}