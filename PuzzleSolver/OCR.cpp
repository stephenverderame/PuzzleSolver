#include "OCR.h"
using namespace Math;
/**
 * From the origin, determines the angle in which the most dark pixels lie on a straight line
 * Ideally the line picked is parallel to a row of text in the search
 * @param img        image to perform angle find on
 * @param origin     optional parameter (nullptr if not set), custom origin point for the search
 * @param skewBounds optional parameter (nullptr if not set), enforces that the angle must be within these bounds
 * @return           float up to the tenth place in precision for the detected skew angle
*/
float CV::findSkewAngle(IMG::Img & img, Math::point * origin, Bounds * skewBounds) {
	if (origin == nullptr)
		*origin = getOrigin(img);
	int startX = origin->x;
	int startY = origin->y;
	std::vector<point> line;
	int * accumulator = new int[360];
	memset(accumulator, 0, 360 * sizeof(int));
	int diagnol = sqrt(img.width() * img.width() + img.height() * img.height());
	for (int i = 0; i < diagnol; i++) {
		line.push_back({ startX + i, startY });
	}
	for (int theta = 0; theta < 360; theta++) {
		matrix rotMatrix = Math::rotationMatrix(theta);
		for (int i = 0; i < img.width(); i++) {
			point linePt = { line[i].x - startX, line[i].y - startY };
			point testPoint = rotMatrix * linePt;
			testPoint.x += startX;
			testPoint.y += startY;
			int pixel = img.getPixel(testPoint).avg() < 100 ? 1 : 0;
			if (testPoint.x < 0 || testPoint.x >= img.width() || testPoint.y < 0 || testPoint.y >= img.height()) pixel = 0;
			accumulator[theta] += pixel;
		}
	}
	int maxPixels = INT_MIN;
	int indexMax = 0;
	for (int i = 0; i < 360; i++) {
		if(skewBounds != NULL)
			if (i < floor(skewBounds->min) || i > ceil(skewBounds->max)) continue;
		int old = maxPixels;
		maxPixels = max(maxPixels, accumulator[i]);
		if (old != maxPixels) indexMax = i;
	}
#pragma region extraPrecision
	int indexMaxMinor = 0;
	memset(accumulator, 0, 10 * sizeof(int));
	for (int minor = -5; minor < 5; minor++) {
		float theta = indexMax + (minor / 10.0);
		matrix rotMat = rotationMatrix(theta);
		for (int i = 0; i < img.width(); i++) {
			point linePt = { line[i].x - startX, line[i].y - startY };
			point testPoint = rotMat * linePt;
			testPoint.x += startX;
			testPoint.y += startY;
			int pixel = img.getPixel(testPoint).avg() < 100 ? 1 : 0;
			if (testPoint.x < 0 || testPoint.x >= img.width() || testPoint.y < 0 || testPoint.y >= img.height()) pixel = 0;
			accumulator[minor + 5] += pixel;
		}
	}
	maxPixels = INT_MIN;
	for (int i = 0; i < 10; i++) {
		int old = maxPixels;
		maxPixels = max(maxPixels, accumulator[i]);
		if (old != maxPixels) indexMaxMinor = i;
	}
#pragma endregion
	float angle = indexMax + ((indexMaxMinor - 5) / 10.0);
	RECT r;
	GetClientRect(gui::GUI::useWindow(), &r);
	InvalidateRect(gui::GUI::useWindow(), &r, TRUE);
	printf("Original angle: %f \n", angle);
	//end testing
	delete[] accumulator;
	if (abs(angle - 90) < 20) {
		angle -= 90;
	}
	else if (angle > 90) {
		if (abs(180 - angle) < 90) {
			angle = 180 - angle;
			angle = -angle;
		}
	}
	return angle;

}
/**
 * Initializes search grid
 * Saves bounding boxes by adding rows to characterLocations[][]
 * Depends on seekImage
 * Assumes image is rotation corrected (majority of skew is "fixed")
 * @see SearchGrid
 */
