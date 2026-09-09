#include<iostream>
#include<functional>
#include<memory>
#include<vector>
#include<stack>

template<typename T>
class ObjectPool {
private:
	std::vector<char*> chunks;
	std::stack<T*> freeList;
	size_t chunkSize;

	void allocateChunk() {
		char* block = new char[sizeof(T) * chunkSize];
		chunks.push_back(block);

		for (size_t i = 0; i < chunkSize; i++) {
			T* obj = reinterpret_cast<T*>(block + i * sizeof(T));
			freeList.push(obj);
		}
	}

public:
	ObjectPool(size_t size = 64) : chunkSize(size) {
		allocateChunk();
	}

	~ObjectPool() {
		for (char* chunk : chunks) {
			delete[] chunk;
		}
	}

	template<typename...Args>
	std::unique_ptr<T, std::function<void(T*)>> acquire(Args&&...args) {
		if (freeList.empty()) {
			allocateChunk();
		}

		T* ptr = freeList.top();
		freeList.pop();

		new(ptr)T(std::forward<Args>(args)...);

		return std::unique_ptr<T, std::function<void(T*)>>(
			ptr, [this](T* p) {
				if (p) {
					p->~T();
					freeList.push(p);
				}
			}
		);
	}

	size_t available() const {
		return freeList.size();
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

	auto p1 = pool.acquire(100, "Alice");
	std::cout << "玩家: " << p1->name << ", HP: " << p1->hp << std::endl;
	std::cout << "空闲对象数: " << pool.available() << std::endl;

	{
		auto p2 = pool.acquire(200, "Bob");
		std::cout << "玩家: " << p2->name << ", HP: " << p2->hp << std::endl;
		std::cout << "空闲对象数: " << pool.available() << std::endl;
	}
	std::cout << "p2 离开作用域后，空闲对象数: " << pool.available() << std::endl;

	return 0;
}