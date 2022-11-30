#pragma once

#include"tools.h"
#include<string>
#include<math.h>
#include<iostream>
using namespace std;

typedef uint8_t cell;

#define WORD_SIZE 64

using namespace std;

namespace filter {
	class BaseFilter
	{
	public:
		virtual void add(uint64_t key) = 0;

		virtual bool query(uint64_t key)const = 0;

		virtual std::string to_string() const = 0;
		virtual ~BaseFilter() {}
	};
	class BloomFilter : public BaseFilter
	{
	private:
		uint8_t* _data;		// container
		size_t _cells;		// length in byte
		size_t _m;			// length in bit
		int _k;				// number of hash functions
		Hasher _hasher;		// hash function
	public:
		BloomFilter();

		~BloomFilter() override {
			delete[] _data;
		}

		BloomFilter(size_t n, double fpp) : _hasher() {
			this->_k = calcK(fpp);
			this->_m = round(n * (double)_k / log(2));
			this->_cells = (_m + 7) >> 3;

			this->_data = new uint8_t[_cells];
			std::fill_n(_data, _cells, 0);
		}

		BloomFilter(size_t m, int k) : _hasher() {
			this->_k = k;
			this->_m = m;
			this->_cells = (m + 7) >> 3;

			this->_data = new uint8_t[_cells];
			std::fill_n(_data, _cells, 0);
		}

		void add(uint64_t key)override {
			uint64_t hash = _hasher(key);
			uint64_t a = (hash >> 32) | (hash << 32);
			uint64_t b = hash;
			for (int i = 0; i < _k; i++) {
				// �����Ӻ����ã�contains���ܸ㷴��
				a += b;
				set_bit(a);
			}
		}

		bool query(uint64_t key)const override {
			uint64_t hash = _hasher(key);
			uint64_t a = (hash >> 32) | (hash << 32);
			uint64_t b = hash;
			for (int i = 0; i < _k; i++) {
				a += b;
				if (!(get_bit(a))) {
					return false;
				}
			}
			return true;
		}

		void set_bit(size_t pos) {
			pos %= _m;
			_data[pos >> 3] |= (uint8_t)1 << (pos & 7);
		}
		bool get_bit(size_t pos) const {
			pos %= _m;
			return ((_data[pos >> 3] >> (pos & 7)) & 1) != 0;
		}

		static inline int calcK(double fpp) {
			return max(1, (int)round(-log(fpp) / log(2)));
		}

		// getter and setter
		size_t m()const { return _m; }
		size_t cells()const { return _cells; }
		int  k()const { return _k; }
		std::string to_string()const override {
			return "type: BloomFilter\nmemory use: " + std::to_string(_cells) + "\tm: " + std::to_string(_m) + "\tk: " + std::to_string(_k);
		}
	};
}

namespace filter{
	class Shifting_m : public BaseFilter
	{
	private:
		uint8_t* _data;				// container
		size_t _m;					// number of bits in bf
		size_t _cells;				// number of cells in bf
		int _k;						// number of hash functions
		Hasher _hasher;				// hash function
		const static int MAX_OFFSET = WORD_SIZE - 7;

	public:
		Shifting_m();
		Shifting_m(size_t m, int k);
		Shifting_m(size_t n, double fpp);
		~Shifting_m() override;
		void add(uint64_t key)override;
		bool query(uint64_t key)const override;
		void set_bit_with_offset(uint64_t pos, uint64_t offset);
		bool get_bit_with_offset(uint64_t pos, uint64_t offset)const;
		std::string to_string()const override {
			return "type: Shifting Bloom Filter Membership\nmemory use: " + std::to_string(_cells) + "\tm: " + std::to_string(_m) + "\tk : " + std::to_string(_k);
		}
	};

	inline Shifting_m::Shifting_m() :_data(nullptr), _m(0), _cells(0), _k(0), _hasher() {	}

	inline Shifting_m::~Shifting_m()
	{
		delete[] _data;
	}

	inline Shifting_m::Shifting_m(size_t m, int k) :_m(m), _cells((m + 7 + MAX_OFFSET) >> 3), _k((k + 1) / 2), _hasher() {
		_data = new uint8_t[_cells];
		fill_n(_data, _cells, 0);
	}

