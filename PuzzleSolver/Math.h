#pragma once
#include <initializer_list>
#include <math.h>
#include <memory>
#include <exception>
#undef radians
#undef min
#undef max
namespace Math {
	struct point {
		int x;
		int y;
		bool operator==(const point & other) const;
	};
	struct matrix {
		double a, b,
			   c, d;
	};
	constexpr double PI = 3.14159265358979323846;

	double radians(double degrees);
	double degrees(double radians);
	/**
	 * @return Math::point Matrix-Vector product
	*/
	point matVecProd(const matrix & mat, const point & vec);
	matrix rotationMatrix(const double degrees);

	point& operator*(const matrix & mat, const point & vec);
	point& operator+(const point& a, const point& b);
	point& operator-(const point& a, const point& b);
	point& operator+=(point& cur, const point& b);
	point& operator-=(point& cur, const point& b);

	int gcd(int a, int b) noexcept;

	template<typename T>
	constexpr inline T min(T a, T b) noexcept {
		return a < b ? a : b;
	}

	template<typename T>
	constexpr inline T max(T a, T b) noexcept {
		return a < b ? b : a;
	}

	template<unsigned short dimensions, typename T>
	class Vector {
	private:
		T * data;
	public:
		Vector() {
			static_assert(dimensions > 0, "Math::Vector must be at least 1 dimension");
			data = new T[dimensions]; 
		}
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
		T& operator[](const unsigned short index) {
			if (index >= dimensions) throw std::out_of_range("Math::Vector access attempt at a nonexistant dimension");
			return data[index];
		}
		~Vector() {
			delete[] data;
		}
		Vector& operator+=(const T & scalar) noexcept {
			for (int i = 0; i < dimensions; ++i)
				data[i] += scalar;
			return *this;
		}
	};

	using vec3f = Vector<3, double>;

	/**
	 * Determines if lines defined by endpoints s1, f1 and s2, f2 intersect
	 * @return intersection point or {INT_MAX, INT_MIN} if they are parallel
	 * No check is done to make sure return value is on the line between enpoints
	 */
	point lineIntersection(point s1, point f1, point s2, point f2);
	/**
	 * Determines if line defined by end points lineStart, lineEnd intersects rect defined by topLeft, btmRt
	 * Internally tests lineIntersection on all four sides of rect with line
	*/
	bool lineRectIntersection(point lineStart, point lineEnd, point topLeft, point btmRt);

	constexpr int int_max = std::numeric_limits<int>::max();
	constexpr int int_min = std::numeric_limits<int>::min();
	

}