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

Math::point & Math::operator+(const point & a, const point & b)
{
	point resultant = { a.x + b.x, a.y + b.y };
	return resultant;
}

Math::point & Math::operator-(const point & a, const point & b)
{
	point resultant = { a.x - b.x, a.y - b.y };
	return resultant;
}

Math::point & Math::operator+=(point & cur, const point & b)
{
	cur.x += b.x;
	cur.y += b.y;
	return cur;

}

Math::point & Math::operator-=(point & cur, const point & b)
{
	cur.x -= b.x;
	cur.y -= b.y;
	return cur;
}

int Math::gcd(int a, int b)
{
	if (b == 0) return a;
	return gcd(b, a % b);
}
