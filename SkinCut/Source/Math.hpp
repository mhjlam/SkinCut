// Includes portions of SimpleMath -- Simplified C++ Math wrapper for DirectXMath
// http://go.microsoft.com/fwlink/?LinkId=248929


#pragma once

#include <array>
#include <vector>
#include <functional>

#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <DirectXPackedVector.h>



namespace SkinCut
{

// Class definitions
namespace Math
{

constexpr double EPSILON = 1e-6; // 1e-6 = 0.000001
constexpr double ONE_THIRD = 1.0/3.0;
constexpr double PI = 3.1415926535897932384626433832795;
constexpr double TWO_PI = 6.283185307179586476925286766559;

struct Vector2;
struct Vector3;
struct Vector4;
struct Matrix;
struct Ray;
struct Plane;
struct Sphere;
struct Triangle;
struct Quaternion;
struct Quadrilateral;


// Utility operations

bool Equal(float x, float y, float ep = EPSILON);							// True if x is within a margin of epsilon of y
bool Bound(float s, float minimum, float maximum, float ep = EPSILON);		// True if s is between min and max within a margin of epsilon
bool Equal(const Vector3& v0, const Vector3& v1, float ep = EPSILON);		// True if vectors are within a margin of epsilon of each other

float Sign(float f);														// Returns -1 if negative; 0 if 0; +1 if positive
float Clamp(float value, float minimum, float maximum);						// Returns clamped value



// Geometric computations

float PointLineDistance(Vector2& p, Vector2& p0, Vector2& p1);
float PointLineDistance(Vector3& p, Vector3& p0, Vector3& p1);

void Barycentric(Vector3& p, Vector3& v0, Vector3& v1, Vector3& v2, float &u, float &v, float &w);

Ray CreateRay(Vector2& screenPos, Vector2& window, Matrix& projection, Matrix& view);
Ray CreateRay(Vector2& screenPos, Vector2& window, Matrix& projection, Matrix& view, Matrix& world);



// Intersection tests

bool RayTriangleIntersection(Ray& ray, const Triangle& triangle, float& t, float& u, float& v);
bool RayQuadIntersection(const Ray& ray, const Quadrilateral& quad, float& t);
bool RayQuadIntersection(const Ray& ray, const Quadrilateral& quad, float& t, float& u, float& v);
bool RayPlaneIntersection(const Ray& ray, const Plane& plane, float& t);
bool RaySphereIntersection(Ray& ray, Sphere& sphere, float& t);
bool RayPolygonIntersection(Ray& ray, std::vector<Vector3> polygon, float& t);

bool SegmentPointIntersection(Vector3& p0, Vector3& p1, Vector3& p);
bool SegmentTriangleIntersection(Vector3& p0, Vector3& p1, Triangle& triangle, Vector3& point);
bool SegmentQuadIntersection(Vector3& p0, Vector3& p1, Quadrilateral& quad, Vector3& point);
bool SegmentPlaneIntersection(Vector3& p0, Vector3& p1, Plane& plane, Vector3& point);
bool SegmentSphereIntersection(Vector3& p0, Vector3& p1, Sphere& sphere, Vector3& point);
bool SegmentPolygonIntersection(Vector3& p0, Vector3& p1, std::vector<Vector3> polygon, Vector3& point);

bool TriangleTriangleIntersection(Triangle& triangle0, Triangle& triangle1, Vector3& P0, Vector3& P1);
bool TriangleQuadIntersection(Triangle& triangle, Quadrilateral& quad, Vector3& P0, Vector3& P1);




// Vector (2D)

struct Vector2 : public DirectX::XMFLOAT2 
{
	Vector2() : DirectX::XMFLOAT2(0.f, 0.f) {}
	explicit Vector2(float x) : DirectX::XMFLOAT2(x, x) {}
	Vector2(float _x, float _y) : DirectX::XMFLOAT2(_x, _y) {}
	explicit Vector2(_In_reads_(2) const float *pArray) : DirectX::XMFLOAT2(pArray) {}
	Vector2(DirectX::FXMVECTOR V) { XMStoreFloat2(this, V); }

	operator DirectX::XMVECTOR() const { return XMLoadFloat2(this); }

	// Comparison operators
	bool operator< (const Vector2& V) const; // added
	bool operator==(const Vector2& V) const;
	bool operator!=(const Vector2& V) const;

	// Assignment operators
	Vector2& operator= (const Vector2& V) { x = V.x; y = V.y; return *this; }
	Vector2& operator+=(const Vector2& V);
	Vector2& operator-=(const Vector2& V);
	Vector2& operator*=(const Vector2& V);
	Vector2& operator*=(float S);
	Vector2& operator/=(float S);

	// Unary operators
	Vector2 operator+() const { return *this; }
	Vector2 operator-() const { return Vector2(-x, -y); }

	// Binary operators
	friend Vector2 operator+(const Vector2& V1, const Vector2& V2);
	friend Vector2 operator-(const Vector2& V1, const Vector2& V2);
	friend Vector2 operator-(const Vector2& V, const float F);
	friend Vector2 operator*(const Vector2& V1, const Vector2& V2);
	friend Vector2 operator*(const Vector2& V, float S);
	friend Vector2 operator/(const Vector2& V1, const Vector2& V2);
	friend Vector2 operator*(float S, const Vector2& V);

	// Casting operators
	operator float*();
	operator const float*() const;

	// Vector operations
	bool InBounds(const Vector2& Bounds) const;
	float Length() const;
	float LengthSquared() const;
	float Dot(const Vector2& V) const;
	Vector2 Cross(const Vector2& V) const;
	static float Dot(const Vector2& v1, const Vector2& v2);
	static float Length(const Vector2& v);
	static Vector2 Cross(const Vector2& v1, const Vector2& v2);
	void Normalize();
	static Vector2 Normalize(const Vector2& v1);
	void Clamp(const Vector2& vmin, const Vector2& vmax);
	void Clamp(const Vector2& vmin, const Vector2& vmax, Vector2& result) const;

	// Static functions
	static float Distance(const Vector2& v1, const Vector2& v2);
	static float DistanceSquared(const Vector2& v1, const Vector2& v2);
	static void Min(const Vector2& v1, const Vector2& v2, Vector2& result);
	static void Max(const Vector2& v1, const Vector2& v2, Vector2& result);
	static void Lerp(const Vector2& v1, const Vector2& v2, float t, Vector2& result);
	static void SmoothStep(const Vector2& v1, const Vector2& v2, float t, Vector2& result);
	static void Barycentric(const Vector2& v1, const Vector2& v2, const Vector2& v3, float f, float g, Vector2& result);
	static void CatmullRom(const Vector2& v1, const Vector2& v2, const Vector2& v3, const Vector2& v4, float t, Vector2& result);
	static void Hermite(const Vector2& v1, const Vector2& t1, const Vector2& v2, const Vector2& t2, float t, Vector2& result);
	static void Reflect(const Vector2& ivec, const Vector2& nvec, Vector2& result);
	static void Refract(const Vector2& ivec, const Vector2& nvec, float refractionIndex, Vector2& result);
	static void Transform(const Vector2& v, const Quaternion& quat, Vector2& result);
	static void Transform(const Vector2& v, const Matrix& m, Vector2& result);
	static void Transform(_In_reads_(count) const Vector2* varray, size_t count, const Matrix& m, _Out_writes_(count) Vector2* resultArray);
	static void Transform(const Vector2& v, const Matrix& m, Vector4& result);
	static void Transform(_In_reads_(count) const Vector2* varray, size_t count, const Matrix& m, _Out_writes_(count) Vector4* resultArray);
	static void TransformNormal(const Vector2& v, const Matrix& m, Vector2& result);
	static void TransformNormal(_In_reads_(count) const Vector2* varray, size_t count, const Matrix& m, _Out_writes_(count) Vector2* resultArray);

	static Vector2 Min(const Vector2& v1, const Vector2& v2);
	static Vector2 Max(const Vector2& v1, const Vector2& v2);
	static Vector2 Lerp(const Vector2& v1, const Vector2& v2, float t);
	static Vector2 SmoothStep(const Vector2& v1, const Vector2& v2, float t);
	static Vector2 Barycentric(const Vector2& v1, const Vector2& v2, const Vector2& v3, float f, float g);
	static Vector2 CatmullRom(const Vector2& v1, const Vector2& v2, const Vector2& v3, const Vector2& v4, float t);
	static Vector2 Hermite(const Vector2& v1, const Vector2& t1, const Vector2& v2, const Vector2& t2, float t);
	static Vector2 Reflect(const Vector2& ivec, const Vector2& nvec);
	static Vector2 Refract(const Vector2& ivec, const Vector2& nvec, float refractionIndex);
	static Vector2 Transform(const Vector2& v, const Quaternion& quat);
	static Vector2 Transform(const Vector2& v, const Matrix& m);
	static Vector2 TransformNormal(const Vector2& v, const Matrix& m);
};



// Vector (3D)

struct Vector3 : public DirectX::XMFLOAT3
{
	Vector3() : DirectX::XMFLOAT3(0.f, 0.f, 0.f) {}
	explicit Vector3(float x) : DirectX::XMFLOAT3(x, x, x) {}
	Vector3(float _x, float _y, float _z) : DirectX::XMFLOAT3(_x, _y, _z) {}
	explicit Vector3(_In_reads_(3) const float *pArray) : DirectX::XMFLOAT3(pArray) {}
	Vector3(DirectX::FXMVECTOR V) { DirectX::XMStoreFloat3(this, V); }

	operator DirectX::XMVECTOR() const { return DirectX::XMLoadFloat3(this); }

	// Comparison operators
	bool operator<(const Vector3& V) const; // added
	bool operator>(const Vector3& V) const; // added
	bool operator==(const Vector3& V) const;
	bool operator!=(const Vector3& V) const;

	// Assignment operators
	Vector3& operator=(const Vector3& V);
	Vector3& operator+=(const Vector3& V);
	Vector3& operator-=(const Vector3& V);
	Vector3& operator*=(const Vector3& V);
	Vector3& operator*=(float S);
	Vector3& operator/=(float S);

	// Unary operators
	Vector3 operator+() const;
	Vector3 operator-() const;

	// Binary operators
	friend Vector3 operator+(const Vector3& V1, const Vector3& V2);
	friend Vector3 operator-(const Vector3& V1, const Vector3& V2);
	friend Vector3 operator*(const Vector3& V1, const Vector3& V2);
	friend Vector3 operator*(const Vector3& V, float S);
	friend Vector3 operator/(const Vector3& V1, const Vector3& V2);
	friend Vector3 operator*(float S, const Vector3& V);
	friend Vector3 operator^(const Vector3& V1, const Vector3& V2);

	// Casting operators
	operator float*();
	operator const float*() const;

	// Vector operations
	void Normalize();
	void Clamp(const Vector3& vmin, const Vector3& vmax);
	void Clamp(const Vector3& vmin, const Vector3& vmax, Vector3& result) const;

	bool InBounds(const Vector3& Bounds) const;
	float Length() const;
	float LengthSquared() const;
	float Dot(const Vector3& V) const;
	Vector3 Cross(const Vector3& V) const;

	// Static functions
	static float Dot(const Vector3& v1, const Vector3& v2);
	static float Length(const Vector3& v1);
	static float LengthSquared(const Vector3& v1);
	static float Distance(const Vector3& v1, const Vector3& v2);
	static float DistanceSquared(const Vector3& v1, const Vector3& v2);
	static void Min(const Vector3& v1, const Vector3& v2, Vector3& result);
	static void Max(const Vector3& v1, const Vector3& v2, Vector3& result);
	static void Lerp(const Vector3& v1, const Vector3& v2, float t, Vector3& result);
	static void SmoothStep(const Vector3& v1, const Vector3& v2, float t, Vector3& result);
	static void Barycentric(const Vector3& v1, const Vector3& v2, const Vector3& v3, float f, float g, Vector3& result);
	static void CatmullRom(const Vector3& v1, const Vector3& v2, const Vector3& v3, const Vector3& v4, float t, Vector3& result);
	static void Hermite(const Vector3& v1, const Vector3& t1, const Vector3& v2, const Vector3& t2, float t, Vector3& result);
	static void Reflect(const Vector3& ivec, const Vector3& nvec, Vector3& result);
	static void Refract(const Vector3& ivec, const Vector3& nvec, float refractionIndex, Vector3& result);
	static void Transform(const Vector3& v, const Quaternion& quat, Vector3& result);
	static void Transform(const Vector3& v, const Matrix& m, Vector3& result);
	static void Transform(_In_reads_(count) const Vector3* varray, size_t count, const Matrix& m, _Out_writes_(count) Vector3* resultArray);
	static void Transform(const Vector3& v, const Matrix& m, Vector4& result);
	static void Transform(_In_reads_(count) const Vector3* varray, size_t count, const Matrix& m, _Out_writes_(count) Vector4* resultArray);
	static void TransformNormal(const Vector3& v, const Matrix& m, Vector3& result);
	static void TransformNormal(_In_reads_(count) const Vector3* varray, size_t count, const Matrix& m, _Out_writes_(count) Vector3* resultArray);

	static Vector3 Null();
	static Vector3 Cross(const Vector3& v1, const Vector3& v2);
	static Vector3 Normalize(const Vector3& v1);
	static Vector3 Min(const Vector3& v1, const Vector3& v2);
	static Vector3 Max(const Vector3& v1, const Vector3& v2);
	static Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);
	static Vector3 SmoothStep(const Vector3& v1, const Vector3& v2, float t);
	static Vector3 Barycentric(const Vector3& v1, const Vector3& v2, const Vector3& v3, float f, float g);
	static Vector3 CatmullRom(const Vector3& v1, const Vector3& v2, const Vector3& v3, const Vector3& v4, float t);
	static Vector3 Hermite(const Vector3& v1, const Vector3& t1, const Vector3& v2, const Vector3& t2, float t);
	static Vector3 Reflect(const Vector3& ivec, const Vector3& nvec);
	static Vector3 Refract(const Vector3& ivec, const Vector3& nvec, float refractionIndex);
	static Vector3 Transform(const Vector3& v, const Quaternion& quat);
	static Vector3 Transform(const Vector3& v, const Matrix& m);
	static Vector3 TransformNormal(const Vector3& v, const Matrix& m);
};



// Vector (4D)

struct Vector4 : public DirectX::XMFLOAT4
{
	Vector4() : DirectX::XMFLOAT4(0.f, 0.f, 0.f, 0.f) {}
	explicit Vector4(float x) : DirectX::XMFLOAT4(x, x, x, x) {}
	Vector4(float _x, float _y, float _z, float _w) : DirectX::XMFLOAT4(_x, _y, _z, _w) {}
	explicit Vector4(_In_reads_(4) const float *pArray) : DirectX::XMFLOAT4(pArray) {}
	Vector4(DirectX::FXMVECTOR V) { DirectX::XMStoreFloat4(this, V); }

	operator DirectX::XMVECTOR() const { return DirectX::XMLoadFloat4(this); }

	// Comparison operators
	bool operator<(const Vector4& V) const; // added
	bool operator>(const Vector4& V) const; // added
	bool operator==(const Vector4& V) const;
	bool operator!=(const Vector4& V) const;

	// Assignment operators
	Vector4& operator= (const Vector4& V) { x = V.x; y = V.y; z = V.z; w = V.w; return *this; }
	Vector4& operator+=(const Vector4& V);
	Vector4& operator-=(const Vector4& V);
	Vector4& operator*=(const Vector4& V);
	Vector4& operator*=(float S);
	Vector4& operator/=(float S);

	// Unary operators
	Vector4 operator+() const { return *this; }
	Vector4 operator-() const;

	// Binary operators
	friend Vector4 operator+(const Vector4& V1, const Vector4& V2);
	friend Vector4 operator-(const Vector4& V1, const Vector4& V2);
	friend Vector4 operator*(const Vector4& V1, const Vector4& V2);
	friend Vector4 operator*(const Vector4& V, float S);
	friend Vector4 operator/(const Vector4& V1, const Vector4& V2);
	friend Vector4 operator*(float S, const Vector4& V);

	// Casting operators
	operator float*();
	operator const float*() const;

	// Vector operations
	bool InBounds(const Vector4& Bounds) const;
	void Cross(const Vector4& v1, const Vector4& v2, Vector4& result) const;
	void Normalize();
	void Clamp(const Vector4& vmin, const Vector4& vmax);
	void Clamp(const Vector4& vmin, const Vector4& vmax, Vector4& result) const;

