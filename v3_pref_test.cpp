#include<iostream>
#include<functional>
#include<memory>
#include<vector>
#include<stack>
#include<chrono>

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

void testnewdelete(int count) {
	auto start = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < count; i++) {
		int* p = new int(i);
		delete p;
	}

	auto end = std::chrono::high_resolution_clock::now();
	std::cout << "new/delete:"
		<< std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
		<< std::endl;
}

void testObjectpool(int count) {
	ObjectPool<int>pool(256);

	auto start = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < count; i++) {
		auto p = pool.acquire(i);
	}

	auto end = std::chrono::high_resolution_clock::now();
	std::cout << "ObjectPool:"
		<< std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
		<< std::endl;
}

int main() {
	std::cout << "测试次数: 1,000,000 次分配+回收" << std::endl;
	const int COUNT = 1000000;
	testnewdelete(COUNT);
	testObjectpool(COUNT);

	return 0;
}