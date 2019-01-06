#pragma once
#include <Image.h>
#include <memory>
#include <assert.h>
#include "Math.h"
namespace IMG {
	class ImgOutOfRangeException : public std::exception {
	public:
		const char * what() const noexcept override {
			return "attempted access on a point that is not within the matrix's bounds";
		}
	};
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
	/**
	 * Stores raw bitmap data.
	 * When copying an Img, avoids creation of internal Gdi bitmaps and DIB sections
	 * @see Img
	 * @see ImgMememto
	*/
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
		/** 
		 * Constructor which does NOT initialize any important state
		 * @see Img::loadFromPath()
		 * @see Img::createNew()
		 * @see Img::loadFrom()
		*/
		Img() : loaded(false) {};

		/**
		 * Loads an image at the specefied path.
		 * Underlying API code uses stb_image
		*/
		void loadFromPath(const char * s);
		void createNew(int width, int height);
		unsigned int width() const noexcept;
		unsigned int height() const noexcept;
		void setPixel(const pixel c) noexcept;
		void setPixel(const channel r, const channel g, const channel b, const int x, const int y) noexcept;
		void rect(Math::point topLeft, Math::point btmRight, color c);
		void drawRect(Math::point topLeft, Math::point btmRight, color c);
		color getPixel(const Math::point p) const throw(ImgOutOfRangeException);
		color getPixel(const int x, const int y) const throw(ImgOutOfRangeException);

		/**
		 * Saves the image as a 24-bit RGB bitmap
		 * @param path filename to be saved. Should end with ".bmp"
		*/
		void saveAsBmp(const char * path) const;

		//* Wrapper for Image::greyscale(). Turns an image monochrome
		void greyscale();
		/**
		 * Actually makes an image greyscale
		 * @param function pointer to subclass of GrayFunc which determines implementation of GrayScale algorithm
		*/
		void trueGrayscale(std::unique_ptr<GrayFunc> && function);

		//* Resizes the image, does not rescale. Image is also set to black
		void resize(int width, int height);

		/**
		 * Gets integral image table value for specefied pixel
		 * Throws IMG::ImgOutOfRangeException if specefied point is not within the matrix
		*/
		int integralImageValue(int x, int y);
		ImgMemento getMemento();
		void loadFrom(ImgMemento memento);
		bool isLoaded() noexcept { return loaded; }
		void drawLine(Math::point start, Math::point end, color c);

		//* Sets image to all black
		void clear() noexcept;
		bool xInBounds(int x) noexcept;
		bool yInBounds(int y) noexcept;
		/**
		 * For pictures coming from a camera, rotates 90 degrees to put into coreect orientation
		 * Overall the function completely turns the image by 90 degrees
		*/
		void transpose();

		/**
		 * Resamples entire image by the given factor
		 * @param scaler resampling factor. 
		*/
		void scaleByFactor(double scaler);

		void scaleTo(int width, int height);
	};

	class LightweightImage {

	};
}