	float Length() const;
	float LengthSquared() const;
	float Dot(const Vector4& V) const;
	Vector4 Cross(const Vector4& v1, const Vector4& v2) const;

	// Static functions
	static float Distance(const Vector4& v1, const Vector4& v2);
	static float DistanceSquared(const Vector4& v1, const Vector4& v2);
	static void Min(const Vector4& v1, const Vector4& v2, Vector4& result);
	static void Max(const Vector4& v1, const Vector4& v2, Vector4& result);
	static void Lerp(const Vector4& v1, const Vector4& v2, float t, Vector4& result);
	static void SmoothStep(const Vector4& v1, const Vector4& v2, float t, Vector4& result);
	static void Barycentric(const Vector4& v1, const Vector4& v2, const Vector4& v3, float f, float g, Vector4& result);
	static void CatmullRom(const Vector4& v1, const Vector4& v2, const Vector4& v3, const Vector4& v4, float t, Vector4& result);
	static void Hermite(const Vector4& v1, const Vector4& t1, const Vector4& v2, const Vector4& t2, float t, Vector4& result);
	static void Reflect(const Vector4& ivec, const Vector4& nvec, Vector4& result);
	static void Refract(const Vector4& ivec, const Vector4& nvec, float refractionIndex, Vector4& result);
	static void Transform(const Vector2& v, const Quaternion& quat, Vector4& result);
	static void Transform(const Vector3& v, const Quaternion& quat, Vector4& result);
	static void Transform(const Vector4& v, const Quaternion& quat, Vector4& result);
	static void Transform(const Vector4& v, const Matrix& m, Vector4& result);
	static void Transform(_In_reads_(count) const Vector4* varray, size_t count, const Matrix& m, _Out_writes_(count) Vector4* resultArray);

	static Vector4 Normalize(const Vector4& v1);
	static Vector4 Min(const Vector4& v1, const Vector4& v2);
	static Vector4 Max(const Vector4& v1, const Vector4& v2);
	static Vector4 Lerp(const Vector4& v1, const Vector4& v2, float t);
	static Vector4 SmoothStep(const Vector4& v1, const Vector4& v2, float t);
	static Vector4 Barycentric(const Vector4& v1, const Vector4& v2, const Vector4& v3, float f, float g);
	static Vector4 CatmullRom(const Vector4& v1, const Vector4& v2, const Vector4& v3, const Vector4& v4, float t);
	static Vector4 Hermite(const Vector4& v1, const Vector4& t1, const Vector4& v2, const Vector4& t2, float t);
	static Vector4 Reflect(const Vector4& ivec, const Vector4& nvec);
	static Vector4 Refract(const Vector4& ivec, const Vector4& nvec, float refractionIndex);
	static Vector4 Transform(const Vector2& v, const Quaternion& quat);
	static Vector4 Transform(const Vector3& v, const Quaternion& quat);
	static Vector4 Transform(const Vector4& v, const Quaternion& quat);
	static Vector4 Transform(const Vector4& v, const Matrix& m);
};



// RGB Color

struct Color : public DirectX::XMFLOAT4
{
	Color() : DirectX::XMFLOAT4(0, 0, 0, 1.f) {}
	Color(float _r, float _g, float _b) : DirectX::XMFLOAT4(_r, _g, _b, 1.f) {}
	Color(float _r, float _g, float _b, float _a) : DirectX::XMFLOAT4(_r, _g, _b, _a) {}
	explicit Color(const Vector3& clr) : DirectX::XMFLOAT4(clr.x, clr.y, clr.z, 1.f) {}
	explicit Color(const Vector4& clr) : DirectX::XMFLOAT4(clr.x, clr.y, clr.z, clr.w) {}
	explicit Color(_In_reads_(4) const float *pArray) : DirectX::XMFLOAT4(pArray) {}
	Color(DirectX::FXMVECTOR V) { DirectX::XMStoreFloat4(this, V); }
	Color(const DirectX::XMVECTORF32& V) { DirectX::XMStoreFloat4(this, V); }

	explicit Color(const DirectX::PackedVector::XMCOLOR& Packed); // BGRA Direct3D 9 D3DCOLOR packed color
	explicit Color(const DirectX::PackedVector::XMUBYTEN4& Packed); // RGBA XNA Game Studio packed color

	operator DirectX::XMVECTOR() const { return DirectX::XMLoadFloat4(this); }
	operator float*() { return reinterpret_cast<float*>(this); }
	operator const float*() const { return reinterpret_cast<const float*>(this); }

	// Comparison operators
	bool operator<(const Color& c) const;
	bool operator>(const Color& c) const;
	bool operator==(const Color& c) const;
	bool operator!=(const Color& c) const;

	// Assignment operators
	Color& operator= (const Color& c) { x = c.x; y = c.y; z = c.z; w = c.w; return *this; }
	Color& operator+=(const Color& c);
	Color& operator-=(const Color& c);
	Color& operator*=(const Color& c);
	Color& operator*=(float S);
	Color& operator/=(const Color& c);

	// Unary operators
	Color operator+() const { return *this; }
	Color operator-() const;

	// Binary operators
	friend Color operator+(const Color& C1, const Color& C2);
	friend Color operator-(const Color& C1, const Color& C2);
	friend Color operator*(const Color& C1, const Color& C2);
	friend Color operator*(const Color& C, float S);
	friend Color operator/(const Color& C1, const Color& C2);
	friend Color operator*(float S, const Color& C);

	// Properties
	void R(float r) { x = r; }
	void G(float g) { y = g; }
	void B(float b) { z = b; }
	void A(float a) { w = a; }

	float R() const { return x; }
	float G() const { return y; }
	float B() const { return z; }
	float A() const { return w; }

	// Color operations
	DirectX::PackedVector::XMCOLOR BGRA() const;
	DirectX::PackedVector::XMUBYTEN4 RGBA() const;

	Vector3 ToVector3() const;
	Vector4 ToVector4() const;

	void Negate();
	void Negate(Color& result) const;
	void Saturate();
	void Saturate(Color& result) const;
	void Premultiply();
	void Premultiply(Color& result) const;
	void AdjustSaturation(float sat);
	void AdjustSaturation(float sat, Color& result) const;
	void AdjustContrast(float contrast);
	void AdjustContrast(float contrast, Color& result) const;

	// Static functions
	static void Modulate(const Color& c1, const Color& c2, Color& result); 
	static void Lerp(const Color& c1, const Color& c2, float t, Color& result);

	static Color Modulate(const Color& c1, const Color& c2);
	static Color Lerp(const Color& c1, const Color& c2, float t);

	static Color RGBtoHSV(const Color& rgb);
	static Color HSVtoRGB(const Color& hsv);
};



// Matrix 4x4 (assumes right-handed coordinates)

struct Matrix : public DirectX::XMFLOAT4X4
{
	Matrix() : DirectX::XMFLOAT4X4(1.f, 0.f, 0.f, 0.f,
								   0.f, 1.f, 0.f, 0.f,
								   0.f, 0.f, 1.f, 0.f,
								   0.f, 0.f, 0.f, 1.f) {}

	Matrix(float m00, float m01, float m02, float m03,
		   float m10, float m11, float m12, float m13,
		   float m20, float m21, float m22, float m23,
		   float m30, float m31, float m32, float m33) : 
		DirectX::XMFLOAT4X4(m00, m01, m02, m03,
							m10, m11, m12, m13,
							m20, m21, m22, m23,
							m30, m31, m32, m33) {}

	explicit Matrix(const Vector2& r0, const Vector2& r1) : 
		DirectX::XMFLOAT4X4(r0.x, r0.y, 0.0f, 0.0f,
							r1.x, r1.y, 0.0f, 0.0f,
							0.0f, 0.0f, 0.0f, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f) {}

	explicit Matrix(const Vector3& r0, const Vector3& r1, const Vector3& r2) : 
		DirectX::XMFLOAT4X4(r0.x, r0.y, r0.z, 0.0f,
							r1.x, r1.y, r1.z, 0.0f,
							r2.x, r2.y, r2.z, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f) {}

	explicit Matrix(const Vector4& r0, const Vector4& r1, const Vector4& r2, const Vector4& r3) : 
		DirectX::XMFLOAT4X4(r0.x, r0.y, r0.z, r0.w,
							r1.x, r1.y, r1.z, r1.w,
							r2.x, r2.y, r2.z, r2.w,
							r3.x, r3.y, r3.z, r3.w) {}

	explicit Matrix(_In_reads_(16) const float *pArray) : DirectX::XMFLOAT4X4(pArray) {}
	Matrix(DirectX::CXMMATRIX M) { DirectX::XMStoreFloat4x4(this, M); }

	operator DirectX::XMMATRIX() const { return DirectX::XMLoadFloat4x4(this); }

	// Comparison operators
	bool operator==(const Matrix& M) const;
	bool operator!=(const Matrix& M) const;

	// Assignment operators
	Matrix& operator= (const Matrix& M) { memcpy_s(this, sizeof(float)*16, &M, sizeof(float)*16); return *this; }
	Matrix& operator+=(const Matrix& M);
	Matrix& operator-=(const Matrix& M);
	Matrix& operator*=(const Matrix& M);
	Matrix& operator*=(float S);
	Matrix& operator/=(const Matrix& M); // Element-wise divide
	Matrix& operator/=(float S);

	// Unary operators
	Matrix operator+() const { return *this; }
	Matrix operator-() const;

	// Binary operators
	friend Matrix operator+(const Matrix& M1, const Matrix& M2);
	friend Matrix operator-(const Matrix& M1, const Matrix& M2);
	friend Matrix operator*(const Matrix& M1, const Matrix& M2);
	friend Matrix operator*(const Matrix& M, float S);
	friend Matrix operator/(const Matrix& M, float S);
	friend Matrix operator/(const Matrix& M1, const Matrix& M2); // Element-wise divide
	friend Matrix operator*(float S, const Matrix& M);

	// Casting operators
	operator float*();
	operator const float*() const;

	// Properties
	void Up(const Vector3& v) { _21 = v.x; _22 = v.y; _23 = v.z; }
	void Down(const Vector3& v) { _21 = -v.x; _22 = -v.y; _23 = -v.z; }
	void Right(const Vector3& v) { _11 = v.x; _12 = v.y; _13 = v.z; }
	void Left(const Vector3& v) { _11 = -v.x; _12 = -v.y; _13 = -v.z; }
	void Forward(const Vector3& v) { _31 = -v.x; _32 = -v.y; _33 = -v.z; }
	void Backward(const Vector3& v) { _31 = v.x; _32 = v.y; _33 = v.z; }
	void Translation(const Vector3& v) { _41 = v.x; _42 = v.y; _43 = v.z; }

	Vector3 Up() const { return Vector3(_21, _22, _23); }
	Vector3 Down() const { return Vector3(-_21, -_22, -_23); }
	Vector3 Right() const { return Vector3(_11, _12, _13); }
	Vector3 Left() const { return Vector3(-_11, -_12, -_13); }
	Vector3 Forward() const { return Vector3(-_31, -_32, -_33); }
	Vector3 Backward() const { return Vector3(_31, _32, _33); }
	Vector3 Translation() const { return Vector3(_41, _42, _43); }

	// Matrix operations
	void Transpose(Matrix& result) const;
	void Invert(Matrix& result) const;

	bool Decompose(Vector3& scale, Quaternion& rotation, Vector3& translation);
	float Determinant() const;

	Matrix Transpose() const;
	Matrix Invert() const;

	// Static functions
	static void Lerp(const Matrix& M1, const Matrix& M2, float t, Matrix& result);
	static void Transform(const Matrix& M, const Quaternion& rotation, Matrix& result);

	static Matrix Identity();
	static Matrix CreateTranslation(const Vector3& Position);
	static Matrix CreateTranslation(float x, float y, float z);
	static Matrix CreateScale(const Vector3& scales);
	static Matrix CreateScale(float xs, float ys, float zs);
	static Matrix CreateScale(float scale);
	static Matrix CreateRotationX(float radians);
	static Matrix CreateRotationY(float radians);
	static Matrix CreateRotationZ(float radians);
	static Matrix CreateFromAxisAngle(const Vector3& axis, float angle);
	static Matrix CreatePerspectiveFieldOfView(float fov, float aspectRatio, float nearPlane, float farPlane);
	static Matrix CreatePerspective(float width, float height, float nearPlane, float farPlane);
	static Matrix CreatePerspectiveOffCenter(float left, float right, float bottom, float top, float nearPlane, float farPlane);
	static Matrix CreateOrthographic(float width, float height, float zNearPlane, float zFarPlane);
	static Matrix CreateOrthographicOffCenter(float left, float right, float bottom, float top, float zNearPlane, float zFarPlane);
	static Matrix CreateLookAt(const Vector3& Position, const Vector3& target, const Vector3& up);
	static Matrix CreateWorld(const Vector3& Position, const Vector3& forward, const Vector3& up);
	static Matrix CreateFromQuaternion(const Quaternion& quat);
	static Matrix CreateFromYawPitchRoll(float yaw, float pitch, float roll);
	static Matrix CreateShadow(const Vector3& lightDir, const Plane& plane);
	static Matrix CreateReflection(const Plane& plane);
	static Matrix Lerp(const Matrix& M1, const Matrix& M2, float t);
	static Matrix Transform(const Matrix& M, const Quaternion& rotation);
};



// Quaternion

struct Quaternion : public DirectX::XMFLOAT4
{
	Quaternion() : DirectX::XMFLOAT4(0, 0, 0, 1.f) {}
	Quaternion(float _x, float _y, float _z, float _w) : DirectX::XMFLOAT4(_x, _y, _z, _w) {}
	Quaternion(const Vector3& v, float scalar) : DirectX::XMFLOAT4(v.x, v.y, v.z, scalar) {}
	explicit Quaternion(const Vector4& v) : DirectX::XMFLOAT4(v.x, v.y, v.z, v.w) {}
	explicit Quaternion(_In_reads_(4) const float *pArray) : DirectX::XMFLOAT4(pArray) {}
	Quaternion(DirectX::FXMVECTOR V) { DirectX::XMStoreFloat4(this, V); }

	operator DirectX::XMVECTOR() const { return DirectX::XMLoadFloat4(this); }

	// Comparison operators
	bool operator==(const Quaternion& q) const;
	bool operator!=(const Quaternion& q) const;

	// Assignment operators
	Quaternion& operator=(const Quaternion& q) { x = q.x; y = q.y; z = q.z; w = q.w; return *this; }
	Quaternion& operator+=(const Quaternion& q);
	Quaternion& operator-=(const Quaternion& q);
	Quaternion& operator*=(const Quaternion& q);
	Quaternion& operator*=(float S);
	Quaternion& operator/=(const Quaternion& q);

	// Unary operators
	Quaternion operator+() const { return *this; }
	Quaternion operator-() const;

	// Binary operators
	friend Quaternion operator+(const Quaternion& Q1, const Quaternion& Q2);
	friend Quaternion operator-(const Quaternion& Q1, const Quaternion& Q2);
	friend Quaternion operator*(const Quaternion& Q1, const Quaternion& Q2);
	friend Quaternion operator*(const Quaternion& Q, float S);
	friend Quaternion operator/(const Quaternion& Q1, const Quaternion& Q2);
	friend Quaternion operator*(float S, const Quaternion& Q);

	// Quaternion operations
	void Normalize();
	void Normalize(Quaternion& result) const;
	void Conjugate();
	void Conjugate(Quaternion& result) const;
	void Inverse(Quaternion& result) const;

	float Length() const;
	float LengthSquared() const;
	float Dot(const Quaternion& Q) const;

	// Static functions
	static void Lerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion& result);
	static void Slerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion& result);
	static void Concatenate(const Quaternion& q1, const Quaternion& q2, Quaternion& result);

	static Quaternion CreateFromAxisAngle(const Vector3& axis, float angle);
	static Quaternion CreateFromYawPitchRoll(float yaw, float pitch, float roll);
	static Quaternion CreateFromRotationMatrix(const Matrix& M);
	static Quaternion Lerp(const Quaternion& q1, const Quaternion& q2, float t);
	static Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float t);
	static Quaternion Concatenate(const Quaternion& q1, const Quaternion& q2);
};



// Plane

