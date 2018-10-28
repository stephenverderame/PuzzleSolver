#include "NeuralNetwork.h"
#include <assert.h>
int ML::Random::count = 0;

ML::Matrix::Matrix(int r, int c) : rows(r), columns(c)
{
	data.resize(r * c, 0);
	this->size_ = r * c;
}

void ML::Matrix::set(int row, int column, double x) throw(std::out_of_range)
{
	if (row < 0 || row >= rows || column < 0 || column >= columns) throw std::out_of_range("Matrix element does not exist");
	data[row * columns + column] = x;
}

void ML::Matrix::set(int i, double x) throw(std::out_of_range)
{
	if (i < 0 || i >= size_) throw std::out_of_range("Matrix element does not exist");
	data[i] = x;
}

double ML::Matrix::get(int row, int column) const throw(std::out_of_range)
{
	if (row < 0 || row >= rows || column < 0 || column >= columns) throw std::out_of_range("Matrix element does not exist");
	return data[row * columns + column];
}
double ML::Matrix::get(int i) const throw(std::out_of_range)
{
	if (i < 0 || i >= size_) throw std::out_of_range("Element not within the matrix");
	return data[i];
}
/**
 * (Was) Matrix multiplication with cache blocking and loop unrolling
*/
ML::Matrix ML::Matrix::operator*(const Matrix & other) const throw(ML::MathUndefinedException)
{
	if (columns != other.rows) throw MathUndefinedException();
	Matrix output(rows, other.columns);
	if (other.columns == 1) {
		for (int i = 0; i < rows; ++i) {
			for (int j = 0; j < columns; ++j)
				output.set(i, output.get(i) + get(i, j) * other.get(j));
		}
	}
	else assert(!"Not matrix vector product");
	return output;
}

ML::Matrix ML::Matrix::operator-(const Matrix & other) const throw(ML::MathUndefinedException)
{
	if (rows != other.rows || columns != other.columns) throw MathUndefinedException();
	Matrix output(rows, columns);
	for (int i = 0; i < output.size(); i += 4) {
		try {
			output.set(i, get(i) - other.get(i));
			output.set(i + 1, get(i + 1) - other.get(i + 1));
			output.set(i + 2, get(i + 2) - other.get(i + 2));
			output.set(i + 3, get(i + 3) - other.get(i + 3));
		}
		catch (std::out_of_range & e) {
			break;
		}
	}
	return output;
}

ML::Matrix ML::Matrix::operator+(const Matrix & other) const throw(ML::MathUndefinedException)
{
	
	if (rows != other.rows || columns != other.columns) throw MathUndefinedException();
	Matrix output(rows, columns);
	for (int i = 0; i < output.size(); i += 4) {
		try {
			output.set(i, get(i) + other.get(i));
			output.set(i + 1, get(i + 1) + other.get(i + 1));
			output.set(i + 2, get(i + 2) + other.get(i + 2));
			output.set(i + 3, get(i + 3) + other.get(i + 3));
		}
		catch (std::out_of_range & e) {
			break;
		}
	}
	return output;
}

void ML::Matrix::function(std::function<double(double)> f)
{
	for (int i = 0; i < size(); i += 4) {
		try {
			set(i, f(get(i)));
			set(i + 1, f(get(i + 1)));
			set(i + 2, f(get(i + 2)));
			set(i + 3, f(get(i + 3)));
		}
		catch (std::out_of_range & e) {
			break;
		}
	}
}

ML::NeuralNetwork::NeuralNetwork(std::initializer_list<size_t> list)
{
	biases.resize(list.size());
	weights.resize(list.size() - 1);

	for (int i = 0; i < list.size(); ++i) {
		biases[i].setDimensions(*(list.begin() + i), 1); //column vector
		if (i > 0) {
			//W * n(i - 1)
			weights[i - 1].setDimensions(*(list.begin() + i), *(list.begin() + (i - 1)));
		}
	}
}

ML::Matrix ML::NeuralNetwork::calculate(const Matrix & input)
{
	Matrix resultant = input;
	for (int i = 0; i < weights.size(); ++i) {
		resultant = weights[i] * resultant - biases[i + 1];
		resultant.function([](double x) -> double {return x / (1 + abs(x)); }); //not a true sigmoid but close enough and much faster
	}
	return resultant;
}

void ML::NeuralNetwork::populate(const char * filename)
{
	FILE * file = fopen(filename, "rb");
	if (file != NULL) {
		for (int i = 0; i < weights.size(); ++i) {
			double * weightMatrix = new double[weights[i].size()];
			fread(weightMatrix, sizeof(double), weights[i].size(), file);
			for (int j = 0; j < weights[i].size(); ++j)
				weights[i].set(j, weightMatrix[j]);
			delete[] weightMatrix;
		 }
		for (int i = 1; i < biases.size(); ++i) {
			double * biasVector = new double[biases[i].size()];
			fread(biasVector, sizeof(double), biases[i].size(), file);
			for (int j = 0; j < biases[i].size(); ++j)
				biases[i].set(j, biasVector[j]);
			delete[] biasVector;
		}
		fclose(file);
	}
}

void ML::NeuralNetwork::populate()
{
	for (auto& m : weights) {
		for (int i = 0; i < m.size(); ++i)
			m.set(i, Random::getNormal());
	}
	for (auto& b : biases) {
		for (int i = 0; i < b.size(); ++i)
			b.set(i, Random::getNormal());
	}
}

double ML::Random::getNormal()
{
	srand((clock() + count++) * count);
	double r = rand();
	return r / static_cast<double>(RAND_MAX);

}

int ML::Random::getInt(int min, int max)
{
	srand((clock() + count++) * count);
	return rand() % max + min;
}
