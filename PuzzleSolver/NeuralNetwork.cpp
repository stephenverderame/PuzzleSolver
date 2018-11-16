#include "NeuralNetwork.h"
#include <assert.h>
#include <Windows.h>
#include "img.h"
int ML::Random::count = 0;

ML::Matrix::Matrix(int r, int c) : rows(r), columns(c)
{
	data.resize(r * c, 0);
	this->size_ = r * c;
}

ML::Matrix::Matrix(const Matrix & other) : rows(other.rows), columns(other.columns), size_(other.size_)
{
	data.insert(data.end(), other.data.begin(), other.data.end());
}

ML::Matrix & ML::Matrix::operator=(const Matrix & other)
{
	rows = other.rows;
	columns = other.columns;
	size_ = other.size_;
	data = other.data;
	return *this;
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
 * Now just basic matrix multiplication and basic matrix vector multiplication for simplicity and testing
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
	else {
		for (int i = 0; i < rows; ++i) {
			for (int j = 0; j < columns; ++j) {
				for (int k = 0; k < other.rows; ++k) {
					output.set(i, j, output.get(i, j) + get(i, k) * other.get(k, j));
				}
			}
		}
	}
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

ML::Matrix ML::Matrix::transpose() const
{
	Matrix output(columns, rows);
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < columns; ++j) {
			output.set(j, i, get(i, j));
		}
	}
	return output;
}

ML::Matrix ML::Matrix::elementMultiply(const Matrix other) const throw(ML::MathUndefinedException)
{
	if (rows != other.rows || columns != other.columns) throw ML::MathUndefinedException();
	Matrix out(rows, columns);
	for (int i = 0; i < size_; ++i)
		out.data[i] = data[i] * other.data[i];
	return out;
}

ML::Matrix ML::Matrix::apply(std::function<double(double)> f) const
{
	Matrix output(rows, columns);
	for (int i = 0; i < size(); i += 4) {
		try {
			output.set(i, f(get(i)));
			output.set(i + 1, f(get(i + 1)));
			output.set(i + 2, f(get(i + 2)));
			output.set(i + 3, f(get(i + 3)));
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

void ML::Matrix::print(FILE * out) const
{
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < columns; ++j) {
			fprintf(out, "%f ", get(i));
		}
		fprintf(out, "\n");
	}
}

ML::NeuralNetwork::NeuralNetwork(std::initializer_list<size_t> list)
{
	testMat.setDimensions(26, 100);
	testMat.function([](double x) -> double {
		return Random::getInt();
	});
	biases.resize(list.size());
	weights.resize(list.size() - 1);

	for (int i = 0; i < list.size(); ++i) {
		biases[i].setDimensions(*(list.begin() + i), 1); //column vector
		if (i > 0) {
			//W * n(i - 1)
			weights[i - 1].setDimensions(*(list.begin() + i), *(list.begin() + (i - 1)));
		}
	}
	results.resize(weights.size());
}

ML::Matrix ML::NeuralNetwork::calculate(const Matrix & input)
{
	this->input = input;
	Matrix resultant = input;
	for (int i = 0; i < weights.size(); ++i) {
		resultant = (weights[i] * resultant) + biases[i + 1];
		resultant.function([](double x) -> double {return x / (1 + exp(-x)); });
		results.add(resultant);
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
			m.set(i, Random::getNormal(10000));
	}
	for (auto& b : biases) {
		for (int i = 0; i < b.size(); ++i)
			b.set(i, Random::getNormal(10000));
	}
}

void ML::NeuralNetwork::learn(const Matrix & calc, const Matrix & real)
{
	//First weight = weights[0]
	//First bias   =  biases[1]
	//First result = results[0]
	
	std::function<double(double)> learningRate = [](double x) -> double {return x * 0.7; };

	Matrix dJdB2 = (calc - real).elementMultiply(calc.apply(sigmoidDerivitive));
	Matrix dJdB1 = (weights[1].transpose() * dJdB2).elementMultiply(results[0].apply(sigmoidDerivitive));
	Matrix dJdW2 = dJdB2 * results[0].transpose();
	Matrix dJdW1 = dJdB1 * input.transpose();

	weights[0] = weights[0] - (dJdW1.apply(learningRate));
	weights[1] = weights[1] - (dJdW2.apply(learningRate));
	biases[1] = biases[1] - (dJdB1.apply(learningRate));
	biases[2] = biases[2] - (dJdB2.apply(learningRate));
}

void ML::NeuralNetwork::train()
{
	std::vector<Matrix> inputs;
	std::vector<Matrix> results;
	WIN32_FIND_DATA fData;
	HANDLE hand = FindFirstFile("C:\\Users\\stephen\\Documents\\Visual Studio 2015\\Projects\\PuzzleSolver\\PuzzleSolver\\letters\\*", &fData);
	char fileRead[MAX_PATH];
	while (hand != INVALID_HANDLE_VALUE) {
		printf("%s \n", fData.cFileName);
		sprintf_s(fileRead, MAX_PATH, "C:\\Users\\stephen\\Documents\\Visual Studio 2015\\Projects\\PuzzleSolver\\PuzzleSolver\\letters\\%s", fData.cFileName);
		if (!isalpha(fData.cFileName[0])) {
			FindNextFile(hand, &fData);
			continue;
		}
		IMG::Img img;
		img.loadFromPath(fileRead);
		Matrix mat(100, 1);
		for (int i = 0; i < 100; ++i) {
			int x = i % 10;
			int y = i / 10;
			mat.set(i, img.getPixel(x, y).avg());
		}
		inputs.push_back(mat);
		Matrix out(26, 1);
		out.set(toupper(fData.cFileName[0]) - 'A', 1.0);
		results.push_back(out);
		if (FindNextFile(hand, &fData) == FALSE) break;
	}
	int count = 0;
	while (count++ < 30) {
		int right = 0;
		for (int i = 0; i < inputs.size(); ++i) {
			char c = 'A' - 1;
			double max_ = DBL_MIN;
			for (int j = 0; j < results[i].size(); ++j) {
 				if (results[i].get(j) > max_) {
					c = j + 'A';
					max_ = results[i].get(j);
				}
			}
			Matrix out = calculate(inputs[i]);

			char c2 = 'A' - 1;
			for (int j = 0; j < out.size(); ++j) {
				if (out.get(j) > max_) {
					c2 = j + 'A';
					max_ = out.get(j);
				}
			}
			if (c == c2) ++right;
			learn(out, results[i]);
		}
		printf("#%d:  %d / %d \n", count, right, inputs.size());
	}	
}

double ML::Random::getNormal()
{
	srand((clock() + count++) * count);
	double r = rand();
	return r / static_cast<double>(RAND_MAX);

}

double ML::Random::getNormal(int limit)
{
	srand((clock() + count++) * count);
	return static_cast<double>(rand() % limit + 1) / limit - 0.5;
}

int ML::Random::getInt(int min, int max)
{
	srand((clock() + count++) * count);
	return rand() % max + min;
}

int ML::Random::getInt()
{
	srand((clock() + count++) * count);
	return rand();
}