struct Plane : public DirectX::XMFLOAT4
{
	Plane() : DirectX::XMFLOAT4(0.f, 1.f, 0.f, 0.f) {}
	Plane(float _x, float _y, float _z, float _w) : DirectX::XMFLOAT4(_x, _y, _z, _w) {}
	Plane(const Vector3& normal, float d) : DirectX::XMFLOAT4(normal.x, normal.y, normal.z, d) {}
	Plane(const Vector3& point1, const Vector3& point2, const Vector3& point3);
	Plane(const Vector3& point, const Vector3& normal);
	explicit Plane(const Vector4& v) : DirectX::XMFLOAT4(v.x, v.y, v.z, v.w) {}
	explicit Plane(_In_reads_(4) const float *pArray) : DirectX::XMFLOAT4(pArray) {}
	Plane(DirectX::FXMVECTOR V) { DirectX::XMStoreFloat4(this, V); }

	operator DirectX::XMVECTOR() const { return DirectX::XMLoadFloat4(this); }

	// Comparison operators
	bool operator==(const Plane& p) const;
	bool operator!=(const Plane& p) const;

	// Assignment operators
	Plane& operator=(const Plane& p) { x = p.x; y = p.y; z = p.z; w = p.w; return *this; }

	// Properties
	void Normal(const Vector3& normal) { x = normal.x; y = normal.y; z = normal.z; }
	void D(float d) { w = d; }

	Vector3 Normal() const { return Vector3(x, y, z); }
	float D() const { return w; }

	// Plane operations
	void Normalize();
	void Normalize(Plane& result) const;

	float Dot(const Vector4& v) const;
	float DotCoordinate(const Vector3& Position) const;
	float DotNormal(const Vector3& normal) const;

	// Static functions
	static void Transform(const Plane& plane, const Matrix& M, Plane& result);
	static void Transform(const Plane& plane, const Quaternion& rotation, Plane& result);

	static Plane Transform(const Plane& plane, const Matrix& M);
	static Plane Transform(const Plane& plane, const Quaternion& rotation); // Input quaternion must be the inverse transpose of the transformation
};



// Ray

struct Ray
{
	Vector3 origin;
	Vector3 direction;

	Ray() : origin(0,0,0), direction(0,0,1) {}
	Ray(const Vector3& pos, const Vector3& dir) : origin(pos), direction(dir) {}

	// Comparison operators
	bool operator==(const Ray& r) const;
	bool operator!=(const Ray& r) const;

	// Ray operations
	bool Intersects(const DirectX::BoundingSphere& sphere, _Out_ float& Dist) const;
	bool Intersects(const DirectX::BoundingBox& box, _Out_ float& Dist) const;
	bool Intersects(const Vector3& v0, const Vector3& v1, const Vector3& v2, _Out_ float& Dist) const;
	bool Intersects(const Plane& plane, _Out_ float& Dist) const;
};




// Triangle (3D; 3-simplex)

struct Triangle
{
	Vector3 v0, v1, v2;
	Triangle() : v0(0,0,0), v1(0,0,0), v2(0,0,0) {}
	Triangle(const Vector3& v_0, const Vector3& v_1, const Vector3& v_2) : v0(v_0), v1(v_1), v2(v_2) {}
};



// Quadrilateral (3D)

struct Quadrilateral
{
	Vector3 v0, v1, v2, v3;
	Quadrilateral() : v0(0,0,0), v1(0,0,0), v2(0,0,0), v3(0,0,0) {}
	Quadrilateral(Vector3 v_0, Vector3 v_1, Vector3 v_2, Vector3 v_3) : v0(v_0), v1(v_1), v2(v_2), v3(v_3) {}
};



// Sphere

struct Sphere
{
	Vector3 center;
	float radius;

	Sphere() : center(0,0,0), radius(0.0f) {}
	Sphere(Vector3 c, float r) : center(c), radius(r) {}
};


}



// Inline implementation

namespace Math
{

/****************************************************************************
 * Vector2
 ****************************************************************************/

inline bool Vector2::operator<(const Vector2& V) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR v2 = XMLoadFloat2(&V);
	return XMVector2Less(v1, v2);
}

inline bool Vector2::operator==(const Vector2& V) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR v2 = XMLoadFloat2(&V);
	return XMVector2Equal(v1, v2);
}

inline bool Vector2::operator!=(const Vector2& V) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR v2 = XMLoadFloat2(&V);
	return XMVector2NotEqual(v1, v2);
}


inline Vector2& Vector2::operator+=(const Vector2& V)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR v2 = XMLoadFloat2(&V);
	XMVECTOR X = XMVectorAdd(v1,v2);
	XMStoreFloat2(this, X);
	return *this;
}

inline Vector2& Vector2::operator-=(const Vector2& V)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR v2 = XMLoadFloat2(&V);
	XMVECTOR X = XMVectorSubtract(v1,v2);
	XMStoreFloat2(this, X);
	return *this;
}

inline Vector2& Vector2::operator*=(const Vector2& V)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR v2 = XMLoadFloat2(&V);
	XMVECTOR X = XMVectorMultiply(v1,v2);
	XMStoreFloat2(this, X);
	return *this;
}

inline Vector2& Vector2::operator*=(float S)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR X = XMVectorScale(v1,S);
	XMStoreFloat2(this, X);
	return *this;
} 

inline Vector2& Vector2::operator/=(float S)
{
	using namespace DirectX;
	assert(S != 0.0f);
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR X = XMVectorScale(v1, 1.f/S);
	XMStoreFloat2(this, X);
	return *this;
}


inline Vector2::operator float*()
{
	return (float*) &x;
}

inline Vector2::operator const float*() const
{
	return (const float*) &x;
}


inline Vector2 operator+(const Vector2& V1, const Vector2& V2)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(&V1);
	XMVECTOR v2 = XMLoadFloat2(&V2);
	XMVECTOR X = XMVectorAdd(v1,v2);
	Vector2 R; XMStoreFloat2(&R, X); return R;
}

inline Vector2 operator-(const Vector2& V1, const Vector2& V2)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(&V1);
	XMVECTOR v2 = XMLoadFloat2(&V2);
	XMVECTOR X = XMVectorSubtract(v1,v2);
	Vector2 R; XMStoreFloat2(&R, X); return R;
}

inline Vector2 operator-(const Vector2& V, const float f)
{
	using namespace DirectX;
	XMVECTOR V1 = XMLoadFloat2(&V);
	XMFLOAT2 F(f, f);
	XMVECTOR V2 = XMLoadFloat2(&F);
	XMVECTOR X = XMVectorSubtract(V1, V2);
	Vector2 R; XMStoreFloat2(&R, X); return R;
}

inline Vector2 operator*(const Vector2& V1, const Vector2& V2)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(&V1);
	XMVECTOR v2 = XMLoadFloat2(&V2);
	XMVECTOR X = XMVectorMultiply(v1,v2);
	Vector2 R; XMStoreFloat2(&R, X); return R;
}

inline Vector2 operator*(const Vector2& V, float S)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(&V);
	XMVECTOR X = XMVectorScale(v1,S);
	Vector2 R; XMStoreFloat2(&R, X); return R;
}

inline Vector2 operator/(const Vector2& V1, const Vector2& V2)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(&V1);
	XMVECTOR v2 = XMLoadFloat2(&V2);
	XMVECTOR X = XMVectorDivide(v1,v2);
	Vector2 R;
	XMStoreFloat2(&R, X);
	return R;
}

inline Vector2 operator*(float S, const Vector2& V)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(&V);
	XMVECTOR X = XMVectorScale(v1,S);
	Vector2 R;
	XMStoreFloat2(&R, X);
	return R;
}


inline float Vector2::Length(const Vector2& v)
{
	using namespace DirectX;
	XMVECTOR V = XMLoadFloat2(&v);
	XMVECTOR X = XMVector2Length(V);
	return XMVectorGetX(X);
}

inline float Vector2::Dot(const Vector2& v1, const Vector2& v2)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR X = XMVector2Dot(x1, x2);
	return XMVectorGetX(X);
}

inline Vector2 Vector2::Cross(const Vector2& v1, const Vector2& v2)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR R = XMVector2Cross(x1, x2);

	Vector2 result;
	XMStoreFloat2(&result, R);
	return result;
}

inline bool Vector2::InBounds(const Vector2& Bounds) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR v2 = XMLoadFloat2(&Bounds);
	return XMVector2InBounds(v1, v2);
}

inline float Vector2::Length() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR X = XMVector2Length(v1);
	return XMVectorGetX(X);
}

inline float Vector2::LengthSquared() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR X = XMVector2LengthSq(v1);
	return XMVectorGetX(X);
}

inline float Vector2::Dot(const Vector2& V) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR v2 = XMLoadFloat2(&V);
	XMVECTOR X = XMVector2Dot(v1, v2);
	return XMVectorGetX(X);
}

inline Vector2 Vector2::Cross(const Vector2& V) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR v2 = XMLoadFloat2(&V);
	XMVECTOR R = XMVector2Cross(v1, v2);

	Vector2 result;
	XMStoreFloat2(&result, R);
	return result;
}

inline void Vector2::Normalize()
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR X = XMVector2Normalize(v1);
	XMStoreFloat2(this, X);
}

inline Vector2 Vector2::Normalize(const Vector2& v1)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR R = XMVector3Normalize(x1);

	Vector2 result;
	XMStoreFloat2(&result, R);
	return result;
}

inline void Vector2::Clamp(const Vector2& vmin, const Vector2& vmax)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR v2 = XMLoadFloat2(&vmin);
	XMVECTOR v3 = XMLoadFloat2(&vmax);
	XMVECTOR X = XMVectorClamp(v1, v2, v3);
	XMStoreFloat2(this, X);
}

inline void Vector2::Clamp(const Vector2& vmin, const Vector2& vmax, Vector2& result) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR v2 = XMLoadFloat2(&vmin);
	XMVECTOR v3 = XMLoadFloat2(&vmax);
	XMVECTOR X = XMVectorClamp(v1, v2, v3);
	XMStoreFloat2(&result, X);
}


inline float Vector2::Distance(const Vector2& v1, const Vector2& v2)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR V = XMVectorSubtract(x2, x1);
	XMVECTOR X = XMVector2Length(V);
	return XMVectorGetX(X);
}

inline float Vector2::DistanceSquared(const Vector2& v1, const Vector2& v2)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR V = XMVectorSubtract(x2, x1);
	XMVECTOR X = XMVector2LengthSq(V);
	return XMVectorGetX(X);
}

inline void Vector2::Min(const Vector2& v1, const Vector2& v2, Vector2& result)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR X = XMVectorMin(x1, x2);
	XMStoreFloat2(&result, X);
}

inline Vector2 Vector2::Min(const Vector2& v1, const Vector2& v2)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR X = XMVectorMin(x1, x2);

	Vector2 result;
	XMStoreFloat2(&result, X);
	return result;
}

inline void Vector2::Max(const Vector2& v1, const Vector2& v2, Vector2& result)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR X = XMVectorMax(x1, x2);
	XMStoreFloat2(&result, X);
}

inline Vector2 Vector2::Max(const Vector2& v1, const Vector2& v2)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR X = XMVectorMax(x1, x2);

	Vector2 result;
	XMStoreFloat2(&result, X);
	return result;
}

inline void Vector2::Lerp(const Vector2& v1, const Vector2& v2, float t, Vector2& result)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR X = XMVectorLerp(x1, x2, t);
	XMStoreFloat2(&result, X);
}

inline Vector2 Vector2::Lerp(const Vector2& v1, const Vector2& v2, float t)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR X = XMVectorLerp(x1, x2, t);

	Vector2 result;
	XMStoreFloat2(&result, X);
	return result;
}

inline void Vector2::SmoothStep(const Vector2& v1, const Vector2& v2, float t, Vector2& result)
{
	using namespace DirectX;
	t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
	t = t*t*(3.f - 2.f*t);
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR X = XMVectorLerp(x1, x2, t);
	XMStoreFloat2(&result, X);
}

inline Vector2 Vector2::SmoothStep(const Vector2& v1, const Vector2& v2, float t)
{
	using namespace DirectX;
	t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
	t = t*t*(3.f - 2.f*t);
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR X = XMVectorLerp(x1, x2, t);

	Vector2 result;
	XMStoreFloat2(&result, X);
	return result;
}

inline void Vector2::Barycentric(const Vector2& v1, const Vector2& v2, const Vector2& v3, float f, float g, Vector2& result)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR x3 = XMLoadFloat2(&v3);
	XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);
	XMStoreFloat2(&result, X);
}

inline Vector2 Vector2::Barycentric(const Vector2& v1, const Vector2& v2, const Vector2& v3, float f, float g)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR x3 = XMLoadFloat2(&v3);
	XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);

	Vector2 result;
	XMStoreFloat2(&result, X);
	return result;
}

inline void Vector2::CatmullRom(const Vector2& v1, const Vector2& v2, const Vector2& v3, const Vector2& v4, float t, Vector2& result)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR x3 = XMLoadFloat2(&v3);
	XMVECTOR x4 = XMLoadFloat2(&v4);
	XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);
	XMStoreFloat2(&result, X);
}

inline Vector2 Vector2::CatmullRom(const Vector2& v1, const Vector2& v2, const Vector2& v3, const Vector2& v4, float t)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR x3 = XMLoadFloat2(&v3);
	XMVECTOR x4 = XMLoadFloat2(&v4);
	XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);

	Vector2 result;
	XMStoreFloat2(&result, X);
	return result;
}

inline void Vector2::Hermite(const Vector2& v1, const Vector2& t1, const Vector2& v2, const Vector2& t2, float t, Vector2& result)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&t1);
	XMVECTOR x3 = XMLoadFloat2(&v2);
	XMVECTOR x4 = XMLoadFloat2(&t2);
	XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);
	XMStoreFloat2(&result, X);
}

inline Vector2 Vector2::Hermite(const Vector2& v1, const Vector2& t1, const Vector2& v2, const Vector2& t2, float t)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&t1);
	XMVECTOR x3 = XMLoadFloat2(&v2);
	XMVECTOR x4 = XMLoadFloat2(&t2);
	XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);

	Vector2 result;
	XMStoreFloat2(&result, X);
	return result;
}

inline void Vector2::Reflect(const Vector2& ivec, const Vector2& nvec, Vector2& result)
{
	using namespace DirectX;
	XMVECTOR i = XMLoadFloat2(&ivec);
	XMVECTOR n = XMLoadFloat2(&nvec);
	XMVECTOR X = XMVector2Reflect(i, n);
	XMStoreFloat2(&result, X);
}

inline Vector2 Vector2::Reflect(const Vector2& ivec, const Vector2& nvec)
{
	using namespace DirectX;
	XMVECTOR i = XMLoadFloat2(&ivec);
	XMVECTOR n = XMLoadFloat2(&nvec);
	XMVECTOR X = XMVector2Reflect(i, n);

	Vector2 result;
	XMStoreFloat2(&result, X);
	return result;
}

inline void Vector2::Refract(const Vector2& ivec, const Vector2& nvec, float refractionIndex, Vector2& result)
{
	using namespace DirectX;
	XMVECTOR i = XMLoadFloat2(&ivec);
	XMVECTOR n = XMLoadFloat2(&nvec);
	XMVECTOR X = XMVector2Refract(i, n, refractionIndex);
	XMStoreFloat2(&result, X);
}

inline Vector2 Vector2::Refract(const Vector2& ivec, const Vector2& nvec, float refractionIndex)
{
	using namespace DirectX;
	XMVECTOR i = XMLoadFloat2(&ivec);
	XMVECTOR n = XMLoadFloat2(&nvec);
	XMVECTOR X = XMVector2Refract(i, n, refractionIndex);

	Vector2 result;
	XMStoreFloat2(&result, X);
	return result;
}

inline void Vector2::Transform(const Vector2& v, const Quaternion& quat, Vector2& result)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(&v);
	XMVECTOR q = XMLoadFloat4(&quat);
	XMVECTOR X = XMVector3Rotate(v1, q);
	XMStoreFloat2(&result, X);
}

inline Vector2 Vector2::Transform(const Vector2& v, const Quaternion& quat)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(&v);
	XMVECTOR q = XMLoadFloat4(&quat);
	XMVECTOR X = XMVector3Rotate(v1, q);

	Vector2 result;
	XMStoreFloat2(&result, X);
	return result;
}

inline void Vector2::Transform(const Vector2& v, const Matrix& m, Vector2& result)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(&v);
	XMMATRIX M = XMLoadFloat4x4(&m);
	XMVECTOR X = XMVector2TransformCoord(v1, M);
	XMStoreFloat2(&result, X);
}

inline Vector2 Vector2::Transform(const Vector2& v, const Matrix& m)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(&v);
	XMMATRIX M = XMLoadFloat4x4(&m);
	XMVECTOR X = XMVector2TransformCoord(v1, M);

	Vector2 result;
	XMStoreFloat2(&result, X);
	return result;
}

