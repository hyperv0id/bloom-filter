#include<random>
#include<fstream>
#include<iostream>
#include<math.h>
#include <string>
#include <unordered_set>
#define DENSITY 1e-8
#define MAX_DATASIZE (size_t)9e8
using namespace std;

default_random_engine r_engine(1112);//随机数发生器
mt19937 generator(r_engine);

normal_distribution<double> gaussian_dist(0, 1); // 均值方差
uniform_real_distribution<double> uni_dist(0, 1); // 最大最小
exponential_distribution<double> exp_dist(1);	// 指数 sigma sx e^sx


uint32_t next_gaussian() {
	return abs(gaussian_dist(generator) / DENSITY);
}
uint32_t next_uniform() {
	return (uni_dist(generator) / DENSITY);
}
uint32_t next_exp() {
	return (exp_dist(generator) / DENSITY);
}


void gen_data(const char* path, uint32_t times, uint32_t (*next_data)()) {
	std::ofstream file(path, std::ios::out);
	uint32_t r;
	while (times--) {
		r = (*next_data)();
		file << r << "\n";
		if (times % (int)(1e6) == 0) {
			file.flush();
		}
	}
	file.flush();
	file.close();
}


void gen_with_exist(const char* path, uint32_t times, uint32_t (*next_data)()) {
	std::ofstream file;
	std::unordered_set<uint32_t> bs(times);
	file.open(path, std::ios::out);
	uint32_t r;
	while (times--) {
		r = (*next_data)();
		file << r << " " << bs.count(r) ? 1 : 0; file << "\n";

		bs.insert(r);
		if (times % (int)(1e6) == 0) {
			file.flush();
		}
	}
	file.flush();
	file.close();
	cout << "n = " << bs.size() << endl;
}

//
//int main() {
//	uint32_t times = 1e7;
//	gen_with_exist("dataset/gaussian_1e8.txt", times, next_gaussian);
//	gen_with_exist("dataset/uniform_1e8.txt", times, next_uniform);
//	gen_with_exist("dataset/exp_1e8.txt", times, next_exp);
//}
