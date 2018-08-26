#include "OCR.h"
POINT matrixMultiply(float * matrix, POINT vector) {
	POINT result = { 0, 0 };
	result.x = matrix[0] * vector.x + matrix[1] * vector.y;
	result.y = matrix[2] * vector.x + matrix[3] * vector.y;
	return result;
}
float findSkewAngle(Image * img, POINT * origin, Bounds * skewBounds) {
	if (origin == nullptr)
		*origin = getOrigin(img);
	int startX = origin->x;
	int startY = origin->y;
	std::vector<POINT> line;
	int * accumulator = new int[360];
	memset(accumulator, 0, 360 * sizeof(int));
	int diagnol = sqrt(img->getWidth() * img->getWidth() + img->getHeight() * img->getHeight());
	for (int i = 0; i < diagnol; i++) {
		line.push_back({ startX + i, startY });
	}
	for (int theta = 0; theta < 360; theta++) {
		float rotationMatrix[] = {
			cos(radians(theta)), -sin(radians(theta)),
			sin(radians(theta)),  cos(radians(theta))
		};
		for (int i = 0; i < img->getWidth(); i++) {
			POINT linePt = { line[i].x - startX, line[i].y - startY };
			POINT testPoint = matrixMultiply(rotationMatrix, linePt);
			testPoint.x += startX;
			testPoint.y += startY;
			int pixel = img->getPixel(testPoint.x, testPoint.y).avg() < 100 ? 1 : 0;
			if (testPoint.x < 0 || testPoint.x >= img->getWidth() || testPoint.y < 0 || testPoint.y >= img->getHeight()) pixel = 0;
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
		float rotationMatrix[] = {
			cos(radians(theta)), -sin(radians(theta)),
			sin(radians(theta)),  cos(radians(theta))
		};
		for (int i = 0; i < img->getWidth(); i++) {
			POINT linePt = { line[i].x - startX, line[i].y - startY };
			POINT testPoint = matrixMultiply(rotationMatrix, linePt);
			testPoint.x += startX;
			testPoint.y += startY;
			int pixel = img->getPixel(testPoint.x, testPoint.y).avg() < 100 ? 1 : 0;
			if (testPoint.x < 0 || testPoint.x >= img->getWidth() || testPoint.y < 0 || testPoint.y >= img->getHeight()) pixel = 0;
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
	//Testing
/*	for (int i = 0; i < diagnol; i++) {
		float theta = angle;
		float rotationMatrix[] = {
			cos(radians(theta)), -sin(radians(theta)),
			sin(radians(theta)),  cos(radians(theta))
		};
		POINT linePt = { line[i].x - startX, line[i].y - startY };
		POINT drawPoint = matrixMultiply(rotationMatrix, linePt);
		drawPoint.x += startX;
		drawPoint.y += startY;
		if (drawPoint.x > 0 && drawPoint.x < img->getWidth() && drawPoint.y > 0 && drawPoint.y < img->getHeight()) {
			img->setPixel(drawPoint.x, drawPoint.y, Color{ 0, 0, 255 });
		}
	}*/
//	img->setPixel(startX, startY, Color{ 0, 255, 0 });
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
std::vector<Square> getCharacterLocations(Image * img)
{
#pragma region columnSpaces
	int * accumulator = new int[img->getWidth()];
	memset(accumulator, 0, img->getWidth() * sizeof(int));
	std::vector<Space> spaces;
	std::vector<int> spaceSizes;
	int totalIntensity = img->integralImageValue(img->getWidth() - 1, img->getHeight() - 1) - img->integralImageValue(0, img->getHeight() - 1) - img->integralImageValue(img->getWidth() - 1, 0) + img->integralImageValue(0, 0);
	int totalColumnAvg = totalIntensity / img->getWidth();
	int totalRowAvg = totalIntensity / img->getHeight();
	for (int i = 0; i < img->getWidth(); i++) {
		for (int y = 0; y < img->getHeight(); y++) {
			accumulator[i] += img->getPixel(i, y).avg();
		}
	}
	int lastDark = 0;
	for (int i = 0; i <= img->getWidth(); i++) {
		if (i == img->getWidth()) {
			spaces.push_back({ lastDark + 1, i - lastDark });
			spaceSizes.push_back(i - lastDark);
			break;
		}
		if (accumulator[i] < totalColumnAvg) {
			if (i - lastDark > 1) {
				spaces.push_back({ lastDark + 1, i - lastDark });
				spaceSizes.push_back(i - lastDark);
			}
			lastDark = i;
		}
	}
	int avgSpaceSize = 0;
	for (int i = 0; i < spaceSizes.size(); i++) {
		for (int j = 0; j < spaceSizes.size(); j++) {
			if (spaceSizes[j] < spaceSizes[i]) {
				int size = spaceSizes[i];
				spaceSizes[i] = spaceSizes[j];
				spaceSizes[j] = size;
			}
		}
	}
	if (spaceSizes.size() % 2) //if odd number
		avgSpaceSize = spaceSizes[(spaceSizes.size() / 2) + 1];
	else {
		avgSpaceSize = (spaceSizes[spaceSizes.size() / 2] + spaceSizes[spaceSizes.size() / 2 + 1]) / 2;
	}
	printf("Median space size: %d \n", avgSpaceSize);
	for (int i = 0; i < spaces.size(); i++) { //combining small spaces close together
		if (i == 0) continue;
		int x = 1;
		while (i - x >= 0 && spaces[i - x].start == -1) ++x;
		int lastSpace = i - x;
		if (spaces[i].size <= round(.2 * avgSpaceSize) || abs(spaces[i].start - (spaces[lastSpace].start + spaces[lastSpace].size)) <= round(.3 * avgSpaceSize)) {
			spaces[lastSpace].size = spaces[i].start + spaces[i].size - spaces[lastSpace].start;
			spaces[i] = { -1, -1 };
		}
	}
	if (spaces.size() == 0) printf("no spaces!");
	int lastSpaceSize = 0;
	Space equalSpacing{ -1, -1 };
	for (int i = 0; i < spaces.size(); i++) {
		int lastSpace = max(i - 1, 0);
		Space s = spaces[i];
		if (s.size == -1 || s.start == -1) continue;
		if (abs(s.size - avgSpaceSize) <= round(.5 * avgSpaceSize)) {
			if (equalSpacing.size != -1/* && abs(s.size - lastSpaceSize) < .1 * lastSpaceSize*/) {
				equalSpacing.size = s.start + s.size - equalSpacing.start;
				lastSpaceSize = s.size;
			}
			else if (equalSpacing.size == -1) {
				equalSpacing.start = spaces[lastSpace].start + spaces[lastSpace].size;
				equalSpacing.size = s.start + s.size - equalSpacing.start;
				lastSpaceSize = s.size;
			}
		}
		else {
			if (s.size - avgSpaceSize > round(.5 * avgSpaceSize) && equalSpacing.size != -1) {
				equalSpacing.size = s.start - equalSpacing.start;
				break;
			}
		}
	}
/*	printf("Equal spacing from: %d to %d \n", equalSpacing.start, equalSpacing.start + equalSpacing.size);
	for (int i = 0; i < img->getHeight(); i++) {
		img->setPixel(equalSpacing.start, i, { 255, 0, 0 });
		img->setPixel(equalSpacing.start + equalSpacing.size, i, { 255, 0, 0 });
	}*/
#pragma endregion
#pragma region horzSpacing
	int * horzAccumulator = new int[img->getHeight()];
	memset(horzAccumulator, 0, img->getHeight() * sizeof(int));
	std::vector<Space> horzSpaces;
	std::vector<int> horzSpaceSizes;
	for (int i = 0; i < img->getHeight(); i++) {
		for(int x = 0; x < img->getWidth(); x++)
			horzAccumulator[i] += img->getPixel(x, i).avg();
	}
	lastDark = 0;
	for (int i = 0; i <= img->getHeight(); i++) {
		if (i == img->getHeight()) {
			horzSpaces.push_back({ lastDark + 1, i - lastDark });
			horzSpaceSizes.push_back(i - lastDark);
			break;
		}
		if (horzAccumulator[i] < totalRowAvg) {
			if (i - lastDark > 1) {
				horzSpaces.push_back({ lastDark + 1, i - lastDark });
				horzSpaceSizes.push_back(i - lastDark);
			}
			lastDark = i;
		}
	}
	avgSpaceSize = 0;
	for (int i = 0; i < horzSpaceSizes.size(); i++) {
		for (int j = 0; j < horzSpaceSizes.size(); j++) {
			if (horzSpaceSizes[j] < horzSpaceSizes[i]) {
				int size = horzSpaceSizes[i];
				horzSpaceSizes[i] = horzSpaceSizes[j];
				horzSpaceSizes[j] = size;
			}
		}
	}
	if (horzSpaceSizes.size() % 2) //if odd number
		avgSpaceSize = horzSpaceSizes[(horzSpaceSizes.size() / 2) + 1];
	else {
		avgSpaceSize = (horzSpaceSizes[horzSpaceSizes.size() / 2] + horzSpaceSizes[horzSpaceSizes.size() / 2 + 1]) / 2;
	}
	printf("Median HorzSpace size: %d \n", avgSpaceSize);
	for (int i = 0; i < horzSpaces.size(); i++) {
		if (i == 0) continue;
		int x = 1;
		while (i - x >= 0 && spaces[i - x].start == -1) ++x;
		int lastSpace = i - x;
		if (horzSpaces[i].size <= round(.2 * avgSpaceSize) || abs(horzSpaces[i].start - (horzSpaces[lastSpace].start + horzSpaces[lastSpace].size)) <= round(.3 * avgSpaceSize)) {
			horzSpaces[lastSpace].size = horzSpaces[i].start + horzSpaces[i].size - horzSpaces[lastSpace].start;
			horzSpaces[i] = { -1, -1 };
		}
	}
#ifdef DEBUGGING_SPACE
	for (Space s : horzSpaces) {
		printf("Horz Space Size: %d \n", s.size);
		for (int i = 0; i < img->getWidth(); i++) {
			for (int y = s.start; y < s.start + s.size; y++) {
				img->setPixel(i, y, { 0, 0, 255 });
			}
		}
	}
	printf("\n");
	for (Space s : spaces) {
		printf("Vert Space Size: %d \n", s.size);
		for (int i = 0; i < img->getHeight(); i++) {
			for (int x = s.start; x < s.start + s.size; x++) {
				img->setPixel(x, i, { 0, 0, 255 });
			}
		}
	}
#endif
	printf("Next! \n");
	if (horzSpaces.size() == 0) printf("no spaces!");
	lastSpaceSize = 0;
	Space horzEqualSpacing{ -1, -1 };
	for (int i = 0; i < horzSpaces.size(); i++) {
		int lastSpace = max(i - 1, 0);
		Space s = horzSpaces[i];
		if (s.size == -1 || s.start == -1) continue;
		if (abs(s.size - avgSpaceSize) <= round(.5 * avgSpaceSize)) {
			if (horzEqualSpacing.size != -1/* && abs(s.size - lastSpaceSize) < .1 * lastSpaceSize*/) {
				horzEqualSpacing.size = s.start + s.size - horzEqualSpacing.start;
				lastSpaceSize = s.size;
			}
			else if (horzEqualSpacing.size == -1) {
				horzEqualSpacing.start = horzSpaces[lastSpace].start + horzSpaces[lastSpace].size;
				horzEqualSpacing.size = s.start + s.size - horzEqualSpacing.start;
				lastSpaceSize = s.size;
			}
		}
		else {
			printf("Stopped bc %d size \n", s.size);
			if (s.size - avgSpaceSize > round(.5 * avgSpaceSize) && horzEqualSpacing.size != -1) {
				horzEqualSpacing.size = s.start - horzEqualSpacing.start;
				break;
			}
		}
	}
/*	for (int i = 0; i < img->getWidth(); i++) {
		img->setPixel(i, horzEqualSpacing.start, { 255, 0, 0 });
		img->setPixel(i, horzEqualSpacing.start + horzEqualSpacing.size, { 255, 0, 0 });
	}*/
#pragma endregion
#ifndef DEBUGGING_SPACE
	spaces.push_back({ (int)img->getWidth(), 0 });
	horzSpaces.push_back({ (int)img->getHeight(), 0 });
	std::vector<Square> characters;
	for (int x = 0; x < spaces.size() - 1; x++) {
		if (spaces[x].size == -1) continue;
		if (spaces[x].start + spaces[x].size >= equalSpacing.start && spaces[x].start <= equalSpacing.start + equalSpacing.size) {
			for (int y = 0; y < horzSpaces.size() - 1; y++) {
				if (horzSpaces[y].start == -1) continue;
				if (horzSpaces[y].start + horzSpaces[y].size >= horzEqualSpacing.start && horzSpaces[y].start <= horzEqualSpacing.start + horzEqualSpacing.size) {
					Square sq;
					int i = 1;
					while (x + i < spaces.size() && spaces[x + i].start == -1) ++i;
					int nextX = x + i;
					i = 1;
					while (y + i < horzSpaces.size() && horzSpaces[y + i].start == -1) ++i;
					int nextY = y + i;
					sq.x = spaces[x].start + spaces[x].size;
					sq.y = horzSpaces[y].start + horzSpaces[y].size;
					sq.width = spaces[nextX].start - (spaces[x].start + spaces[x].size);
					sq.height = horzSpaces[nextY].start - (horzSpaces[y].start + horzSpaces[y].size);
					if (sq.width < 3 || sq.height < 3) continue;
					if (sq.y > horzEqualSpacing.start + horzEqualSpacing.size) continue;
					if (sq.x > equalSpacing.start + equalSpacing.size) continue;
					int upY = sq.y;
					int downY = sq.y + sq.height;
					int leftX = sq.x;
					int rightX = sq.x + sq.width;
					bool done[4] = { false, false, false, false };
					while (!(done[0] && done[1] && done[2] && done[3])) {
						int upAcc = 0, downAcc = 0;
						for (int i = -1; i < sq.width + 1; i++) {
							upAcc += img->getPixel(sq.x + i, upY).avg() < 180 ? 1 : 0;
							downAcc += img->getPixel(sq.x + i, downY).avg() < 180 ? 1 : 0;
						}
						if (upAcc > 1 && upY <= sq.y) upY--;
//						else if (upY >= sq.y && upAcc <= 1) upY++;
						else done[0] = true;
						if (downAcc > 1 && downY >= sq.y + sq.height) downY++;
//						else if (downY <= sq.y + sq.height && downAcc <= 1) downY--;
						else done[1] = true;

						int leftAcc = 0, rightAcc = 0;
						for (int i = -1; i < sq.height + 1; i++) {
							leftAcc += img->getPixel(leftX, sq.y + i).avg() < 180 ? 1 : 0;
							rightAcc += img->getPixel(rightX, sq.y + i).avg() < 180 ? 1 : 0;
						}
						if (leftAcc > 1 && leftX <= sq.x) leftX--;
//						else if (leftAcc <= 1 && leftX >= sq.x) leftX++;
						else done[2] = true;
						if (rightAcc > 1 && rightX >= sq.x + sq.width) rightX++;
//						else if (rightAcc <= 1 && rightX <= sq.x + sq.width) rightX--;
						else done[3] = true;
						
					}
					sq.y = upY;
					sq.height = downY - sq.y;
					sq.x = leftX;
					sq.width = rightX - sq.x;					
					if (sq.width < 3 || sq.height < 3) continue;
					if (sq.y > horzEqualSpacing.start + horzEqualSpacing.size) continue;
					characters.push_back(sq);
				}
			}
		}
	}
#endif
#ifndef DEBUGGING_SPACE
#ifdef SHOW_DEBUG_CHARS
	for (Square s : characters) {
		for (int i = s.y; i < s.y + s.height; i++) {
			img->setPixel(s.x, i, { 0, 255, 0 });
			img->setPixel(s.x + s.width, i, { 0, 255, 0 });
		}
		for (int i = s.x; i < s.x + s.width; i++) {
			img->setPixel(i, s.y, { 0, 255, 0 });
			img->setPixel(i, s.y + s.height, { 0, 255, 0 });
		}
	}
#endif
#endif
	RECT r;
	GetClientRect(gui::GUI::useWindow(), &r);
	InvalidateRect(gui::GUI::useWindow(), &r, TRUE);
	delete[] accumulator;
	delete[] horzAccumulator;
#ifndef DEBUGGING_SPACE
	return characters;
#else
	return std::vector<Square>();
#endif
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
SearchGrid identifyLetters(Image * img, std::vector<Square> locations)
{
	SearchGrid grid;
	std::vector<KnownSample *> letters;
	WIN32_FIND_DATA fData;
	HANDLE hand = FindFirstFile("C:\\Users\\stephen\\Documents\\Visual Studio 2015\\Projects\\PuzzleSolver\\PuzzleSolver\\letters\\*", &fData);
	char fileRead[MAX_PATH];
	while (hand != INVALID_HANDLE_VALUE) {		
		if (isBmp(fData.cFileName)) {
			printf("%s \n", fData.cFileName);
			sprintf_s(fileRead, MAX_PATH, "C:\\Users\\stephen\\Documents\\Visual Studio 2015\\Projects\\PuzzleSolver\\PuzzleSolver\\letters\\%s", fData.cFileName);
			if (!isalpha(fData.cFileName[0])) printf("img name is not a letter! \n");
			letters.push_back(new KnownSample{ new Image(fileRead), (char)toupper(fData.cFileName[0]) });			
		}
		if (FindNextFile(hand, &fData) == FALSE) break;
	}
	for (int i = 0; i < locations.size(); i++) {
		std::pair<char, double> minDifference = std::make_pair(0, DBL_MAX);
		Image * letter = new Image(locations[i].width, locations[i].height);
		for (int x = 0; x < locations[i].width; x++) {
			for (int y = 0; y < locations[i].height; y++) {
				letter->setPixel(x, y, img->getPixel(x + locations[i].x, y + locations[i].y));
			}
		}
		if (i == 4) letter->saveBmp("testUnknownPreScale.bmp");
		letter->scaleTo(SAMPLE_WIDTH, SAMPLE_HEIGHT);
		if (i == 4) letter->saveBmp("testUnknown.bmp");
		for (int j = 0; j < letters.size(); j++) {
			double diffScore = 0;
			for (int k = 0; k < SAMPLE_WIDTH * SAMPLE_HEIGHT; k++) {
				int x = k % SAMPLE_WIDTH;
				int y = k / SAMPLE_WIDTH;
				diffScore += pow(letter->getPixel(x, y).avg() - letters[j]->image->getPixel(x, y).avg(), 2);
			}
			if (diffScore < minDifference.second)
				minDifference = std::make_pair(letters[j]->letter, diffScore);
		}
		grid.addLetter(minDifference.first, locations[i].x, locations[i].y);
		delete letter;
	}
	for (int i = 0; i < letters.size(); i++)
		delete letters[i];
	grid.iterateRowbyRow();
	return grid;
 }
 void SearchGrid::search(Image * img, std::vector<Square> locations, std::vector<std::string> words)
 {
	 std::map<std::string, int> foundWords;
	 std::map<std::string, Line> foundWordsPos;
	 printf("testGetLetters: %c %c %c \n", getLetter(maxColumns, maxRows), getLetter(maxColumns - 1, maxRows - 1), getLetter(maxColumns - 2, maxRows - 2));
	 for (std::string word : words) {
		 std::vector<std::string> possibilities;
		 std::vector<Line> lines;
		 for (int i = 0; i < (maxRows + 1) * (maxColumns + 1); i++) {
			 int x = i / (maxColumns + 1);
			 int y = i % (maxColumns + 1);
			 Letter letter = *letters[i];
			 for (int j = 0; j < word.size(); j++) {
				 if (letter == word[j]) {
					 if (/*x <= (maxColumns + 1) - (word.size() - j)*/ true) {
						 Line l;
						 l.start = { x - j, y };
						 l.end = {  x + (int)word.size() - j - 1, y };
						 if (!l.outOfBounds(maxRows + 1, maxColumns + 1)) {
							 std::stringstream p;
							 for (int k = -j; k < word.size() - j; k++) {
								 p << getLetter(x + k, y);
							 }
							 possibilities.push_back(p.str());
							 lines.push_back(l);
						 }
					 }
					 if (/*y <= (maxRows + 1) - (word.size() - j)*/ true) {
						 Line l;
						 std::stringstream s;
						 l.start = { x, y - j };
						 l.end = { x, y + (int)word.size() - j - 1 };
						 if (!l.outOfBounds(maxRows + 1, maxColumns + 1)) {
							 for (int k = -j; k < word.size() - j; k++) {
								 s << getLetter(x, y + k);
							 }
							 possibilities.push_back(s.str());
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
							 for (int k = -j; k < word.size() - j; k++) {
								 s << getLetter(x + k, y - k);
							 }
							 possibilities.push_back(s.str());
							 lines.push_back(l);
						 }
					 }
					 if (/*x <= (maxColumns + 1) - (word.size() - j) && y <= maxRows - (word.size() - j) */ true) {
						 Line l;
						 std::stringstream s;
						 l.start = { x - j, y - j };
						 l.end = { x + (int)word.size() - j - 1, y + (int)word.size() - j - 1 };
						 if (!l.outOfBounds(maxRows + 1, maxColumns + 1)) {
							 for (int k = -j; k < word.size() - j; k++) {
								 s << getLetter(x + k, y + k);
							 }
							 possibilities.push_back(s.str());
							 lines.push_back(l);
						 }
					 }
					 if (/*x + 1 >= word.size() - j  && y + 1 >= word.size() - j*/ true) {
						 Line l;
						 std::stringstream s;
						 l.start = { x + j, y + j };
						 l.end = { x - ((int)word.size() - j - 1), y - ((int)word.size() - j - 1) };
						 if (!l.outOfBounds(maxRows + 1, maxColumns + 1)) {
							 for (int k = -j; k < word.size() - j; k++) {
								 s << getLetter(x - k, y - k);
							 }
							 possibilities.push_back(s.str());
							 lines.push_back(l);
						 }
					 }
					 if (/*x + 1 >= word.size() - j && y <= (maxRows + 1) - (word.size() - j)*/ true) {
						 Line l;
						 std::stringstream s;
						 l.start = { x + j, y - j };
						 l.end = { x - ((int)word.size() - j - 1), y + (int)word.size() - j - 1 };
						 if (!l.outOfBounds(maxRows + 1, maxColumns + 1)) {
							 for (int k = -j; k < word.size() - j; k++) {
								 s << getLetter(x - k, y + k);
							 }
							 possibilities.push_back(s.str());
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
							 for (int k = -j; k < word.size() - j; k++) {
								 s << getLetter(x - k, y);
							 }
							 possibilities.push_back(s.str());
							 lines.push_back(l);
						 }
					 }
					 if (/*y + 1 >= word.size() - j*/ true) {
						 Line l;
						 std::stringstream s;
						 l.start = { x, y + j };
						 l.end = { x, y - ((int)word.size() - j - 1) };
						 if (!l.outOfBounds(maxRows + 1, maxColumns + 1)) {
							 for (int k = -j; k < word.size() - j; k++) {
								 s << getLetter(x, y - k);
							 }
							 possibilities.push_back(s.str());
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
				}
			}
			else {
				foundWords.insert(std::make_pair(word, maxSame.second));
				foundWordsPos.insert(std::make_pair(word, lines[maxSame.first]));
			}
		}
	 }
	 for (auto it = foundWordsPos.begin(); it != foundWordsPos.end(); it++) {
		 printf("Found word: %s Score: %d / %d \n", it->first.c_str(), foundWords[it->first], it->first.size());
		 Line line = it->second;
		 Square start = locations[line.start.x * (maxRows + 1) + line.start.y];
		 Square end = locations[line.end.x * (maxRows + 1) + line.end.y];
		 POINT startpt = { start.x + (start.width / 2), start.y + (start.height / 2) };
		 POINT endpt = { end.x + (end.width / 2), end.y + (end.height / 2) };
		 printf("Line from (%d, %d) to (%d, %d) \n", startpt.x, startpt.y, endpt.x, endpt.y);
		 printf("Which is (%d, %d) to (%d, %d) \n", line.start.x, line.start.y, line.end.x, line.end.y);
		 double dist = sqrt((endpt.x - startpt.x) * (endpt.x - startpt.x) + (endpt.y - startpt.y) * (endpt.y - startpt.y));
		 double theta = acos((endpt.x - startpt.x) / dist);
		 if ((line.start.x != line.end.x && line.start.y > line.end.y) || (line.start.x == line.end.x && line.start.y > line.end.y)) theta *= -1;
		 float matrix[] = {
			 cos(theta),	-sin(theta),
			 sin(theta),	 cos(theta)
		 };
		 for (int i = 0; i < floor(dist); i++) {
			 POINT p = { i, 0 };
			 POINT pr = matrixMultiply(matrix, p);
			 pr.x += startpt.x;
			 pr.y += startpt.y;
			 img->setPixel(pr.x, pr.y, { 255, 0, 0 });
		 }
		 
	 }
	 RECT r;
	 GetClientRect(gui::GUI::useWindow(), &r);
	 InvalidateRect(gui::GUI::useWindow(), &r, TRUE);
 }
 void SearchGrid::copyFrom(SearchGrid g)
 {
	 if (letters.size() > 0) {
		 for (int i = 0; i < letters.size(); i++)
			 delete letters[i];
		 letters.erase(letters.begin(), letters.end());
	 }
	 for (auto l : g.letters) {
		 letters.push_back(new Letter(*l));
	 }
	 lastRow = g.lastRow;
	 lastColumn = g.lastColumn;
	 maxRows = g.maxRows;
	 maxColumns = g.maxColumns;
	 row0Y = g.row0Y;
	 column0X = g.column0X;
 }
 void augmentDataSet(std::vector<Square> locations, std::vector<char> knowns, Image * img, int firstKnown)
 {
	 int size = min(locations.size(), firstKnown + knowns.size());
	 for (int i = firstKnown; i < size; i++) {
		 ImgPtr image = new Image(locations[i].width, locations[i].height);
		 for (int j = 0; j < locations[i].width * locations[i].height; j++) {
			 int x = j % locations[i].width;
			 int y = j / locations[i].width;
			 image->setPixel(x, y, img->getPixel(x + locations[i].x, y + locations[i].y));
		 }
		 char path[MAX_PATH];
		 srand(clock());
		 int id = rand() % 1000;
		 sprintf_s(path, MAX_PATH, "C:\\Users\\stephen\\Documents\\Visual Studio 2015\\Projects\\Puzzle Solver + GDI API\\Puzzle Solver + GDI API\\letters\\%c %drw.bmp", knowns[i], id);
		 image->scaleTo(SAMPLE_WIDTH, SAMPLE_HEIGHT);
		 image->saveBmp(path);
	 }
 }
POINT getOrigin(Image * img)
{
	int startY = 0;
	int startX = 0;
	int totalAvg = img->integralImageValue(img->getWidth() - 1, img->getHeight() - 1) - img->integralImageValue(0, img->getHeight() - 1) - img->integralImageValue(img->getWidth() - 1, 0) + img->integralImageValue(0, 0);
	totalAvg /= (img->getWidth() * img->getHeight());
	printf("%d \n", totalAvg);
	for (int i = 0; i < img->getWidth() * img->getHeight(); i++) {
		int x = i % img->getWidth();
		int y = i / img->getWidth();
		Color c = img->getPixel(x, y);
		if (c.avg() < 100) {
			int xp = min(x + (.15 * img->getWidth()), img->getWidth() - 1);
			int yp = min(y + (.15 * img->getHeight()), img->getHeight() - 1);
			int xm = max(x - (.15 * img->getWidth()), 0);
			int ym = max(y - (.15 * img->getHeight()), 0);
			if (xp != (int)(x + (.15 * img->getWidth())))
				xm -= (x + .15 * img->getWidth()) - (img->getWidth() - 1);
			else if (xm != (int)(x - (.15 * img->getWidth())))
				xp += 0 - (x - .15 * img->getWidth());
			if (yp != (int)(y + .15 * img->getHeight()))
				ym -= (y + .15 * img->getHeight()) - (img->getHeight() - 1);
			else if (ym != (int)(y - .15 * img->getHeight()))
				yp += 0 - (y - .15 * img->getHeight());
			int avg = img->integralImageValue(xp, yp) - img->integralImageValue(xm, yp) - img->integralImageValue(xp, ym) - img->integralImageValue(xm, ym);
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
void rotateImage(Image * img, float theta, POINT origin)
{
	int diagnol = ceil(sqrt(img->getWidth() * img->getWidth() + img->getHeight() * img->getHeight()));
	Color * buffer = new Color[diagnol * diagnol];
	//Debugging
	for (int i = 0; i < diagnol * diagnol; i++) {
//		buffer[i] = Color{ 0, 255, 0 };
		buffer[i] = Color{ 255, 255, 255 };
	}
	//end debugging
	//mapping from destination back to source and picks out source pixel to reduce the "holes" in the image
	float rotationMatrix[] = { //rotating to the negative angle is same thing as taking inverse of rotation matrix to that angle
		cos(radians(-theta)),  -sin(radians(-theta)),
		sin(radians(-theta)),  cos(radians(-theta))
	};
	for (int i = 0; i < diagnol * diagnol; i++) {
		int x = i % diagnol;
		int y = i / diagnol;
		x -= (diagnol - img->getWidth()) / 2;
		y -= (diagnol - img->getWidth()) / 2;
		POINT rtPt = { x - origin.x, y - origin.y };
		POINT rotated = matrixMultiply(rotationMatrix, rtPt);
		rotated.x += origin.x;
		rotated.y += origin.y;
		if (rotated.x > 0 && rotated.x < img->getWidth() && rotated.y > 0 && rotated.y < img->getHeight())
			buffer[i] = img->getPixel(rotated.x, rotated.y);

	}
	img->resize(diagnol, diagnol);
	PAINTSTRUCT p;
	HDC dc = BeginPaint(gui::GUI::useWindow(), &p);
	for (int i = 0; i < diagnol * diagnol; i++) {
		int x = i % diagnol;
		int y = i / diagnol;
		channel * bits = img->getRawData();
		if (bits == nullptr) printf("Bits is null \n");
		img->setPixel(x, y, buffer[i]);
	}
	EndPaint(gui::GUI::useWindow(), &p);
	RECT r;
	GetClientRect(gui::GUI::useWindow(), &r);
	InvalidateRect(gui::GUI::useWindow(), &r, TRUE);
	delete[] buffer;
}
Rect::operator Square()
{
	return{ topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y };
}

void SearchGrid::addLetter(char c, int x, int y)
{
	if (letters.size() < 1) {
		row0Y = y;
		column0X = x;
		lastRow = std::make_pair(0, y);
		lastColumn = std::make_pair(0, x);
		letters.push_back(new Letter{ 0, 0, c });
	}
	else {
		int currentRow;
		int currentColumn;
		if (abs(lastRow.second - y) < 5)
			currentRow = lastRow.first;
		else if (y > lastRow.second) {
			//this row is further down
			lastRow = std::make_pair(lastRow.first + 1, y);
			currentRow = lastRow.first;
			maxRows = max(maxRows, currentRow);
		}
		else if (y < lastRow.second) {
			if (abs(row0Y - y) < 5) {
				lastRow = std::make_pair(0, row0Y);
				currentRow = 0;
//				printf("Row reset \n");
			}
			else
				printf("IDK how to handle this row \n");
		}

		if (abs(lastColumn.second - x) < 10) {
			currentColumn = lastColumn.first;
//			printf("Current column \n");
		}
		else if (x > lastColumn.second) {
			lastColumn = std::make_pair(lastColumn.first + 1, x);
			currentColumn = lastColumn.first;
			maxColumns = max(maxColumns, currentColumn);
//			printf("Column greater: %d \n", currentColumn);
		}
		else if (x < lastColumn.second) {
			if (abs(x - column0X) < 10) {
				lastColumn = std::make_pair(0, column0X);
				currentColumn = 0;
//				printf("Column reset \n");
			}
			else
				printf("IDK column \n");
		}
//		printf("%d %d \n", currentRow, currentColumn);
		letters.push_back(new Letter{ currentRow, currentColumn, c });
	}
}

SearchGrid::~SearchGrid()
{
	for (int i = 0; i < letters.size(); i++) {
		delete letters[i];
	}
}

void SearchGrid::iterateRowbyRow()
{
	printf("Max rows: %d   Max columns: %d \n", maxRows, maxColumns);
	for (int r = 0; r <= maxRows; r++) {
		for (int c = 0; c <= maxColumns; c++) {
			for (Letter * l : letters) {
				if (l->row == r && l->column == c)
					printf("%c ", l->letter);
			}
		}
		printf("\n");
	}
}

bool Letter::operator==(char c)
{
	std::vector<std::vector<char>> confusionSets;
	if (letter == c) return true;
	return false;
}

bool Line::outOfBounds(int maxRows, int maxColumns)
{
	if (start.x < 0 || start.x >= maxColumns || start.y < 0 || start.y >= maxRows ||
		end.x < 0 || end.x >= maxColumns || end.y < 0 || end.y >= maxRows) return true;
	return false;
}