_Use_decl_annotations_
inline void Vector2::Transform(const Vector2* varray, size_t count, const Matrix& m, Vector2* resultArray)
{
	using namespace DirectX;
	XMMATRIX M = XMLoadFloat4x4(&m);
	XMVector2TransformCoordStream(resultArray, sizeof(XMFLOAT2), varray, sizeof(XMFLOAT2), count, M);
}

inline void Vector2::Transform(const Vector2& v, const Matrix& m, Vector4& result)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(&v);
	XMMATRIX M = XMLoadFloat4x4(&m);
	XMVECTOR X = XMVector2Transform(v1, M);
	XMStoreFloat4(&result, X);
}

_Use_decl_annotations_
inline void Vector2::Transform(const Vector2* varray, size_t count, const Matrix& m, Vector4* resultArray)
{
	using namespace DirectX;
	XMMATRIX M = XMLoadFloat4x4(&m);
	XMVector2TransformStream(resultArray, sizeof(XMFLOAT4), varray, sizeof(XMFLOAT2), count, M);
}

inline void Vector2::TransformNormal(const Vector2& v, const Matrix& m, Vector2& result)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(&v);
	XMMATRIX M = XMLoadFloat4x4(&m);
	XMVECTOR X = XMVector2TransformNormal(v1, M);
	XMStoreFloat2(&result, X);
}

inline Vector2 Vector2::TransformNormal(const Vector2& v, const Matrix& m)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(&v);
	XMMATRIX M = XMLoadFloat4x4(&m);
	XMVECTOR X = XMVector2TransformNormal(v1, M);

	Vector2 result;
	XMStoreFloat2(&result, X);
	return result;
}

_Use_decl_annotations_
inline void Vector2::TransformNormal(const Vector2* varray, size_t count, const Matrix& m, Vector2* resultArray)
{
	using namespace DirectX;
	XMMATRIX M = XMLoadFloat4x4(&m);
	XMVector2TransformNormalStream(resultArray, sizeof(XMFLOAT2), varray, sizeof(XMFLOAT2), count, M);
}




/****************************************************************************
 * Vector3
 ****************************************************************************/

inline bool Vector3::operator<(const Vector3& V) const
{
	return std::tie(x, y, z) < std::tie(V.x, V.y, V.z);
}

inline bool Vector3::operator>(const Vector3& V) const
{
	return std::tie(x, y, z) > std::tie(V.x, V.y, V.z);
}

inline bool Vector3::operator==(const Vector3& V) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR v2 = XMLoadFloat3(&V);
	return XMVector3Equal(v1, v2);
}

inline bool Vector3::operator!=(const Vector3& V) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR v2 = XMLoadFloat3(&V);
	return XMVector3NotEqual(v1, v2);
}


inline Vector3& Vector3::operator=(const Vector3& V)
{
	x = V.x;
	y = V.y;
	z = V.z;
	return *this;
}

inline Vector3& Vector3::operator+=(const Vector3& V)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR v2 = XMLoadFloat3(&V);
	XMVECTOR X = XMVectorAdd(v1,v2);
	XMStoreFloat3(this, X);
	return *this;
}

inline Vector3& Vector3::operator-=(const Vector3& V)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR v2 = XMLoadFloat3(&V);
	XMVECTOR X = XMVectorSubtract(v1,v2);
	XMStoreFloat3(this, X);
	return *this;
}

inline Vector3& Vector3::operator*=(const Vector3& V)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR v2 = XMLoadFloat3(&V);
	XMVECTOR X = XMVectorMultiply(v1,v2);
	XMStoreFloat3(this, X);
	return *this;
}

inline Vector3& Vector3::operator*=(float S)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR X = XMVectorScale(v1,S);
	XMStoreFloat3(this, X);
	return *this;
} 

inline Vector3& Vector3::operator/=(float S)
{
	using namespace DirectX;
	assert(S != 0.0f);
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR X = XMVectorScale(v1, 1.f/S);
	XMStoreFloat3(this, X);
	return *this;
} 


inline Vector3 Vector3::operator+() const
{
	return *this;
}

inline Vector3 Vector3::operator-() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR X = XMVectorNegate(v1);
	Vector3 R;
	XMStoreFloat3(&R, X);
	return R;
}


inline Vector3::operator float*()
{
	return (float*) &x;
}

inline Vector3::operator const float*() const
{
	return (const float*) &x;
}


inline Vector3 operator+(const Vector3& V1, const Vector3& V2)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(&V1);
	XMVECTOR v2 = XMLoadFloat3(&V2);
	XMVECTOR X = XMVectorAdd(v1,v2);
	Vector3 R;
	XMStoreFloat3(&R, X);
	return R;
}

inline Vector3 operator-(const Vector3& V1, const Vector3& V2)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(&V1);
	XMVECTOR v2 = XMLoadFloat3(&V2);
	XMVECTOR X = XMVectorSubtract(v1,v2);
	Vector3 R;
	XMStoreFloat3(&R, X);
	return R;
}

inline Vector3 operator*(const Vector3& V1, const Vector3& V2)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(&V1);
	XMVECTOR v2 = XMLoadFloat3(&V2);
	XMVECTOR X = XMVectorMultiply(v1,v2);
	Vector3 R;
	XMStoreFloat3(&R, X);
	return R;
}

inline Vector3 operator*(const Vector3& V, float S)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(&V);
	XMVECTOR X = XMVectorScale(v1,S);
	Vector3 R;
	XMStoreFloat3(&R, X);
	return R;
}

inline Vector3 operator/(const Vector3& V1, const Vector3& V2)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(&V1);
	XMVECTOR v2 = XMLoadFloat3(&V2);
	XMVECTOR X = XMVectorDivide(v1,v2);
	Vector3 R;
	XMStoreFloat3(&R, X);
	return R;
}

inline Vector3 operator*(float S, const Vector3& V)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(&V);
	XMVECTOR X = XMVectorScale(v1,S);
	Vector3 R;
	XMStoreFloat3(&R, X);
	return R;
}

inline Vector3 operator^(const Vector3& V1, const Vector3& V2)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(&V1);
	XMVECTOR v2 = XMLoadFloat3(&V2);
	XMVECTOR R = XMVector3Cross(v1, v2);

	Vector3 result;
	XMStoreFloat3(&result, R);
	return result;
}



inline bool Vector3::InBounds(const Vector3& Bounds) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR v2 = XMLoadFloat3(&Bounds);
	return XMVector3InBounds(v1, v2);
}

inline float Vector3::Length() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR X = XMVector3Length(v1);
	return XMVectorGetX(X);
}

inline float Vector3::LengthSquared() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR X = XMVector3LengthSq(v1);
	return XMVectorGetX(X);
}

inline float Vector3::Dot(const Vector3& V) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR v2 = XMLoadFloat3(&V);
	XMVECTOR X = XMVector3Dot(v1, v2);
	return XMVectorGetX(X);
}

inline Vector3 Vector3::Cross(const Vector3& V) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR v2 = XMLoadFloat3(&V);
	XMVECTOR R = XMVector3Cross(v1, v2);

	Vector3 result;
	XMStoreFloat3(&result, R);
	return result;
}

inline void Vector3::Normalize()
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR X = XMVector3Normalize(v1);
	XMStoreFloat3(this, X);
}

inline void Vector3::Clamp(const Vector3& vmin, const Vector3& vmax)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR v2 = XMLoadFloat3(&vmin);
	XMVECTOR v3 = XMLoadFloat3(&vmax);
	XMVECTOR X = XMVectorClamp(v1, v2, v3);
	XMStoreFloat3(this, X);
}

inline void Vector3::Clamp(const Vector3& vmin, const Vector3& vmax, Vector3& result) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR v2 = XMLoadFloat3(&vmin);
	XMVECTOR v3 = XMLoadFloat3(&vmax);
	XMVECTOR X = XMVectorClamp(v1, v2, v3);
	XMStoreFloat3(&result, X);
}



inline Vector3 Vector3::Null()
{
	return Vector3(0.f);
}

inline float Vector3::Dot(const Vector3& v1, const Vector3& v2)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR X = XMVector3Dot(x1, x2);
	return XMVectorGetX(X);
}

inline Vector3 Vector3::Cross(const Vector3& v1, const Vector3& v2)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR R = XMVector3Cross(x1, x2);

	Vector3 result;
	XMStoreFloat3(&result, R);
	return result;
}

inline float Vector3::Length(const Vector3& v1)
{
	using namespace DirectX;
	XMVECTOR V = XMLoadFloat3(&v1);
	XMVECTOR X = XMVector3Length(V);
	return XMVectorGetX(X);
}

inline float Vector3::LengthSquared(const Vector3& v1)
{
	using namespace DirectX;
	XMVECTOR V = XMLoadFloat3(&v1);
	XMVECTOR X = XMVector3LengthSq(V);
	return XMVectorGetX(X);
}

inline Vector3 Vector3::Normalize(const Vector3& v1)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR R = XMVector3Normalize(x1);

	Vector3 result;
	XMStoreFloat3(&result, R);
	return result;
}

inline float Vector3::Distance(const Vector3& v1, const Vector3& v2)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR V = XMVectorSubtract(x2, x1);
	XMVECTOR X = XMVector3Length(V);
	return XMVectorGetX(X);
}

inline float Vector3::DistanceSquared(const Vector3& v1, const Vector3& v2)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR V = XMVectorSubtract(x2, x1);
	XMVECTOR X = XMVector3LengthSq(V);
	return XMVectorGetX(X);
}

inline void Vector3::Min(const Vector3& v1, const Vector3& v2, Vector3& result)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR X = XMVectorMin(x1, x2);
	XMStoreFloat3(&result, X);
}

inline Vector3 Vector3::Min(const Vector3& v1, const Vector3& v2)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR X = XMVectorMin(x1, x2);

	Vector3 result;
	XMStoreFloat3(&result, X);
	return result;
}

inline void Vector3::Max(const Vector3& v1, const Vector3& v2, Vector3& result)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR X = XMVectorMax(x1, x2);
	XMStoreFloat3(&result, X);
}

inline Vector3 Vector3::Max(const Vector3& v1, const Vector3& v2)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR X = XMVectorMax(x1, x2);

	Vector3 result;
	XMStoreFloat3(&result, X);
	return result;
}

inline void Vector3::Lerp(const Vector3& v1, const Vector3& v2, float t, Vector3& result)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR X = XMVectorLerp(x1, x2, t);
	XMStoreFloat3(&result, X);
}

inline Vector3 Vector3::Lerp(const Vector3& v1, const Vector3& v2, float t)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR X = XMVectorLerp(x1, x2, t);

	Vector3 result;
	XMStoreFloat3(&result, X);
	return result;
}

inline void Vector3::SmoothStep(const Vector3& v1, const Vector3& v2, float t, Vector3& result)
{
	using namespace DirectX;
	t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
	t = t*t*(3.f - 2.f*t);
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR X = XMVectorLerp(x1, x2, t);
	XMStoreFloat3(&result, X);
}

inline Vector3 Vector3::SmoothStep(const Vector3& v1, const Vector3& v2, float t)
{
	using namespace DirectX;
	t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
	t = t*t*(3.f - 2.f*t);
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR X = XMVectorLerp(x1, x2, t);

	Vector3 result;
	XMStoreFloat3(&result, X);
	return result;
}

inline void Vector3::Barycentric(const Vector3& v1, const Vector3& v2, const Vector3& v3, float f, float g, Vector3& result)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR x3 = XMLoadFloat3(&v3);
	XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);
	XMStoreFloat3(&result, X);
}

inline Vector3 Vector3::Barycentric(const Vector3& v1, const Vector3& v2, const Vector3& v3, float f, float g)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR x3 = XMLoadFloat3(&v3);
	XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);

	Vector3 result;
	XMStoreFloat3(&result, X);
	return result;
}

inline void Vector3::CatmullRom(const Vector3& v1, const Vector3& v2, const Vector3& v3, const Vector3& v4, float t, Vector3& result)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR x3 = XMLoadFloat3(&v3);
	XMVECTOR x4 = XMLoadFloat3(&v4);
	XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);
	XMStoreFloat3(&result, X);
}

inline Vector3 Vector3::CatmullRom(const Vector3& v1, const Vector3& v2, const Vector3& v3, const Vector3& v4, float t)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR x3 = XMLoadFloat3(&v3);
	XMVECTOR x4 = XMLoadFloat3(&v4);
	XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);

	Vector3 result;
	XMStoreFloat3(&result, X);
	return result;
}

inline void Vector3::Hermite(const Vector3& v1, const Vector3& t1, const Vector3& v2, const Vector3& t2, float t, Vector3& result)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&t1);
	XMVECTOR x3 = XMLoadFloat3(&v2);
	XMVECTOR x4 = XMLoadFloat3(&t2);
	XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);
	XMStoreFloat3(&result, X);
}

inline Vector3 Vector3::Hermite(const Vector3& v1, const Vector3& t1, const Vector3& v2, const Vector3& t2, float t)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&t1);
	XMVECTOR x3 = XMLoadFloat3(&v2);
	XMVECTOR x4 = XMLoadFloat3(&t2);
	XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);

	Vector3 result;
	XMStoreFloat3(&result, X);
	return result;
}

inline void Vector3::Reflect(const Vector3& ivec, const Vector3& nvec, Vector3& result)
{
	using namespace DirectX;
	XMVECTOR i = XMLoadFloat3(&ivec);
	XMVECTOR n = XMLoadFloat3(&nvec);
	XMVECTOR X = XMVector3Reflect(i, n);
	XMStoreFloat3(&result, X);
}

inline Vector3 Vector3::Reflect(const Vector3& ivec, const Vector3& nvec)
{
	using namespace DirectX;
	XMVECTOR i = XMLoadFloat3(&ivec);
	XMVECTOR n = XMLoadFloat3(&nvec);
	XMVECTOR X = XMVector3Reflect(i, n);

	Vector3 result;
	XMStoreFloat3(&result, X);
	return result;
}

inline void Vector3::Refract(const Vector3& ivec, const Vector3& nvec, float refractionIndex, Vector3& result)
{
	using namespace DirectX;
	XMVECTOR i = XMLoadFloat3(&ivec);
	XMVECTOR n = XMLoadFloat3(&nvec);
	XMVECTOR X = XMVector3Refract(i, n, refractionIndex);
	XMStoreFloat3(&result, X);
}

inline Vector3 Vector3::Refract(const Vector3& ivec, const Vector3& nvec, float refractionIndex)
{
	using namespace DirectX;
	XMVECTOR i = XMLoadFloat3(&ivec);
	XMVECTOR n = XMLoadFloat3(&nvec);
	XMVECTOR X = XMVector3Refract(i, n, refractionIndex);

	Vector3 result;
	XMStoreFloat3(&result, X);
	return result;
}

inline void Vector3::Transform(const Vector3& v, const Quaternion& quat, Vector3& result)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(&v);
	XMVECTOR q = XMLoadFloat4(&quat);
	XMVECTOR X = XMVector3Rotate(v1, q);
	XMStoreFloat3(&result, X);
}

inline Vector3 Vector3::Transform(const Vector3& v, const Quaternion& quat)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(&v);
	XMVECTOR q = XMLoadFloat4(&quat);
	XMVECTOR X = XMVector3Rotate(v1, q);

	Vector3 result;
	XMStoreFloat3(&result, X);
	return result;
}

inline void Vector3::Transform(const Vector3& v, const Matrix& m, Vector3& result)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(&v);
	XMMATRIX M = XMLoadFloat4x4(&m);
	XMVECTOR X = XMVector3TransformCoord(v1, M);
	XMStoreFloat3(&result, X);
}

inline Vector3 Vector3::Transform(const Vector3& v, const Matrix& m)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(&v);
	XMMATRIX M = XMLoadFloat4x4(&m);
	XMVECTOR X = XMVector3TransformCoord(v1, M);

	Vector3 result;
	XMStoreFloat3(&result, X);
	return result;
}

_Use_decl_annotations_
inline void Vector3::Transform(const Vector3* varray, size_t count, const Matrix& m, Vector3* resultArray)
{
	using namespace DirectX;
	XMMATRIX M = XMLoadFloat4x4(&m);
	XMVector3TransformCoordStream(resultArray, sizeof(XMFLOAT3), varray, sizeof(XMFLOAT3), count, M);
}

