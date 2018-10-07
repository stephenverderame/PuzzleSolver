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

int Math::gcd(int a, int b) noexcept
{
	if (b == 0) return a;
	return gcd(b, a % b);
}

Math::point Math::lineIntersection(point s1, point f1, point s2, point f2)
{
	//gets equation of a perpendicular line to the original in ax + by = c form
	//a1(x) + b1(y) = c1
	double a1 = f1.y - s1.y;
	double b1 = s1.x - f1.x;
	double c1 = a1 * s1.x + b1 * s1.y;

	//a2(x) + b2(y) = c2
	double a2 = f2.y - s2.y;
	double b2 = s2.x - f2.x;
	double c2 = a2 * s2.x + b2 * s2.y;
	/*
		[ a1	 b1	]	[x]	 =	[c1]
		[ a2	 b2	]	[y]	 =	[c2]
	*/
	double determinent = a1 * b2 - a2 * b1;
	if (determinent == 0) {
		//lines are parallel
		return{ INT_MAX, INT_MIN };
	}
	/*		   1	     [b2	 -b1]   [c1]  =   [x]
	      ----------     [-a2	  a1]   [c2]  =   [y]
		  a1*b2 - a2*b1
	*/
	double x = (b2 * c1 - b1 * c2) / determinent;
	double y = (a1 * c2 - a2 * c1) / determinent;
	return{ static_cast<int>(x), static_cast<int>(y) };
}

bool Math::lineRectIntersection(point lineStart, point lineEnd, point topLeft, point btmRt)
{
	point i1 = lineIntersection(lineStart, lineEnd, topLeft, { btmRt.x, topLeft.y });
	point i2 = lineIntersection(lineStart, lineEnd, btmRt, { topLeft.x, btmRt.y });

	point i3 = lineIntersection(lineStart, lineEnd, topLeft, { topLeft.x, btmRt.y });
	point i4 = lineIntersection(lineStart, lineEnd, btmRt, { btmRt.x, topLeft.y });
	if (i1.y == topLeft.y && i1.x >= topLeft.x && i1.x <= btmRt.x) return true;
	if (i2.y == btmRt.y && i2.x >= topLeft.x && i2.x <= btmRt.x) return true;
	if (i3.x == topLeft.x && i3.y >= topLeft.y && i3.y <= btmRt.y) return true;
	if (i4.x == btmRt.x && i4.y >= topLeft.y && i4.y <= btmRt.y) return true;
	return false;
}

bool Math::point::operator==(const point & other) const
{
	return x == other.x && y == other.y;
}
