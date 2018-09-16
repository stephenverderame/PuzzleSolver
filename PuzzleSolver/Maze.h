#pragma once
#include "img.h"
#include "Math.h"
#include "Grid.h"
#include <thread>
#include <mutex>
#include <vector>
#include "Observer.h"
#include "CustomCursor.h"
namespace CV {
	class Maze : public Notification::Subject {
	private:
		IMG::Img & img;
		std::mutex sMu, iMu;
		bool solving;
		bool invalidate;
		std::vector<int> buffer;
	public:
		void calculatePathThread(Math::point start, Math::point end, Math::point crop_1 = { 0, 0 }, Math::point crop_2 = { 0, 0 });
	public:
		Maze(IMG::Img & i);
		void loadMaze(IMG::Img & im);
		void choosePoint();
		void calculatePath(Math::point start, Math::point end, Math::point crop_1 = { 0, 0 }, Math::point crop_2 = { 0, 0 });
		bool isSolving();
	};
}