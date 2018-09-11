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

	using pointList = std::vector<std::pair<Math::point, Math::point>>;
	class Hough {
	private:
		std::vector<std::vector<uint32_t>> accumulator;
		size_t accumulatorHeight;
		size_t accumulatorWidth;
		struct {
			double x;
			double y;
		} center;
		IMG::Img & image;
	public:
		Hough(const Hough & other) = delete;
		Hough& operator=(const Hough & other) = delete;
	public:
		Hough(IMG::Img & img) : image(img) {};
		void transform(IMG::Img & img);
		pointList getLines(uint32_t threshold);

	};


	float findSkewAngle(IMG::Img & img, Math::point * origin = nullptr, Bounds * skewBounds = nullptr);
	Math::point getOrigin(IMG::Img & img);
	void rotateImage(IMG::Img & img, float theta, Math::point origin);
	void augmentDataSet(std::vector<Square> locations, std::vector<char> knowns, IMG::Img & img, int firstKnown = 0);
}