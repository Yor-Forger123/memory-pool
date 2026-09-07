#include<iostream>
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
		for(char* chunk : chunks) {
			delete[] chunk;
		}
	}

	T* acquire() {
		if (freeList.empty()) {
			allocateChunk();
		}

		T* obj = freeList.top();
		freeList.pop();
		return obj;
	}

	void release(T* obj) {
		freeList.push(obj);
	}

	size_t available() const {
		return freeList.size();
	}
};

int main() {
	ObjectPool<int> pool(10);

	int* p1 = pool.acquire();
	p1 = new (p1) int(42); 
	std::cout << "p1=" << *p1 << std::endl;
	std::cout << "&p1=" << p1 << std::endl;
	std::cout << "空闲对象数:" << pool.available() << std::endl;


	int* p2 = pool.acquire();
	p2 = new(p2)int(24);
	std::cout << "p2=" << *p2 << std::endl;
	std::cout << "空闲对象数:" << pool.available() << std::endl;

	pool.release(p1);
	std::cout << "释放p1后，空闲对象数:" << pool.available() << std::endl;

	int* p3 = pool.acquire();
	*p3 = 100;
	std::cout << "p3=" << *p3 << std::endl;
	std::cout << "&p3=" << p3 << std::endl;
	std::cout << "空闲对象数:" << pool.available() << std::endl;
}