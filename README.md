# 高性能内存池 (Memory Pool)
## 🌟 地表最强！从零手写的性能猛兽，让 new/delete 原地退役！ (๑•̀ㅂ•́)و✧

一个从零实现的高性能对象池，用于替换频繁的 `new`/`delete` 操作。
目标：**降低内存分配延迟，提升程序吞吐量**。

## ✨ 当前进度：版本二 (v2)

- [x] **核心功能**：实现基于空闲链表 (Free List) 的对象池
- [x] **O(1) 分配与回收**：从栈顶快速获取/归还对象地址
- [x] **预分配机制**：一次性申请大块内存 (`new char[]`)，避免频繁系统调用
- [x] **模板支持**：可管理任意类型的对象 (`ObjectPool<MyClass>`)
- [x] **RAII 自动管理**：接入 `std::unique_ptr`，对象离开作用域时自动归还池中，杜绝内存泄漏

## 🎯 后续迭代目标

- [ ] **版本三**：添加性能测试，用数据证明比 `new`/`delete` 更快
- [ ] **版本四**：支持完美转发构造参数、异常安全保证

## 🚀 快速体验 (v2)

```cpp
#include "v2_smart_ptr.cpp" // 或直接包含你的 ObjectPool 定义

struct Player {
    int hp;
    std::string name;
    Player(int h, std::string n) : hp(h), name(std::move(n)) {}
};

int main() {
    ObjectPool<Player> pool(10);

    // 使用智能指针，无需手动归还
    auto p1 = pool.acquire(100, "Alice");
    std::cout << p1->name << ": " << p1->hp << std::endl;
    // p1 离开作用域时自动析构并归还到池中

    return 0;
}
