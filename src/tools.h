#pragma once
#include <random>


// https://github.com/lemire/fastrange
uint32_t rand32() {
	return (rand() ^ (rand() << 15) ^ (rand() << 30));
}
uint64_t rand64() {
	return (((uint64_t)rand32()) << 32) | rand32();
}

class Hasher
{
	uint64_t seed;
public:
	Hasher() {
		std::random_device random;
		seed = random();
		seed <<= 32;
		seed |= random();
	}

	inline static uint64_t murmur64(uint64_t h) {
		h ^= h >> 33;
		h *= UINT64_C(0xff51afd7ed558ccd);
		h ^= h >> 33;
		h *= UINT64_C(0xc4ceb9fe1a85ec53);
		h ^= h >> 33;
		return h;
	}

	inline uint64_t operator()(uint64_t key) const {
		return murmur64(key + seed);
	}
	inline uint64_t operator()(uint64_t key, uint64_t mod) const {
		return murmur64(key + seed) % mod;
	}
	~Hasher() {}

private:

};

//! 2的几次方刚好大于val?
int nearestLarger2Power(int val) {
	int ans = 1;
	while ((1 << ans) <= val)
		ans++;
	return ans;
}

//! 组合数
int combinationNum(int n, int _m) {
	unsigned long ans = 1;
	for (int i = n; i > n - _m; --i)
		ans *= i;
	for (int i = 2; i <= _m; ++i)
		ans /= i;
	return ans;
}

uint64_t getBit(uint32_t index) { return 1ULL << (index & 63); }

class BitSet
{
public:
	BitSet() :_data(nullptr), _siz(0), _cells(0) {}
	BitSet(size_t siz) {
		this->_siz = siz;
		this->_cells = _siz >> 3;
		this->_data = new uint8_t[_cells];
		std::fill_n(_data, _cells, 0);
	}
	~BitSet() { 
		delete[] _data;
	}

	void set_bit(size_t pos) {
		pos %= _siz;
		_data[pos >> 3] |= (uint8_t)1 << (pos & 7);
	}
	bool get_bit(size_t pos) const{
		pos %= _siz;
		return (_data[pos >> 3] >> (pos & 7)) & 1;
	}
	
	uint8_t* begin()const{
		return _data;
	}

private:
	uint8_t* _data;
	size_t _siz;
	size_t _cells;
};