inline void Vector3::Transform(const Vector3& v, const Matrix& m, Vector4& result)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(&v);
	XMMATRIX M = XMLoadFloat4x4(&m);
	XMVECTOR X = XMVector3Transform(v1, M);
	XMStoreFloat4(&result, X);
}

_Use_decl_annotations_
inline void Vector3::Transform(const Vector3* varray, size_t count, const Matrix& m, Vector4* resultArray)
{
	using namespace DirectX;
	XMMATRIX M = XMLoadFloat4x4(&m);
	XMVector3TransformStream(resultArray, sizeof(XMFLOAT4), varray, sizeof(XMFLOAT3), count, M);
}

inline void Vector3::TransformNormal(const Vector3& v, const Matrix& m, Vector3& result)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(&v);
	XMMATRIX M = XMLoadFloat4x4(&m);
	XMVECTOR X = XMVector3TransformNormal(v1, M);
	XMStoreFloat3(&result, X);
}

inline Vector3 Vector3::TransformNormal(const Vector3& v, const Matrix& m)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(&v);
	XMMATRIX M = XMLoadFloat4x4(&m);
	XMVECTOR X = XMVector3TransformNormal(v1, M);

	Vector3 result;
	XMStoreFloat3(&result, X);
	return result;
}

_Use_decl_annotations_
inline void Vector3::TransformNormal(const Vector3* varray, size_t count, const Matrix& m, Vector3* resultArray)
{
	using namespace DirectX;
	XMMATRIX M = XMLoadFloat4x4(&m);
	XMVector3TransformNormalStream(resultArray, sizeof(XMFLOAT3), varray, sizeof(XMFLOAT3), count, M);
}




/****************************************************************************
 * Vector4
 ****************************************************************************/

inline bool Vector4::operator<(const Vector4& V) const
{
	return std::tie(x, y, z, w) < std::tie(V.x, V.y, V.z, V.w);
}

inline bool Vector4::operator>(const Vector4& V) const
{
	return std::tie(x, y, z, w) > std::tie(V.x, V.y, V.z, V.w);
}

inline bool Vector4::operator==(const Vector4& V) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR v2 = XMLoadFloat4(&V);
	return XMVector4Equal(v1, v2);
}

inline bool Vector4::operator!=(const Vector4& V) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR v2 = XMLoadFloat4(&V);
	return XMVector4NotEqual(v1, v2);
}


inline Vector4& Vector4::operator+=(const Vector4& V)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR v2 = XMLoadFloat4(&V);
	XMVECTOR X = XMVectorAdd(v1,v2);
	XMStoreFloat4(this, X);
	return *this;
}

inline Vector4& Vector4::operator-=(const Vector4& V)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR v2 = XMLoadFloat4(&V);
	XMVECTOR X = XMVectorSubtract(v1,v2);
	XMStoreFloat4(this, X);
	return *this;
}

inline Vector4& Vector4::operator*=(const Vector4& V)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR v2 = XMLoadFloat4(&V);
	XMVECTOR X = XMVectorMultiply(v1,v2);
	XMStoreFloat4(this, X);
	return *this;
}

inline Vector4& Vector4::operator*=(float S)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR X = XMVectorScale(v1,S);
	XMStoreFloat4(this, X);
	return *this;
} 

inline Vector4& Vector4::operator/=(float S)
{
	using namespace DirectX;
	assert(S != 0.0f);
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR X = XMVectorScale(v1, 1.f/S);
	XMStoreFloat4(this, X);
	return *this;
} 


inline Vector4 Vector4::operator-() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR X = XMVectorNegate(v1);
	Vector4 R;
	XMStoreFloat4(&R, X);
	return R;
}


inline Vector4::operator float*()
{
	return (float*) &x;
}

inline Vector4::operator const float*() const
{
	return (const float*) &x;
}


inline Vector4 operator+(const Vector4& V1, const Vector4& V2)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(&V1);
	XMVECTOR v2 = XMLoadFloat4(&V2);
	XMVECTOR X = XMVectorAdd(v1,v2);
	Vector4 R;
	XMStoreFloat4(&R, X);
	return R;
}

inline Vector4 operator-(const Vector4& V1, const Vector4& V2)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(&V1);
	XMVECTOR v2 = XMLoadFloat4(&V2);
	XMVECTOR X = XMVectorSubtract(v1,v2);
	Vector4 R;
	XMStoreFloat4(&R, X);
	return R;
}

inline Vector4 operator* (const Vector4& V1, const Vector4& V2)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(&V1);
	XMVECTOR v2 = XMLoadFloat4(&V2);
	XMVECTOR X = XMVectorMultiply(v1,v2);
	Vector4 R;
	XMStoreFloat4(&R, X);
	return R;
}

inline Vector4 operator*(const Vector4& V, float S)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(&V);
	XMVECTOR X = XMVectorScale(v1,S);
	Vector4 R;
	XMStoreFloat4(&R, X);
	return R;
}

inline Vector4 operator/(const Vector4& V1, const Vector4& V2)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(&V1);
	XMVECTOR v2 = XMLoadFloat4(&V2);
	XMVECTOR X = XMVectorDivide(v1,v2);
	Vector4 R;
	XMStoreFloat4(&R, X);
	return R;
}

inline Vector4 operator*(float S, const Vector4& V)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(&V);
	XMVECTOR X = XMVectorScale(v1,S);
	Vector4 R;
	XMStoreFloat4(&R, X);
	return R;
}


inline bool Vector4::InBounds(const Vector4& Bounds) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR v2 = XMLoadFloat4(&Bounds);
	return XMVector4InBounds(v1, v2);
}

inline float Vector4::Length() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR X = XMVector4Length(v1);
	return XMVectorGetX(X);
}

inline float Vector4::LengthSquared() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR X = XMVector4LengthSq(v1);
	return XMVectorGetX(X);
}

inline float Vector4::Dot(const Vector4& V) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR v2 = XMLoadFloat4(&V);
	XMVECTOR X = XMVector4Dot(v1, v2);
	return XMVectorGetX(X);
}

inline void Vector4::Cross(const Vector4& v1, const Vector4& v2, Vector4& result) const
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(this);
	XMVECTOR x2 = XMLoadFloat4(&v1);
	XMVECTOR x3 = XMLoadFloat4(&v2);
	XMVECTOR R = XMVector4Cross(x1, x2, x3);
	XMStoreFloat4(&result, R);
}

inline Vector4 Vector4::Cross(const Vector4& v1, const Vector4& v2) const
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(this);
	XMVECTOR x2 = XMLoadFloat4(&v1);
	XMVECTOR x3 = XMLoadFloat4(&v2);
	XMVECTOR R = XMVector4Cross(x1, x2, x3);

	Vector4 result;
	XMStoreFloat4(&result, R);
	return result;
}

inline void Vector4::Normalize()
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR X = XMVector4Normalize(v1);
	XMStoreFloat4(this, X);
}

inline Vector4 Vector4::Normalize(const Vector4& v1)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR R = XMVector4Normalize(x1);

	Vector4 result;
	XMStoreFloat4(&result, R);
	return result;
}

inline void Vector4::Clamp(const Vector4& vmin, const Vector4& vmax)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR v2 = XMLoadFloat4(&vmin);
	XMVECTOR v3 = XMLoadFloat4(&vmax);
	XMVECTOR X = XMVectorClamp(v1, v2, v3);
	XMStoreFloat4(this, X);
}

inline void Vector4::Clamp(const Vector4& vmin, const Vector4& vmax, Vector4& result) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR v2 = XMLoadFloat4(&vmin);
	XMVECTOR v3 = XMLoadFloat4(&vmax);
	XMVECTOR X = XMVectorClamp(v1, v2, v3);
	XMStoreFloat4(&result, X);
}


inline float Vector4::Distance(const Vector4& v1, const Vector4& v2)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR V = XMVectorSubtract(x2, x1);
	XMVECTOR X = XMVector4Length(V);
	return XMVectorGetX(X);
}

inline float Vector4::DistanceSquared(const Vector4& v1, const Vector4& v2)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR V = XMVectorSubtract(x2, x1);
	XMVECTOR X = XMVector4LengthSq(V);
	return XMVectorGetX(X);
}

inline void Vector4::Min(const Vector4& v1, const Vector4& v2, Vector4& result)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR X = XMVectorMin(x1, x2);
	XMStoreFloat4(&result, X);
}

inline Vector4 Vector4::Min(const Vector4& v1, const Vector4& v2)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR X = XMVectorMin(x1, x2);

	Vector4 result;
	XMStoreFloat4(&result, X);
	return result;
}

inline void Vector4::Max(const Vector4& v1, const Vector4& v2, Vector4& result)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR X = XMVectorMax(x1, x2);
	XMStoreFloat4(&result, X);
}

inline Vector4 Vector4::Max(const Vector4& v1, const Vector4& v2)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR X = XMVectorMax(x1, x2);

	Vector4 result;
	XMStoreFloat4(&result, X);
	return result;
}

inline void Vector4::Lerp(const Vector4& v1, const Vector4& v2, float t, Vector4& result)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR X = XMVectorLerp(x1, x2, t);
	XMStoreFloat4(&result, X);
}

inline Vector4 Vector4::Lerp(const Vector4& v1, const Vector4& v2, float t)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR X = XMVectorLerp(x1, x2, t);

	Vector4 result;
	XMStoreFloat4(&result, X);
	return result;
}

inline void Vector4::SmoothStep(const Vector4& v1, const Vector4& v2, float t, Vector4& result)
{
	using namespace DirectX;
	t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
	t = t*t*(3.f - 2.f*t);
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR X = XMVectorLerp(x1, x2, t);
	XMStoreFloat4(&result, X);
}

inline Vector4 Vector4::SmoothStep(const Vector4& v1, const Vector4& v2, float t)
{
	using namespace DirectX;
	t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
	t = t*t*(3.f - 2.f*t);
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR X = XMVectorLerp(x1, x2, t);

	Vector4 result;
	XMStoreFloat4(&result, X);
	return result;
}

inline void Vector4::Barycentric(const Vector4& v1, const Vector4& v2, const Vector4& v3, float f, float g, Vector4& result)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR x3 = XMLoadFloat4(&v3);
	XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);
	XMStoreFloat4(&result, X);
}

inline Vector4 Vector4::Barycentric(const Vector4& v1, const Vector4& v2, const Vector4& v3, float f, float g)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR x3 = XMLoadFloat4(&v3);
	XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);

	Vector4 result;
	XMStoreFloat4(&result, X);
	return result;
}

inline void Vector4::CatmullRom(const Vector4& v1, const Vector4& v2, const Vector4& v3, const Vector4& v4, float t, Vector4& result)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR x3 = XMLoadFloat4(&v3);
	XMVECTOR x4 = XMLoadFloat4(&v4);
	XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);
	XMStoreFloat4(&result, X);
}

inline Vector4 Vector4::CatmullRom(const Vector4& v1, const Vector4& v2, const Vector4& v3, const Vector4& v4, float t)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR x3 = XMLoadFloat4(&v3);
	XMVECTOR x4 = XMLoadFloat4(&v4);
	XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);

	Vector4 result;
	XMStoreFloat4(&result, X);
	return result;
}

inline void Vector4::Hermite(const Vector4& v1, const Vector4& t1, const Vector4& v2, const Vector4& t2, float t, Vector4& result)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&t1);
	XMVECTOR x3 = XMLoadFloat4(&v2);
	XMVECTOR x4 = XMLoadFloat4(&t2);
	XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);
	XMStoreFloat4(&result, X);
}

inline Vector4 Vector4::Hermite(const Vector4& v1, const Vector4& t1, const Vector4& v2, const Vector4& t2, float t)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&t1);
	XMVECTOR x3 = XMLoadFloat4(&v2);
	XMVECTOR x4 = XMLoadFloat4(&t2);
	XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);

	Vector4 result;
	XMStoreFloat4(&result, X);
	return result;
}

inline void Vector4::Reflect(const Vector4& ivec, const Vector4& nvec, Vector4& result)
{
	using namespace DirectX;
	XMVECTOR i = XMLoadFloat4(&ivec);
	XMVECTOR n = XMLoadFloat4(&nvec);
	XMVECTOR X = XMVector4Reflect(i, n);
	XMStoreFloat4(&result, X);
}

inline Vector4 Vector4::Reflect(const Vector4& ivec, const Vector4& nvec)
{
	using namespace DirectX;
	XMVECTOR i = XMLoadFloat4(&ivec);
	XMVECTOR n = XMLoadFloat4(&nvec);
	XMVECTOR X = XMVector4Reflect(i, n);

	Vector4 result;
	XMStoreFloat4(&result, X);
	return result;
}

inline void Vector4::Refract(const Vector4& ivec, const Vector4& nvec, float refractionIndex, Vector4& result)
{
	using namespace DirectX;
	XMVECTOR i = XMLoadFloat4(&ivec);
	XMVECTOR n = XMLoadFloat4(&nvec);
	XMVECTOR X = XMVector4Refract(i, n, refractionIndex);
	XMStoreFloat4(&result, X);
}

inline Vector4 Vector4::Refract(const Vector4& ivec, const Vector4& nvec, float refractionIndex)
{
	using namespace DirectX;
	XMVECTOR i = XMLoadFloat4(&ivec);
	XMVECTOR n = XMLoadFloat4(&nvec);
	XMVECTOR X = XMVector4Refract(i, n, refractionIndex);

	Vector4 result;
	XMStoreFloat4(&result, X);
	return result;
}

inline void Vector4::Transform(const Vector2& v, const Quaternion& quat, Vector4& result)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(&v);
	XMVECTOR q = XMLoadFloat4(&quat);
	XMVECTOR X = XMVector3Rotate(v1, q);
	X = XMVectorSelect(g_XMIdentityR3, X, g_XMSelect1110); // result.w = 1.f
	XMStoreFloat4(&result, X);
}

inline Vector4 Vector4::Transform(const Vector2& v, const Quaternion& quat)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(&v);
	XMVECTOR q = XMLoadFloat4(&quat);
	XMVECTOR X = XMVector3Rotate(v1, q);
	X = XMVectorSelect(g_XMIdentityR3, X, g_XMSelect1110); // result.w = 1.f

	Vector4 result;
	XMStoreFloat4(&result, X);
	return result;
}

inline void Vector4::Transform(const Vector3& v, const Quaternion& quat, Vector4& result)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(&v);
	XMVECTOR q = XMLoadFloat4(&quat);
	XMVECTOR X = XMVector3Rotate(v1, q);
	X = XMVectorSelect(g_XMIdentityR3, X, g_XMSelect1110); // result.w = 1.f
	XMStoreFloat4(&result, X);
}

inline Vector4 Vector4::Transform(const Vector3& v, const Quaternion& quat)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(&v);
	XMVECTOR q = XMLoadFloat4(&quat);
	XMVECTOR X = XMVector3Rotate(v1, q);
	X = XMVectorSelect(g_XMIdentityR3, X, g_XMSelect1110); // result.w = 1.f

	Vector4 result;
	XMStoreFloat4(&result, X);
	return result;
}

inline void Vector4::Transform(const Vector4& v, const Quaternion& quat, Vector4& result)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(&v);
	XMVECTOR q = XMLoadFloat4(&quat);
	XMVECTOR X = XMVector3Rotate(v1, q);
	X = XMVectorSelect(v1, X, g_XMSelect1110); // result.w = v.w
	XMStoreFloat4(&result, X);
}

inline Vector4 Vector4::Transform(const Vector4& v, const Quaternion& quat)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(&v);
	XMVECTOR q = XMLoadFloat4(&quat);
	XMVECTOR X = XMVector3Rotate(v1, q);
	X = XMVectorSelect(v1, X, g_XMSelect1110); // result.w = v.w

	Vector4 result;
	XMStoreFloat4(&result, X);
	return result;
}

inline void Vector4::Transform(const Vector4& v, const Matrix& m, Vector4& result)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(&v);
	XMMATRIX M = XMLoadFloat4x4(&m);
	XMVECTOR X = XMVector4Transform(v1, M);
	XMStoreFloat4(&result, X);
}

inline Vector4 Vector4::Transform(const Vector4& v, const Matrix& m)
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(&v);
	XMMATRIX M = XMLoadFloat4x4(&m);
	XMVECTOR X = XMVector4Transform(v1, M);

	Vector4 result;
	XMStoreFloat4(&result, X);
	return result;
}

