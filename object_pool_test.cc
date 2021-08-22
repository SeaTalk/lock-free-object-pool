#include "object_pool.h"
#include <iostream>
using namespace stone::util;

struct A {
    int a;
};

int main() {
    ObjectPool<A> aop(3);
    auto first_node = aop.GetOne();
    A *first = aop.GetObj(first_node);
    first->a = 1;
    auto second_node = aop.GetOne();
    A *second = aop.GetObj(second_node);
    second->a = 2;
    auto third_node = aop.GetOne();
    A *third = aop.GetObj(third_node);
    third->a = 3;

    aop.Reclaim(third_node);
    aop.Reclaim(second_node);
    aop.Reclaim(first_node);

    std::vector<decltype(first_node)> reclaims;
    reclaims.reserve(3);
    for (int i = 0; i < 3; ++i) {
        auto tmp = aop.GetOne();
        reclaims.emplace_back(tmp);
        auto *obj = aop.GetObj(tmp);
        std::cout << obj->a << std::endl;
    }
    std::for_each(reclaims.begin(), reclaims.end(), [&](decltype(first_node) &o) {
        aop.Reclaim(o);
    });
    return 0;
}
