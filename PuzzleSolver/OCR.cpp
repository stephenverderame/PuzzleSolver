#include "OCR.h"
using namespace Math;
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
/*
void CV::SearchGrid::getCharacterLocations()
{
#pragma region columnSpaces
	int * accumulator = new int[seekImage.width()];
	memset(accumulator, 0, seekImage.width() * sizeof(int));
	std::vector<Space> spaces;
	std::vector<int> spaceSizes;
	int totalIntensity = seekImage.integralImageValue(seekImage.width() - 1, seekImage.height() - 1) - seekImage.integralImageValue(0, seekImage.height() - 1) - seekImage.integralImageValue(seekImage.width() - 1, 0) + seekImage.integralImageValue(0, 0);
	int totalColumnAvg = totalIntensity / seekImage.width();
	int totalRowAvg = totalIntensity / seekImage.height();
	for (int i = 0; i < seekImage.width(); i++) {
		for (int y = 0; y < seekImage.height(); y++) {
			accumulator[i] += seekImage.getPixel({ i, y }).avg();
		}
	}
	int lastDark = 0;
	for (int i = 0; i <= seekImage.width(); i++) {
		if (i == seekImage.width()) {
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
			if (equalSpacing.size != -1) {
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
	printf("Equal spacing from: %d to %d \n", equalSpacing.start, equalSpacing.start + equalSpacing.size);
	for (int i = 0; i < seekImage.height(); i++) {
		seekImage.setPixel({ { 255, 0, 0 }, equalSpacing.start, i, });
		seekImage.setPixel({ { 255, 0, 0 }, equalSpacing.start + equalSpacing.size, i, });
	}
#pragma endregion
#pragma region horzSpacing
	int * horzAccumulator = new int[seekImage.height()];
	memset(horzAccumulator, 0, seekImage.height() * sizeof(int));
	std::vector<Space> horzSpaces;
	std::vector<int> horzSpaceSizes;
	for (int i = 0; i < seekImage.height(); i++) {
		for(int x = 0; x < seekImage.width(); x++)
			horzAccumulator[i] += seekImage.getPixel(x, i ).avg();
	}
	lastDark = 0;
	for (int i = 0; i <= seekImage.height(); i++) {
		if (i == seekImage.height()) {
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
		for (int i = 0; i < seekImage.width(); i++) {
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
			if (horzEqualSpacing.size != -1) {
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
	for (int i = 0; i < seekImage.width(); i++) {
		seekImage.setPixel({ { 255, 0, 0 }, i, horzEqualSpacing.start});
		seekImage.setPixel({ { 255, 0, 0 }, i, horzEqualSpacing.start + horzEqualSpacing.size });
	}
#pragma endregion
#ifndef DEBUGGING_SPACE
	spaces.push_back({ (int)seekImage.width(), 0 });
	horzSpaces.push_back({ (int)seekImage.height(), 0 });
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
							upAcc += seekImage.getPixel(sq.x + i, upY).avg() < 180 ? 1 : 0;
							downAcc += seekImage.getPixel(sq.x + i, downY).avg() < 180 ? 1 : 0;
						}
						if (upAcc > 1 && upY <= sq.y) upY--;
//						else if (upY >= sq.y && upAcc <= 1) upY++;
						else done[0] = true;
						if (downAcc > 1 && downY >= sq.y + sq.height) downY++;
//						else if (downY <= sq.y + sq.height && downAcc <= 1) downY--;
						else done[1] = true;

						int leftAcc = 0, rightAcc = 0;
						for (int i = -1; i < sq.height + 1; i++) {
							leftAcc += seekImage.getPixel(leftX, sq.y + i).avg() < 180 ? 1 : 0;
							rightAcc += seekImage.getPixel(rightX, sq.y + i).avg() < 180 ? 1 : 0;
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
					locations.push_back(sq);
				}
			}
		}
	}
#endif
#ifndef DEBUGGING_SPACE
#ifdef SHOW_DEBUG_CHARS
	for (Square s : characters) {
		for (int i = s.y; i < s.y + s.height; i++) {
			seekImage.setPixel(s.x, i, { 0, 255, 0 });
			seekImage.setPixel(s.x + s.width, i, { 0, 255, 0 });
		}
		for (int i = s.x; i < s.x + s.width; i++) {
			seekImage.setPixel(i, s.y, { 0, 255, 0 });
			seekImage.setPixel(i, s.y + s.height, { 0, 255, 0 });
		}
	}
#endif
#endif
	RECT r;
	GetClientRect(gui::GUI::useWindow(), &r);
	InvalidateRect(gui::GUI::useWindow(), &r, TRUE);
	delete[] accumulator;
	delete[] horzAccumulator;
}
*/
void CV::SearchGrid::init()
{
	seekImage.trueGrayscale(std::make_unique<IMG::LumFunc>());
	auto img = cannyEdgeDetection(seekImage, 0.05, 0.0002);
	Hough hough;
	hough.transform(*img);
	auto list = hough.getLines(50);
	printf("%d lines found!\n", list.size());
	decltype(list) newList;
	for (decltype(auto) line : list) {
		auto d = line.second - line.first;
		if (abs(d.y) < 50 || abs(d.x) < 50) {
			newList.push_back(line);
		}
	}
	seekImage.greyscale();
	int i = 0;
	std::vector<Square> foundLetters;
	for (auto it = newList.cbegin(); it != newList.cend(); ++it) {
		for (auto itt = newList.cbegin(); itt != newList.cend(); ++itt) {
			if (it == itt) continue;
			point intersect = lineIntersection(it->first, it->second, itt->first, itt->second);
			if (intersect.x != INT_MAX && intersect.y != INT_MIN) {
				point start = { -1, -1 };
				for (int y1 = 0; y1 <= 10 && seekImage.yInBounds(intersect.y - y1); ++y1) {
					for (int x1 = 0; x1 <= 10 && seekImage.xInBounds(intersect.x + x1); ++x1) {
						if (seekImage.getPixel(intersect + point{ x1, -y1 }).avg() < 100) {
							start = intersect + point{ x1, -y1 };
							x1 = y1 = 20;
						}
					}
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
	for (auto it = foundLetters.begin(); it != foundLetters.end(); ++it) {
		for (auto itt = foundLetters.begin(); itt != foundLetters.end(); ++itt) {
			if (it == itt) continue;
			if ((itt->x == -1 && itt->y == -1) || (it->x == -1 && it->y == -1)) continue;
			if (it->x * itt->x + it->y * itt->y < 225 || (itt->x >= it->x && itt->x <= it->x + it->width && itt->y >= it->y && itt->y <= it->y + it->height)) {
				it->x = min(it->x, itt->x);
				it->y = min(it->y, itt->y);
				it->width = max(it->width, itt->width);
				it->height = max(it->height, itt->height);
				itt->x = -1;
				itt->y = -1;
			}
		}
	}
	RB_TREE<6> widthVals, heightVals;
	OCR_TREE xVals;
	OCR_TREE yVals;
	for (auto it = foundLetters.cbegin(); it != foundLetters.cend(); ++it) {
		if (it->x == -1 && it->y == -1) continue;
		widthVals.insert(it->width);
		widthVals.insert(it->height);
	}
	auto widthList = widthVals.inorderList();
	auto heightList = widthVals.inorderList();
	int widthMedian = widthList.size() % 2 ? widthList[widthList.size() / 2 + 1] : (widthList[widthList.size() / 2] + widthList[widthList.size() / 2 + 1]) / 2;
	int heightMedian = heightList.size() % 2 ? heightList[heightList.size() / 2 + 1] : (heightList[heightList.size() / 2] + heightList[heightList.size() / 2 + 1]) / 2;
	printf("Median width: %d Median height: %d \n", widthMedian, heightMedian);
	for (auto it = foundLetters.cbegin(); it != foundLetters.cend(); ++it) {
		if (it->x == -1 && it->y == -1) continue;
		if (abs(it->width - widthMedian) <= .3 * widthMedian && abs(it->height - heightMedian) <= .3 * heightMedian) {
			xVals.insert(it->x);
			yVals.insert(it->y);
		}
	};
	std::vector<int> xList = xVals.inorderList();
	std::vector<int> yList = yVals.inorderList();
	std::vector<int> xSpace, ySpace;
	for (int i = 0; i < xList.size(); ++i) {
		if (i == 0) xSpace.push_back(xList[i]);
		else {
			int v = i;
			while (v >= 0 && xList[i] == yList[v]) --v;
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
	int xMedian = xSpace.size() % 2 ? xSpace[xSpace.size() / 2 + 1] : (xSpace[xSpace.size() / 2] + xSpace[xSpace.size() / 2 + 1]) / 2;
	int yMedian = ySpace.size() % 2 ? ySpace[ySpace.size() / 2 + 1] : (ySpace[xSpace.size() / 2] + ySpace[ySpace.size() / 2 + 1]) / 2;
	printf("Medians %d %d\n", xMedian, yMedian);
	printf("XSpaceSize: %d XListSize: %d \n", xSpace.size(), xList.size());
	assert(xSpace.size() == xList.size() && ySpace.size() == yList.size() && "Spaces and coordinate lists don't match up");
	std::vector<std::pair<int, int>> xSpaces;
	std::vector<std::pair<int, int>> ySpaces;
	int v = 0;
	xSpaces.push_back(std::make_pair(seekImage.width(), 0));
	for (int i = 0; i < xList.size(); ++i) {
		if (abs(xSpace[i] - xMedian) <= .3 * xMedian || (i + 1 < xList.size() && abs(xSpace[i + 1] - xMedian) <= .3 * xMedian)) {
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
		if (abs(ySpace[i] - yMedian) <= .3 * yMedian || (i + 1 < yList.size() && abs(ySpace[i + 1] - yMedian) <= .3 * yMedian)) {
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
	std::pair<int, int> xSpaceFinal{ 0, 0 }, ySpaceFinal{ 0, 0 };
	for (auto it : xSpaces)
		if (it.second - it.first > xSpaceFinal.second - xSpaceFinal.first)
			xSpaceFinal = it;
	for (auto it : ySpaces)
		if (it.second - it.first > ySpaceFinal.second - ySpaceFinal.first)
			ySpaceFinal = it;
	printf("Found search area\n");
#ifdef DEBUGGING
	seekImage.drawRect({ xSpaceFinal.first, ySpaceFinal.first }, { xSpaceFinal.second, ySpaceFinal.second }, { 255, 0, 0 });
#endif
	OCR_TREE finalX, finalY;
	std::vector<Square> tempList;
	for (auto it = foundLetters.cbegin(); it != foundLetters.cend(); ++it) {
		if (it->x + it->width >= xSpaceFinal.first - 10 && it->x <= xSpaceFinal.second + 10 && it->y + it->height >= ySpaceFinal.first - 10 && it->y <= ySpaceFinal.second + 10) {
#ifdef DEBUGGING
			seekImage.drawRect({ it->x, it->y }, { it->x + it->width, it->y + it->height }, { 0, 0, 255 });
#endif
			tempList.push_back(*it);
			finalX.insert(it->x);
			finalY.insert(it->y);
		}
	}
	columnPositions = finalX.inorderListNR();
	rowPositions = finalY.inorderListNR();
	characterLocations.resize(columnPositions.size());
	for (int i = 0; i < columnPositions.size(); ++i)
		characterLocations[i].resize(rowPositions.size());
	characters.resize(columnPositions.size());
	for (int i = 0; i < columnPositions.size(); ++i)
		characters[i].resize(rowPositions.size(), '?');
	printf("Init all vectors\n");
	printf("Grid: %d columns by %d rows \n", columnPositions.size(), rowPositions.size());
	int iv = 0;
	for (auto it : tempList) {
		printf("%d ", iv++);
		int minx = int_max;
		int miny = int_max;
		int yRow = 0, xRow = 0;
		int px = 0;
		for (auto x = columnPositions.cbegin(); x != columnPositions.cend(); ++px, ++x) {
			int temp = minx;
			minx = min(minx, (it.x - *x) * (it.x - *x));
			if (temp != minx) xRow = px;
		}
		int py = 0;
		for (auto y = rowPositions.cbegin(); y != rowPositions.cend(); ++py, ++y) {
			int temp = miny;
			miny = min(miny, (it.y - *y) * (it.y - *y));
			if (temp != miny) yRow = py;
		}
		printf("Closest to: (%d, %d)\n", xRow, yRow);
		characterLocations[xRow][yRow] = it;
	}
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
			newImg->saveBmp("testReadLetter.bmp");
			delete buffer;
		}

	}
	else printf("Data.ml not found!\n");
	printf("testNumber: %d \n", testNumber);
	for (int x = 0; x < columnPositions.size(); x++) {
		for (int y = 0; y < rowPositions.size(); ++y) {
			std::pair<char, double> minDifference = std::make_pair(0, DBL_MAX);
			if (characterLocations[x][y].width > 0 && characterLocations[x][y].height > 0) {
				std::unique_ptr<Image> letter = std::make_unique<Image>(characterLocations[x][y].width, characterLocations[x][y].height);
				for (int x2 = 0; x2 < characterLocations[x][y].width; x2++) {
					for (int y2 = 0; y2 < characterLocations[x][y].height; y2++) {
						letter->setPixel(x2, y2, (Color)seekImage.getPixel(x2 + characterLocations[x][y].x, y2 + characterLocations[x][y].y));
					}
				}
				letter->scaleTo(SAMPLE_WIDTH, SAMPLE_HEIGHT);
				//		if (i == 4) letter->saveBmp("testUnknown.bmp");
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
				characters[x][y] = minDifference.first;
//				matchLetter(std::shared_ptr<KnownSample>(new KnownSample{ letter, minDifference.first }));
			}
		}
	}
	iterateRowbyRow();
 }
 void CV::SearchGrid::search(std::vector<std::string> words)
 {
	 std::map<std::string, int> foundWords;
	 std::map<std::string, Line> foundWordsPos;
	 std::map<std::string, std::vector<POINT>> possibleLetterLocations;
	 int maxColumns = columnPositions.size() - 1;
	 int maxRows = rowPositions.size() - 1;
	 printf("testGetLetters: %c %c %c \n", getLetter(maxColumns, maxRows), getLetter(maxColumns - 1, maxRows - 1), getLetter(maxColumns - 2, maxRows - 2));
	 for (std::string word : words) {
		 std::vector<std::string> possibilities;
		 std::vector<std::vector<POINT>> possibleCharLocations;
		 std::vector<Line> lines;
		 for (int i = 0; i < (maxRows + 1) * (maxColumns + 1); i++) {
			 int x = i % (maxColumns + 1);
			 int y = i / (maxColumns + 1);
			 char letter = characters[x][y];
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
		 Square start = characterLocations[line.start.x][line.start.y];
		 Square end = characterLocations[line.end.x][line.end.y];
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
					 Square location = characterLocations[x][y];
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
 std::pair<char, CV::Square> CV::SearchGrid::getLetterNearest(Math::point p)
 {
	 int minx = int_max, xPos = 0;
	 int miny = int_max, yPos = 0;
	 for (int i = 0; i < columnPositions.size(); ++i) {
		 if ((p.x - columnPositions[i]) * (p.x - columnPositions[i]) < minx) {
			 minx = (p.x - columnPositions[i]) * (p.x - columnPositions[i]);
			 xPos = i;
		 }
	 }
	 for (int i = 0; i < rowPositions.size(); ++i) {
		 if ((p.y - rowPositions[i]) * (p.y - rowPositions[i]) < miny) {
			 miny = (p.y - rowPositions[i]) * (p.y - rowPositions[i]);
			 yPos = i;
		 }
	 }
	 return std::pair<char, Square>(characters[xPos][yPos], characterLocations[xPos][yPos]);
 }
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
	 //in this case, only looking for horizontal and vertical lines
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
		 if (((magnitudes[i] - minMag) / (maxMag - minMag)) < lowerThreshold) {
			 continue;
		 }
		 bool isEdge = true;
		 if (directions[i] > 112.5 && directions[i] <= 157.5) {
			 isEdge = false;
//			 if (y > 0 && x < img.width() - 1 && magnitudes[i] <= magnitudes[(y - 1) * img.width() + (x + 1)]) isEdge = false;
//			 if (y < img.height() - 1 && x > 0 && magnitudes[i] <= magnitudes[(y + 1) * img.width() + (x - 1)]) isEdge = false;
		 }
		 else if (directions[i] > 67.5 && directions[i] <= 112.5) {
			 isEdge = false;
			 if (y > 0 && magnitudes[i] <= magnitudes[(y - 1) * img.width() + x]) isEdge = false;
			 if (y < img.height() - 1 && magnitudes[i] <= magnitudes[(y + 1) * img.width() + x]) isEdge = false;
		 }
		 else if (directions[i] > 22.5 && directions[i] <= 67.5) {
			 isEdge = false;
//			 if (y > 0 && x > 0 && magnitudes[i] <= magnitudes[(y - 1) * img.width() + (x - 1)]) isEdge = false;
//			 if (y < img.height() - 1 && x < img.width() - 1 && magnitudes[i] <= magnitudes[(y + 1) * img.width() + (x + 1)]) isEdge = false;
		 }
		 else {
			 if (x > 0 && magnitudes[i] <= magnitudes[y * img.width() + (x - 1)]) isEdge = false;
			 if (x < img.width() - 1 && magnitudes[i] <= magnitudes[y * img.width() + (x + 1)]) isEdge = false;
		 }
		 if (isEdge) {
			 if (((magnitudes[i] - minMag) / (maxMag - minMag)) >= upperThreshold)
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
//						 resultant->setPixel(255, 255, 255, x, y);
//						 imageChanged = true;
					 }
					 if (y < img.height() - 1 && x > 0 && resultant->getPixel(x - 1, y + 1).avg() == 255) {
//						 resultant->setPixel(255, 255, 255, x, y);
//						 imageChanged = true;
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
//						 resultant->setPixel(255, 255, 255, x, y);
//						 imageChanged = true;
					 }
					 if (y < img.height() - 1 && resultant->getPixel(x + 1, y + 1).avg() == 255) {
//						 resultant->setPixel(255, 255, 255, x, y);
//						 imageChanged = true;
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
/*
void CV::SearchGrid::addLetter(char c, int x, int y)
{
	if (lettersInGrid.size() < 1) {
		row0Y = y;
		column0X = x;
		lastRow = std::make_pair(0, y);
		lastColumn = std::make_pair(0, x);
		lettersInGrid.push_back(std::shared_ptr<Letter>(new Letter{ 0, 0, c }));
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
		lettersInGrid.push_back(std::shared_ptr<Letter>(new Letter{ currentRow, currentColumn, c }));
	}
}
*/
CV::SearchGrid::SearchGrid(IMG::Img & wordSearch) : seekImage(wordSearch)
{
}

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
	printf("Max rows: %d   Max columns: %d \n", rowPositions.size(), columnPositions.size());
	for (int r = 0; r < rowPositions.size(); r++) {
		for (int c = 0; c < columnPositions.size(); c++) {
			printf("%c ", characters[c][r]);
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

void CV::Kernel::scale(double scaler)
{
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			k[x][y] *= scaler;
		}
	}
}

void CV::ConnectedComponents::findConnectedComponents(IMG::Img & image)
{
	valueArray.resize(image.width());
	labelArray.resize(image.width());
	for (int i = 0; i < image.width(); ++i) {
		valueArray[i].resize(image.height());
		labelArray[i].resize(image.height());
	}
	for (int i = 0; i < image.width() * image.height(); ++i) {
		int x = i % image.width();
		int y = i / image.width();
		int a = image.getPixel(x, y).avg();
		if (a > 100)
			valueArray[x][y] = 0;
		else
			valueArray[x][y] = 1;
	}
	uint32_t label = 0;
	for (int y = 0; y < image.height(); ++y) {
		for (int x = 0; x < image.width(); ++x) {
			if (x > 0 && valueArray[x - 1][y] == valueArray[x][y])
				labelArray[x][y] = labelArray[x - 1][y];
			else if (y > 0 && valueArray[x][y - 1] == valueArray[x][y])
				labelArray[x][y] = labelArray[x][y - 1];
			else if (x > 0 && y > 0 && valueArray[x - 1][y - 1] == valueArray[x][y])
				labelArray[x][y] = labelArray[x - 1][y - 1];
			else if (y > 0 && x < image.width() - 1 && valueArray[x + 1][y - 1] == valueArray[x][y])
				labelArray[x][y] = labelArray[x + 1][y - 1];
			else
				labelArray[x][y] = label++;
		}
	}
	this->labels = label;
}

std::vector<CV::Square> CV::ConnectedComponents::componentLocations()
{
	std::vector<uint32_t> minX(labels), minY(labels), maxX(labels), maxY(labels);
	for (uint32_t i = 0; i < labels; ++i) {
		minX[i] = UINT_MAX;
		minY[i] = UINT_MAX;
		maxX[i] = 0;
		maxY[i] = 0;
	}
	for (size_t j = 0; j < labelArray.size() * labelArray[0].size(); ++j) {
		uint32_t x = j % labelArray.size();
		uint32_t y = j / labelArray.size();
		uint32_t label = labelArray[x][y];
		minX[label] = min(minX[label], x);
		minY[label] = min(minY[label], y);
		maxX[label] = max(maxX[label], x);
		maxY[label] = max(maxY[label], y);
	}
	std::vector<Square> locations(labels);
	for (uint32_t i = 0; i < labels; ++i) {
		locations[i] = { static_cast<int>(minX[i]), static_cast<int>(minY[i]), static_cast<int>(maxX[i] - minX[i]), static_cast<int>(maxY[i] - minY[i]) };

	}
	return locations;
}