_Use_decl_annotations_
inline void Vector4::Transform(const Vector4* varray, size_t count, const Matrix& m, Vector4* resultArray)
{
	using namespace DirectX;
	XMMATRIX M = XMLoadFloat4x4(&m);
	XMVector4TransformStream(resultArray, sizeof(XMFLOAT4), varray, sizeof(XMFLOAT4), count, M);
}




/****************************************************************************
 * Matrix
 ****************************************************************************/

inline bool Matrix::operator==(const Matrix& M) const
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_41));

	XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._11));
	XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._21));
	XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._31));
	XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._41));

	return (XMVector4Equal(x1, y1) && XMVector4Equal(x2, y2) && XMVector4Equal(x3, y3) && XMVector4Equal(x4, y4)) != 0;
}

inline bool Matrix::operator!=(const Matrix& M) const
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_41));

	XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._11));
	XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._21));
	XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._31));
	XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._41));

	return (XMVector4NotEqual(x1, y1) && XMVector4NotEqual(x2, y2) && XMVector4NotEqual(x3, y3) && XMVector4NotEqual(x4, y4)) != 0;
}


inline Matrix& Matrix::operator+=(const Matrix& M)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_41));

	XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._11));
	XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._21));
	XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._31));
	XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._41));

	x1 = XMVectorAdd(x1, y1);
	x2 = XMVectorAdd(x2, y2);
	x3 = XMVectorAdd(x3, y3);
	x4 = XMVectorAdd(x4, y4);

	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_41), x4);
	return *this;
}

inline Matrix& Matrix::operator-=(const Matrix& M)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_41));

	XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._11));
	XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._21));
	XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._31));
	XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._41));

	x1 = XMVectorSubtract(x1, y1);
	x2 = XMVectorSubtract(x2, y2);
	x3 = XMVectorSubtract(x3, y3);
	x4 = XMVectorSubtract(x4, y4);

	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_41), x4);
	return *this;
}

inline Matrix& Matrix::operator*=(const Matrix& M)
{
	using namespace DirectX;
	XMMATRIX M1 = XMLoadFloat4x4(this);
	XMMATRIX M2 = XMLoadFloat4x4(&M);
	XMMATRIX X = XMMatrixMultiply(M1, M2);
	XMStoreFloat4x4(this, X);
	return *this;
}

inline Matrix& Matrix::operator*=(float S)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_41));

	x1 = XMVectorScale(x1, S);
	x2 = XMVectorScale(x2, S);
	x3 = XMVectorScale(x3, S);
	x4 = XMVectorScale(x4, S);

	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_41), x4);
	return *this;
}

inline Matrix& Matrix::operator/=(float S)
{
	using namespace DirectX;
	assert(S != 0.f);
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_41));

	float rs = 1.f / S;

	x1 = XMVectorScale(x1, rs);
	x2 = XMVectorScale(x2, rs);
	x3 = XMVectorScale(x3, rs);
	x4 = XMVectorScale(x4, rs);

	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_41), x4);
	return *this;
}

inline Matrix& Matrix::operator/=(const Matrix& M)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_41));

	XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._11));
	XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._21));
	XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._31));
	XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._41));

	x1 = XMVectorDivide(x1, y1);
	x2 = XMVectorDivide(x2, y2);
	x3 = XMVectorDivide(x3, y3);
	x4 = XMVectorDivide(x4, y4);

	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_41), x4);
	return *this;
}


inline Matrix Matrix::operator-() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_11));
	XMVECTOR v2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_21));
	XMVECTOR v3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_31));
	XMVECTOR v4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_41));

	v1 = XMVectorNegate(v1);
	v2 = XMVectorNegate(v2);
	v3 = XMVectorNegate(v3);
	v4 = XMVectorNegate(v4);

	Matrix R;
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._11), v1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._21), v2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._31), v3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._41), v4);
	return R;
}


inline Matrix::operator float*()
{
	return (float*) &_11;
}

inline Matrix::operator const float*() const
{
	return (const float*) &_11;
}


inline Matrix operator+(const Matrix& M1, const Matrix& M2)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._41));

	XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._11));
	XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._21));
	XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._31));
	XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._41));

	x1 = XMVectorAdd(x1, y1);
	x2 = XMVectorAdd(x2, y2);
	x3 = XMVectorAdd(x3, y3);
	x4 = XMVectorAdd(x4, y4);

	Matrix R;
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._41), x4);
	return R;
}

inline Matrix operator-(const Matrix& M1, const Matrix& M2)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._41));

	XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._11));
	XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._21));
	XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._31));
	XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._41));

	x1 = XMVectorSubtract(x1, y1);
	x2 = XMVectorSubtract(x2, y2);
	x3 = XMVectorSubtract(x3, y3);
	x4 = XMVectorSubtract(x4, y4);

	Matrix R;
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._41), x4);
	return R;
}

inline Matrix operator*(const Matrix& M1, const Matrix& M2)
{
	using namespace DirectX;
	XMMATRIX m1 = XMLoadFloat4x4(&M1);
	XMMATRIX m2 = XMLoadFloat4x4(&M2);
	XMMATRIX X = XMMatrixMultiply(m1, m2);

	Matrix R;
	XMStoreFloat4x4(&R, X);
	return R;
}

inline Matrix operator*(const Matrix& M, float S)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._41));

	x1 = XMVectorScale(x1, S);
	x2 = XMVectorScale(x2, S);
	x3 = XMVectorScale(x3, S);
	x4 = XMVectorScale(x4, S);

	Matrix R;
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._41), x4);
	return R;
}

inline Matrix operator/(const Matrix& M, float S)
{
	using namespace DirectX;
	assert(S != 0.f);

	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._41));

	float rs = 1.f / S;

	x1 = XMVectorScale(x1, rs);
	x2 = XMVectorScale(x2, rs);
	x3 = XMVectorScale(x3, rs);
	x4 = XMVectorScale(x4, rs);

	Matrix R;
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._41), x4);
	return R;
}

inline Matrix operator/(const Matrix& M1, const Matrix& M2)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._41));

	XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._11));
	XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._21));
	XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._31));
	XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._41));

	x1 = XMVectorDivide(x1, y1);
	x2 = XMVectorDivide(x2, y2);
	x3 = XMVectorDivide(x3, y3);
	x4 = XMVectorDivide(x4, y4);

	Matrix R;
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._41), x4);
	return R;
}

inline Matrix operator*(float S, const Matrix& M)
{
	using namespace DirectX;

	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._41));

	x1 = XMVectorScale(x1, S);
	x2 = XMVectorScale(x2, S);
	x3 = XMVectorScale(x3, S);
	x4 = XMVectorScale(x4, S);

	Matrix R;
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._41), x4);
	return R;
}


inline bool Matrix::Decompose(Vector3& scale, Quaternion& rotation, Vector3& translation)
{
	using namespace DirectX;

	XMVECTOR s, r, t;

	if (!XMMatrixDecompose(&s, &r, &t, *this))
		return false;

	XMStoreFloat3(&scale, s);
	XMStoreFloat4(&rotation, r);
	XMStoreFloat3(&translation, t);

	return true;
}

inline Matrix Matrix::Transpose() const
{
	using namespace DirectX;
	XMMATRIX M = XMLoadFloat4x4(this);
	Matrix R;
	XMStoreFloat4x4(&R, XMMatrixTranspose(M));
	return R;
}

inline void Matrix::Transpose(Matrix& result) const
{
	using namespace DirectX;
	XMMATRIX M = XMLoadFloat4x4(this);
	XMStoreFloat4x4(&result, XMMatrixTranspose(M));
}

inline Matrix Matrix::Invert() const
{
	using namespace DirectX;
	XMMATRIX M = XMLoadFloat4x4(this);
	Matrix R;
	XMVECTOR det;
	XMStoreFloat4x4(&R, XMMatrixInverse(&det, M));
	return R;
}

inline void Matrix::Invert(Matrix& result) const
{
	using namespace DirectX;
	XMMATRIX M = XMLoadFloat4x4(this);
	XMVECTOR det;
	XMStoreFloat4x4(&result, XMMatrixInverse(&det, M));
}

inline float Matrix::Determinant() const
{
	using namespace DirectX;
	XMMATRIX M = XMLoadFloat4x4(this);
	return XMVectorGetX(XMMatrixDeterminant(M));
}


inline Matrix Matrix::Identity()
{
	using namespace DirectX;
	return Matrix(1.f, 0.f, 0.f, 0.f,
				  0.f, 1.f, 0.f, 0.f,
				  0.f, 0.f, 1.f, 0.f,
				  0.f, 0.f, 0.f, 1.f);
}

inline Matrix Matrix::CreateTranslation(const Vector3& Position)
{
	using namespace DirectX;
	Matrix R;
	XMStoreFloat4x4(&R, XMMatrixTranslation(Position.x, Position.y, Position.z));
	return R;
}

inline Matrix Matrix::CreateTranslation(float x, float y, float z)
{
	using namespace DirectX;
	Matrix R;
	XMStoreFloat4x4(&R, XMMatrixTranslation(x, y, z));
	return R;
}

inline Matrix Matrix::CreateScale(const Vector3& scales)
{
	using namespace DirectX;
	Matrix R;
	XMStoreFloat4x4(&R, XMMatrixScaling(scales.x, scales.y, scales.z));
	return R;
}

inline Matrix Matrix::CreateScale(float xs, float ys, float zs)
{
	using namespace DirectX;
	Matrix R;
	XMStoreFloat4x4(&R, XMMatrixScaling(xs, ys, zs));
	return R;
}

inline Matrix Matrix::CreateScale(float scale)
{
	using namespace DirectX;
	Matrix R;
	XMStoreFloat4x4(&R, XMMatrixScaling(scale, scale, scale));
	return R;
}

inline Matrix Matrix::CreateRotationX(float radians)
{
	using namespace DirectX;
	Matrix R;
	XMStoreFloat4x4(&R, XMMatrixRotationX(radians));
	return R;
}

inline Matrix Matrix::CreateRotationY(float radians)
{
	using namespace DirectX;
	Matrix R;
	XMStoreFloat4x4(&R, XMMatrixRotationY(radians));
	return R;
}

inline Matrix Matrix::CreateRotationZ(float radians)
{
	using namespace DirectX;
	Matrix R;
	XMStoreFloat4x4(&R, XMMatrixRotationZ(radians));
	return R;
}

inline Matrix Matrix::CreateFromAxisAngle(const Vector3& axis, float angle)
{
	using namespace DirectX;
	Matrix R;
	XMVECTOR a = XMLoadFloat3(&axis);
	XMStoreFloat4x4(&R, XMMatrixRotationAxis(a, angle));
	return R;
}

inline Matrix Matrix::CreatePerspectiveFieldOfView(float fov, float aspectRatio, float nearPlane, float farPlane)
{
	using namespace DirectX;
	Matrix R;
	XMStoreFloat4x4(&R, XMMatrixPerspectiveFovRH(fov, aspectRatio, nearPlane, farPlane));
	return R;
}

inline Matrix Matrix::CreatePerspective(float width, float height, float nearPlane, float farPlane)
{
	using namespace DirectX;
	Matrix R;
	XMStoreFloat4x4(&R, XMMatrixPerspectiveRH(width, height, nearPlane, farPlane));
	return R;
}

inline Matrix Matrix::CreatePerspectiveOffCenter(float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
	using namespace DirectX;
	Matrix R;
	XMStoreFloat4x4(&R, XMMatrixPerspectiveOffCenterRH(left, right, bottom, top, nearPlane, farPlane));
	return R;
}

inline Matrix Matrix::CreateOrthographic(float width, float height, float zNearPlane, float zFarPlane)
{
	using namespace DirectX;
	Matrix R;
	XMStoreFloat4x4(&R, XMMatrixOrthographicRH(width, height, zNearPlane, zFarPlane));
	return R;
}

inline Matrix Matrix::CreateOrthographicOffCenter(float left, float right, float bottom, float top, float zNearPlane, float zFarPlane)
{
	using namespace DirectX;
	Matrix R;
	XMStoreFloat4x4(&R, XMMatrixOrthographicOffCenterRH(left, right, bottom, top, zNearPlane, zFarPlane));
	return R;
}

inline Matrix Matrix::CreateLookAt(const Vector3& eye, const Vector3& target, const Vector3& up)
{
	using namespace DirectX;
	Matrix R;
	XMVECTOR eyev = XMLoadFloat3(&eye);
	XMVECTOR targetv = XMLoadFloat3(&target);
	XMVECTOR upv = XMLoadFloat3(&up);
	XMStoreFloat4x4(&R, XMMatrixLookAtRH(eyev, targetv, upv));
	return R;
}

inline Matrix Matrix::CreateWorld(const Vector3& Position, const Vector3& forward, const Vector3& up)
{
	using namespace DirectX;
	XMVECTOR zaxis = XMVector3Normalize(XMVectorNegate(XMLoadFloat3(&forward)));
	XMVECTOR yaxis = XMLoadFloat3(&up);
	XMVECTOR xaxis = XMVector3Normalize(XMVector3Cross(yaxis, zaxis));
	yaxis = XMVector3Cross(zaxis, xaxis);
	
	Matrix R;
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&R._11), xaxis);
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&R._21), yaxis);
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&R._31), zaxis);
	R._14 = R._24 = R._34 = 0.f;
	R._41 = Position.x; R._42 = Position.y; R._43 = Position.z;
	R._44 = 1.f;
	return R;
}

inline Matrix Matrix::CreateFromQuaternion(const Quaternion& rotation)
{
	using namespace DirectX;
	Matrix R;
	XMVECTOR quatv = XMLoadFloat4(&rotation);
	XMStoreFloat4x4(&R, XMMatrixRotationQuaternion(quatv));
	return R;
}

inline Matrix Matrix::CreateFromYawPitchRoll(float yaw, float pitch, float roll)
{
	using namespace DirectX;
	Matrix R;
	XMStoreFloat4x4(&R, XMMatrixRotationRollPitchYaw(pitch, yaw, roll));
	return R;
}

inline Matrix Matrix::CreateShadow(const Vector3& lightDir, const Plane& plane)
{
	using namespace DirectX;
	Matrix R;
	XMVECTOR light = XMLoadFloat3(&lightDir);
	XMVECTOR planev = XMLoadFloat4(&plane);
	XMStoreFloat4x4(&R, XMMatrixShadow(planev, light));
	return R;
}

inline Matrix Matrix::CreateReflection(const Plane& plane)
{
	using namespace DirectX;
	Matrix R;
	XMVECTOR planev = XMLoadFloat4(&plane);
	XMStoreFloat4x4(&R, XMMatrixReflect(planev));
	return R;
}

inline void Matrix::Lerp(const Matrix& M1, const Matrix& M2, float t, Matrix& result)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._41));

	XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._11));
	XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._21));
	XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._31));
	XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._41));

	x1 = XMVectorLerp(x1, y1, t);
	x2 = XMVectorLerp(x2, y2, t);
	x3 = XMVectorLerp(x3, y3, t);
	x4 = XMVectorLerp(x4, y4, t);

	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result._11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result._21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result._31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result._41), x4);
}

inline Matrix Matrix::Lerp(const Matrix& M1, const Matrix& M2, float t)
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._41));

	XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._11));
	XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._21));
	XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._31));
	XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._41));

	x1 = XMVectorLerp(x1, y1, t);
	x2 = XMVectorLerp(x2, y2, t);
	x3 = XMVectorLerp(x3, y3, t);
	x4 = XMVectorLerp(x4, y4, t);

	Matrix result;
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result._11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result._21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result._31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result._41), x4);
	return result;
}

inline void Matrix::Transform(const Matrix& M, const Quaternion& rotation, Matrix& result)
{
	using namespace DirectX;
	XMVECTOR quatv = XMLoadFloat4(&rotation);

	XMMATRIX M0 = XMLoadFloat4x4(&M);
	XMMATRIX M1 = XMMatrixRotationQuaternion(quatv);

	XMStoreFloat4x4(&result, XMMatrixMultiply(M0, M1));
}

inline Matrix Matrix::Transform(const Matrix& M, const Quaternion& rotation)
{
	using namespace DirectX;
	XMVECTOR quatv = XMLoadFloat4(&rotation);

	XMMATRIX M0 = XMLoadFloat4x4(&M);
	XMMATRIX M1 = XMMatrixRotationQuaternion(quatv);

	Matrix result;
	XMStoreFloat4x4(&result, XMMatrixMultiply(M0, M1));
	return result;
}




/****************************************************************************
 * Plane
 ****************************************************************************/