	inline Shifting_m::Shifting_m(size_t n, double fpp) : _hasher()
	{
		_m = round((double)n * log(fpp) / log(0.6204));
		_k = (round(((double)_m / n) * 0.7009) + 1) / 2;
		_cells = (_m + 8 + MAX_OFFSET) >> 3;
		_data = new uint8_t[_cells];
		fill_n(_data, _cells, 0);
	}

	inline void Shifting_m::add(uint64_t key) {
		// add a key
		uint64_t hash = _hasher(key);
		uint64_t a = (hash >> 32) | (hash << 32);
		uint64_t b = hash;

		uint64_t offset = ((a + b * ((uint64_t)_k + 1))) % _m;
		offset = offset % ((uint64_t)MAX_OFFSET - 1) + 1;
		for (int i = 0; i < _k; i++) {
			// �����Ӻ����ã�contains���ܸ㷴��
			a += b;
			set_bit_with_offset(a, offset);
		}
	}

	inline bool Shifting_m::query(uint64_t key)const {
		// test a key
		uint64_t hash = _hasher(key);
		uint64_t a = (hash >> 32) | (hash << 32);
		uint64_t b = hash;

		uint64_t offset = ((a + b * ((uint64_t)_k + 1))) % _m;
		offset = offset % ((uint64_t)MAX_OFFSET - 1) + 1;

		for (int i = 0; i < _k; i++) {
			// �����Ӻ����ã�contains���ܸ㷴��
			a += b;
			if (!get_bit_with_offset(a, offset)) { return false; }
		}

		return true;
	}

	inline void Shifting_m::set_bit_with_offset(uint64_t pos, uint64_t offset)
	{
		// set two in one access
		pos %= _m;
		uint64_t* ptr = (uint64_t*)((_data)+(pos >> 3));// �ҵ�pos�����ֽ�λ��
		int byte_idx = pos % 8; // bit�����ֽ��еĵ� j λ��������64bit���� 64-jλ
		*ptr = *ptr | (1ULL << byte_idx) | (1ULL << (byte_idx + offset));// һ���Դ�������bit
	}

	inline bool Shifting_m::get_bit_with_offset(uint64_t pos, uint64_t offset)const
	{
		// get bits in one access
		pos %= _m;
		uint64_t* ptr = (uint64_t*)((char*)_data + (pos >> 3));// �ҵ�pos�����ֽ�λ��

		int byte_idx = pos % 8; // bit�����ֽ��еĵ� j λ
		uint64_t cmp = (1ULL << byte_idx) | (1ULL << (byte_idx + offset));
		return (*ptr & (cmp)) == cmp;// һ���Դ�������bit
	}

	class SpatialBF : public BaseFilter
	{
	private:
		uint8_t* _data;
		uint64_t _m;
		uint64_t _cells;
		int _k;
		int _areas;
		uint8_t _byte_per_cell;
		void (filter::SpatialBF::* set_area)(uint64_t idx, int area);
		int (filter::SpatialBF::* get_area)(uint64_t idx)const;
		Hasher _hasher;
	public:
		SpatialBF(size_t m, int k, int area)
			:_m(m), _cells(m), _k(k), _areas(area), _hasher() {
			// REMIND: ����������ʵ���н� _m ��Ϊ�� 2���ݣ�����û��������

			// ����area��С�����С��ʹ�ú���ָ�룬����Ҫÿ���ж�
			if (_areas <= 255) {
				_byte_per_cell = 1;
				set_area = &filter::SpatialBF::set_area1;
				get_area = &filter::SpatialBF::get_area1;
			}
			else {
				_byte_per_cell = 2;
				set_area = &filter::SpatialBF::set_area2;
				get_area = &filter::SpatialBF::get_area2;
			}

			_data = new uint8_t[_cells * _byte_per_cell];
			fill_n(_data, _cells * _byte_per_cell, 0);
		}

		~SpatialBF() override {
			delete[] _data;
		}

		void add(uint64_t key)override {
			add(key, 1);
		}

		void add(uint64_t key, int area) {
			uint64_t a = _hasher(key);
			uint64_t b = (a >> 32) | (a << 32);
			for (int i = 0; i < _k; i++)
			{
				a += b;
				(this->*set_area)(a, area);
			}
		}


