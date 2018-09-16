#pragma once
#include <initializer_list>
#include <math.h>
#undef radians
#undef min
#undef max
namespace Math {
	struct point {
		int x;
		int y;
	};
	struct matrix {
		double a, b,
			   c, d;
	};
	constexpr double PI = 3.14159265358979323846;

	double radians(double degrees);
	double degrees(double radians);
	point matVecProd(const matrix & mat, const point & vec);
	matrix rotationMatrix(const double degrees);

	point& operator*(const matrix & mat, const point & vec);
	point& operator+(const point& a, const point& b);
	point& operator-(const point& a, const point& b);
	point& operator+=(point& cur, const point& b);
	point& operator-=(point& cur, const point& b);

	int gcd(int a, int b);

	template<typename T>
	inline T min(T a, T b) {
		return a < b ? a : b;
	}

	template<typename T>
	inline T max(T a, T b) {
		return a < b ? b : a;
	}

	template<unsigned short dimensions, typename T>
	class Vector {
	private:
		T * data;
	public:
		Vector() { data = new T[dimensions]; }
		Vector(std::initializer_list<T> list) : Vector() {
			int i = 0;
			for (auto item : list) {
				if (i >= dimensions) break;
				data[i++] = item;
			}
		}
		Vector(const Vector<dimensions, T> & other) : Vector() {
			for (int i = 0; i < dimensions; ++i)
				data[i] = other.data[i];
		}
		Vector& operator=(const Vector<dimensions, T> & other) {
			for (int i = 0; i < dimensions; ++i)
				data[i] = other.data[i];
			return *this;
		}
		T& operator[](const int index) {
			return data[index];
		}
		~Vector() {
			delete[] data;
		}
		Vector& operator+=(const T & scalar) {
			for (int i = 0; i < dimensions; ++i)
				data[i] += scalar;
			return *this;
		}
	};

	using vec3f = Vector<3, double>;
}