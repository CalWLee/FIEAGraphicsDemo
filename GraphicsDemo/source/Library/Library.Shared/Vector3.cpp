#include "pch.h"
#include "Vector3.h"

using namespace DirectX;
namespace Library
{
	Vector3::Vector3()
		: x(0), y(0), z(0)
	{

	}

	Vector3::Vector3(float nX, float nY, float nZ)
		: x(nX), y(nY), z(nZ)
	{

	}


	Vector3::Vector3(Vector3& rhs)
		: x(rhs.x), y(rhs.y), z(rhs.z)
	{

	}

	Vector3& Vector3::operator=(Vector3& rhs)
	{
		if (*this != rhs)
		{
			x = rhs.x;
			y = rhs.y;
			z = rhs.z;
		}

		return *this;
	}


	Vector3::Vector3(Vector3&& rhs)
		: x(rhs.x), y(rhs.y), z(rhs.z)
	{
		rhs.x = 0;
		rhs.y = 0;
		rhs.z = 0;
	}

	Vector3& Vector3::operator=(Vector3&& rhs)
	{
		if (*this != rhs)
		{
			x = rhs.x;
			y = rhs.y;
			z = rhs.z;


			rhs.x = 0;
			rhs.y = 0;
			rhs.z = 0;
		}

		return *this;
	}

	float Vector3::GetMagnitude()
	{
		return sqrt(GetSqrMagnitude());
	}

	float Vector3::GetSqrMagnitude()
	{
		return x*x + y*y + z*z;
	}
	
	Vector3& Vector3::GetNormalized()
	{
		return Vector3(x / GetMagnitude(), y / GetMagnitude(), z / GetMagnitude());
	}

	void Vector3::Set(float nX, float nY, float nZ)
	{
		x = nX;
		y = nY;
		z = nZ;
	}

	float Vector3::Dot(Vector3& lhs, Vector3& rhs)
	{
		return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z);
	}

	Vector3 Vector3::Cross(Vector3& lhs, Vector3& rhs)
	{
	}

	Vector3 Vector3::Scale(Vector3& lhs, Vector3& rhs)
	{
		lhs.x *= rhs.x;
		lhs.y *= rhs.y;
		lhs.z *= rhs.z;

	}

	Vector3 Vector3::Max(Vector3& lhs, Vector3& rhs)
	{

	}

	Vector3 Vector3::Min(Vector3& lhs, Vector3& rhs)
	{

	}

	Vector3 Vector3::Lerp(Vector3& a, Vector3& b, float t)
	{

	}


	void Vector3::Normalize()
	{
		x /= GetMagnitude();
		y /= GetMagnitude();
		z /= GetMagnitude();
	}


	Vector3& Vector3::operator-(Vector3& rhs)
	{
		if (*this == rhs)
		{
			return Vector3(x * 2, y * 2, z * 2);
		}

		return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
	}

	Vector3& Vector3::operator+(Vector3& rhs)
	{
		if (*this == rhs)
		{
			return Vector3();
		}

		return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
	}

	Vector3& Vector3::operator*(float f)
	{
		if (f == 0)
		{
			return Vector3();
		}
		else if (f == 1)
		{
			return *this;
		}

		return Vector3(x*f, y*f, z*f);
	}

	Vector3& Vector3::operator/(float f)
	{
		if (f == 0)
		{
			throw new std::exception("Divide by zero exception.");
		}
		else if (f == 1)
		{
			return *this;
		}

		return Vector3(x/f, y/f, z/f);
	}


	bool Vector3::operator==(Vector3& rhs)
	{
		return (x == rhs.x) && (y == rhs.y) && (z == rhs.z);
	}

	bool Vector3::operator!=(Vector3& rhs)
	{
		return !((x == rhs.x) && (y == rhs.y) && (z == rhs.z));
	}

}