		// FIXME: return int, not bool
		bool query(uint64_t key)const override {
			return query_area(key) != 0;
		}
		// FIXME: return int, not bool
		int query_area(uint64_t key)const {
			int area = INT_MAX;
			uint64_t a = _hasher(key);
			uint64_t b = (a >> 32) | (a << 32);

			for (int i = 0; area != 0 && i < _k; i++)
			{
				a += b;
				area = min(area, (this->*get_area)(a));
			}
			return area;
		}

		void set_area1(uint64_t idx, int area) {
			// TODO: set area when byte_per_cell is 1
			idx %= _m;
			_data[idx] = std::max(_data[idx], (uint8_t)area);
		}
		void set_area2(uint64_t idx, int area) {
			// TODO: set area when byte_per_cell is 2
			idx %= _m;
			uint16_t* pos = (uint16_t*)(_data + idx * 2);
			*pos = std::max(*pos, (uint16_t)area);
		}

		int get_area1(uint64_t idx)const {
			idx %= _m;
			return _data[idx];
		}
		int get_area2(uint64_t idx)const {
			idx %= _m;
			uint16_t* pos = (uint16_t*)(_data + idx * 2);
			return *pos;
		}

		string to_string()const override {
			return "type: Spatial Bloom Filter\nmemory use: " + std::to_string(_cells)
				+ "\tm: " + std::to_string(_m) + "\tk: " + std::to_string(_k);
		}

	};
}

namespace filter{
	class PartitionBF : public BaseFilter
	{
	public:
		std::string to_string()const override {
			return "type: Partition";
		}
		~PartitionBF()override
		{
			if (_data != NULL) {
				for (size_t i = 0; i < _partitions; i++)
				{
					delete[] _data[i];
				}delete[] _data;
			}
		}

		PartitionBF(size_t m, int k) :_k(k), _m(m), _partitions(k), _hasher() {
			_bits_per_part = (_m + _partitions - 1) / _partitions;
			_cells_per_part = (_bits_per_part + 8 - 1) / 8;
			_data = new uint8_t * [_partitions];
			for (size_t i = 0; i < _partitions; i++)
			{
				_data[i] = new uint8_t[_cells_per_part];
				std::fill_n(_data[i], _cells_per_part, 0);
			}
		}

		void add(uint64_t key)override {
			uint64_t hash = _hasher(key);
			uint64_t a = (hash >> 32) | (hash << 32);
			uint64_t b = hash;
			for (size_t i = 0; i < _k; i++)
			{
				a += b;
				set_bit(a, i);
			}
		}

		bool query(uint64_t key)const override {
			uint64_t hash = _hasher(key);
			uint64_t a = (hash >> 32) | (hash << 32);
			uint64_t b = hash;
			for (size_t i = 0; i < _k; i++)
			{
				a += b;
				if (!get_bit(a, i)) { return false; }
			}
			return true;
		}

		void set_bit(uint64_t pos, int part) {
			pos %= _bits_per_part;
			_data[part][pos >> 3] |= (uint8_t)1 << (pos & 7);
		}

		bool get_bit(uint64_t pos, int part) const {
			pos %= _bits_per_part;
			return ((_data[part][pos >> 3] >> (pos & 7)) & 1) != 0;
		}



	private:
		uint8_t** _data;	// �洢����
		int _k;				// ��ϣ��������
		size_t _m;			// bit��
		int _partitions;	// ������
		int _bits_per_part;	// ÿ���������� bit
		int _cells_per_part;// ÿ���������� cell(uint8_t)
		Hasher _hasher;		// ��ϣ����
	};

	class DynamicBF : public BaseFilter
	{
	public:
		DynamicBF();
		DynamicBF(double fpp, uint32_t base_size = 1e5, double modifier = 1.5, double density_add = 0.1)
			:_fpp(fpp), _mod(modifier), _density_add(density_add), _last_m(base_size) {
			add_filter();
		}

		~DynamicBF()override {
			for (auto filter : _filters) {
				delete filter;
			}
		}

		void add(uint64_t key)override {
			_filters.back()->add(key);

			// ͳ����Ϣ
			_inserts.back()++;

			if ((double)_inserts.back() / _total_m > _density_add) {
				add_filter();
			}
		}