inline Plane::Plane(const Vector3& point1, const Vector3& point2, const Vector3& point3)
{
	using namespace DirectX;
	XMVECTOR P0 = XMLoadFloat3(&point1);
	XMVECTOR P1 = XMLoadFloat3(&point2);
	XMVECTOR P2 = XMLoadFloat3(&point3);
	XMStoreFloat4(this, XMPlaneFromPoints(P0, P1, P2)); // uses left-handed coordinate system!
}

inline Plane::Plane(const Vector3& point, const Vector3& normal)
{
	using namespace DirectX;
	XMVECTOR P = XMLoadFloat3(&point);
	XMVECTOR N = XMLoadFloat3(&normal);
	XMStoreFloat4(this, XMPlaneFromPointNormal(P, N));
}


inline bool Plane::operator==(const Plane& p) const
{
	using namespace DirectX;
	XMVECTOR p1 = XMLoadFloat4(this);
	XMVECTOR p2 = XMLoadFloat4(&p);
	return XMPlaneEqual(p1, p2);
}

inline bool Plane::operator!=(const Plane& p) const
{
	using namespace DirectX;
	XMVECTOR p1 = XMLoadFloat4(this);
	XMVECTOR p2 = XMLoadFloat4(&p);
	return XMPlaneNotEqual(p1, p2);
}


inline void Plane::Normalize()
{
	using namespace DirectX;
	XMVECTOR p = XMLoadFloat4(this);
	XMStoreFloat4(this, XMPlaneNormalize(p));
}

inline void Plane::Normalize(Plane& result) const
{
	using namespace DirectX;
	XMVECTOR p = XMLoadFloat4(this);
	XMStoreFloat4(&result, XMPlaneNormalize(p));
}

inline float Plane::Dot(const Vector4& v) const
{
	using namespace DirectX;
	XMVECTOR p = XMLoadFloat4(this);
	XMVECTOR v0 = XMLoadFloat4(&v);
	return XMVectorGetX(XMPlaneDot(p, v0));
}

inline float Plane::DotCoordinate(const Vector3& Position) const
{
	using namespace DirectX;
	XMVECTOR p = XMLoadFloat4(this);
	XMVECTOR v0 = XMLoadFloat3(&Position);
	return XMVectorGetX(XMPlaneDotCoord(p, v0));
}

inline float Plane::DotNormal(const Vector3& normal) const
{
	using namespace DirectX;
	XMVECTOR p = XMLoadFloat4(this);
	XMVECTOR n0 = XMLoadFloat3(&normal);
	return XMVectorGetX(XMPlaneDotNormal(p, n0));
}


inline void Plane::Transform(const Plane& plane, const Matrix& M, Plane& result)
{
	using namespace DirectX;
	XMVECTOR p = XMLoadFloat4(&plane);
	XMMATRIX m0 = XMLoadFloat4x4(&M);
	XMStoreFloat4(&result, XMPlaneTransform(p, m0));
}

inline Plane Plane::Transform(const Plane& plane, const Matrix& M)
{
	using namespace DirectX;
	XMVECTOR p = XMLoadFloat4(&plane);
	XMMATRIX m0 = XMLoadFloat4x4(&M);

	Plane result;
	XMStoreFloat4(&result, XMPlaneTransform(p, m0));
	return result;
}

inline void Plane::Transform(const Plane& plane, const Quaternion& rotation, Plane& result)
{
	using namespace DirectX;
	XMVECTOR p = XMLoadFloat4(&plane);
	XMVECTOR q = XMLoadFloat4(&rotation);
	XMVECTOR X = XMVector3Rotate(p, q);
	X = XMVectorSelect(p, X, g_XMSelect1110); // result.d = plane.d
	XMStoreFloat4(&result, X);
}

inline Plane Plane::Transform(const Plane& plane, const Quaternion& rotation)
{
	using namespace DirectX;
	XMVECTOR p = XMLoadFloat4(&plane);
	XMVECTOR q = XMLoadFloat4(&rotation);
	XMVECTOR X = XMVector3Rotate(p, q);
	X = XMVectorSelect(p, X, g_XMSelect1110); // result.d = plane.d

	Plane result;
	XMStoreFloat4(&result, X);
	return result;
}



/****************************************************************************
 * Quaternion
 ****************************************************************************/

inline bool Quaternion::operator==(const Quaternion& q) const
{
	using namespace DirectX;
	XMVECTOR q1 = XMLoadFloat4(this);
	XMVECTOR q2 = XMLoadFloat4(&q);
	return XMQuaternionEqual(q1, q2);
}

inline bool Quaternion::operator!=(const Quaternion& q) const
{
	using namespace DirectX;
	XMVECTOR q1 = XMLoadFloat4(this);
	XMVECTOR q2 = XMLoadFloat4(&q);
	return XMQuaternionNotEqual(q1, q2);
}


inline Quaternion& Quaternion::operator+=(const Quaternion& q)
{
	using namespace DirectX;
	XMVECTOR q1 = XMLoadFloat4(this);
	XMVECTOR q2 = XMLoadFloat4(&q);
	XMStoreFloat4(this, XMVectorAdd(q1, q2));
	return *this;
}

inline Quaternion& Quaternion::operator-=(const Quaternion& q)
{
	using namespace DirectX;
	XMVECTOR q1 = XMLoadFloat4(this);
	XMVECTOR q2 = XMLoadFloat4(&q);
	XMStoreFloat4(this, XMVectorSubtract(q1, q2));
	return *this;
}

inline Quaternion& Quaternion::operator*=(const Quaternion& q)
{
	using namespace DirectX;
	XMVECTOR q1 = XMLoadFloat4(this);
	XMVECTOR q2 = XMLoadFloat4(&q);
	XMStoreFloat4(this, XMQuaternionMultiply(q1, q2));
	return *this;
}

inline Quaternion& Quaternion::operator*=(float S)
{
	using namespace DirectX;
	XMVECTOR q = XMLoadFloat4(this);
	XMStoreFloat4(this, XMVectorScale(q, S));
	return *this;
}

inline Quaternion& Quaternion::operator/=(const Quaternion& q)
{
	using namespace DirectX;
	XMVECTOR q1 = XMLoadFloat4(this);
	XMVECTOR q2 = XMLoadFloat4(&q);
	q2 = XMQuaternionInverse(q2);
	XMStoreFloat4(this, XMQuaternionMultiply(q1, q2));
	return *this;
}


inline Quaternion Quaternion::operator-() const
{
	using namespace DirectX;
	XMVECTOR q = XMLoadFloat4(this);

	Quaternion R;
	XMStoreFloat4(&R, XMVectorNegate(q));
	return R;
}


inline Quaternion operator+(const Quaternion& Q1, const Quaternion& Q2)
{
	using namespace DirectX;
	XMVECTOR q1 = XMLoadFloat4(&Q1);
	XMVECTOR q2 = XMLoadFloat4(&Q2);

	Quaternion R;
	XMStoreFloat4(&R, XMVectorAdd(q1, q2));
	return R;
}

inline Quaternion operator-(const Quaternion& Q1, const Quaternion& Q2)
{
	using namespace DirectX;
	XMVECTOR q1 = XMLoadFloat4(&Q1);
	XMVECTOR q2 = XMLoadFloat4(&Q2);

	Quaternion R;
	XMStoreFloat4(&R, XMVectorSubtract(q1, q2));
	return R;
}

inline Quaternion operator*(const Quaternion& Q1, const Quaternion& Q2)
{
	using namespace DirectX;
	XMVECTOR q1 = XMLoadFloat4(&Q1);
	XMVECTOR q2 = XMLoadFloat4(&Q2);

	Quaternion R;
	XMStoreFloat4(&R, XMQuaternionMultiply(q1, q2));
	return R;
}

inline Quaternion operator*(const Quaternion& Q, float S)
{
	using namespace DirectX;
	XMVECTOR q = XMLoadFloat4(&Q);

	Quaternion R;
	XMStoreFloat4(&R, XMVectorScale(q, S));
	return R;
}

inline Quaternion operator/(const Quaternion& Q1, const Quaternion& Q2)
{
	using namespace DirectX;
	XMVECTOR q1 = XMLoadFloat4(&Q1);
	XMVECTOR q2 = XMLoadFloat4(&Q2);
	q2 = XMQuaternionInverse(q2);

	Quaternion R;
	XMStoreFloat4(&R, XMQuaternionMultiply(q1, q2));
	return R;
}

inline Quaternion operator*(float S, const Quaternion& Q)
{
	using namespace DirectX;
	XMVECTOR q1 = XMLoadFloat4(&Q);

	Quaternion R;
	XMStoreFloat4(&R, XMVectorScale(q1, S));
	return R;
}


inline float Quaternion::Length() const
{
	using namespace DirectX;
	XMVECTOR q = XMLoadFloat4(this);
	return XMVectorGetX(XMQuaternionLength(q));
}

inline float Quaternion::LengthSquared() const
{
	using namespace DirectX;
	XMVECTOR q = XMLoadFloat4(this);
	return XMVectorGetX(XMQuaternionLengthSq(q));
}

inline void Quaternion::Normalize()
{
	using namespace DirectX;
	XMVECTOR q = XMLoadFloat4(this);
	XMStoreFloat4(this, XMQuaternionNormalize(q));
}

inline void Quaternion::Normalize(Quaternion& result) const
{
	using namespace DirectX;
	XMVECTOR q = XMLoadFloat4(this);
	XMStoreFloat4(&result, XMQuaternionNormalize(q));
}

inline void Quaternion::Conjugate()
{
	using namespace DirectX;
	XMVECTOR q = XMLoadFloat4(this);
	XMStoreFloat4(this, XMQuaternionConjugate(q));
}

inline void Quaternion::Conjugate(Quaternion& result) const
{
	using namespace DirectX;
	XMVECTOR q = XMLoadFloat4(this);
	XMStoreFloat4(&result, XMQuaternionConjugate(q));
}

inline void Quaternion::Inverse(Quaternion& result) const
{
	using namespace DirectX;
	XMVECTOR q = XMLoadFloat4(this);
	XMStoreFloat4(&result, XMQuaternionInverse(q));
}

inline float Quaternion::Dot(const Quaternion& q) const
{
	using namespace DirectX;
	XMVECTOR q1 = XMLoadFloat4(this);
	XMVECTOR q2 = XMLoadFloat4(&q);
	return XMVectorGetX(XMQuaternionDot(q1, q2));
}


inline Quaternion Quaternion::CreateFromAxisAngle(const Vector3& axis, float angle)
{
	using namespace DirectX;
	XMVECTOR a = XMLoadFloat3(&axis);

	Quaternion R;
	XMStoreFloat4(&R, XMQuaternionRotationAxis(a, angle));
	return R;
}

inline Quaternion Quaternion::CreateFromYawPitchRoll(float yaw, float pitch, float roll)
{
	using namespace DirectX;
	Quaternion R;
	XMStoreFloat4(&R, XMQuaternionRotationRollPitchYaw(pitch, yaw, roll));
	return R;
}

inline Quaternion Quaternion::CreateFromRotationMatrix(const Matrix& M)
{
	using namespace DirectX;
	XMMATRIX M0 = XMLoadFloat4x4(&M);

	Quaternion R;
	XMStoreFloat4(&R, XMQuaternionRotationMatrix(M0));
	return R;
}

inline void Quaternion::Lerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion& result)
{
	using namespace DirectX;
	XMVECTOR Q0 = XMLoadFloat4(&q1);
	XMVECTOR Q1 = XMLoadFloat4(&q2);

	XMVECTOR dot = XMVector4Dot(Q0, Q1);

	XMVECTOR R;
	if (XMVector4GreaterOrEqual(dot, XMVectorZero()))
	{
		R = XMVectorLerp(Q0, Q1, t);
	}
	else
	{
		XMVECTOR tv = XMVectorReplicate(t);
		XMVECTOR t1v = XMVectorReplicate(1.f - t);
		XMVECTOR X0 = XMVectorMultiply(Q0, t1v);
		XMVECTOR X1 = XMVectorMultiply(Q1, tv);
		R = XMVectorSubtract(X0, X1);
	}

	XMStoreFloat4(&result, XMQuaternionNormalize(R));
}

inline Quaternion Quaternion::Lerp(const Quaternion& q1, const Quaternion& q2, float t)
{
	using namespace DirectX;
	XMVECTOR Q0 = XMLoadFloat4(&q1);
	XMVECTOR Q1 = XMLoadFloat4(&q2);

	XMVECTOR dot = XMVector4Dot(Q0, Q1);

	XMVECTOR R;
	if (XMVector4GreaterOrEqual(dot, XMVectorZero()))
	{
		R = XMVectorLerp(Q0, Q1, t);
	}
	else
	{
		XMVECTOR tv = XMVectorReplicate(t);
		XMVECTOR t1v = XMVectorReplicate(1.f - t);
		XMVECTOR X0 = XMVectorMultiply(Q0, t1v);
		XMVECTOR X1 = XMVectorMultiply(Q1, tv);
		R = XMVectorSubtract(X0, X1);
	}

	Quaternion result;
	XMStoreFloat4(&result, XMQuaternionNormalize(R));
	return result;
}

inline void Quaternion::Slerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion& result)
{
	using namespace DirectX;
	XMVECTOR Q0 = XMLoadFloat4(&q1);
	XMVECTOR Q1 = XMLoadFloat4(&q2);
	XMStoreFloat4(&result, XMQuaternionSlerp(Q0, Q1, t));
}

inline Quaternion Quaternion::Slerp(const Quaternion& q1, const Quaternion& q2, float t)
{
	using namespace DirectX;
	XMVECTOR Q0 = XMLoadFloat4(&q1);
	XMVECTOR Q1 = XMLoadFloat4(&q2);

	Quaternion result;
	XMStoreFloat4(&result, XMQuaternionSlerp(Q0, Q1, t));
	return result;
}

inline void Quaternion::Concatenate(const Quaternion& q1, const Quaternion& q2, Quaternion& result)
{
	using namespace DirectX;
	XMVECTOR Q0 = XMLoadFloat4(&q1);
	XMVECTOR Q1 = XMLoadFloat4(&q2);
	XMStoreFloat4(&result, XMQuaternionMultiply(Q1, Q0));
}

inline Quaternion Quaternion::Concatenate(const Quaternion& q1, const Quaternion& q2)
{
	using namespace DirectX;
	XMVECTOR Q0 = XMLoadFloat4(&q1);
	XMVECTOR Q1 = XMLoadFloat4(&q2);

	Quaternion result;
	XMStoreFloat4(&result, XMQuaternionMultiply(Q1, Q0));
	return result;
}



/****************************************************************************
 * Color
 ****************************************************************************/

inline Color::Color(const DirectX::PackedVector::XMCOLOR& Packed)
{
	using namespace DirectX;
	XMStoreFloat4(this, PackedVector::XMLoadColor(&Packed));
}

inline Color::Color(const DirectX::PackedVector::XMUBYTEN4& Packed)
{
	using namespace DirectX;
	XMStoreFloat4(this, PackedVector::XMLoadUByteN4(&Packed));
}


inline bool Color::operator<(const Color& c) const
{
	// luminosity function: Y = 0.2126R + 0.7152G + 0.0722B
	return (0.2126*x + 0.7152*y + 0.0722*z) < (0.2126*c.x + 0.7152*c.y + 0.0722*c.z);
}

inline bool Color::operator>(const Color& c) const
{
	return (0.2126*x + 0.7152*y + 0.0722*z) > (0.2126*c.x + 0.7152*c.y + 0.0722*c.z);
}

inline bool Color::operator==(const Color& c) const
{
	using namespace DirectX;
	XMVECTOR c1 = XMLoadFloat4(this);
	XMVECTOR c2 = XMLoadFloat4(&c);
	return XMColorEqual(c1, c2);
}

inline bool Color::operator!=(const Color& c) const
{
	using namespace DirectX;
	XMVECTOR c1 = XMLoadFloat4(this);
	XMVECTOR c2 = XMLoadFloat4(&c);
	return XMColorNotEqual(c1, c2);
}


inline Color& Color::operator+=(const Color& c)
{
	using namespace DirectX;
	XMVECTOR c1 = XMLoadFloat4(this);
	XMVECTOR c2 = XMLoadFloat4(&c);
	XMStoreFloat4(this, XMVectorAdd(c1, c2));
	return *this;
}

inline Color& Color::operator-=(const Color& c)
{
	using namespace DirectX;
	XMVECTOR c1 = XMLoadFloat4(this);
	XMVECTOR c2 = XMLoadFloat4(&c);
	XMStoreFloat4(this, XMVectorSubtract(c1, c2));
	return *this;
}

