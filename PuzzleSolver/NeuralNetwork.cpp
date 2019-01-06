#include "NeuralNetwork.h"
#include <assert.h>
#include <Windows.h>
#include "img.h"
#include <sstream>
#include <iomanip>
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

ML::Matrix::Matrix(const std::initializer_list<std::initializer_list<double>>& m)
{
	rows = m.size();
	columns = m.begin()->size();
	size_ = m.size() * m.begin()->size();
	for (auto l : m) {
		data.insert(data.end(), l.begin(), l.end());
	}
}

ML::Matrix & ML::Matrix::operator=(const Matrix & other)
{
	rows = other.rows;
	columns = other.columns;
	size_ = other.size_;
	data = other.data;
	return *this;
}

ML::Matrix & ML::Matrix::operator=(const std::initializer_list<double>& list)
{
	assert(list.size() == size_ && "Sizes must be equal");
	for (int i = 0; i < size_; ++i)
		data[i] = *(list.begin() + i);
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
double ML::Matrix::summate() const
{
	double sum = 0;
	for (auto n : data)
		sum += n;
	return sum;
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

ML::Matrix ML::operator*(const double c, const ML::Matrix & m)
{
	ML::Matrix mat = m;
	for (int i = 0; i < mat.size(); ++i)
		mat.set(i, m.get(i) * c);
	return mat;
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

ML::Matrix & ML::Matrix::operator-=(const Matrix & other) throw(ML::MathUndefinedException)
{
	if (rows != other.rows || columns != other.columns) throw ML::MathUndefinedException();
	for (int i = 0; i < size(); ++i)
		data[i] -= other.data[i];
	return *this;
}

ML::Matrix & ML::Matrix::operator+=(const Matrix & other) throw(ML::MathUndefinedException)
{
	if (rows != other.rows && columns != other.columns) throw ML::MathUndefinedException();
	for (int i = 0; i < size_; ++i)
		data[i] += other.data[i];
	return *this;

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
			fprintf(out, "%f ", get(i, j));
		}
		fprintf(out, "\n");
	}
}

void ML::Matrix::randomize()
{
	for (int i = 0; i < size(); ++i)
		data[i] = Random::getNormal(10000);
}

void ML::Matrix::zero()
{
	std::fill(data.begin(), data.end(), 0);
}

ML::NeuralNetwork::NeuralNetwork(std::initializer_list<size_t> list) : layers(list.size()), learningRate(1)
{
	biases.resize(list.size() - 1);
	sumBiases.resize(list.size() - 1);
	weights.resize(list.size() - 1);
	sumWeights.resize(list.size() - 1);

	for (int i = 1; i < list.size(); ++i) {
		biases[i - 1].setDimensions(*(list.begin() + i), 1); //column vector
		sumBiases[i - 1].setDimensions(*(list.begin() + i), 1);
		weights[i - 1].setDimensions(*(list.begin() + i), *(list.begin() + (i - 1)));
		sumWeights[i - 1].setDimensions(*(list.begin() + i), *(list.begin() + (i - 1)));
	}
	results.resize(list.size());
	differentialLearning.e = false;

}

ML::Matrix ML::NeuralNetwork::calculate(const Matrix & input)
{
	Matrix resultant = input;
	results.add(resultant);
	for (int i = 0; i < weights.size(); ++i) {
		resultant = (weights[i] * resultant) + biases[i];
		resultant.function(sigmoid);
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
	//results[0] = input matrix
/*
	const ML::Matrix dCdB2 = ML::summation(2 * (calc - real)) * (ML::sigDerivative(weights[1] * results[1] + biases[1]));
	const ML::Matrix dCdW2 = dCdB2 * results[1].transpose();

	const ML::Matrix dCda0 = weights[1].transpose() * dCdB2;
	const ML::Matrix dCdB1 = ML::summation(dCda0) * (ML::sigDerivative(weights[0] * input + biases[0]));
	const ML::Matrix dCdW1 = dCdB1 * input.transpose();

	weights[0] -= dCdW1;
	weights[1] -= dCdW2;
	biases[0] -= dCdB1;
	biases[1] -= dCdB2;
*/
	ML::Matrix cost = 2 * (calc - real);
	for (int i = weights.size() - 1; i >= 0; --i) {
		ML::Matrix dCdB = cost.elementMultiply(ML::sigDerivative(weights[i] * results[i] + biases[i]));
		ML::Matrix dCdW = dCdB * results[i].transpose();
		cost = weights[i].transpose() * dCdB;

		if (differentialLearning.e) {
			weights[i] -= differentialLearning.rates[i] * dCdW;
			biases[i] -= differentialLearning.rates[i] * dCdB;
		}
		else {
			weights[i] -= learningRate * dCdW;
			biases[i] -= learningRate * dCdB;
		}
	}
	
}
void ML::NeuralNetwork::learnBatch(const Matrix & calc, const Matrix & real) const
{
	ML::Matrix cost = 2 * (calc - real);
	for (int i = weights.size() - 1; i >= 0; --i) {
		ML::Matrix dCdB = cost.elementMultiply(ML::sigDerivative(weights[i] * results[i] + biases[i]));
		ML::Matrix dCdW = dCdB * results[i].transpose();
		cost = weights[i].transpose() * dCdB;

		if (differentialLearning.e) {
			sumWeights[i] += differentialLearning.rates[i] * dCdW;
			sumBiases[i] += differentialLearning.rates[i] * dCdB;
		}
		else {
			sumWeights[i] += learningRate * dCdW;
			sumBiases[i] += learningRate * dCdB; 
		}
	}

}
void ML::NeuralNetwork::update(int batchSize)
{

	for (int i = 0; i < weights.size(); ++i) {
		weights[i] -= (1.0 / batchSize) * sumWeights[i];
		biases[i] -= (1.0 / batchSize) * sumBiases[i];
		sumWeights[i].zero();
		sumBiases[i].zero(); 
	}
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
	while (count++ < 50) {
		int right = 0;
		for (int i = 0; i < inputs.size(); ++i) {
			char c = -1;
			double max_ = DBL_MIN;
			for (int j = 0; j < results[i].size(); ++j) {
 				if (results[i].get(j) > max_) {
					c = j;
					max_ = results[i].get(j);
				}
			}
			const Matrix out = calculate(inputs[i]);

			max_ = DBL_MIN;
			char c2 = -1;
			for (int j = 0; j < out.size(); ++j) {
				if (out.get(j) > max_) {
					c2 = j;
					max_ = out.get(j);
				}
			}
			if (c == c2 && c != -1) ++right;
			learn(out, results[i]);
		}
		printf("#%d:  %d / %d \n", count, right, inputs.size());
	}	
}

void ML::NeuralNetwork::setDifferentialLearningRates(std::vector<double> x)
{
	differentialLearning.e = true;
	differentialLearning.rates = x;
}

std::vector<double> ML::NeuralNetwork::getDifferentialLearningRates() const throw(std::string)
{
	if (!differentialLearning.e) throw std::string("Differential learning not enabled");
	return differentialLearning.rates;
}

void ML::NeuralNetwork::save(const char * filename) const
{
	FILE * file = fopen(filename, "w");
	if (file == NULL) throw std::exception("Cannot open file");
	fprintf(file, "%d\n", layers);
	for (auto b : biases) {
		fprintf(file, "%d\n", b.getRows());
		b.print(file);
	}
	for (auto w : weights) {
		fprintf(file, "%d %d\n", w.getRows(), w.getColumns());
		w.print(file);
	}
	fclose(file);
}

void ML::NeuralNetwork::load(const char * filename)
{
	std::ifstream in;
	in.open(filename);
	if (!in.is_open()) throw std::exception("Cannot open file");
	std::string line;
	std::getline(in, line);
	layers = std::stoi(line);
	for (int i = 0; i < layers - 1; ++i) { //biases
		std::getline(in, line);
		int rows = std::stoi(line);
		Matrix m(rows, 1);
		for (int j = 0; j < rows; ++j) {
			std::getline(in, line);
			m.set(j, std::stod(line));
		}
		biases.push_back(m);
	}
	for (int i = 0; i < layers - 1; ++i) {
		std::getline(in, line);
		int rows = std::stoi(line.substr(0, line.find(' ')));
		int cols = std::stoi(line.substr(line.find(' ') + 1));
		Matrix m(rows, cols);
		for (int i = 0; i < rows; ++i) {
			std::getline(in, line);
			std::istringstream iss(line);
			std::string s;
			int j = 0;
			while (std::getline(iss, s, ' ')) {
				m.set(i, j, std::stod(s));
				++j;
			}
		}
		weights.push_back(m);
	}
	in.close();
}

double ML::Random::getNormal()
{
	srand((clock() + count++) * count);
	const double r = rand();
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
ML::Matrix operator*(const ML::Matrix & m, const double c)
{
	ML::Matrix out = m;
	for (int i = 0; i < out.size(); ++i)
		out.set(i, m.get(i) * c);
	return out;
}

IMG::Img ML::pickOutLetter(const IMG::Img & letter)
{
	int minx = letter.width(), miny = letter.height(), maxx = 0, maxy = 0;
	for (int i = 0; i < letter.width(); ++i) {
		for (int j = 0; j < letter.height(); ++j) {
			if (letter.getPixel(i, j).avg() <= 127) {
				minx = min(minx, i);
				miny = min(miny, j);
				maxx = max(maxx, i);
				maxy = max(maxy, j);
			}
		}
	}
	assert(minx < maxx && miny < maxy && "Failed to pick out image");
	IMG::Img img;
	img.createNew(maxx - minx, maxy - miny);
	for (int i = 0; i < img.width(); ++i) {
		for (int j = 0; j < img.height(); ++j) {
			img.setPixel(IMG::pixel{ letter.getPixel(i + minx, j + miny), Math::point{i, j} });
		}
	}
	return img;

}

void ML::trainNetwork(NeuralNetwork & nn, bool train)
{
	std::ifstream in;
	in.open("C:\\Users\\stephen\\Desktop\\badLetters.txt");
	if (!in.is_open()) printf("Can't open bLetters!");
	std::vector<std::pair<int, int>> badLetters;
	std::string line;
	while (std::getline(in, line)) {
		if (line.size() < 5) continue;
		std::pair<int, int> range;
		try {
			range.first = std::stoi(line.substr(0, line.find(' ')));
			range.second = std::stoi(line.substr(line.find('-') + 2));
			badLetters.push_back(range);
		}
		catch (std::invalid_argument & e) {
			printf("No number in %s \n", line.c_str());
		}
	}
	const char * generalPath = "C:\\Users\\stephen\\Desktop\\coding stuff\\Letter Dataset\\Fnt";
	const int s_width = 10, s_height = 10;
	int correctN = 0, iterations = 0;
	for (int j = 0; j < 10000; ++j) {
			int i = Random::getInt(0, 26);
			int sample = Random::getInt(1, 1016);
			bool repeat;
			do {
				repeat = false;
				for (const auto p : badLetters) {
					if (sample >= p.first && sample <= p.second) {
						repeat = true;
						sample = Random::getInt(1, 1016);
						break;
					}
				}
			} while (repeat);
			//		printf("%d --> %d\n", i, sample);
			std::stringstream imgPath;
			imgPath << generalPath << "\\Sample0" << (i + 11) << "\\img0" << (i + 11) << "-0" << std::setw(4) << std::setfill('0') << sample << ".png";
			struct stat filecheck;
			if (stat(imgPath.str().c_str(), &filecheck) != 0) {
				printf("%s does not exist!\n", imgPath.str().c_str());
				throw std::exception("Image cannot be opened!");
			}
			IMG::Img img;
			img.loadFromPath(imgPath.str().c_str());
			img = pickOutLetter(img);
			img.scaleTo(s_width, s_height);
			Matrix m(s_width * s_height, 1);

			for (int k = 0; k < s_width * s_height; ++k) {
				m.set(k, img.getPixel(k % s_width, k / s_width).avg());
			}
			Matrix correct(26, 1);
			correct.zero();
			correct.set(i, 1.0);
			Matrix out = nn.calculate(m);
			double maxVal = DBL_MIN;
			int maxIn = -1;
			for (int k = 0; k < out.size(); ++k) {
				if (out.get(k) > maxVal) {
					maxVal = out.get(k);
					maxIn = k;
				}
			}
			if (maxIn == i) correctN++;
			if(train) nn.learn(out, correct);
			++iterations;
			if (iterations % 10 == 0) printf("%d / %d\n", correctN, iterations);
	}
}

ML::Matrix ML::sigDerivative(const Matrix & m)
{
	Matrix out = m;
	return out.apply(ML::sigmoid);
}

double ML::summation(const ML::Matrix & m)
{
	double sum = 0;
	for (int i = 0; i < m.size(); ++i)
		sum += m.get(i);
	return sum;
}

