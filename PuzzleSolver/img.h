#pragma once
#include <Image.h>
#include <memory>
#include <assert.h>
#include "Math.h"
namespace IMG {
	struct color {
		unsigned char red;
		unsigned char green;
		unsigned char blue;
		int avg();
		int total();
		explicit operator Color();
	};
	struct pixel {
		color c;
		Math::point p;		
	};
	struct imgState {
		unsigned int width;
		unsigned int height;
		std::vector<color> imgData;
		imgState(const imgState & other);
		imgState(unsigned int width = 0, unsigned int height = 0) : width(width), height(height) { imgData.reserve(width * height); };
		imgState& operator=(const imgState & other);
	};
	class Img;
	class ImgMemento {
		friend class Img;
	private:
		imgState state;
	public:
		imgState getState() { return state; }
		ImgMemento(IMG::Img * img);
	};
	class GrayFunc {
	public:
		virtual color change(const color & original) = 0;
	};
	class LightFunc : public GrayFunc {
	public:
		color change(const color & original) override;
	};
	class LumFunc : public GrayFunc {
	public:
		color change(const color & original) override;
	};
	class Img {
	private:
		std::shared_ptr<Image> img;
		bool loaded;
	public:
		Img() : loaded(false) {};
		void loadFromPath(const char * s);
		void createNew(int width, int height);
		unsigned int width() const;
		unsigned int height() const;
		void setPixel(const pixel c);
		void setPixel(const channel r, const channel g, const channel b, const int x, const int y);
		void rect(Math::point topLeft, Math::point btmRight, color c);
		void drawRect(Math::point topLeft, Math::point btmRight, color c);
		color getPixel(const Math::point p) const;
		color getPixel(const int x, const int y) const;
		void saveAsBmp(const char * path) const;
		void greyscale();
		void trueGrayscale(std::shared_ptr<GrayFunc> function);
		void resize(int width, int height);
		int integralImageValue(int x, int y);
		ImgMemento getMemento();
		void loadFrom(ImgMemento memento);
		bool isLoaded() { return loaded; }
		void drawLine(Math::point start, Math::point end, color c);
		void clear();
	};
}