void CV::SearchGrid::init()
{
	/**
	  Perform canny edge detection followed by a hough transform to find vertical and horizontal lines
	  Lines determined if the change in x or y between its enpoints is small in proportion to image dimensions
	  At the intersection of the hough transform lines, look for a dark pixel within a small radius
	  Create and expand a bounding box around that pixel until there are no more connected dark pixels
	*/

	Kernel guassian({
		1,  4,  6,  4, 1,
		4, 16, 24, 16, 4,
		6, 24, 36, 24, 6,
		4, 16, 24, 16, 4,
		1,  4,  6,  4, 1
	});
	guassian.scale(1 / 256.0);
	seekImage.trueGrayscale(std::make_unique<IMG::LumFunc>());
//	guassian.kernelConvolution(seekImage);
	int avg = (seekImage.integralImageValue(seekImage.width() - 1, seekImage.height() - 1) - seekImage.integralImageValue(seekImage.width() - 1, 0) - seekImage.integralImageValue(0, seekImage.height() - 1) + seekImage.integralImageValue(0, 0)) / (seekImage.width() * seekImage.height());
	avg = 255 - avg;
	auto img = cannyEdgeDetection(seekImage);
	Hough hough;
	hough.transform(*img);
	avg *= 1.7;
	printf("AVG: %d \n", avg);
	auto list = hough.getLines(avg);
	printf("%d lines found!\n", list.size());
	decltype(list) newList;
	for (decltype(auto) line : list) {
		auto d = line.second - line.first;
		if ((abs(d.y) < img->height() * 0.05 && abs(d.x) > img->width() * 0.95) || (abs(d.x) < img->width() * 0.05 && abs(d.y) > img->height() * 0.95)) {
			newList.push_back(line);
		}
	}
	seekImage.greyscale();
	std::vector<Square> foundLetters;
	for (auto it = newList.cbegin(); it != newList.cend(); ++it) {
		for (auto itt = newList.cbegin(); itt != newList.cend(); ++itt) {
			if (it == itt) continue;
			point intersect = lineIntersection(it->first, it->second, itt->first, itt->second);
			if (intersect.x != INT_MAX && intersect.y != INT_MIN) {
				point start = { -1, -1 };
				for (int dist = 0; dist <= 10; ++dist) {
					bool found = false;
					point tpLeft = { intersect.x - dist, intersect.y - dist };
					point btmRit = { intersect.x + dist, intersect.y + dist };
					for (int i = tpLeft.x; i <= btmRit.x; ++i) {
						if (seekImage.xInBounds(i) && seekImage.yInBounds(tpLeft.y) && seekImage.getPixel({ i, tpLeft.y }).avg() <= 100) {
							found = true; start = { i, tpLeft.y }; break;
						}
					}
					if (found) break;
					for (int i = tpLeft.x; i <= btmRit.x; ++i) {
						if (seekImage.xInBounds(i) && seekImage.yInBounds(btmRit.y) && seekImage.getPixel({ i, btmRit.y }).avg() <= 100) {
							found = true; start = { i, btmRit.y }; break;
						}
					}
					if (found) break;
					for (int i = tpLeft.y; i <= btmRit.y; ++i) {
						if (seekImage.xInBounds(tpLeft.x) && seekImage.yInBounds(i) && seekImage.getPixel({ tpLeft.x, i }).avg() <= 100) {
							found = true; start = { tpLeft.x, i }; break;
						}
					}
					if (found) break;
					for (int i = tpLeft.y; i <= btmRit.y; ++i) {
						if (seekImage.xInBounds(btmRit.x) && seekImage.yInBounds(i) && seekImage.getPixel({ btmRit.x, i }).avg() <= 100) {
							found = true; start = { btmRit.x, i }; break;
						}
					}
					if (found) break;
				}
#ifdef DEBUGGING
				seekImage.setPixel(0, 255, 0, start.x, start.y);
#endif
				if (seekImage.xInBounds(start.x) && seekImage.yInBounds(start.y)) {
					int maxX = 0, minX = 0;
					int minY = 0, maxY = 0;
					int yoffset = 0;
					int direction = 1;
					while (true) {
						int xoffset = 0;
						int nxoffset = 0;
						int totalIntensity = 0;
						if (seekImage.yInBounds(start.y + yoffset)) {
							bool in = false, in2 = false;
							while (seekImage.xInBounds(start.x + xoffset) && (xoffset < maxX || (in = seekImage.getPixel(start + point{ xoffset, yoffset }).avg() < 100))) {
								if (in || seekImage.getPixel(start + point{ xoffset, yoffset }).avg() < 100) totalIntensity++;
								in = false;
								++xoffset;
							}
							while (seekImage.xInBounds(start.x + nxoffset) && (nxoffset > minX || (in2 = seekImage.getPixel(start + point{ nxoffset, yoffset }).avg() < 100))) {
								if (in2 || seekImage.getPixel(start + point{ nxoffset, yoffset }).avg() < 100) totalIntensity++;
								in2 = false;
								--nxoffset;
							}
						}
						maxX = max(maxX, xoffset);
						minX = min(minX, nxoffset);
						maxY = max(maxY, yoffset);
						minY = min(minY, yoffset);
						if (totalIntensity == 0) {
							if (direction == -1) break;
							direction = -1;
							yoffset = 0;
						}
						yoffset += direction;
					}
					foundLetters.push_back({ start.x + minX, start.y + minY, start.x + maxX - (start.x + minX), start.y + maxY - (start.y + minY) });
				}
			}

		}

	}
	//* For all bounding boxes that overlap or are very close together: merge them
	for (auto it = foundLetters.begin(); it != foundLetters.end(); ++it) {
		int ittCount = 0;
		for (auto itt = foundLetters.begin(); itt != foundLetters.end(); ++ittCount, ++itt) {
			if (it == itt) continue;
			if ((itt->x == -1 && itt->y == -1) || (it->x == -1 && it->y == -1)) continue;
			if ((it->x - itt->x) * (it->x - itt->x) + (it->y - itt->y) * (it->y - itt->y) < 225 || (itt->x >= it->x && itt->x <= it->x + it->width && itt->y >= it->y && itt->y <= it->y + it->height)) {
				it->x = min(it->x, itt->x);
				it->y = min(it->y, itt->y);
				it->width = max(it->width, itt->width);
				it->height = max(it->height, itt->height);
				itt->x = -1;
				itt->y = -1;
			}
		}
	}
	/**
	  Calculate median width and median height of all bounding boxes
	  For all bounding boxes within x% of those medians, create a sorted list of their x and y coordinates
	  Use the ordered list of coordinates to create an ordered list of horizontal and vertical spaces between bounding boxes
	  Determine median space in horizontal and vertical direction
	*/

	RB_TREE<6> widthVals, heightVals;
	OCR_TREE xVals;
	OCR_TREE yVals;
	for (auto it = foundLetters.cbegin(); it != foundLetters.cend(); ++it) {
		if (it->x == -1 && it->y == -1) continue;
		widthVals.insert(it->width);
		heightVals.insert(it->height);
	}
	auto widthList = widthVals.inorderList();
	auto heightList = heightVals.inorderList();
	int widthMedian = widthList.size() % 2 ? widthList[widthList.size() / 2] : (widthList[widthList.size() / 2] + widthList[widthList.size() / 2 - 1]) / 2;
	int heightMedian = heightList.size() % 2 ? heightList[heightList.size() / 2] : (heightList[heightList.size() / 2] + heightList[heightList.size() / 2 - 1]) / 2;
	printf("Median width: %d Median height: %d \n", widthMedian, heightMedian);
	for (auto it = foundLetters.cbegin(); it != foundLetters.cend(); ++it) {
		if (it->x == -1 && it->y == -1) continue;
		if (abs(it->width - widthMedian) <= .3 * widthMedian && abs(it->height - heightMedian) <= .3 * heightMedian) {
			xVals.insert(it->x);
			yVals.insert(it->y);
		}
	};
	std::vector<int> xList = xVals.inorderListNR();
	std::vector<int> yList = yVals.inorderListNR();
	std::vector<int> xSpace, ySpace;
	for (int i = 0; i < xList.size(); ++i) {
		if (i == 0) xSpace.push_back(xList[i]);
		else {
			int v = i;
			while (v >= 0 && xList[i] == xList[v]) --v;
			xSpace.push_back(xList[i] - xList[v]);
		}
	}
	for (int i = 0; i < yList.size(); ++i) {
		if (i == 0) ySpace.push_back(yList[i]);
		else {
			int v = i;
			while (v >= 0 && yList[i] == yList[v]) --v;
			ySpace.push_back(yList[i] - yList[v]);
		}
	}
	int xMedian = xSpace.size() % 2 ? xSpace[xSpace.size() / 2] : (xSpace[xSpace.size() / 2] + xSpace[xSpace.size() / 2 - 1]) / 2;
	int yMedian = ySpace.size() % 2 ? ySpace[ySpace.size() / 2] : (ySpace[xSpace.size() / 2] + ySpace[ySpace.size() / 2 - 1]) / 2;
	printf("Medians %d %d\n", xMedian, yMedian);
	printf("XSpaceSize: %d XListSize: %d \n", xSpace.size(), xList.size());
	assert(xSpace.size() == xList.size() && ySpace.size() == yList.size() && "Spaces and coordinate lists don't match up");
	std::vector<std::pair<int, int>> xSpaces;
	std::vector<std::pair<int, int>> ySpaces;
	int v = 0;
	/**
	  creates pairs <startPos, endPos> to represent groups of equal spacing
	  when a large space is found, a new group is added to the list
	  the largest group is deemed as the true search area
	  calculation is down twice, once in the x direction, once in the y direction
	*/

	xSpaces.push_back(std::make_pair(seekImage.width(), 0));
	for (int i = 0; i < xList.size(); ++i) {
		if (abs(xSpace[i] - xMedian) <= .5 * xMedian || (i + 1 < xList.size() && abs(xSpace[i + 1] - xMedian) <= .5 * xMedian)) {
			xSpaces[v].first = min(xList[i], xSpaces[v].first);
			xSpaces[v].second = max(xList[i], xSpaces[v].second);
		}
		else {
			xSpaces.push_back(std::make_pair(seekImage.width(), 0));
			++v;
			xSpaces[v].first = min(xList[i], xSpaces[v].first);
			xSpaces[v].second = max(xList[i], xSpaces[v].second);
		}
	}
	v = 0;
	ySpaces.push_back(std::make_pair(seekImage.height(), 0));
	for (int i = 0; i < yList.size(); ++i) {
		if (abs(ySpace[i] - yMedian) <= .5 * yMedian || (i + 1 < yList.size() && abs(ySpace[i + 1] - yMedian) <= .5 * yMedian)) {
			ySpaces[v].first = min(yList[i], ySpaces[v].first);
			ySpaces[v].second = max(yList[i], ySpaces[v].second);
		}
		else {
			++v;
			ySpaces.push_back(std::make_pair(seekImage.height(), 0));
			ySpaces[v].first = min(yList[i], ySpaces[v].first);
			ySpaces[v].second = max(yList[i], ySpaces[v].second);
		}
	}
	//* Finds the biggest group of relativly equal spacing and determines that as the search area

	std::pair<int, int> xSpaceFinal{ 0, 0 }, ySpaceFinal{ 0, 0 };
	for (auto it : xSpaces)
		if (it.second - it.first > xSpaceFinal.second - xSpaceFinal.first)
			xSpaceFinal = it;
	for (auto it : ySpaces)
		if (it.second - it.first > ySpaceFinal.second - ySpaceFinal.first)
			ySpaceFinal = it;
	printf("Found search area\n");
#ifdef DEBUGGING
	//* draw search area
	seekImage.drawRect({ xSpaceFinal.first, ySpaceFinal.first }, { xSpaceFinal.second, ySpaceFinal.second }, { 255, 0, 0 });
#endif
	/**
	  For all bounding boxes in the search area, draw central point on image
	  Perform the hough transform to find horizontal lines which will be the rows of the search
	  For all lines, identify the bounding boxes it passes through and sort them based on x-coordinate
	  Add each row to grid and reset image to its saved state before drawing occurred
	*/

	std::vector<Square> tempList;
	int itCount = 0;
	IMG::ImgMemento backup = seekImage.getMemento();
	seekImage.clear();
	for (auto it = foundLetters.cbegin(); it != foundLetters.cend(); ++itCount, ++it) {
		if ((it->x != -1 && it->y != -1) && it->x + it->width >= xSpaceFinal.first - 20 && it->x <= xSpaceFinal.second + 20 && it->y + it->height >= ySpaceFinal.first - 20 && it->y <= ySpaceFinal.second + 20) {
#ifdef DEBUGGING
			seekImage.drawRect({ it->x, it->y }, { it->x + it->width, it->y + it->height }, { 0, 0, 255 });
#endif
//			printf("(%d, %d)\n", it->x, it->y);
			tempList.push_back(*it);
			int px = (it->x + it->x + it->width) / 2;
			int py = (it->y + it->y + it->height) / 2;
			seekImage.rect({ px - 2, py - 2 }, { px + 2, py + 2 }, { 255, 255, 255 });
		}
	}
	printf("Init all vectors\n");
	Hough detectGrid;
	detectGrid.transform(seekImage);
	auto lines = detectGrid.getLines(80);
	printf("Detected: %d\n", lines.size());
	std::vector<std::pair<point, point>> linesFinal;
	for (auto l : lines) {
		if ((abs(l.second.x - l.first.x) > seekImage.width() * .5 && abs(l.second.y - l.first.y) < seekImage.height() * .5)) {
			bool add = true;
			for(auto lf : linesFinal)
				if (isCloseTo(Line{ lf.first, lf.second }, Line{ l.first, l.second })) { add = false; break; }
			if (add) {
				seekImage.drawLine(l.first, l.second, { 255, 0, 0 });
				linesFinal.push_back(l);
			}
		}
	}
	std::vector<Square> row;
	for (auto line : linesFinal) {
//		printf("Y: %d\n", line.first.y);
		for (auto it = tempList.cbegin(); it != tempList.end(); ++it) {
			auto sq = *it;
			if (lineRectIntersection(line.first, line.second, { sq.x, sq.y }, { sq.x + sq.width, sq.y + sq.height })) {
				row.push_back(sq);
//				point center = { (sq.x + sq.x + sq.width) / 2, (sq.y + sq.y + sq.height) / 2 };
//				seekImage.rect({ center.x - 2, center.y - 2 }, { center.x + 2, center.y + 2 }, { 0, 255, 0 });
			}
		}
		letterXSort(row);
		characterLocations.push_back(row);
		row.clear();
	}
	characters.resize(characterLocations.size());
	for (int i = 0; i < characterLocations.size(); ++i)
		characters[i].resize(characterLocations[i].size(), '?');
	rows = characterLocations.size();
	columns = characterLocations[0].size();
	printf("Grid of %d rows by %d columns \n", rows, columns);
//	auto rc = characterLocations[0][2];
//	point cr = { (rc.x + rc.x + rc.width) / 2, (rc.y + rc.y + rc.height) / 2 };
//	seekImage.rect({ cr.x - 2, cr.y - 2 }, { cr.x + 2, cr.y + 2 }, { 0, 255, 0 });
	seekImage.loadFrom(backup);
	printf("Done with init\n");

}
#define SAMPLE_WIDTH 10
#define SAMPLE_HEIGHT 10
bool isBmp(char * path) {
	char * pointer = path;
	char ending[] = ".bmp";
	int match = 0;
	while (*pointer != '\0') {
		if (*pointer == ending[match])
			match++;
		else match = 0;
		pointer++;
	}
	return match == sizeof(ending) - 1 ? true : false;
}
void CV::SearchGrid::identifyLetters()
{
	std::vector<std::unique_ptr<KnownSample>> knownLetters;
	WIN32_FIND_DATA fData;
	HANDLE hand = FindFirstFile("C:\\Users\\stephen\\Documents\\Visual Studio 2015\\Projects\\PuzzleSolver\\PuzzleSolver\\letters\\*", &fData);
	char fileRead[MAX_PATH];
	while (hand != INVALID_HANDLE_VALUE) {
		if (isBmp(fData.cFileName)) {
			printf("%s \n", fData.cFileName);
			sprintf_s(fileRead, MAX_PATH, "C:\\Users\\stephen\\Documents\\Visual Studio 2015\\Projects\\PuzzleSolver\\PuzzleSolver\\letters\\%s", fData.cFileName);
			if (!isalpha(fData.cFileName[0])) printf("img name is not a letter! \n");
			knownLetters.push_back(std::make_unique<KnownSample>(new Image(fileRead), (char)toupper(fData.cFileName[0])));
		}
		if (FindNextFile(hand, &fData) == FALSE) break;
	}
	FILE * file = fopen("C:\\Users\\stephen\\Documents\\Visual Studio 2015\\Projects\\PuzzleSolver\\PuzzleSolver\\data.ml", "rb");
	int testNumber = 0;
	printf("Letter size: %d \n", knownLetters.size());
	if (file != NULL) {
		fseek(file, 0, SEEK_END);
		long size = ftell(file);
		fseek(file, 0, SEEK_SET);
		if (size % 101) printf("Not a multiple of 101 bytes... \n");
		long iterations = size / 101;
		printf("Data file size: %d, iterations: %d \n", size, iterations);
		for (int i = 0; i < iterations; i++) {
			Image * newImg = new Image(SAMPLE_WIDTH, SAMPLE_HEIGHT);
			unsigned char * buffer = new unsigned char[SAMPLE_WIDTH * SAMPLE_HEIGHT];
			fread(buffer, 1, 100, file);
			for (int i = 0; i < 100; i++) {
				int x = i % 10;
				int y = i / 10;
				newImg->setPixel(x, y, { buffer[i], buffer[i], buffer[i] });
			}
			char letter;
			fread(&letter, 1, 1, file);
			printf("Reading %c \n", letter);
			knownLetters.push_back(std::make_unique<KnownSample>(newImg, letter));
			testNumber = knownLetters.size() - 1;
//			newImg->saveBmp("testReadLetter.bmp");
			delete buffer;
		}

	}
	else printf("Data.ml not found!\n");
	printf("testNumber: %d \n", testNumber);
	for (int y = 0; y < rows; y++) {
		for (int x = 0; x < columns; ++x) {
			std::pair<char, double> minDifference = std::make_pair(0, DBL_MAX);
			if (characterLocations[y][x].width > 3 && characterLocations[y][x].height > 3 && seekImage.xInBounds(characterLocations[y][x].x) && seekImage.yInBounds(characterLocations[y][x].y)) {
				std::unique_ptr<Image> letter = std::make_unique<Image>(characterLocations[y][x].width, characterLocations[y][x].height);
				for (int x2 = 0; x2 < characterLocations[y][x].width; x2++) {
					for (int y2 = 0; y2 < characterLocations[y][x].height; y2++) {
						letter->setPixel(x2, y2, (Color)seekImage.getPixel(x2 + characterLocations[y][x].x, y2 + characterLocations[y][x].y));
					}
				}
//				if (y == 1 && x == 2) letter->saveBmp("testUnknownPreScale.bmp");
				letter->scaleTo(SAMPLE_WIDTH, SAMPLE_HEIGHT);
//				if (y == 1 && x == 2) letter->saveBmp("testUnknown.bmp");
				for (int j = 0; j < knownLetters.size(); j++) {
					double diffScore = 0;
					for (int k = 0; k < SAMPLE_WIDTH * SAMPLE_HEIGHT; k++) {
						int x2 = k % SAMPLE_WIDTH;
						int y2 = k / SAMPLE_WIDTH;
						diffScore += pow(letter->getPixel(x2, y2).avg() - knownLetters[j]->image->getPixel(x2, y2).avg(), 2);
					}
					if (diffScore < minDifference.second)
						minDifference = std::make_pair(knownLetters[j]->letter, diffScore);
				}
//				addLetter(minDifference.first, locations[i].x, locations[i].y);
				characters[y][x] = minDifference.first;
//				matchLetter(std::shared_ptr<KnownSample>(new KnownSample{ letter, minDifference.first }));
			}
		}
	}
	iterateRowbyRow();
 }
