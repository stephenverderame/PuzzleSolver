#include "Math.h"

double Math::radians(double degrees)
{
	return degrees * (PI / 180.0);
}

double Math::degrees(double radians)
{
	return radians * (180.0 / PI);
}

Math::point Math::matVecProd(const matrix & mat, const point & vec)
{
	Math::point p;
	p.x = mat.a * vec.x + mat.b * vec.y;
	p.y = mat.c * vec.x + mat.d * vec.y;
	return p;
}

Math::matrix Math::rotationMatrix(const double degrees)
{
	double rad = radians(degrees);
	matrix m = {
		cos(rad),		-sin(rad),
		sin(rad),		 cos(rad)
	};
	return m;
}

Math::point & Math::operator*(const matrix & mat, const point & vec)
{
	return Math::matVecProd(mat, vec);
}