inline Color& Color::operator*=(const Color& c)
{
	using namespace DirectX;
	XMVECTOR c1 = XMLoadFloat4(this);
	XMVECTOR c2 = XMLoadFloat4(&c);
	XMStoreFloat4(this, XMVectorMultiply(c1, c2));
	return *this;
}

inline Color& Color::operator*=(float S)
{
	using namespace DirectX;
	XMVECTOR c = XMLoadFloat4(this);
	XMStoreFloat4(this, XMVectorScale(c, S));
	return *this;
}

inline Color& Color::operator/=(const Color& c)
{
	using namespace DirectX;
	XMVECTOR c1 = XMLoadFloat4(this);
	XMVECTOR c2 = XMLoadFloat4(&c);
	XMStoreFloat4(this, XMVectorDivide(c1, c2));
	return *this;
}


inline Color Color::operator-() const
{
	using namespace DirectX;
	XMVECTOR c = XMLoadFloat4(this);
	Color R;
	XMStoreFloat4(&R, XMVectorNegate(c));
	return R;
}


inline Color operator+(const Color& C1, const Color& C2)
{
	using namespace DirectX;
	XMVECTOR c1 = XMLoadFloat4(&C1);
	XMVECTOR c2 = XMLoadFloat4(&C2);
	Color R;
	XMStoreFloat4(&R, XMVectorAdd(c1, c2));
	return R;
}

inline Color operator-(const Color& C1, const Color& C2)
{
	using namespace DirectX;
	XMVECTOR c1 = XMLoadFloat4(&C1);
	XMVECTOR c2 = XMLoadFloat4(&C2);
	Color R;
	XMStoreFloat4(&R, XMVectorSubtract(c1, c2));
	return R;
}

inline Color operator*(const Color& C1, const Color& C2)
{
	using namespace DirectX;
	XMVECTOR c1 = XMLoadFloat4(&C1);
	XMVECTOR c2 = XMLoadFloat4(&C2);
	Color R;
	XMStoreFloat4(&R, XMVectorMultiply(c1, c2));
	return R;
}

inline Color operator*(const Color& C, float S)
{
	using namespace DirectX;
	XMVECTOR c = XMLoadFloat4(&C);
	Color R;
	XMStoreFloat4(&R, XMVectorScale(c, S));
	return R;
}

inline Color operator/(const Color& C1, const Color& C2)
{
	using namespace DirectX;
	XMVECTOR c1 = XMLoadFloat4(&C1);
	XMVECTOR c2 = XMLoadFloat4(&C2);
	Color R;
	XMStoreFloat4(&R, XMVectorDivide(c1, c2));
	return R;
}

inline Color operator*(float S, const Color& C)
{
	using namespace DirectX;
	XMVECTOR c1 = XMLoadFloat4(&C);
	Color R;
	XMStoreFloat4(&R, XMVectorScale(c1, S));
	return R;
}


inline DirectX::PackedVector::XMCOLOR Color::BGRA() const
{
	using namespace DirectX;
	XMVECTOR clr = XMLoadFloat4(this);
	PackedVector::XMCOLOR Packed;
	PackedVector::XMStoreColor(&Packed, clr);
	return Packed;
}

inline DirectX::PackedVector::XMUBYTEN4 Color::RGBA() const
{
	using namespace DirectX;
	XMVECTOR clr = XMLoadFloat4(this);
	PackedVector::XMUBYTEN4 Packed;
	PackedVector::XMStoreUByteN4(&Packed, clr);
	return Packed;
}

inline Vector3 Color::ToVector3() const
{
	return Vector3(x, y, z);
}

inline Vector4 Color::ToVector4() const
{
	return Vector4(x, y, z, w);
}

inline void Color::Negate()
{
	using namespace DirectX;
	XMVECTOR c = XMLoadFloat4(this);
	XMStoreFloat4(this, XMColorNegative(c));
}

inline void Color::Negate(Color& result) const
{
	using namespace DirectX;
	XMVECTOR c = XMLoadFloat4(this);
	XMStoreFloat4(&result, XMColorNegative(c));
}

inline void Color::Saturate()
{
	using namespace DirectX;
	XMVECTOR c = XMLoadFloat4(this);
	XMStoreFloat4(this, XMVectorSaturate(c));
}

inline void Color::Saturate(Color& result) const
{
	using namespace DirectX;
	XMVECTOR c = XMLoadFloat4(this);
	XMStoreFloat4(&result, XMVectorSaturate(c));
}

inline void Color::Premultiply()
{
	using namespace DirectX;
	XMVECTOR c = XMLoadFloat4(this);
	XMVECTOR a = XMVectorSplatW(c);
	a = XMVectorSelect(g_XMIdentityR3, a, g_XMSelect1110);
	XMStoreFloat4(this, XMVectorMultiply(c, a));
}

inline void Color::Premultiply(Color& result) const
{
	using namespace DirectX;
	XMVECTOR c = XMLoadFloat4(this);
	XMVECTOR a = XMVectorSplatW(c);
	a = XMVectorSelect(g_XMIdentityR3, a, g_XMSelect1110);
	XMStoreFloat4(&result, XMVectorMultiply(c, a));
}

inline void Color::AdjustSaturation(float sat)
{
	using namespace DirectX;
	XMVECTOR c = XMLoadFloat4(this);
	XMStoreFloat4(this, XMColorAdjustSaturation(c, sat));
}

inline void Color::AdjustSaturation(float sat, Color& result) const
{
	using namespace DirectX;
	XMVECTOR c = XMLoadFloat4(this);
	XMStoreFloat4(&result, XMColorAdjustSaturation(c, sat));
}

inline void Color::AdjustContrast(float contrast)
{
	using namespace DirectX;
	XMVECTOR c = XMLoadFloat4(this);
	XMStoreFloat4(this, XMColorAdjustContrast(c, contrast));
}

inline void Color::AdjustContrast(float contrast, Color& result) const
{
	using namespace DirectX;
	XMVECTOR c = XMLoadFloat4(this);
	XMStoreFloat4(&result, XMColorAdjustContrast(c, contrast));
}


inline void Color::Modulate(const Color& c1, const Color& c2, Color& result)
{
	using namespace DirectX;
	XMVECTOR C0 = XMLoadFloat4(&c1);
	XMVECTOR C1 = XMLoadFloat4(&c2);
	XMStoreFloat4(&result, XMColorModulate(C0, C1));
}

inline Color Color::Modulate(const Color& c1, const Color& c2)
{
	using namespace DirectX;
	XMVECTOR C0 = XMLoadFloat4(&c1);
	XMVECTOR C1 = XMLoadFloat4(&c2);

	Color result;
	XMStoreFloat4(&result, XMColorModulate(C0, C1));
	return result;
}

inline void Color::Lerp(const Color& c1, const Color& c2, float t, Color& result)
{
	using namespace DirectX;
	XMVECTOR C0 = XMLoadFloat4(&c1);
	XMVECTOR C1 = XMLoadFloat4(&c2);
	XMStoreFloat4(&result, XMVectorLerp(C0, C1, t));
}

inline Color Color::Lerp(const Color& c1, const Color& c2, float t)
{
	using namespace DirectX;
	XMVECTOR C0 = XMLoadFloat4(&c1);
	XMVECTOR C1 = XMLoadFloat4(&c2);

	Color result;
	XMStoreFloat4(&result, XMVectorLerp(C0, C1, t));
	return result;
}

inline Color Color::RGBtoHSV(const Color& rgb)
{
	float r = rgb.x, g = rgb.y, b = rgb.z;
	float mx = std::max(r, std::max(g, b));
	float mn = std::min(r, std::min(g, b));
	float h = 0.f, v = mx;
	
	float d = mx - mn;
	float s = mx == 0.0f ? 0.0f : d / mx;

	if (mx == mn) {
		h = 0; // achromatic
	}
	else {
		if (mx == r) {
			h = (g - b) / d + (g < b ? 6.0f : 0.0f);
		}
		else if (mx == g) {
			h = (b - r) / d + 2.0f;
		}
		else if (mx == b) {
			h = (r - g) / d + 4.0f;
		}

		h /= 6.0f;
	}

	return Color(h, s, v, rgb.w); // preserve alpha
}

inline Color Color::HSVtoRGB(const Color& hsv)
{
	float h = hsv.x, s = hsv.y, v = hsv.z;
	float r = 0.f, g = 0.f, b = 0.f;

	int i = int(h * 6);
	float f = h * 6 - i;
	float p = v * (1 - s);
	float q = v * (1 - f * s);
	float t = v * (1 - (1 - f) * s);

	switch (i % 6) {
		case 0: r = v, g = t, b = p; break;
		case 1: r = q, g = v, b = p; break;
		case 2: r = p, g = v, b = t; break;
		case 3: r = p, g = q, b = v; break;
		case 4: r = t, g = p, b = v; break;
		case 5: r = v, g = p, b = q; break;
	}

	return Color(r, g, b, hsv.w); // preserve alpha
}



/****************************************************************************
 * Ray
 ****************************************************************************/

inline bool Ray::operator==(const Ray& r) const
{
	using namespace DirectX;
	XMVECTOR r1p = XMLoadFloat3(&origin);
	XMVECTOR r2p = XMLoadFloat3(&r.origin);
	XMVECTOR r1d = XMLoadFloat3(&direction);
	XMVECTOR r2d = XMLoadFloat3(&r.direction);
	return XMVector3Equal(r1p, r2p) && XMVector3Equal(r1d, r2d);
}

inline bool Ray::operator!=(const Ray& r) const
{
	using namespace DirectX;
	XMVECTOR r1p = XMLoadFloat3(&origin);
	XMVECTOR r2p = XMLoadFloat3(&r.origin);
	XMVECTOR r1d = XMLoadFloat3(&direction);
	XMVECTOR r2d = XMLoadFloat3(&r.direction);
	return XMVector3NotEqual(r1p, r2p) && XMVector3NotEqual(r1d, r2d);
}


inline bool Ray::Intersects(const DirectX::BoundingSphere& sphere, _Out_ float& Dist) const
{
	return sphere.Intersects(origin, direction, Dist);
}

inline bool Ray::Intersects(const DirectX::BoundingBox& box, _Out_ float& Dist) const
{
	return box.Intersects(origin, direction, Dist);
}

inline bool Ray::Intersects(const Vector3& v0, const Vector3& v1, const Vector3& v2, _Out_ float& Dist) const
{
	return DirectX::TriangleTests::Intersects(origin, direction, v0, v1, v2, Dist);
}

inline bool Ray::Intersects(const Plane& plane, _Out_ float& Dist) const
{
	using namespace DirectX;

	XMVECTOR p = XMLoadFloat4(&plane);
	XMVECTOR dir = XMLoadFloat3(&direction);

	XMVECTOR nd = XMPlaneDotNormal(p, dir);

	if (XMVector3LessOrEqual(XMVectorAbs(nd), g_RayEpsilon))
	{
		Dist = 0.f;
		return false;
	}
	else
	{
		// t = -(dot(n,origin) + D) / dot(n,dir)
		XMVECTOR pos = XMLoadFloat3(&origin);
		XMVECTOR v = XMPlaneDotNormal(p, pos);
		v = XMVectorAdd(v, XMVectorSplatW(p));
		v = XMVectorDivide(v, nd);
		float dist = -XMVectorGetX(v);

		if (dist < 0)
		{
			Dist = 0.f;
			return false;
		}
		else
		{
			Dist = dist;
			return true;
		}
	}
}



} // namespace Math

} // namespace Thesis




/****************************************************************************
 * Support for SimpleMath and Standard C++ Library containers
 ****************************************************************************/

namespace std
{
	template<> struct less<SkinCut::Math::Vector2>
	{
		bool operator()(const SkinCut::Math::Vector2& V1, const SkinCut::Math::Vector2& V2) const
		{
			return ((V1.x < V2.x) || ((V1.x == V2.x) && (V1.y < V2.y)));
		}
	};

	template<> struct less<SkinCut::Math::Vector3>
	{
		bool operator()(const SkinCut::Math::Vector3& V1, const SkinCut::Math::Vector3& V2) const
		{
			return ((V1.x < V2.x) || 
				   ((V1.x == V2.x) && (V1.y < V2.y)) || 
				   ((V1.x == V2.x) && (V1.y == V2.y) && (V1.z < V2.z)));
		}
	};

	template<> struct less<SkinCut::Math::Vector4>
	{
		bool operator()(const SkinCut::Math::Vector4& V1, const SkinCut::Math::Vector4& V2) const
		{
			return ((V1.x < V2.x) || 
				   ((V1.x == V2.x) && (V1.y < V2.y)) || 
				   ((V1.x == V2.x) && (V1.y == V2.y) && (V1.z < V2.z)) || 
				   ((V1.x == V2.x) && (V1.y == V2.y) && (V1.z == V2.z) && (V1.w < V2.w)));
		}
	};

	template<> struct less<SkinCut::Math::Matrix>
	{
		bool operator()(const SkinCut::Math::Matrix& M1, const SkinCut::Math::Matrix& M2) const
		{
			if (M1._11 != M2._11) return M1._11 < M2._11;
			if (M1._12 != M2._12) return M1._12 < M2._12;
			if (M1._13 != M2._13) return M1._13 < M2._13;
			if (M1._14 != M2._14) return M1._14 < M2._14;
			if (M1._21 != M2._21) return M1._21 < M2._21;
			if (M1._22 != M2._22) return M1._22 < M2._22;
			if (M1._23 != M2._23) return M1._23 < M2._23;
			if (M1._24 != M2._24) return M1._24 < M2._24;
			if (M1._31 != M2._31) return M1._31 < M2._31;
			if (M1._32 != M2._32) return M1._32 < M2._32;
			if (M1._33 != M2._33) return M1._33 < M2._33;
			if (M1._34 != M2._34) return M1._34 < M2._34;
			if (M1._41 != M2._41) return M1._41 < M2._41;
			if (M1._42 != M2._42) return M1._42 < M2._42;
			if (M1._43 != M2._43) return M1._43 < M2._43;
			if (M1._44 != M2._44) return M1._44 < M2._44;

			return false;
		}
	};

	template<> struct less<SkinCut::Math::Plane>
	{
		bool operator()(const SkinCut::Math::Plane& P1, const SkinCut::Math::Plane& P2) const
		{
			return ((P1.x < P2.x) || 
				   ((P1.x == P2.x) && (P1.y < P2.y)) || 
				   ((P1.x == P2.x) && (P1.y == P2.y) && (P1.z < P2.z)) || 
				   ((P1.x == P2.x) && (P1.y == P2.y) && (P1.z == P2.z) && (P1.w < P2.w)));
		}
	};

	template<> struct less<SkinCut::Math::Quaternion>
	{
		bool operator()(const SkinCut::Math::Quaternion& Q1, const SkinCut::Math::Quaternion& Q2) const
		{
			return ((Q1.x < Q2.x) || 
				   ((Q1.x == Q2.x) && (Q1.y < Q2.y)) || 
				   ((Q1.x == Q2.x) && (Q1.y == Q2.y) && (Q1.z < Q2.z)) || 
				   ((Q1.x == Q2.x) && (Q1.y == Q2.y) && (Q1.z == Q2.z) && (Q1.w < Q2.w)));
		}
	};

	template<> struct less<SkinCut::Math::Color>
	{
		bool operator()(const SkinCut::Math::Color& C1, const SkinCut::Math::Color& C2) const
		{
			return ((C1.x < C2.x) || 
				   ((C1.x == C2.x) && (C1.y < C2.y)) || 
				   ((C1.x == C2.x) && (C1.y == C2.y) && (C1.z < C2.z)) || 
				   ((C1.x == C2.x) && (C1.y == C2.y) && (C1.z == C2.z) && (C1.w < C2.w)));
		}
	};

	template<> struct less<SkinCut::Math::Ray>
	{
		bool operator()(const SkinCut::Math::Ray& R1, const SkinCut::Math::Ray& R2) const
		{
			if (R1.origin.x != R2.origin.x) return R1.origin.x < R2.origin.x;
			if (R1.origin.y != R2.origin.y) return R1.origin.y < R2.origin.y;
			if (R1.origin.z != R2.origin.z) return R1.origin.z < R2.origin.z;

			if (R1.direction.x != R2.direction.x) return R1.direction.x < R2.direction.x;
			if (R1.direction.y != R2.direction.y) return R1.direction.y < R2.direction.y;
			if (R1.direction.z != R2.direction.z) return R1.direction.z < R2.direction.z;

			return false;
		}
	};
}

