#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "img.h"
#include "Math.h"
#include "res.h"
#include <GUIS.h>
#include <sstream>
#include <fstream>
#include <time.h>
#include <map>
#include <algorithm>
#include "RedBlackTree.h"
#undef max
//#define DEBUGGING_SPACE
//#define SHOW_DEBUG_CHARS
namespace CV {
	struct Square {
		int x;
		int y;
		int width;
		int height;
		Square() : x(0), y(0), width(0), height(0) {};
		Square(int x, int y, int width, int height) : x(x), y(y), width(width), height(height) {};
	};
	struct Space {
		int start;
		int size;
	};
	struct Letter {
		int row;
		int column;
		char letter;
		bool operator==(Letter other) { return *this == other.letter; }
		bool operator==(char c);
	};
	struct Line {
		Math::point start, end;
		inline bool outOfBounds(int maxRows = 20, int maxColumns = 20);
	};
	struct Bounds {
		float min;
		float max;
	};
	struct KnownSample {
		Image * image;
		char letter;
		~KnownSample() { delete image; }
		operator Image*() { return image; }
		Image * operator->() { return image; }
		KnownSample(const KnownSample& other) : letter(other.letter) {
			image = new Image(other.image->getWidth(), other.image->getHeight());
			for (int i = 0; i < image->getWidth() * image->getHeight(); i++) {
				int x = i % image->getWidth();
				int y = i / image->getWidth();
				image->setPixel(x, y, other.image->getPixel(x, y));
			}
		}
		KnownSample(Image * ptr, char let) : image(ptr), letter(let) {};
		KnownSample& operator=(const KnownSample & other);
	};
	using pointList = std::vector<std::pair<Math::point, Math::point>>;
	class SearchGrid {
	private:
		std::vector<int> columnPositions, rowPositions;
		std::vector<std::vector<Square>> characterLocations;
		std::vector<std::vector<char>> characters;
		IMG::Img & seekImage;
	private:
		void getCharacterLocations();
		void init();
		void identifyLetters();
	public:
		void addLetter(char c, int x, int y);
		SearchGrid(IMG::Img & wordSearch);
		void load(IMG::Img & searchImage);
		void iterateRowbyRow();
		char getLetter(int columns, int rows) {
			if (columns >= columnPositions.size() || columns < 0 || rows >= rowPositions.size() || rows < 0) return '-';
			return characters[columns][rows];
		}
		void search(std::vector<std::string> words);
		void copyFrom(SearchGrid g);
		bool isEmpty() { return lettersInGrid.size() == 0; }
		SearchGrid& operator=(const SearchGrid & other);
		SearchGrid(const SearchGrid& other);
		IMG::Img & getEditedImage() { return seekImage; }
	};
	class ConnectedComponents {
		std::vector<std::vector<char>> valueArray;
		std::vector<std::vector<uint32_t>> labelArray;
		uint32_t labels;
	public:
		void findConnectedComponents(IMG::Img & image);
		std::vector<Square> componentLocations();
	};

	class Hough {
	private:
		std::vector<std::vector<uint32_t>> accumulator;
		size_t accumulatorHeight;
		size_t accumulatorWidth;
		size_t imageWidth, imageHeight;
		struct {
			double x;
			double y;
		} center;
	public:
		Hough(const Hough & other) = delete;
		Hough& operator=(const Hough & other) = delete;
	public:
		Hough() = default;
		void transform(IMG::Img & img);
		pointList getLines(uint32_t threshold);
		void display(const char * filename) const;

	};
	class Kernel {
	private:
		std::vector<std::vector<double>> k;
		int width;
		int height;
	public:
		Kernel(int width, int height);
		Kernel(std::initializer_list<double> list);
		std::shared_ptr<IMG::Img> apply(IMG::Img & img);
		std::vector<Math::vec3f> apply_matrix(IMG::Img & img);
		void testShowMatrix(FILE * out);
		void scale(double scalar);
	};

	float findSkewAngle(IMG::Img & img, Math::point * origin = nullptr, Bounds * skewBounds = nullptr);
	Math::point getOrigin(IMG::Img & img);
	void rotateImage(IMG::Img & img, float theta, Math::point origin);
	void augmentDataSet(std::vector<Square> locations, std::vector<char> knowns, IMG::Img & img, int firstKnown = 0);
	std::shared_ptr<IMG::Img> cannyEdgeDetection(IMG::Img & img, const double upperThreshold = 0.1, const double lowerThreshold = 0.05);
	std::shared_ptr<IMG::Img> sobelEdgeDetection(IMG::Img & img, std::vector<double> * magnitudes = nullptr, std::vector<double> * directions = nullptr);
}
namespace CV {
	template <typename T = int, int buffer = 20>
	class OCRCompare : public RB::RBCompare<T> {
	private:
		bool lessThanP(const T & a, const T & b) const noexcept override {
			return a < b - buffer;
		}
		bool greaterThanP(const T & a, const T & b) const noexcept override {
			return a > b + buffer;
		}
	};
	using OCR_COMPARE = OCRCompare<>;
	template<int BUFFER = 20>
	using RB_COMPARE = OCRCompare<int, BUFFER>;
	using OCR_TREE = RB::RedBlackTree<int, OCR_COMPARE>;
	template<int BUFFER = 20>
	using RB_TREE = RB::RedBlackTree<int, RB_COMPARE<BUFFER>>;
}