/**
 * Using the initialized characters[][] to search for diagnol, vertical, horizontal and backwords words
 * Will select the closest match and if there is a letter that does not match up correctly, will save that in the data.ml file
 * @param words list of words (strings) to search
 * @see SearchGrid
*/
 void CV::SearchGrid::search(std::vector<std::string> words)
 {
	 std::map<std::string, int> foundWords;
	 std::map<std::string, Line> foundWordsPos;
	 std::map<std::string, std::vector<POINT>> possibleLetterLocations;
	 int maxColumns = columns - 1;
	 int maxRows = rows - 1;
	 printf("testGetLetters: %c %c %c \n", getLetter(maxColumns, maxRows), getLetter(maxColumns - 1, maxRows - 1), getLetter(maxColumns - 2, maxRows - 2));
	 for (std::string word : words) {
		 std::vector<std::string> possibilities;
		 std::vector<std::vector<POINT>> possibleCharLocations;
		 std::vector<Line> lines;
		 for (int i = 0; i < (maxRows + 1) * (maxColumns + 1); i++) {
			 int x = i % (maxColumns + 1);
			 int y = i / (maxColumns + 1);
			 char letter = characters[y][x];
			 for (int j = 0; j < word.size(); j++) {
				 if (letter == word[j]) {
					 if (/*x <= (maxColumns + 1) - (word.size() - j)*/ true) {
						 Line l;
						 l.start = { x - j, y };
						 l.end = {  x + (int)word.size() - j - 1, y };
						 if (!l.outOfBounds(maxRows + 1, maxColumns + 1)) {
							 std::stringstream p;
							 std::vector<POINT> pts;
							 for (int k = -j; k < word.size() - j; k++) {
								 p << getLetter(x + k, y);
								 pts.push_back({ x + k, y });
							 }
							 possibilities.push_back(p.str());
							 possibleCharLocations.push_back(pts);
							 lines.push_back(l);
						 }
					 }
					 if (/*y <= (maxRows + 1) - (word.size() - j)*/ true) {
						 Line l;
						 std::stringstream s;
						 l.start = { x, y - j };
						 l.end = { x, y + (int)word.size() - j - 1 };
						 if (!l.outOfBounds(maxRows + 1, maxColumns + 1)) {
							 std::vector<POINT> pts;
							 for (int k = -j; k < word.size() - j; k++) {
								 s << getLetter(x, y + k);
								 pts.push_back({ x, y + k });
							 }
							 possibilities.push_back(s.str());
							 possibleCharLocations.push_back(pts);
							 lines.push_back(l);
						 }
					 }
#pragma region diagnol
					 if (/*x <= (maxColumns + 1) - (word.size() - j) && y + 1 >= word.size() - j*/ true) {
						 Line l;
						 std::stringstream s;
						 l.start = { x - j, y + j };
						 l.end = { x + (int)word.size() - j - 1, y - ((int)word.size() - j - 1)};
						 if (!l.outOfBounds(maxRows + 1, maxColumns + 1)) {
							 std::vector<POINT> pts;
							 for (int k = -j; k < word.size() - j; k++) {
								 s << getLetter(x + k, y - k);
								 pts.push_back({ x + k, y - k });
							 }
							 possibilities.push_back(s.str());
							 possibleCharLocations.push_back(pts);
							 lines.push_back(l);
						 }
					 }
					 if (/*x <= (maxColumns + 1) - (word.size() - j) && y <= maxRows - (word.size() - j) */ true) {
						 Line l;
						 std::stringstream s;
						 l.start = { x - j, y - j };
						 l.end = { x + (int)word.size() - j - 1, y + (int)word.size() - j - 1 };
						 if (!l.outOfBounds(maxRows + 1, maxColumns + 1)) {
							 std::vector<POINT> pts;
							 for (int k = -j; k < word.size() - j; k++) {
								 s << getLetter(x + k, y + k);
								 pts.push_back({ x + k, y + k });
							 }
							 possibilities.push_back(s.str());
							 possibleCharLocations.push_back(pts);
							 lines.push_back(l);
						 }
					 }
					 if (/*x + 1 >= word.size() - j  && y + 1 >= word.size() - j*/ true) {
						 Line l;
						 std::stringstream s;
						 l.start = { x + j, y + j };
						 l.end = { x - ((int)word.size() - j - 1), y - ((int)word.size() - j - 1) };
						 if (!l.outOfBounds(maxRows + 1, maxColumns + 1)) {
							 std::vector<POINT> pts;
							 for (int k = -j; k < word.size() - j; k++) {
								 s << getLetter(x - k, y - k);
								 pts.push_back({ x - k, y - k });
							 }
							 possibilities.push_back(s.str());
							 possibleCharLocations.push_back(pts);
							 lines.push_back(l);
						 }
					 }
					 if (/*x + 1 >= word.size() - j && y <= (maxRows + 1) - (word.size() - j)*/ true) {
						 Line l;
						 std::stringstream s;
						 l.start = { x + j, y - j };
						 l.end = { x - ((int)word.size() - j - 1), y + (int)word.size() - j - 1 };
						 if (!l.outOfBounds(maxRows + 1, maxColumns + 1)) {
							 std::vector<POINT> pts;
							 for (int k = -j; k < word.size() - j; k++) {
								 s << getLetter(x - k, y + k);
								 pts.push_back({ x - k, y + k });
							 }
							 possibilities.push_back(s.str());
							 possibleCharLocations.push_back(pts);
							 lines.push_back(l);
						 }
					 }
#pragma endregion
					 if (/*x + 1 >= word.size() - j*/ true) {
						 Line l;
						 std::stringstream s;
						 l.start = { x + j, y };
						 l.end = { x - ((int)word.size() - j - 1), y };
						 if (!l.outOfBounds(maxRows + 1, maxColumns + 1)) {
							 std::vector<POINT> pts;
							 for (int k = -j; k < word.size() - j; k++) {
								 s << getLetter(x - k, y);
								 pts.push_back({ x - k, y });
							 }
							 possibilities.push_back(s.str());
							 possibleCharLocations.push_back(pts);
							 lines.push_back(l);
						 }
					 }
					 if (/*y + 1 >= word.size() - j*/ true) {
						 Line l;
						 std::stringstream s;
						 l.start = { x, y + j };
						 l.end = { x, y - ((int)word.size() - j - 1) };
						 if (!l.outOfBounds(maxRows + 1, maxColumns + 1)) {
							 std::vector<POINT> pts;
							 for (int k = -j; k < word.size() - j; k++) {
								 s << getLetter(x, y - k);
								 pts.push_back({ x, y - k });
							 }
							 possibilities.push_back(s.str());
							 possibleCharLocations.push_back(pts);
							 lines.push_back(l);
						 }
					 }
					 
				 }
			 }
/**/	}
		std::pair<int, int> maxSame = std::make_pair(0, round(word.size() * 0.3));
		for (int j = 0; j < possibilities.size(); j++) {
			int same = 0;
			for (int k = 0; k < word.size(); k++) {
				if (word[k] == possibilities[j][k])
					same++;
			}
			if (same > maxSame.second)
				maxSame = std::make_pair(j, same);
		}
		if (maxSame.second != INT_MIN && possibilities.size() > 0) {
			if (foundWords.find(word) != foundWords.end()) {
				if (foundWords.at(word) < maxSame.second) {
					foundWords[word] = maxSame.second;
					foundWordsPos[word] = lines[maxSame.first];
					possibleLetterLocations[word] = possibleCharLocations[maxSame.first];
				}
			}
			else {
				foundWords.insert(std::make_pair(word, maxSame.second));
				foundWordsPos.insert(std::make_pair(word, lines[maxSame.first]));
				possibleLetterLocations.insert(std::make_pair(word, possibleCharLocations[maxSame.first]));
			}
		}
	 }
	 for (auto it = foundWordsPos.begin(); it != foundWordsPos.end(); it++) {
		 printf("Found word: %s Score: %d / %d \n", it->first.c_str(), foundWords[it->first], it->first.size());
		 Line line = it->second;
		 Square start = characterLocations[line.start.y][line.start.x];
		 Square end = characterLocations[line.end.y][line.end.x];
		 point startpt = { start.x + (start.width / 2), start.y + (start.height / 2) };
		 point endpt = { end.x + (end.width / 2), end.y + (end.height / 2) };
		 printf("Line from (%d, %d) to (%d, %d) \n", startpt.x, startpt.y, endpt.x, endpt.y);
		 printf("Which is (%d, %d) to (%d, %d) \n", line.start.x, line.start.y, line.end.x, line.end.y);
		 double dist = sqrt((endpt.x - startpt.x) * (endpt.x - startpt.x) + (endpt.y - startpt.y) * (endpt.y - startpt.y));
		 double theta = acos((endpt.x - startpt.x) / dist);
		 if ((line.start.x != line.end.x && line.start.y > line.end.y) || (line.start.x == line.end.x && line.start.y > line.end.y)) theta *= -1;
		 matrix m = {
			 cos(theta),	-sin(theta),
			 sin(theta),	 cos(theta)
		 };
		 for (int i = 0; i < floor(dist); i++) {
			 point p = { i, 0 };
			 point pr = m * p;
			 pr.x += startpt.x;
			 pr.y += startpt.y;
			 seekImage.setPixel({ { 255, 0, 0 }, pr });
		 }
		 
	 }
	 FILE * file = fopen("C:\\Users\\stephen\\Documents\\Visual Studio 2015\\Projects\\PuzzleSolver\\PuzzleSolver\\data.ml", "ab");
	 for (auto it = foundWords.begin(); it != foundWords.end(); it++) {
		 //implement 'machine learning'
		 if (possibleLetterLocations[it->first].size() > 0 && it->second != it->first.size()) {
			 for (int i = 0; i < it->first.size(); i++) {
				 int x = possibleLetterLocations[it->first][i].x;
				 int y = possibleLetterLocations[it->first][i].y;
				 if (it->first[i] != getLetter(x, y)) {
					 printf("Mismatch %c\n", getLetter(x, y));
					 Square location = characterLocations[y][x];
					 std::unique_ptr<Image> k = std::make_unique<Image>(location.width, location.height);
					 for (int i = 0; i < location.width; ++i) {
						 for (int j = 0; j < location.height; ++j) {
							 k->setPixel(i, j, static_cast<Color>(seekImage.getPixel(i + location.x, j + location.y)));
						 }
					 }
					 k->scaleTo(SAMPLE_WIDTH, SAMPLE_HEIGHT);
					 for (int i = 0; i < SAMPLE_WIDTH * SAMPLE_HEIGHT; i++) {
						 int x = i % SAMPLE_WIDTH;
						 int y = i / SAMPLE_WIDTH;
						 Color c = k->getPixel(x, y);
						 unsigned char wc = c.avg();
						 fwrite(&wc, 1, 1, file);
					 }
					 char c = it->first[i];
					 fwrite(&c, 1, 1, file);
					 printf("Saving %c...\n", c);
				 }
			 }
		 }
		 else if (possibleLetterLocations[it->first].size() <= 0) {
			 printf("%s has no possible letters??\n", it->first.c_str());
		 }
	 }
	 fclose(file);
	 RECT r;
	 GetClientRect(gui::GUI::useWindow(), &r);
	 InvalidateRect(gui::GUI::useWindow(), &r, TRUE);
 }
 /**
  * Gets letter with closest eulicidean distance from the point
  * @param p point to test
  * @see Math::point
 */
 std::pair<char, CV::Square> CV::SearchGrid::getLetterNearest(Math::point p)
 {
	 int minDist = int_max, xPos = 0, yPos = 0;
	 for (int i = 0; i < rows; ++i) {
		 for (int j = 0; j < columns; ++j) {
			 point test = { (characterLocations[i][j].x + characterLocations[i][j].x + characterLocations[i][j].width) / 2,
							(characterLocations[i][j].y + characterLocations[i][j].y + characterLocations[i][j].height) / 2 };
			 int dist = (test.x - p.x) * (test.x - p.x) + (test.y - p.y) * (test.y - p.y);
			 if (dist < minDist) {
				 minDist = dist;
				 xPos = j;
				 yPos = i;
			 }
		 }
	 }
	 return std::pair<char, Square>(characters[yPos][xPos], characterLocations[yPos][xPos]);
 }
 /**
  * Saves a set of Squares with its corresponding character from knowns
  * @param locations array of Squares to correlate to letters
  * @param knowns array of characters to correlate to squares
  * @param firstKnown offset of locations, locations[firstKnown] correlates to knowns[0]
 */
 void CV::augmentDataSet(std::vector <CV::Square> locations, std::vector<char> knowns, IMG::Img & img, int firstKnown)
 {
	 int size = min(locations.size(), firstKnown + knowns.size());
	 for (int i = firstKnown; i < size; i++) {
		 ImgPtr image = new Image(locations[i].width, locations[i].height);
		 for (int j = 0; j < locations[i].width * locations[i].height; j++) {
			 int x = j % locations[i].width;
			 int y = j / locations[i].width;
			 image->setPixel(x, y, (Color)img.getPixel(x + locations[i].x, y + locations[i].y));
		 }
		 char path[MAX_PATH];
		 srand(clock());
		 int id = rand() % 1000;
		 sprintf_s(path, MAX_PATH, "C:\\Users\\stephen\\Documents\\Visual Studio 2015\\Projects\\Puzzle Solver + GDI API\\Puzzle Solver + GDI API\\letters\\%c %drw.bmp", knowns[i], id);
		 image->scaleTo(SAMPLE_WIDTH, SAMPLE_HEIGHT);
		 image->saveBmp(path);
	 }
 }
 std::unique_ptr<IMG::Img> CV::cannyEdgeDetection(IMG::Img & img, const double upperThreshold, const double lowerThreshold)
 {
	 std::vector<double> directions, magnitudes;
	 auto resultant = sobelEdgeDetection(img, &magnitudes, &directions);
	 for (int i = 0; i < directions.size(); ++i)
		 while (directions[i] < 0) directions[i] += 180;
	 double minMag = std::numeric_limits<double>::max();
	 double maxMag = std::numeric_limits<double>::min();
	 for (int i = 0; i < magnitudes.size(); ++i) {
		 minMag = min(minMag, magnitudes[i]);
		 maxMag = max(maxMag, magnitudes[i]);
	 }
	 resultant->clear();
	 for (int i = 0; i < img.width() * img.height(); ++i) {
		 int x = i % img.width();
		 int y = i / img.width();
		 if (abs((magnitudes[i] - minMag) / (maxMag - minMag)) < lowerThreshold) {
			 continue;
		 }
		 bool isEdge = true;
		 if (directions[i] > 112.5 && directions[i] <= 157.5) {
			 if (y > 0 && x < img.width() - 1 && magnitudes[i] <= magnitudes[(y - 1) * img.width() + (x + 1)]) isEdge = false;
			 if (y < img.height() - 1 && x > 0 && magnitudes[i] <= magnitudes[(y + 1) * img.width() + (x - 1)]) isEdge = false;
		 }
		 else if (directions[i] > 67.5 && directions[i] <= 112.5) {
			 if (y > 0 && magnitudes[i] <= magnitudes[(y - 1) * img.width() + x]) isEdge = false;
			 if (y < img.height() - 1 && magnitudes[i] <= magnitudes[(y + 1) * img.width() + x]) isEdge = false;
		 }
		 else if (directions[i] > 22.5 && directions[i] <= 67.5) {
			 if (y > 0 && x > 0 && magnitudes[i] <= magnitudes[(y - 1) * img.width() + (x - 1)]) isEdge = false;
			 if (y < img.height() - 1 && x < img.width() - 1 && magnitudes[i] <= magnitudes[(y + 1) * img.width() + (x + 1)]) isEdge = false;
		 }
		 else {
			 if (x > 0 && magnitudes[i] <= magnitudes[y * img.width() + (x - 1)]) isEdge = false;
			 if (x < img.width() - 1 && magnitudes[i] <= magnitudes[y * img.width() + (x + 1)]) isEdge = false;
		 }
		 if (isEdge) {
			 if (abs((magnitudes[i] - minMag) / (maxMag - minMag)) >= upperThreshold)
				 resultant->setPixel(255, 255, 255, x, y);
			 else
				 resultant->setPixel(128, 128, 128, x, y);
		 }
	 }
	 bool imageChanged;
	 do {
		 imageChanged = false;
		 for (int i = 0; i < img.width() * img.height(); ++i) {
			 int x = i % img.width();
			 int y = i / img.width();
			 if (resultant->getPixel(x, y).avg() == 128) {
				 if (directions[i] > 112.5 && directions[i] <= 157.5) {
					 if (y > 0 && x < img.width() - 1 && resultant->getPixel(x + 1, y - 1).avg() == 255) {
						 resultant->setPixel(255, 255, 255, x, y);
						 imageChanged = true;
					 }
					 if (y < img.height() - 1 && x > 0 && resultant->getPixel(x - 1, y + 1).avg() == 255) {
						 resultant->setPixel(255, 255, 255, x, y);
						 imageChanged = true;
					 }
				 }
				 else if (directions[i] > 67.5 && directions[i] <= 112.5) {
					 if (y > 0 && resultant->getPixel(x, y - 1).avg() == 255) {
						 resultant->setPixel(255, 255, 255, x, y);
						 imageChanged = true;
					 }
					 if (y < img.height() - 1 && resultant->getPixel(x, y + 1).avg() == 255) {
						 resultant->setPixel(255, 255, 255, x, y);
						 imageChanged = true;
					 }
				 }
				 else if (directions[i] > 22.5 && directions[i] <= 67.5) {
					 if (y > 0 && x > 0 && resultant->getPixel(x - 1, y - 1).avg() == 255) {
						 resultant->setPixel(255, 255, 255, x, y);
						 imageChanged = true;
					 }
					 if (y < img.height() - 1 && x < img.width() - 1 && resultant->getPixel(x + 1, y + 1).avg() == 255) {
						 resultant->setPixel(255, 255, 255, x, y);
						 imageChanged = true;
					 }
				 }
				 else {
					 if (x > 0 && resultant->getPixel(x - 1, y).avg() == 255) {
						 resultant->setPixel(255, 255, 255, x, y);
						 imageChanged = true;
					 }
					 if (x < img.width() - 1 && resultant->getPixel(x + 1, y).avg() == 255) {
						 resultant->setPixel(255, 255, 255, x, y);
						 imageChanged = true;
					 }
				 }
			 }
		 }
	 } while (imageChanged);
	 for (int i = 0; i < img.width() * img.height(); ++i) {
		 int x = i % img.width();
		 int y = i / img.width();
		 if (resultant->getPixel(x, y).avg() == 128)
			 resultant->setPixel(0, 0, 0, x, y);
	 }

	 return resultant;

 }
 std::unique_ptr<IMG::Img> CV::sobelEdgeDetection(IMG::Img & img, std::vector<double> * magnitudes, std::vector<double> * directions)
 {
	 Kernel sobelY({
		 -1, -2, -1,
		  0,  0,  0,
		  1,  2,  1 });
	 Kernel sobelX({
		 -1, 0, 1,
		 -2, 0, 2,
		 -1, 0, 1 });
	 auto gx = sobelX.apply_matrix(img);
	 auto gy = sobelY.apply_matrix(img);
	 std::vector<double> magnitude(img.width() * img.height());
	 double maxMag = std::numeric_limits<double>::min();
	 for (int i = 0; i < img.width() * img.height(); ++i) {
		 int x = i % img.width();
		 int y = i / img.width();
		 magnitude[i] = sqrt(gx[i][0] * gx[i][0] + gy[i][0] * gy[i][0]);
		 if (directions != nullptr) directions->push_back(Math::degrees(atan(gy[i][0] / gx[i][0])));
		 maxMag = max(maxMag, magnitude[i]);
	 }
	 auto newImage = std::make_unique<IMG::Img>();
	 newImage->createNew(img.width(), img.height());
	 newImage->clear();
	 for (int i = 0; i < img.width() * img.height(); ++i) {
		 int x = i % img.width();
		 int y = i / img.width();
		 double val = magnitude[i] * 255.0 / maxMag;
		 channel c = static_cast<channel>(val);
		 newImage->setPixel(c, c, c, x, y);
	 }
	 if (magnitudes != nullptr) *magnitudes = magnitude;
	 return newImage;
 }
 /**
  * Determines if too lines are close to one another
  * @param lines a and b to test
  * @param buffer less than this distance to determine if two lines are close
  * @return true if one of the enpoints are within buffer distance or if there is an intersection and an enpoint is within 15 units
 */
 bool CV::isCloseTo(Line & a, Line & b, int buffer)
 {
	 if ((a.start.x - b.start.x) * (a.start.x - b.start.x) + (a.start.y - b.start.y) * (a.start.y - b.start.y) < buffer * buffer)
//		if ((a.end.x - b.end.x) * (a.end.x - b.end.x) + (a.end.y - b.end.y) * (a.end.y - b.end.y) < buffer * buffer)
			return true;
	 if ((a.start.x - b.end.x) * (a.start.x - b.end.x) + (a.start.y - b.end.y) * (a.start.y - b.end.y) < buffer * buffer)
//		 if ((a.end.x - b.start.x) * (a.end.x - b.start.x) + (a.end.y - b.start.y) * (a.end.y - b.start.y) < buffer * buffer)
			 return true;
	 point p = lineIntersection(a.start, a.end, b.start, b.end);
	 if (p.x != int_max && p.y != int_min) {
		 if (((a.start.x - b.start.x) * (a.start.x - b.start.x) + (a.start.y - b.start.y) * (a.start.y - b.start.y) < 15 * 15) ||
			 ((a.end.x - b.start.x) * (a.end.x - b.start.x) + (a.end.y - b.start.y) * (a.end.y - b.start.y) < 15 * 15))
			 return true;
	 }
	 return false;
 }
 //* insertion sort based off x coordinate
 void CV::letterXSort(std::vector<Square>& row)
 {
	 for (int i = 1; i < row.size(); ++i) {
		 int j = i - 1;
		 auto temp = row[i];
		 while (j >= 0 && temp.x <= row[j].x) {
			 row[j + 1] = row[j];
			 --j;
		 }
		 row[j + 1] = temp;


	 }
 }
 /**
  * Uses integral image to find a suitable origin of rotation
  * Ideally the origin is in the middle of a letter in a row in the search.
  * @return Math::point of the rotational origin
  */
