#pragma once
#include <vector>
template<typename T>
class delete_queue {
	friend T;
public:
	void flush() {
		for (auto& obj : delq)
			obj.clear();
		delq.clear();
	}
private:
	void exec(T& obj) {
		delq.emplace_back(std::move(obj));
	}
private:
	std::vector<T> delq;
};
