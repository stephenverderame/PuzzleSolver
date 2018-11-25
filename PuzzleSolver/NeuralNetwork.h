#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <vector>
#include "Math.h"
#include <functional>
#include <time.h>
#include "Ringbuffer.h"
namespace ML {
	class MathUndefinedException : public std::exception {
	public:
		const char * what() const noexcept override {
			return "Mathmatical operation is undefined";
		}
	};
	class Matrix {
	private:
		int rows, columns, size_;
		std::vector<double> data;
	public:
		Matrix() : rows(0), columns(0), size_(0) {};
		Matrix(int r, int c);
		Matrix(const Matrix & other);
		Matrix& operator=(const Matrix & other);
		Matrix& operator=(const std::initializer_list<double> & list);
		void set(int row, int column, double x) throw(std::out_of_range);
		void set(int i, double x) throw(std::out_of_range);
		double get(int row, int column) const throw(std::out_of_range);
		double get(int i) const throw(std::out_of_range);
		int getRows() const noexcept { return rows; }
		int getColumns() const noexcept { return columns; }
		void setDimensions(int r, int c) noexcept {
			data.resize(r * c, 0);
			rows = r;
			columns = c;
			size_ = r * c;
		}
		int size() const noexcept { return size_; }

		Matrix operator*(const Matrix & other) const throw(ML::MathUndefinedException);
		Matrix operator-(const Matrix & other) const throw(ML::MathUndefinedException);
		Matrix operator+(const Matrix & other) const throw(ML::MathUndefinedException);
		Matrix transpose() const;
		Matrix& operator-=(const Matrix & other) throw(ML::MathUndefinedException);

		//* Element-wise multiplication. Corresponding elements are multiplied to return the output. (Hadamard Product)
		Matrix elementMultiply(const Matrix other) const throw(ML::MathUndefinedException);

		/**
		 * @param f, applies this callable object to all elements within the matrix. Returns result
		*/
		Matrix apply(std::function<double(double)> f) const;

		/**
		 * @param f, applies this callable object to all elements within the matrix
		*/
		void function(std::function<double(double)> f);

		void print(FILE * out = stdout) const;

		/**
		 * Sets up all elements to be Random values between 0 and 1
		 * @see Random::getNormal(int)
		*/
		void randomize();
	};
	Matrix operator*(const double c, const Matrix & m);
	Matrix operator*(const Matrix & m, const double c);

	//* @return, sum of all elements of m
	double summation(const Matrix& m);

	Matrix sigDerivative(const Matrix& m);

	class Random {
	private:
		static int count;
	public:
		/**
		 * Gets random within the range 0 - 1
		*/
		static double getNormal();
		static double getNormal(int limit);

		static int getInt(int min, int max);
		static int getInt();
	};
	class NeuralNetwork {
	private:
		std::vector<Matrix> biases;
		std::vector<Matrix> weights;
		Ring<Matrix> results;
	public:
		/**
		 * Initializes neural network. Each element in list is the amount of nodes in that stage
		 * @param list, each element in list represents the size of the column vectors of nodes
		*/
		NeuralNetwork(std::initializer_list<size_t> list);

		/**
		 * @param input must have the same size as list[0] (constructor initializer list). Is expected to be a column vector
		 * @return output column vector size of list[n]
		*/
		Matrix calculate(const Matrix & input);

		/**
		 * Fills out weights and biases from saved binary data
		*/
		void populate(const char * filename);

		/**
		 * Fills out weights and biases from randomly generated data
		 * @see Random::getNormal(int)
		*/
		void populate();

		//* Backpropogation
		void learn(const Matrix & calc, const Matrix & real);

		//* Trains network with test letters in Letters folder
		void train();

	};

	const static auto sigmoid = [](double x) -> double {return 1 / (exp(-x) + 1); };
	const static auto sigmoidDerivitive = [](double x) -> double { return exp(-x) / pow(1 + exp(-x), 2); };
}