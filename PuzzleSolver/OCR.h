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
		std::vector<std::shared_ptr<Letter>> lettersInGrid;
		std::vector<std::shared_ptr<KnownSample>> identifiedLetters;
		std::vector<Square> locations;
		int row0Y;
		int column0X;
		std::pair<int, int> lastRow;
		std::pair<int, int> lastColumn;
		int maxRows = 0;
		int maxColumns = 0;
		IMG::Img & seekImage;
	private:
		void getCharacterLocations();
		void getCharacterLocations(pointList & lines);
		void identifyLetters();
	public:
		void addLetter(char c, int x, int y);
		SearchGrid(IMG::Img & wordSearch);
		void load(IMG::Img & searchImage);
		void iterateRowbyRow();
		std::pair<int, int> getDimensions() { return std::pair<int, int>(maxRows, maxColumns); }
		void matchLetter(std::shared_ptr<KnownSample> s) { identifiedLetters.push_back(s); }
		std::shared_ptr<Letter> getLetter(int i) { return lettersInGrid[i]; }
		char getLetter(int columns, int rows) { if (rows > maxRows || rows < 0 || columns > maxColumns || columns < 0) return '-'; return lettersInGrid[columns * (maxRows + 1) + rows]->letter; }
		void search(std::vector<std::string> words);
		void copyFrom(SearchGrid g);
		bool isEmpty() { return lettersInGrid.size() == 0; }
		SearchGrid& operator=(const SearchGrid & other);
		SearchGrid(const SearchGrid& other);
		int getLocationsSize() { return locations.size(); }
		Square getLocation(int i) { return locations[i]; }
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
	template <typename T = int>
	class OCRCompare : public RB::RBCompare<T> {
	private:
		bool lessThanP(const T & a, const T & b) const noexcept override {
			return a < b - 20;
		}
		bool greaterThanP(const T & a, const T & b) const noexcept override {
			return a > b + 20;
		}
	};
	using RB_COMPARE = OCRCompare<>;
}