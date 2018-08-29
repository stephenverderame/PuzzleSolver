#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <Canvas.h>
#define _USE_MATH_DEFINES
#include <math.h>
#define radians(x) ((x) * (M_PI / 180.0))
#include "res.h"
#include <GUIS.h>
#include <sstream>
#include <fstream>
#include <time.h>
#include <map>
#include <algorithm>
//#define DEBUGGING_SPACE
//#define SHOW_DEBUG_CHARS
struct Square {
	int x;
	int y;
	int width;
	int height;
};
struct Rect {
	POINT topLeft;
	POINT bottomRight;
	operator Square();
};
struct Space {
	int start;
	int size;
};
struct Pixel {
	int x;
	int y;
	Color c;
};
struct Letter {
	int row;
	int column;
	char letter;
	bool operator==(Letter other) { return *this == other.letter; }
	bool operator==(char c);
};
struct Line {
	POINT start, end;
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
	std::vector <Letter *> letters;
	std::vector <KnownSample *> identifiedLetters;
	int row0Y;
	int column0X;
	std::pair<int, int> lastRow;
	std::pair<int, int> lastColumn;
	int maxRows = 0;
	int maxColumns = 0;
public:
	void addLetter(char c, int x, int y);
	SearchGrid() {};
	~SearchGrid();
	void iterateRowbyRow();
	std::pair<int, int> getDimensions() { return std::pair<int, int>(maxRows, maxColumns); }
	void matchLetter(KnownSample * s) { identifiedLetters.push_back(s); }
	Letter * getLetter(int i) { return letters[i]; }
	char getLetter(int columns, int rows) { if (rows > maxRows || rows < 0 || columns > maxColumns || columns < 0) return '-'; return letters[columns * (maxRows + 1) + rows]->letter; }
	void search(Image * img, std::vector<Square> locations, std::vector<std::string> words);
	void copyFrom(SearchGrid g);
	bool isEmpty() { return letters.size() == 0; }
	SearchGrid& operator=(const SearchGrid & other);
	SearchGrid(const SearchGrid& other);
};
POINT matrixMultiply(float * matrix, POINT vector);
float findSkewAngle(Image * img, POINT * origin = nullptr, Bounds * skewBounds = NULL);
POINT getOrigin(Image * img);
void rotateImage(Image * img, float theta, POINT origin);
std::vector<Square> getCharacterLocations(Image * img);
SearchGrid identifyLetters(Image * img, std::vector<Square> locations);
void augmentDataSet(std::vector<Square> locations, std::vector<char> knowns, Image * img, int firstKnown = 0);