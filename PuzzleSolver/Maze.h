#pragma once
#include "img.h"
#include "Math.h"
#include "Grid.h"
#include <vector>
#include "Observer.h"
#include "CustomCursor.h"
namespace CV {
	class Maze : public Notification::Subject {
	private:
		struct impl;
		IMG::Img & img;
		std::vector<int> buffer;
		/**
		 * Use of pimpl idiom to avoid need to include threading stuff in files including this
		*/
		std::unique_ptr<impl> pimpl;
	public:
		/**
		 * Helper for calculatePath(). Called in a seperate thread to allow gui events to continue to be polled
		 * Should not be called outside the Maze class
		*/
		void calculatePathThread(Math::point start, Math::point end);
	public:
		Maze(IMG::Img & i);
		~Maze();
		void loadMaze(IMG::Img & im);
		void choosePoint();

		/**
		 * Function to calculate the path to get from the start point to end point.
		 * Calls calculatePathThred() in a seperate thread
		*/
		void calculatePath(Math::point start, Math::point end);
		bool isSolving();
	};
}