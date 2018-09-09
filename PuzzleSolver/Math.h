#pragma once
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

	template<typename T>
	inline T min(T a, T b) {
		return a < b ? a : b;
	}

	template<typename T>
	inline T max(T a, T b) {
		return a < b ? b : a;
	}
}