point CV::getOrigin(IMG::Img & img)
{
	int startY = 0;
	int startX = 0;
	int totalAvg = img.integralImageValue(img.width() - 1, img.height() - 1) - img.integralImageValue(0, img.height() - 1) - img.integralImageValue(img.width() - 1, 0) + img.integralImageValue(0, 0);
	totalAvg /= (img.width() * img.height());
	printf("%d \n", totalAvg);
	for (int i = 0; i < img.width() * img.height(); i++) {
		int x = i % img.width();
		int y = i / img.width();
		Color c = (Color)img.getPixel(x, y);
		if (c.avg() < 100) {
			int xp = min(static_cast<unsigned int>(x + (.15 * img.width())), img.width() - 1);
			int yp = min(static_cast<unsigned int>(y + (.15 * img.height())), img.height() - 1);
			int xm = max(x - (.15 * img.width()), 0.0);
			int ym = max(y - (.15 * img.height()), 0.0);
			if (xp != (int)(x + (.15 * img.width())))
				xm -= (x + .15 * img.width()) - (img.width() - 1);
			else if (xm != (int)(x - (.15 * img.width())))
				xp += 0 - (x - .15 * img.width());
			if (yp != (int)(y + .15 * img.height()))
				ym -= (y + .15 * img.height()) - (img.height() - 1);
			else if (ym != (int)(y - .15 * img.height()))
				yp += 0 - (y - .15 * img.height());
			int avg = img.integralImageValue(xp, yp) - img.integralImageValue(xm, yp) - img.integralImageValue(xp, ym) - img.integralImageValue(xm, ym);
			printf("Avg: %d \n", avg);
			avg /= (xp - xm) * (yp - ym);
			avg = abs(avg);
			if (avg < totalAvg) {
				printf("Area avg: %d \n", avg);
				printf("Start (%d,%d) \n", x, y);
				startY = y + 7; //puts point in the middle of most letters
				startX = x;
				break;
			}
		}
	}
	return { startX, startY };
}
/**
 * Rotates entire set of pixels about origin by angle theta
 * Also enlarges image to image diagnol x image diagnol to prevent a part of the image being rotated off the canvas
 * @param theta angle
 * @param origin point to rotate about
*/
void CV::rotateImage(IMG::Img & img, float theta, point origin)
{
	int diagnol = ceil(sqrt(img.width() * img.width() + img.height() * img.height()));
	std::vector<IMG::color> buffer;
	buffer.reserve(diagnol * diagnol);
	//Debugging
	for (int i = 0; i < diagnol * diagnol; i++) {
		buffer[i] = IMG::color{ 255, 255, 255 };
	}
	//end debugging
	//mapping from destination back to source and picks out source pixel to reduce the "holes" in the image
	matrix rotMat = rotationMatrix(-theta);
	for (int i = 0; i < diagnol * diagnol; i++) {
		int x = i % diagnol;
		int y = i / diagnol;
		x -= (diagnol - img.width()) / 2;
		y -= (diagnol - img.width()) / 2;
		point rtPt = { x - origin.x, y - origin.y };
		point rotated = rotMat * rtPt;
		rotated += origin;
		if (rotated.x > 0 && rotated.x < img.width() && rotated.y > 0 && rotated.y < img.height())
			buffer[i] = img.getPixel(rotated.x, rotated.y);

	}
	img.resize(diagnol, diagnol);
	PAINTSTRUCT p;
	HDC dc = BeginPaint(gui::GUI::useWindow(), &p);
	for (int i = 0; i < diagnol * diagnol; i++) {
		int x = i % diagnol;
		int y = i / diagnol;
		img.setPixel({ buffer[i], x, y });
	}
	EndPaint(gui::GUI::useWindow(), &p);
	RECT r;
	GetClientRect(gui::GUI::useWindow(), &r);
	InvalidateRect(gui::GUI::useWindow(), &r, TRUE);
}
CV::SearchGrid::SearchGrid(IMG::Img & wordSearch) : seekImage(wordSearch)
{
}

