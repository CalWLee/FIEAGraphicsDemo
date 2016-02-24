#pragma once
#include <DirectXPackedVector.h>
#include <DirectXMath.h>
using namespace DirectX;
namespace Library
{
	class Vector3
	{
	public:
		Vector3();
		Vector3(float nX, float nY, float nZ);

		Vector3(Vector3& rhs);
		Vector3& operator=(Vector3& rhs);

		Vector3(Vector3&& rhs);
		Vector3& operator=(Vector3&& rhs);

		~Vector3() = default;

		float GetMagnitude();
		float GetSqrMagnitude();
		Vector3& GetNormalized();

		void Set(float nX, float nY, float nZ);
		
		static float Dot(Vector3& lhs, Vector3& rhs);
		static Vector3 Cross(Vector3& lhs, Vector3& rhs);
		static Vector3 Scale(Vector3& lhs, Vector3& rhs);
		static Vector3 Max(Vector3& lhs, Vector3& rhs);
		static Vector3 Min(Vector3& lhs, Vector3& rhs);
		static Vector3 Lerp(Vector3& a, Vector3& b, float t);

		void Normalize();

		Vector3& operator-(Vector3& lhs);
		Vector3& operator+(Vector3& lhs);
		Vector3& operator*(float f);
		Vector3& operator/(float f);

		bool operator==(Vector3& rhs);
		bool operator!=(Vector3& rhs);



		float x;
		float y;
		float z;


	};
}