		bool query(uint64_t key)const override {
			for (auto it = _filters.rbegin(); it != _filters.rend(); it++) {
				if ((*it)->query(key)) {
					return true;
				}
			}
			return false;
		}

		void add_filter() {
			//cout << "addf" << _total_m << _fpp << endl;
			_filters.push_back(new BloomFilter(_last_m, _fpp));
			_total_m += _last_m;
			_last_m = _total_m;
			_last_m *= _mod;

			_inserts.push_back(0);
		}

		std::string to_string()const override {
			return "Dynamic Bloom Filter";
		}
	private:
		vector<BaseFilter*> _filters;
		vector<uint32_t> _inserts;// һ����ӹ����ٴ�
		uint32_t _last_m = 1000;
		uint32_t _total_m = 0;
		double _fpp;			// fpp ��ֵ
		double _mod;			// ÿ����� filter ʱ�Ĵ�С�ǵ�ǰ�ܴ�С�Ķ��ٱ�
		double _density_add;		// ��ʱ��ӹ�����
	};

	class PartitionShiftingBf : public BaseFilter
	{
	public:
		PartitionShiftingBf();
		~PartitionShiftingBf()override {
			for (int i = 0; i < _filters.size(); i++)
			{
				delete _filters[i];
			}
		}
		PartitionShiftingBf(size_t m, int k)
			: _k((k+1)/2),_m(m), _partitions((k+1)/2), _hasher() {
			_bits_per_part = (_m + _k - 1) / _k;
			for (size_t i = 0; i < _k; i++)
			{
				_filters.push_back(new Shifting_m(_bits_per_part, 1));
			}
		}

		void add(uint64_t key)override {
			for (size_t i = 0; i < _k; i++)
			{
				_filters[i]->add(key);
			}
		}
		bool query(uint64_t key)const override {
			for (size_t i = 0; i < _k; i++)
			{
				if (!_filters[i]->query(key)) { return false; }
			}
			return true;
		}

		string to_string()const override {
			return "type: Partition Shifting Membership\nk: "+std::to_string(_k);
		}

	private:
		vector<Shifting_m*> _filters;		// �洢����
		int _k;				// ��ϣ��������
		size_t _m;			// ��bit��
		int _partitions;	// ������
		int _bits_per_part;	// ÿ���������� bit
		Hasher _hasher;		// ��ϣ����
	};

	class IDFilter :public BaseFilter{
	private:
		cell* _data;
		int _m;				// number of bits in bf
		int _k;				// number of hash functions
		int _setNum;		// number of sets	
		int _codeLen;		// number of bits in a code
		Hasher _hasher;
	public:
		~IDFilter()override { delete[] _data; }
		IDFilter(int m, int k, int s)
		:_m(m), _k(k),_setNum(s)
		{
			_codeLen = nearestLarger2Power(_setNum);
			_data = new cell[_m];
			fill_n(_data, _m, 0);
		}
		void add(uint64_t key)override {
			add_area(key, 1);
		}

		void add_area(uint64_t key, int area) {
			// add a key
			uint64_t hash = _hasher(key);
			uint64_t a = (hash >> 32) | (hash << 32);
			uint64_t b = hash;
			for (size_t i = 0; i < _k; i++)
			{
				a += b;
				set_area(a, area);
			}
		}
		void set_area(uint64_t pos, int area) {
			pos %= _m;
			uint32_t* ptr = (uint32_t*)(_data + (pos >> 3));
			*ptr |= area << (pos & 7);
		}
		uint32_t get_area(uint64_t pos)const {
			pos %= _m;
			uint32_t* ptr = (uint32_t*)(_data + (pos >> 3));
			return *ptr >> (pos & 7);
		}
		
		bool query(uint64_t key)const override {
			// TODO
			return query_area(key) != 0;
		}

		int query_area(uint64_t key)const {
			// query a key
			uint64_t hash = _hasher(key);
			uint64_t a = (hash >> 32) | (hash << 32);
			uint64_t b = hash;
			uint32_t area = (1 << _codeLen) - 1;
			for (size_t i = 0; area!=0 && i < _k; i++)
			{
				a += b;
				area &= get_area(a);
			}
			return area;
		}

		string to_string()const override {
			return "type: BloomFilter\nm: " + std::to_string(_m) + "k: " + std::to_string(_k)+"\n";
		}
	};
}