/**
 * Sets up SearchGrid for searching for words
 * Erases all past data from a previous search if the object is reused
 * @see init()
 * @see identifyLetters()
*/
void CV::SearchGrid::load(IMG::Img & search)
{
	seekImage = search;
	characters.erase(characters.begin(), characters.end());
	characterLocations.erase(characterLocations.begin(), characterLocations.end());
//	getCharacterLocations();
	init();
	identifyLetters();
}

void CV::SearchGrid::iterateRowbyRow()
{
	printf("Max rows: %d   Max columns: %d \n", rows, columns);
	for (int r = 0; r < rows; r++) {
		for (int c = 0; c < columns; c++) {
			printf("%c ", characters[r][c]);
		}
		printf("\n");
	}
}

bool CV::Letter::operator==(char c)
{
//	std::vector<std::vector<char>> confusionSets;
	if (letter == c) return true;
	return false;
}

bool CV::Line::outOfBounds(int maxRows, int maxColumns)
{
	if (start.x < 0 || start.x >= maxColumns || start.y < 0 || start.y >= maxRows ||
		end.x < 0 || end.x >= maxColumns || end.y < 0 || end.y >= maxRows) return true;
	return false;
}

CV::KnownSample & CV::KnownSample::operator=(const KnownSample & other)
{
	this->letter = other.letter;
	if (image != nullptr)
		delete image;
	image = new Image(other.image->getWidth(), other.image->getHeight());
	for (int i = 0; i < other.image->getWidth() * other.image->getHeight(); i++) {
		int x = i % other.image->getWidth();
		int y = i / other.image->getWidth();
		image->setPixel(x, y, other.image->getPixel(x, y));
	}
}

