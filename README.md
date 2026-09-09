# 高性能内存池 (Memory Pool) 🔥

## 🌟 地表最强！从零手写的性能猛兽，让 new/delete 原地退役！ (๑•̀ㅂ•́)و✧

一个从零实现的高性能对象池，用于替换频繁的 `new`/`delete` 操作。  
目标：**降低内存分配延迟，提升程序吞吐量**。

## 📊 性能对决：Release 模式

测试环境：1,000,000 次分配与回收，MSVC 编译器，`-O2` 优化

| 方法 | 耗时 (微秒) | 相对性能 |
| :--- | :--- | :--- |
| `new` / `delete` | 17,471 | 基准 (1x) |
| **ObjectPool (v3)** | **6,221** | **快 2.8 倍** |

**结论**：在 Release 优化下，内存池通过预分配和内存复用，将分配延迟降低了近 **65%**。这不仅验证了设计目标，也证明了低延迟系统的基础组件完全可以从零构建。

## 📈 性能演进：从 Debug 到 Release

这个项目最有意思的部分，是 Debug 和 Release 模式下截然不同的性能表现。

| 构建模式 | `new`/`delete` (微秒) | **ObjectPool (微秒)** | 分析 |
| :--- | :--- | :--- | :--- |
| **Debug** | 130,289 | 538,866 | 池化版本因 `std::function` 类型擦除开销而显慢，暴露了高层抽象的代价 |
| **Release (-O2)** | **17,471** | **6,221** | 编译器优化后，池化版本反超，**性能提升 2.8 倍**，验证了内存复用的设计 |
<img width="1723" height="921" alt="image" src="https://github.com/user-attachments/assets/c8963024-4981-46bd-aa89-db3fe58fdd8d" /><img width="1722" height="921" alt="image" src="https://github.com/user-attachments/assets/c0f9f399-f5c1-4de4-98b8-4b5d3cc66ff0" />




**关键收获**：
> **性能分析必须基于 Release 构建。** Debug 数据用于调试逻辑，Release 数据才代表真实性能。一个看似“慢”的设计，在编译器优化下可能完成逆袭。

## ✨ 版本迭代

- [x] **v1**：核心内存池（预分配 + 空闲链表）
- [x] **v2**：接入 `std::unique_ptr`，实现 RAII 自动管理
- [x] **v3**：添加性能测试，用数据证明比 `new`/`delete` 更快
- [ ] **v4**：支持完美转发构造参数、异常安全保证

## 🚀 快速体验 (v2)

```cpp
#include "v2_smart_ptr.cpp"

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
