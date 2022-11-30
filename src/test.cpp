#pragma once
#include"bf.h"
#include"dataset.h"
#include<bitset>
#include<random>
#include <iostream>
#include <fstream>
#include <time.h>
#include <chrono>
#include <unordered_set>

#define N 10'000'000
#define MAX_SIZE 1'000'000'000
#define DEFAULT_FPP 0.003

using namespace std;
using namespace std::chrono;

std::unordered_set<uint32_t> bs;

void test(size_t n, Dataset* dataset, filter::BaseFilter* bf) {
	cout << "-----Test----" << "\n";
	bs.clear();
	double wrong = 0;
	uint32_t r;
	for (size_t i = 0; i < n; i++)
	{
		r = dataset->next();
		bs.insert(r);
		bf->add(r);
	}
	for (size_t i = 0; i < n; i++)
	{
		r = dataset->next();
		if ((bs.count(r)>0) != bf->query(r)) {wrong++;}
		}
	double fpp = wrong / n;

	cout << bf->to_string()<<"\t";
	cout << "n: " << n << "\n";
	cout << "Dataset: " << dataset << "\n";
	cout << "wrong\t" << (int)wrong << "\tfpp: " << fpp << "\n";
	cout << "-------------" << endl;

	std::cout << wrong <<"\t"<< wrong / N << endl;

	fstream f("test/1130.csv", ios::out | ios::app);
	f << bf->to_string() << "\t";
	f << "n: " << n << "\n";
	f << "Dataset: " << dataset->to_string() << "\n";
	f << "wrong\t" << (int)wrong << "\tfpp: " << fpp << "\n";
	f << "-------------" << endl;
	delete bf;
}

void test(size_t m, size_t n, int k, Dataset* dataset, filter::BaseFilter* bf) {
	cout << "-----Test----" << "\n";
	bs.clear();
	double wrong = 0;
	uint32_t r;
	for (size_t i = 0; i < n; i++)
	{
		r = dataset->next();
		bs.insert(r);
		bf->add(r);
	}
	for (size_t i = 0; i < n; i++)
	{
		r = dataset->next();
		if ((bs.count(r)>0) != bf->query(r)) {wrong++;}
}
	double fpp = wrong / n;

	cout << bf->to_string()<<"\t";
	cout << "n: " << n << "\n";
	cout << "Dataset: " << dataset << "\n";
	cout << "wrong\t" << (int)wrong << "\tfpp: " << fpp << "\n";
	cout << "-------------" << endl;

	ofstream file("test/test.csv", ios::out | ios::app);
	file << m << ",";
	file << k << ",";
	file << n << ",";
	file << dataset->to_string() << ",";
	file << (int)wrong << "," << fpp << "\n";
	file.flush(); file.close();
	
	delete bf;
}

void test_from_file(const string& path, filter::BaseFilter* bf) {
	double wrong=0, total=0;
	ifstream file(path, ios::in);
	uint32_t num=0; bool exis=false;
	
	while (file.is_open() && !file.eof()) {
		file >> num >> exis;
		if (bf->query(num) != exis) { wrong++; }
		total++;
		bf->add(num);
}
	std::cout << "dataset: " << path << "\n";
	std::cout << bf->to_string() << "fpp:" << wrong / total;
	std::cout << "\n--------------------------------" << endl;

	file.close();
		
		r = rand32() % N;

	fstream f("test/1130.csv", ios::out | ios::app);
	f << bf->to_string() << "\n";
	f << "Dataset: " << path << "\n";
	f << "wrong\t" << (int)wrong << "\tfpp: " << wrong / total << "\n";
	f << "-------------" << endl;
	f.flush(); f.close();
	delete bf;
}

void testBitset02() {
	bool a, b;
	filter::BloomFilter bf(N, 2);
	uint64_t r;
	bitset<N>* bs = new bitset<N>();

void test_time(int times, filter::BaseFilter* bf) {
	uint64_t* rand_arr = new uint64_t[times];
	for (size_t i = 0; i < times; i++)
	{
		rand_arr[i] = rand64();
}

	auto t1 = high_resolution_clock::now();
	for (size_t i = 0; i < times; i++)
	{
		bf->add(rand_arr[i]);
}
	auto t2 = high_resolution_clock::now();
	auto seconds = duration_cast<std::chrono::seconds>(t2 - t1);
	cout << bf->to_string() << "," << seconds.count() << "\n";
	ofstream f("test/speed.csv", ios::out | ios::app);
	// Ð´ÈëÎÄ¼þ
	f << bf->to_string() << "," << times << "," << seconds.count() << "\n";
	f.flush(); f.close();

	delete bf;
	delete[] rand_arr;
}

int main() {
	int times = 1e8;
	test_time(times, new filter::BloomFilter(times, 10));
	test_time(times, new filter::Shifting_m(times, 10));
	test_time(times, new filter::SpatialBF(times, 10, 255));
	test_time(times, new filter::SpatialBF(times, 10, 256));
	test_time(times, new filter::PartitionBF(times, 10));
	test_time(times, new filter::PartitionShiftingBf(times, 10));
	test_time(times, new filter::IDFilter(times, 10, 255));
	test_time(times, new filter::IDFilter(times, 10, 256));
	return 0;
}