void CV::Hough::transform(IMG::Img & img)
{
	imageWidth = img.width();
	imageHeight = img.height();
	accumulatorHeight = sqrt(2.0) * (double)max(img.height(), img.width());
	accumulatorWidth = 180;
	if (accumulator.size() > 0) accumulator.clear();
	accumulator.resize(accumulatorWidth);
	for (size_t i = 0; i < accumulatorWidth; ++i)
		accumulator[i].resize(accumulatorHeight);
	center.x = img.width() / 2;
	center.y = img.height() / 2;
	for (int y = 0; y < img.height(); ++y) {
		for (int x = 0; x < img.width(); ++x) {
			if (img.getPixel(x, y).avg() > 230) { //if light (dark in actual image) pixel
				for (int t = 0; t < 180; ++t) {
					//r = x cos0 + y sin0
					double r = ((double)x - center.x) * cos(radians(t)) + ((double)y - center.y) * sin(radians(t));
					accumulator[t][(int)round(r + accumulatorHeight / 2.0)] += 1;
				}
			}
		}
	}
}
/**
 * Get straight lines from hough transform with >= threshold amount of light pixels on it
*/
CV::pointList CV::Hough::getLines(uint32_t threshold)
{
	pointList lines;
	for (int r = 0; r < accumulatorHeight; ++r) {
		for (int t = 0; t < accumulatorWidth; ++t) {
			if (accumulator[t][r] >= threshold) {
				uint32_t max = accumulator[t][r];
				//determine max in 9x9 square
				for (int y = -3; y <= 3; ++y) {
					for (int x = -3; x <= 3; ++x) {
						if (y + r >= 0 && y + r < accumulatorHeight && x + t >= 0 && x + t < accumulatorWidth) {
							if (accumulator[x + t][y + r] > max) {
								max = accumulator[x + t][y + r];
								y = x = 5; //break from both loops
							}
						}
					}
				}
				if (max > accumulator[t][r]) continue; //if this point isn't a local max
				point p1, p2;
				if (t >= 45 && t <= 135) { 
					//y = (r - x cos0) / sin0
					p1.x = 0;
					p1.y = ((r - accumulatorHeight / 2.0) - (p1.x - center.x) * cos(radians(t))) / sin(radians(t)) + center.y;
					p2.x = imageWidth;
					p2.y = ((r - accumulatorHeight / 2.0) - (p2.x - center.x) * cos(radians(t))) / sin(radians(t)) + center.y;
				}
				else {
					p1.y = 0;
					p1.x = ((r - accumulatorHeight / 2.0) - (p1.y - center.y) * sin(radians(t))) / cos(radians(t)) + center.x;
					p2.y = imageHeight;
					p2.x = ((r - accumulatorHeight / 2.0) - (p2.y - center.y) * sin(radians(t))) / cos(radians(t)) + center.x;
				}
				lines.push_back(std::make_pair(p1, p2));
			}
		}
	}
	return lines;
}
/*
 * For debug only, saves data from Hough transform
 * Image is a graph of hough space
*/
void CV::Hough::display(const char * filename) const
{
	IMG::Img img;
	img.createNew(accumulatorWidth, accumulatorHeight);
	img.clear();
	uint32_t maxVal = 0;
	for (size_t x = 0; x < accumulatorWidth; ++x) {
		for (size_t y = 0; y < accumulatorHeight; ++y) {
			maxVal = max(maxVal, accumulator[x][y]);
		}
	}

	for (size_t x = 0; x < accumulatorWidth; ++x) {
		for (size_t y = 0; y < accumulatorHeight; ++y) {
			uint32_t intensity = accumulator[x][y];
			double colorVal = intensity * 255.0 / (double)maxVal;
			img.setPixel({ IMG::color{(unsigned char)colorVal, (unsigned char)colorVal, (unsigned char)colorVal}, Math::point{(int)x, (int)y} });
		}
	}

	img.saveAsBmp(filename);
}

