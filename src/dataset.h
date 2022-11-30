#pragma once
#include<random>

std::random_device rd;  //Will be used to obtain a seed for the random number engine
std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()

#define DENSITY 1e-7

class Dataset
{
public:
	virtual int next()const = 0;
	virtual int operator()() { return next(); }
	virtual string to_string() = 0;
};

class UniformDataset:public Dataset
{
private:
	std::uniform_real_distribution<>* distrib;
public:
	UniformDataset() {
		distrib = new std::uniform_real_distribution<>(0, 1);
	}
	int next() const override{
		return (*distrib)(gen) / DENSITY;
	}
	string to_string()override { return "UniformDataset"; }
};

class GaussianDataset : public Dataset
{
private:
	std::normal_distribution<double>* distrib;
public:
	
	GaussianDataset(int sigma) {
		distrib = new std::normal_distribution<>(0, sigma);
	}
	
	int next()const override{
		return (int)abs((*distrib)(gen)) / DENSITY;
	}

	string to_string()override { return "GaussianDataset"; }
};

class ExpDataset:public Dataset
{
private:
	std::exponential_distribution<double>* distrib;
public:
	ExpDataset(int gamma) {
		this->distrib = new std::exponential_distribution<>(gamma);
	}
	int next()const override{
		return (int)((*distrib)(gen) / DENSITY);
	}

	string to_string()override { return "ExponentialDataset"; }
}; 