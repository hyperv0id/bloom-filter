#pragma once
#include"bf.h"

namespace partition {
	class PartitionBF
	{
	public:
		PartitionBF(int k, const size_t m):_k(k), _m(m), _hasher() {
			_partitions = _k;
			_bits_per_part = (m + _k - 1) / k;
			_cells_per_part = (_bits_per_part + 7) >> 3;
			_data = new uint8_t * [_partitions];
			for (int i = 0; i < _partitions; i++)
			{
				_data[i] = new uint8_t[_cells_per_part];
				std::fill_n(_data[i], _cells_per_part, 0);
			}
		}

		~PartitionBF() {
			if (_data != nullptr) {
				for (int i = 0; i < _k; i++)
				{
					delete[] _data[i];
				}delete[] _data;
			}
		}
		
		void add(uint64_t key) {
			uint64_t hash = _hasher(key);
			uint64_t a = (hash >> 32) | (hash << 32);
			uint64_t b = hash;
			for (int i = 0; i < _k; i++)
			{
				a += b;
				set_bit(a, i);
			}
		}

		bool query(uint64_t key) {
			uint64_t hash = _hasher(key);
			uint64_t a = (hash >> 32) | (hash << 32);
			uint64_t b = hash;
			for (int i = 0; i < _k; i++)
			{
				a += b;
				if (!get_bit(a, i)) { return false; }
			}
			return true;
		}

		void set_bit(uint64_t idx, int part) {
			idx %= _bits_per_part;
			_data[part][idx >> 3] |= (1 << (idx & 7));
		}

		bool get_bit(uint64_t idx, int part) {
			idx %= _bits_per_part;
			return (_data[part][idx >> 3] >> (1 << (idx & 7)) &1);
		}

	private:
		uint8_t** _data;	// 存储容器
		int _k;				// 哈希函数个数
		size_t _m;			// bit数
		int _partitions;	// 分区数
		int _bits_per_part;	// 每个分区几个 bit
		int _cells_per_part;// 每个分区几个 cell(uint8_t)
		Hasher _hasher;		// 哈希函数
	};
}