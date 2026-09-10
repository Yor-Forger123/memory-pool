#include <iostream>
#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <stack>
#include <chrono>

template <typename T>
class ObjectPool {
private:
    // Chunk: 封装一块连续内存，负责内存的分配与释放
    // 使用 ::operator new / ::operator delete 直接管理原始内存
    struct Chunk {
        char* data;
        size_t size;

        Chunk(size_t sz) : size(sz) {
            // 只分配原始内存，不构造对象
            data = static_cast<char*>(::operator new(sizeof(T) * sz));
        }

        ~Chunk() {
            // 只释放原始内存，不调用对象析构函数
            ::operator delete(data);
        }
    };

    std::vector<Chunk> chunks;   // 管理所有申请的内存块
    std::stack<T*> freeList;     // 空闲对象地址栈，O(1) 分配与回收
    size_t chunkSize;            // 每个内存块容纳的对象数量

    // 分配一个新的内存块，切割成对象槽位并压入空闲栈
    void allocateChunk() {
        chunks.emplace_back(chunkSize);
        char* block = chunks.back().data;

        for (size_t i = 0; i < chunkSize; i++) {
            T* obj = reinterpret_cast<T*>(block + i * sizeof(T));
            freeList.push(obj);
        }
    }

public:
    explicit ObjectPool(size_t size = 64) : chunkSize(size) {
        allocateChunk();
    }

    // 禁止拷贝：对象池是独占资源，拷贝会导致双重释放
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

    // 允许移动：转移所有权，原池变为空
    ObjectPool(ObjectPool&&) = default;
    ObjectPool& operator=(ObjectPool&&) = default;

    // 从池中获取一个对象
    // 使用完美转发支持任意构造参数，placement new 在预分配内存上构造对象
    template <typename... Args>
    std::unique_ptr<T, std::function<void(T*)>> acquire(Args&&... args) {
        if (freeList.empty()) {
            allocateChunk();
        }

        T* ptr = freeList.top();
        freeList.pop();

        try {
            new (ptr) T(std::forward<Args>(args)...);
        }
        catch (...) {
            // 异常安全：构造失败时把地址归还池中，避免内存泄漏
            freeList.push(ptr);
            throw;
        }

        // 返回 unique_ptr，自定义删除器
        // 对象析构时，不 delete 内存，而是显式调用析构函数并归还地址
        return std::unique_ptr<T, std::function<void(T*)>>(
            ptr,
            [this](T* p) {
                if (p) {
                    p->~T();          // 显式调用析构函数，释放对象内部资源
                    freeList.push(p); // 地址归还池中，等待复用
                }
            }
        );
    }

    // 监控接口：观察池的使用情况
    size_t available() const {
        return freeList.size();
    }

    size_t chunkcount() const {
        return chunks.size();
    }
};

struct Player {
    int hp;
    std::string name;

    Player(int h, const std::string& n) : hp(h), name(n) {
        std::cout << "Player 构造: " << name << " (" << hp << " HP)" << std::endl;
    }

    ~Player() {
        std::cout << "Player 析构: " << name << std::endl;
    }
};

int main() {
    ObjectPool<Player> pool(5);
    std::cout << "初始空闲对象: " << pool.available() << std::endl;
    std::cout << "内存块数量: " << pool.chunkcount() << std::endl;
    std::cout << "-----------------------------" << std::endl;

    auto p1 = pool.acquire(100, "Alice");
    std::cout << "空闲对象: " << pool.available() << std::endl;
    std::cout << "内存块数量: " << pool.chunkcount() << std::endl;

    for (int i = 0; i < 10; ++i) {
        auto p = pool.acquire(50 + i, "Player" + std::to_string(i));
        std::cout << "空闲对象: " << pool.available()
            << ", 内存块数量: " << pool.chunkcount() << std::endl;
    }

    return 0;
}