CV::Kernel::Kernel(int width, int height) : width(width), height(height)
{
	k.resize(width);
	for (int i = 0; i < width; ++i)
		k[i].resize(height);
}

CV::Kernel::Kernel(std::initializer_list<double> list)
{
	assert(list.size() & 1 && list.size() > 2 && "Only odd sized lists are currently implemented");
	height = width = sqrt(list.size());
	assert(height * width == list.size() && "Only square matrices are currently implemented");
	k.resize(width);
	for (int i = 0; i < width; ++i)
		k[i].resize(height);
	int i = 0;
	for (auto element : list) {
		int x = i % width;
		int y = i / width;
		k[x][y] = element;
		++i;
	}
}

/**
 * Applies the kernel to img. Linearly scales results into 0 - 255 range
 * @return unique_ptr to newly created img
*/
std::unique_ptr<IMG::Img> CV::Kernel::apply(IMG::Img & img)
{
	//need to handle overflow of byte
	int j = width / 2;
	printf("J: %d\n", j);
	auto newImage = std::make_unique<IMG::Img>();
	newImage->createNew(img.width(), img.height());
//	newImage->clear();
	double minSum = std::numeric_limits<double>::max();
	double maxSum = std::numeric_limits<double>::min();
	std::vector<vec3f> mat(img.width() * img.height());
	for (int i = 0; i < img.width() * img.height(); ++i) {
		int x = i % img.width();
		int y = i / img.width();
		double sumRed = 0, sumGreen = 0, sumBlue = 0;
//		printf("(%d, %d) --- ", x, y);
		for (int y1 = -j; y1 <= j; ++y1) {
			for (int x1 = -j; x1 <= j; ++x1) {
				if (x + x1 >= 0 && x + x1 < img.width() && y + y1 >= 0 && y + y1 < img.height()) {
					IMG::color c = img.getPixel({ x + x1, y + y1 });
					double r = c.red * k[x1 + j][y1 + j];
					sumGreen += c.green * k[x1 + j][y1 + j];
					sumBlue += c.blue * k[x1 + j][y1 + j];
//					printf("(%d, %d)[%d] * (%d, %d)[%.1f] = %.3f ", x + x1, y + y1, c.red, y1 + j, x1 + j, k[y1 + j][x1 + j], r);
					sumRed += r;
				}
			}
		}
//		printf(" = %f\n", sumRed);
		minSum = min(minSum, min(min(sumRed, sumGreen), sumBlue));
		maxSum = max(maxSum, max(sumRed, max(sumGreen, sumBlue)));
		mat[i] = { sumRed, sumGreen, sumBlue };
	}
	printf("Max: %f  Min: %f \n", maxSum, minSum);
	if (minSum < 0) maxSum -= minSum;
	else printf("Min sum >= 0 \n");
	for (int i = 0; i < mat.size(); ++i) {
		int x = i % img.width();
		int y = i / img.width();
//		printf("Sum: %f ", mat[i][0]);
		if(minSum < 0) mat[i] += -minSum;
//		printf("Sum: %f ", mat[i][0]);
		double red = mat[i][0] * 255.0 / maxSum;
		double green = mat[i][1] * 255.0 / maxSum;
		double blue = mat[i][2] * 255.0 / maxSum;
		//		printf("%f \n", red);
		newImage->setPixel(static_cast<channel>(red), static_cast<channel>(green), static_cast<channel>(blue), x, y);
	}
//	printf("\n\n");
	return newImage;
}
/**
 * Applies kernel to img. Linearly scales into 0 - 255 range.
 * Saves result directly to img.
*/
void CV::Kernel::kernelConvolution(IMG::Img & img)
{
	int j = width / 2;
	double minSum = std::numeric_limits<double>::max();
	double maxSum = std::numeric_limits<double>::min();
	std::vector<vec3f> mat(img.width() * img.height());
	for (int i = 0; i < img.width() * img.height(); ++i) {
		int x = i % img.width();
		int y = i / img.width();
		double sumRed = 0, sumGreen = 0, sumBlue = 0;
		for (int y1 = -j; y1 <= j; ++y1) {
			for (int x1 = -j; x1 <= j; ++x1) {
				if (x + x1 >= 0 && x + x1 < img.width() && y + y1 >= 0 && y + y1 < img.height()) {
					IMG::color c = img.getPixel({ x + x1, y + y1 });
					double r = c.red * k[x1 + j][y1 + j];
					sumGreen += c.green * k[x1 + j][y1 + j];
					sumBlue += c.blue * k[x1 + j][y1 + j];
					sumRed += r;
				}
			}
		}
		minSum = min(minSum, min(min(sumRed, sumGreen), sumBlue));
		maxSum = max(maxSum, max(sumRed, max(sumGreen, sumBlue)));
		mat[i] = { sumRed, sumGreen, sumBlue };
	}
	if (minSum < 0) maxSum -= minSum;
	for (int i = 0; i < mat.size(); ++i) {
		int x = i % img.width();
		int y = i / img.width();
		if (minSum < 0) mat[i] += -minSum;
		double red = mat[i][0] * 255.0 / maxSum;
		double green = mat[i][1] * 255.0 / maxSum;
		double blue = mat[i][2] * 255.0 / maxSum;
		img.setPixel(static_cast<channel>(red), static_cast<channel>(green), static_cast<channel>(blue), x, y);
	}
}
/**
 * Applies kernel to img. Does not scale. Use for multi-pass operations
 * @return unscaled raw data matrix. For each element [0] = red, [1] = blue [2] = green
*/
std::vector<Math::vec3f> CV::Kernel::apply_matrix(IMG::Img & img)
{
	int j = width / 2;
	std::vector<vec3f> mat(img.width() * img.height());
	for (int i = 0; i < img.width() * img.height(); ++i) {
		int x = i % img.width();
		int y = i / img.width();
		double sumRed = 0, sumGreen = 0, sumBlue = 0;
		for (int y1 = -j; y1 <= j; ++y1) {
			for (int x1 = -j; x1 <= j; ++x1) {
				if (x + x1 >= 0 && x + x1 < img.width() && y + y1 >= 0 && y + y1 < img.height()) {
					IMG::color c = img.getPixel({ x + x1, y + y1 });
					double r = c.red * k[x1 + j][y1 + j];
					sumGreen += c.green * k[x1 + j][y1 + j];
					sumBlue += c.blue * k[x1 + j][y1 + j];
//					printf("I(%d, %d) * K(%d, %d) = %.2f ", x + x1, y + y1, x1 + j, y1 + j, r);
					sumRed += r;
				}
			}
		}
//		printf("-> %.2f\n", sumRed);
		mat[i] = { sumRed, sumGreen, sumBlue };
	}
	return mat;
}

void CV::Kernel::testShowMatrix(FILE * out)
{
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			fprintf(out, "%f ", k[j][i]);
		}
		fprintf(out, "\n");
	}
}
/**
 * @param scaler scaler to multiply with each element in the kernel
*/
void CV::Kernel::scale(double scaler)
{
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			k[x][y] *= scaler;
		}
	}
}