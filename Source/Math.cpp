#include "Math.hpp"

#include <array>
#include <cmath>
#include <ctime>
#include <memory>
#include <cstdlib>


using namespace SkinCut;


/*******************************************************************************
Utility operations
*******************************************************************************/

bool Math::Equal(float x, float y, float ep) {
	return std::fabs(x - y) <= ep * std::max(1.0f, std::max(std::fabs(x), std::fabs(y)));
}

bool Math::Equal(const Math::Vector3& v0, const Math::Vector3& v1, float ep) {
	return DirectX::XMVector3NearEqual(v0, v1, DirectX::XMLoadFloat(&ep));
}

bool Math::Bound(float s, float minimum, float maximum, float ep) {
	if ((s > minimum) && (s < maximum)) { 
		return true;
	}
	if ((minimum - ep) < s && s < (maximum + ep)) { 
		return true;
	}
	return false;
}

float Math::Sign(float f) {
	return float((f > 0) - (f < 0));
}

float Math::Clamp(float value, float minimum, float maximum) {
	return std::max(minimum, std::min(value, maximum));
}


/*******************************************************************************
Geometric computations
*******************************************************************************/

float Math::PointLineDistance(Vector2& p, Vector2& p0, Vector2& p1) {
	float l2 = (p0.x-p1.x)*(p0.x-p1.x) + (p0.y-p1.y)*(p0.y-p1.y);
	if (l2 == 0.0) {
		return Vector2::Length(p - p0);
	}

	float t = Vector2::Dot(p - p0, p1 - p0) / l2;
	if (t < 0.0) {
		return Vector2::Length(p - p0);
	}
	else if (t > 1.0) {
		return Vector2::Length(p - p1);
	}

	Vector2 projection = p0 + t * (p1 - p0);
	return Vector2::Length(p - projection);
}

float Math::PointLineDistance(Vector3& p, Vector3& p0, Vector3& p1) {
	// distance = (|(v1-v0)x(v0-p)|)/(|v1-v0|)

	Vector3 vec0 = p1 - p0;
	Vector3 vec1 = p0 - p;

	return Vector3::Length(Vector3::Cross(vec0, vec1)) / Vector3::Length(vec0);
}

void Math::Barycentric(Vector3& p, Vector3& a, Vector3& b, Vector3& c, float &u, float &v, float &w) {
	// http://facultyfp.salisbury.edu/despickler/personal/Resources/Graphics/Resources/barycentric.pdf
	// n  = (b-a) × (c−a)
	// na = (c-b) × (p−b)
	// nb = (a-c) × (p−c)
	// nc = (b-a) × (p−a)
	// 
	// α = (n·na) / |n|²		|n|² = (n·n)
	// β = (n·nb) / |n|²
	// γ = (n·nc) / |n|²

	// 	Vector3 n  = Vector3::Cross(b-a, c-a);
	// 	Vector3 na = Vector3::Cross(c-b, p-b);
	// 	Vector3 nb = Vector3::Cross(a-c, p-c);
	// 
	// 	float ndotn = 1.0f / Vector3::Dot(n, n);
	// 	u = Vector3::Dot(n, na) * ndotn;
	// 	v = Vector3::Dot(n, nb) * ndotn;
	// 	w = 1.0f - u - v;
	//
	// NB: If p lies in the interior of the triangle, all of its barycentric coordinates lie in range [0,1].
	// return (v >= 0.0f) && (w >= 0.0f) && ((v + w) <= 1.0f);

	// Ericson, Christer. Real-time collision detection. CRC Press, 2004. Section 3.4 (pp. 46-52).
	Vector3 v0 = b - a;
	Vector3 v1 = c - a;
	Vector3 v2 = p - a;

	float d00 = Vector3::Dot(v0, v0);
	float d01 = Vector3::Dot(v0, v1);
	float d11 = Vector3::Dot(v1, v1);
	float d20 = Vector3::Dot(v2, v0);
	float d21 = Vector3::Dot(v2, v1);

	float denom = d00 * d11 - d01 * d01;
	v = (d11 * d20 - d01 * d21) / denom;
	w = (d00 * d21 - d01 * d20) / denom;
	u = 1.0f - v - w;

	if (Equal(u, 0.0f, 0.0001f)) { u = 0.0f; }
	if (Equal(u, 1.0f, 0.0001f)) { u = 1.0f; }
	if (Equal(v, 0.0f, 0.0001f)) { v = 0.0f; }
	if (Equal(v, 1.0f, 0.0001f)) { v = 1.0f; }
	if (Equal(w, 0.0f, 0.0001f)) { w = 0.0f; }
	if (Equal(w, 1.0f, 0.0001f)) { w = 1.0f; }
}

Math::Ray Math::CreateRay(Vector2& screenPos, Vector2& viewPort, Matrix& projection, Matrix& view) {
	// Screen space near / far
	DirectX::XMVECTOR near = DirectX::XMVectorSet(screenPos.x, screenPos.y, 0, 1);
	DirectX::XMVECTOR far = DirectX::XMVectorSet(screenPos.x, screenPos.y, 1, 1);

	// Perform reverse viewport transform to get point in normalized device coordinates
	DirectX::XMVECTOR scale = DirectX::XMVectorSet(viewPort.x * 0.5f, -viewPort.y * 0.5f, 1.0f, 1.0f);
	scale = DirectX::XMVectorReciprocal(scale);

	DirectX::XMVECTOR offset = { -1.0f, 1.0f, 0.0f, 0.0f };

	near = DirectX::XMVectorMultiplyAdd(near, scale, offset);
	far = DirectX::XMVectorMultiplyAdd(far, scale, offset);

	// Multiply by inverse world view projection matrix to get point in object space
	DirectX::XMMATRIX transform = DirectX::XMMatrixMultiply(view, projection); // world matrix is identity matrix
	transform = DirectX::XMMatrixInverse(nullptr, transform);

	near = DirectX::XMVector3TransformCoord(near, transform);
	far = DirectX::XMVector3TransformCoord(far, transform);

	Vector3 pos, dir;
	DirectX::XMStoreFloat3(&pos, near);
	DirectX::XMStoreFloat3(&dir, DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(far, near)));

	return Ray(pos, dir);
}

Math::Ray Math::CreateRay(Vector2& windowPos, Vector2& window, Matrix& projection, Matrix& view, Matrix& world) {
	using namespace DirectX;

	XMVECTOR NearScreenSpace = XMVectorSet(windowPos.x, windowPos.y, 0.0f, 1.0f);
	XMVECTOR FarScreenSpace = XMVectorSet(windowPos.x, windowPos.y, 1.0f, 1.0f);

	XMVECTOR NearObjectSpace = XMVector3Unproject(NearScreenSpace, 0.0f, 0.0f, window.x, window.y, 0.0f, 1.0f, projection, view, world);
	XMVECTOR FarObjectSpace = XMVector3Unproject(FarScreenSpace, 0.0f, 0.0f, window.x, window.y, 0.0f, 1.0f, projection, view, world);

	XMVECTOR RayDirection = XMVector3Normalize(DirectX::XMVectorSubtract(FarObjectSpace, NearObjectSpace));

	Vector3 pos, dir;
	XMStoreFloat3(&pos, NearObjectSpace);
	XMStoreFloat3(&dir, RayDirection);
	return Ray(pos, dir);
}


/*******************************************************************************
Intersection tests
*******************************************************************************/

bool Math::RayTriangleIntersection(Ray& ray, const Triangle& triangle, float& t, float& u, float& v) {
	// v1
	// | \
	// |  \
	// |   \
	// |    \
	// v0---v2

	// find vectors for edges that share v0
	Vector3 e1 = triangle.v1 - triangle.v0;
	Vector3 e2 = triangle.v2 - triangle.v0;

	// if determinant is near zero, ray is parallel to triangle
	Vector3 p = Vector3::Cross(ray.direction, e2);
	float det = Vector3::Dot(e1, p);
	if (det < EPSILON) {
		return false;
	}

	// compute u and test bounds
	Vector3 uu = ray.origin - triangle.v0;
	u = Vector3::Dot(uu, p);
	if (u < 0 || u > det) {
		return false;
	}

	// compute v and test bounds
	Vector3 vv = Vector3::Cross(uu, e1);
	v = Vector3::Dot(ray.direction, vv);
	if (v < 0 || u + v > det) {
		return false;
	}

	// compute and scale t
	float detInv = 1.0f / det;
	t = Vector3::Dot(e2, vv) * detInv;

	// scale u and v
	u *= detInv;
	v *= detInv;

	// intersection detected
	return true;
}

bool Math::RayQuadIntersection(const Ray& ray, const Quadrilateral& quad, float& t) {
	// q1---q2
	// | \   |
	// |  \  |
	// |   \ |
	// |    \|
	// q0---q3

	// First test for intersection in ∆q0q1q3

	// find vectors for edges that share q0
	Vector3 e1 = quad.v1 - quad.v0;
	Vector3 e2 = quad.v3 - quad.v0;

	// if determinant is near zero, ray is parallel to quadrilateral
	Vector3 p = Vector3::Cross(ray.direction, e2);
	float det = Vector3::Dot(e1, p);
	if (det < EPSILON) {
		return false;
	}

	float invDet = 1 / det;

	// reject rays that intersect the plane on the left of q0q1
	Vector3 uu = ray.origin - quad.v0;
	float u = Vector3::Dot(uu, p) * invDet;
	if (u < 0) {
		return false;
	}

	// reject rays that intersect the plane on the left of q0q3
	Vector3 vv = Vector3::Cross(uu, e1);
	float v = Vector3::Dot(ray.direction, vv) * invDet;
	if (v < 0) {
		return false;
	}

	// intersection is possibly in ∆q1q2q3
	if ((u + v) > 1) {
		// find vectors for edges that share q2
		Vector3 E3 = quad.v3 - quad.v2;
		Vector3 E4 = quad.v1 - quad.v2;

		// compute determinant for second triangle
		Vector3 P_ = Vector3::Cross(ray.direction, E4);
		float detp = Vector3::Dot(E3, P_);

		float invdet_ = 1 / detp;

		// reject rays that intersect the plane on the left of q2q3
		Vector3 T_ = ray.origin - quad.v2;
		float u_ = Vector3::Dot(T_, P_) * invdet_;
		if (u_ < 0) {
			return false;
		}

		// reject rays that intersect the plane on the left of q2q1
		Vector3 Q_ = Vector3::Cross(T_, E3);
		float v_ = Vector3::Dot(ray.direction, Q_) * invdet_;
		if (v_ < 0) {
			return false;
		}
	}

	// compute the ray parameter of the intersection point
	t = Vector3::Dot(e2, vv) * invDet;

	// reject ray if it does not hit the quadrilateral
	return (t >= 0);
}

bool Math::RayQuadIntersection(const Ray& ray, const Quadrilateral& quad, float& t, float& u, float& v) {
	// Rejects rays that are parallel to q, and rays that intersect the plane of
	// q either on the left of the line q0q1 or on the right of the line q0q3.

	// q1---q2
	// | \   |
	// |  \  |
	// |   \ |
	// |    \|
	// q0---q3

	Vector3 e01 = quad.v1 - quad.v0;
	Vector3 e03 = quad.v3 - quad.v0;
	Vector3 p = Vector3::Cross(ray.direction, e03);

	float det = Vector3::Dot(e01, p);
	if (abs(det) < EPSILON) {
		return false;
	}

	float detInv = 1 / det;

	Vector3 uu = ray.origin - quad.v0;

	float alpha = Vector3::Dot(uu, p) * detInv;
	if (alpha < 0) {
		return false;
	}

	Vector3 q = Vector3::Cross(uu, e01);

	float beta = Vector3::Dot(ray.direction, q) * detInv;
	if (beta < 0.0) { 
		return false;
	}

	if ((alpha + beta) > 1.0) {
		// Reject rays that intersect the plane of q either on the left of the line V11V10 or on the right of the line V11V01.
		Vector3 e23 = quad.v3 - quad.v2;
		Vector3 e21 = quad.v1 - quad.v2;
		Vector3 pPrime = Vector3::Cross(ray.direction, e21);

		float detPrime = Vector3::Dot(e23, pPrime);
		if (abs(detPrime) < EPSILON) {
			return false;
		}

		float invDetPrime = 1 / detPrime;

		Vector3 tPrime = ray.origin - quad.v2;

		float alphaPrime = Vector3::Dot(tPrime, pPrime) * invDetPrime;
		if (alphaPrime < 0) {
			return false;
		}

		Vector3 qPrime = Vector3::Cross(tPrime, e23);

		float betaPrime = Vector3::Dot(ray.direction, qPrime) * invDetPrime;
		if (betaPrime < 0) {
			return false;
		}
	}

	// Compute the ray parameter of the intersection point, and reject the ray if it does not hit q.
	t = Vector3::Dot(e03, q) * detInv;
	if (t < 0) {
		return false;
	}

	// Compute the barycentric coordinates of the fourth vertex.
	// These do not depend on the ray, and can be precomputed and stored with the quadrilateral.

	float alpha11, beta11;
	Vector3 e02 = quad.v2 - quad.v1;
	Vector3 n = Vector3::Cross(e01, e03);

	if ((abs(n.x) >= abs(n.y)) && (abs(n.x) >= abs(n.z))) {
		alpha11 = ((e02.y * e03.z) - (e02.z * e03.y)) / n.x;
		beta11  = ((e01.y * e02.z) - (e01.z * e02.y)) / n.x;
	}
	else if ((abs(n.y) >= abs(n.x)) && (abs(n.y) >= abs(n.z))) {  
		alpha11 = ((e02.z * e03.x) - (e02.x * e03.z)) / n.y;
		beta11  = ((e01.z * e02.x) - (e01.x * e02.z)) / n.y;
	}
	else {
		alpha11 = ((e02.x * e03.y) - (e02.y * e03.x)) / n.z;
		beta11  = ((e01.x * e02.y) - (e01.y * e02.x)) / n.z;
	}

	// Compute the bilinear coordinates of the intersection point (wrt the quadrilateral).
	if (abs(alpha11 - 1) < EPSILON) { // q is a trapezium
		u = alpha;
		if (abs(beta11 - 1) < EPSILON) {
			v = beta; // q is a parallelogram}
		}
		else {
			v = beta / ((u * (beta11 - 1)) + 1); // q is a trapezium
		}
	}
	else if (abs(beta11 - 1) < EPSILON) { // q is a trapezium
		v = beta;
		u = alpha / ((v * (alpha11 - 1)) + 1);
	}
	else {
		float a = 1 - beta11;
		float b = (alpha * (beta11 - 1)) - (beta * (alpha11 - 1)) - 1;
		float c = alpha;
		float d = (b * b) - (4 * a * c);
		float q = -0.5f * (b + ((b < 0 ? -1 : 1) * sqrt(d))); // quadratic formula

		u = q / a;
		if ((u < 0) || (u > 1)) {
			u = c / q;
		}

		v = beta / ((u * (beta11 - 1)) + 1);
	}

	return true;
}

bool Math::RayPlaneIntersection(const Ray& ray, const Plane& plane, float& t) {
	Vector3 n = Vector3::Normalize(plane.Normal());
	Vector3 p0 = n * -plane.w; // point on the plane

	float denom = Vector3::Dot(n, ray.direction);
	if (std::abs(denom) > EPSILON) {
		t = Vector3::Dot(p0 - ray.origin, n) / denom;
		return (t >= 0);
	}
	return false;
}

bool Math::RaySphereIntersection(Ray& ray, Sphere& sphere, float& t) {
	Vector3 l = sphere.center - ray.origin;
	float tca = Vector3::Dot(l, ray.direction);
	if (tca < 0) { 
		return false;
	}

	float d2 = Vector3::Dot(l, l) - tca * tca;
	if (d2 > sphere.radius * sphere.radius) { 
		return false;
	}

	float thc = sqrt(sphere.radius*sphere.radius - d2);
	t = tca - thc;

	return true;
}

// Polygon must be planar and its vertices must be defined in clockwise winding order
bool Math::RayPolygonIntersection(Ray& ray, std::vector<Vector3> polygon, float& t) {
	// First determine ray-plane intersection
	Vector3 e0 = polygon[1] - polygon[0];
	Vector3 e1 = polygon[polygon.size()-1] - polygon[0];
	
	Vector3 normal = Vector3::Cross(e0, e1);
	float d = Vector3::Dot(normal, polygon[0]);

	if (RayPlaneIntersection(ray, Plane(normal, d), t)) {
		Vector3 p = ray.origin + ray.direction * t;

		for (unsigned i = 1; i < polygon.size(); ++i) {
			Vector3 v0 = polygon[i-1];
			Vector3 v1 = polygon[i];

			if (Vector3::Dot(Vector3::Cross(v1 - v0, p - v0), normal) < 0) {
				return false;
			}
		}
		return true;
	}
	return false;
}

bool Math::SegmentPointIntersection(Vector3& p0, Vector3& p1, Vector3& p) {
	Vector3 e01 = p1-p0;
	float len = Vector3::Length(e01);
	e01 /= len; // normalize e01

	float t = Vector3::Dot(e01, p-p0); // project p->p0 onto p0->p1
	if (t < 0 || t > len) { return false; }
	float d = Vector3::Distance(p, p0 + (e01 * t));
	return (d < EPSILON);
}

bool Math::SegmentTriangleIntersection(Vector3& p0, Vector3& p1, Triangle& triangle, Vector3& point) {
	float t, u, v;
	Ray ray(p0, p1-p0);

	if (RayTriangleIntersection(ray, triangle, t, u, v) && t <= 1) {
		point = ray.origin + ray.direction * t;
		return true;
	}
	return false;
}

bool Math::SegmentQuadIntersection(Vector3& p0, Vector3& p1, Quadrilateral& quad, Vector3& point) {
	float t, u, v;

	Ray ray(p0, p1-p0);
	if (RayQuadIntersection(ray, quad, t, u, v) && t <= 1) {
		point = ray.origin + ray.direction * t;
		return true;
	}
	return false;
}

bool Math::SegmentPlaneIntersection(Vector3& p0, Vector3& p1, Plane& plane, Vector3& point) {
	float t;
	Ray ray(p0, p1-p0);

	if (RayPlaneIntersection(ray, plane, t) && t <= 1) {
		point = ray.origin + ray.direction * t;
		return true;
	}
	return false;
}

bool Math::SegmentSphereIntersection(Vector3& p0, Vector3& p1, Sphere& sphere, Vector3& point) {
	float t;
	Ray ray(p0, p1-p0);

	if (RaySphereIntersection(ray, sphere, t) && t <= 1) {
		point = ray.origin + ray.direction * t;
		return true;
	}
	return false;
}

bool Math::SegmentPolygonIntersection(Vector3& p0, Vector3& p1, std::vector<Vector3> polygon, Vector3& point) {
	uint32_t i, j;
	Vector3 e0, e1;
	uint32_t numPoints = static_cast<uint32_t>(polygon.size());

	// uv coordinates of the plane
	float u[4]{};
	float v[4]{};

	// Find two coplanar vectors.
	for (i = 0; i < numPoints-1; i++) {
		e0.x = polygon[i+1].x - polygon[i].x;
		e0.y = polygon[i+1].y - polygon[i].y;
		e0.z = polygon[i+1].z - polygon[i].z;

		if (e0.Length() > 0.0) { 
			break;
		}
	}
		
	for (j = i; j < numPoints-1; j++) {
		e1.x = polygon[j+1].x - polygon[j].x;
		e1.y = polygon[j+1].y - polygon[j].y;
		e1.z = polygon[j+1].z - polygon[j].z;

		if (e1.Length() > 0.0) { 
			break;
		}
	}
	
	// Degenerate polygon.
	if (j == (numPoints - 1)) {
		return false;
	}

	// Compute plane normal.
	Vector3 normal = Vector3::Cross(e0, e1);
	
	// Degenerate polygon.
	if (normal.Length() == 0.0) { 
		return false;
	}

	// Compute plane D constant (distance from origin).
	float d = Vector3::Dot(normal, polygon[0]);
	
	// Direction of the segment.
	Vector3 direction = p1 - p0;

	// Projection of the plane normal onto the segment.
	float projNormal = Vector3::Dot(normal, direction);
	
	// Segment is parallel to plane. 
	if (projNormal == 0.0) { 
		return false;
	}

	// Distance along the segment that the intersection occurs.
	float t = (d - Vector3::Dot(normal, p0)) / projNormal;

	// Segment intersects the plane behind the segment's start or exceeds the segment's length.
	if (t < 0.0 || t > 1.0) { 
		return false;
	}

	// Segment intersects the plane; find the intersection point.
	point = Vector3::Lerp(p0, p1, t);

	// Project 3d points onto 2d plane and apply Jordan curve theorem. 
	// Note: Area of polygon is not preserved in this projection, but it doesn't matter here.
	
	// Determine the axis of projection.
	enum AXIS { X_AXIS, Y_AXIS, Z_AXIS } axis;
	float projNormalX = std::abs(normal.x);
	float projNormalY = std::abs(normal.y);
	float projNormalZ = std::abs(normal.z);

	if (projNormalX > projNormalY) {
		axis = (projNormalX < projNormalZ) ? Z_AXIS : X_AXIS;
	}
	else {
		axis = (projNormalY < projNormalZ) ? Z_AXIS : Y_AXIS;
	}
	
	for (i = 0; i < numPoints; i++) {
		switch (axis) {
			case X_AXIS:
				u[i] = polygon[i].y - point.y;
				v[i] = polygon[i].z - point.z;
				break;

			case Y_AXIS:
				u[i] = polygon[i].x - point.x;
				v[i] = polygon[i].z - point.z;
				break;

			case Z_AXIS:
				u[i] = polygon[i].x - point.x;
				v[i] = polygon[i].y - point.y;
				break;
		}
	}
	
	// Determine number of crossings
	unsigned crossings = 0;
	bool sh = (v[0] < 0.0) ? false : true;

	for (i = 0; i < numPoints; i++) {
		j = i+1;
		if (j == numPoints) { j = 0; }

		bool nsh = (v[j] >= 0.0);

		if (sh != nsh) {
			if ((u[i] > 0.0) && (u[j] > 0.0)) { 
				crossings++; // Line crosses u+
			}
			else if ((u[i] > 0.0) || (u[j] > 0.0)) {
				// Compute intersection on u axis
				float uIntersect = u[i] - v[i] * (u[j] - u[i]) / (v[j] - v[i]);
				if (uIntersect > 0) { crossings++; } // line crosses u+
			}
			sh = nsh;
		}
	}
	return ((crossings % 2) == 1);
}



static bool TriangleIntervals(std::array<Math::Vector3,3>& v, std::array<float,3>& p, std::array<float,3>& d, 
							  float& t1, float& t2, Math::Vector3& p1, Math::Vector3& p2) {
	// v = triangle vertexes
	// p = projection of v onto intersection line
	// d = distance from v to plane

	// t1 = V0 + (V1 - V0) * (D0 / (D0 - D1))
	// t2 = V0 + (V2 - V0) * (D0 / (D0 - D2))
	
	// p1 = P0 + (p1 - P0) * (D0 / (D0 - D1))
	// p2 = P0 + (p2 - P0) * (D0 / (D0 - D2))

	// Test sign of distances; if both are the same sign, the multiplication is positive
	if (d[1] * d[2] > 0) { // dv[1] and dv[2] are on the same side, dv[0] is on the other side
		t1 = p[0] + (p[1] - p[0]) * (d[0] / (d[0] - d[1]));
		t2 = p[0] + (p[2] - p[0]) * (d[0] / (d[0] - d[2]));
		p1 = v[0] + (v[1] - v[0]) * (d[0] / (d[0] - d[1]));
		p2 = v[0] + (v[2] - v[0]) * (d[0] / (d[0] - d[2]));
	}
	else if (d[0] * d[2] > 0) { // dv[0] and dv[2] are on the same side, dv[1] is on the other side
		t1 = p[1] + (p[0] - p[1]) * (d[1] / (d[1] - d[0]));
		t2 = p[1] + (p[2] - p[1]) * (d[1] / (d[1] - d[2]));
		p1 = v[1] + (v[0] - v[1]) * (d[1] / (d[1] - d[0]));
		p2 = v[1] + (v[2] - v[1]) * (d[1] / (d[1] - d[2]));
	}
	else if (d[0] * d[1] > 0) { // dv[0] and dv[1] are on the same side, dv[2] is on the other side
		t1 = p[2] + (p[0] - p[2]) * (d[2] / (d[2] - d[0]));
		t2 = p[2] + (p[1] - p[2]) * (d[2] / (d[2] - d[1]));
		p1 = v[2] + (v[0] - v[2]) * (d[2] / (d[2] - d[0]));
		p2 = v[2] + (v[1] - v[2]) * (d[2] / (d[2] - d[1]));
	}

	// Test value of distances; if all distances are equal to zero, no intersection occurs
	else if (d[0] != 0) { // dv[0] is not coplanar with dv[1] and dv[2]
		t1 = p[0] + (p[1] - p[0]) * (d[0] / (d[0] - d[1]));
		t2 = p[0] + (p[2] - p[0]) * (d[0] / (d[0] - d[2]));
		p1 = v[0] + (v[1] - v[0]) * (d[0] / (d[0] - d[1]));
		p2 = v[0] + (v[2] - v[0]) * (d[0] / (d[0] - d[2]));
	}
	else if (d[1] != 0) { // dv[1] is not coplanar with dv[0] and dv[2]
		t1 = p[1] + (p[0] - p[1]) * (d[1] / (d[1] - d[0]));
		t2 = p[1] + (p[2] - p[1]) * (d[1] / (d[1] - d[2]));
		p1 = v[1] + (v[0] - v[1]) * (d[1] / (d[1] - d[0]));
		p2 = v[1] + (v[2] - v[1]) * (d[1] / (d[1] - d[2]));
	}
	else if (d[2] != 0) { // dv[2] is not coplanar with dv[0] and dv[1]
		t1 = p[2] + (p[0] - p[2]) * (d[2] / (d[2] - d[0]));
		t2 = p[2] + (p[1] - p[2]) * (d[2] / (d[2] - d[1]));
		p1 = v[2] + (v[0] - v[2]) * (d[2] / (d[2] - d[0]));
		p2 = v[2] + (v[1] - v[2]) * (d[2] / (d[2] - d[1]));
	}
	else { // triangles are coplanar
		return true;
	}

	return false;
}


static bool QuadIntervals(std::array<Math::Vector3, 4>& v, std::array<float, 4>& p, std::array<float, 4>& d, 
	                      float& t1, float& t2, Math::Vector3& p1, Math::Vector3& p2) {
	/*
	 * v = coordinates of quad vertices
	 * p = projection of quad vertices onto intersection line
	 * d = distance from quad vertices to plane
	 *
	 * t1 = V0 + (V1 - V0) * (D0 / (D0 - D1))
	 * t2 = V0 + (V2 - V0) * (D0 / (D0 - D2))
	 *
	 * p1 = P0 + (p1 - P0) * (D0 / (D0 - D1))
	 * p2 = P0 + (p2 - P0) * (D0 / (D0 - D2))
	 *
	 * Cases:
	 *  1) two on the same side, two on the other side (4 cases in clock_end)
	 *  2) three on the same side, one on the other side (4 cases in clock_end)
	 * 
	 */
	
	// Test sign of distances; the multiplication of two scalars is positive if both have the same sign
	float sd0 = Math::Sign(d[0]);
	float sd1 = Math::Sign(d[1]);
	float sd2 = Math::Sign(d[2]);
	float sd3 = Math::Sign(d[3]);

	if (d[0] * d[1] * d[2] * d[3] == 0) { return true; } // coplanar

	// Three vertices on one side, the remaining on the other side (or on the plane)
	else if (sd0 == sd1 && sd1 == sd3 && sd3 != sd2) {
		t1 = p[2] + (p[1] - p[2]) * (d[2] / (d[2] - d[1]));	// t1 = v1->v2
		t2 = p[2] + (p[3] - p[2]) * (d[2] / (d[2] - d[3]));	// t2 = v3->v2
		p1 = v[2] + (v[1] - v[2]) * (d[2] / (d[2] - d[1]));
		p2 = v[2] + (v[3] - v[2]) * (d[2] / (d[2] - d[3]));
	}
	else if (sd1 == sd2 && sd2 == sd0 && sd0 != sd3) {
		t1 = p[3] + (p[2] - p[3]) * (d[3] / (d[3] - d[2]));	// t1 = v2->v3
		t2 = p[3] + (p[0] - p[3]) * (d[3] / (d[3] - d[0]));	// t2 = v0->v3
		p1 = v[3] + (v[2] - v[3]) * (d[3] / (d[3] - d[2]));
		p2 = v[3] + (v[0] - v[3]) * (d[3] / (d[3] - d[0]));
	}
	else if (sd2 == sd3 && sd3 == sd1 && sd1 != sd0) {
		t1 = p[0] + (p[3] - p[0]) * (d[0] / (d[0] - d[3]));	// t1 = v3->v0
		t2 = p[0] + (p[1] - p[0]) * (d[0] / (d[0] - d[1]));	// t2 = v1->v0
		p1 = v[0] + (v[3] - v[0]) * (d[0] / (d[0] - d[3]));
		p2 = v[0] + (v[1] - v[0]) * (d[0] / (d[0] - d[1]));
	}
	else if (sd3 == sd0 && sd0 == sd2 && sd2 != sd1) {
		t1 = p[1] + (p[0] - p[1]) * (d[1] / (d[1] - d[0]));	// t1 = v0->v1
		t2 = p[1] + (p[2] - p[1]) * (d[1] / (d[1] - d[2]));	// t2 = v2->v1
		p1 = v[1] + (v[0] - v[1]) * (d[1] / (d[1] - d[0]));
		p2 = v[1] + (v[2] - v[1]) * (d[1] / (d[1] - d[2]));
	}

	// Two vertices on one side, the other two on the other side (or on the plane)
	else if (sd0 == sd1 && sd2 == sd3 && sd0 != sd2) { // v0 and v1 on one side, v2 and v3 on the other
		t1 = p[2] + (p[1] - p[2]) * (d[2] / (d[2] - d[1]));	// t1 = v1->v2
		t2 = p[3] + (p[0] - p[3]) * (d[3] / (d[3] - d[0]));	// t2 = v0->v3
		p1 = v[2] + (v[1] - v[2]) * (d[2] / (d[2] - d[1]));
		p2 = v[3] + (v[0] - v[3]) * (d[3] / (d[3] - d[0]));
	}
	else if (sd1 == sd2 && sd3 == sd0 && sd1 != sd3) { // v1 and v2 on one side, v2 and v3 on the other
		t1 = p[1] + (p[0] - p[1]) * (d[1] / (d[1] - d[0]));	// t1 = v0->v1
		t2 = p[2] + (p[3] - p[2]) * (d[2] / (d[2] - d[3]));	// t2 = v3->v2
		p1 = v[1] + (v[0] - v[1]) * (d[1] / (d[1] - d[0]));
		p2 = v[2] + (v[3] - v[2]) * (d[2] / (d[2] - d[3]));
	}
	else {
		throw std::exception("QuadIntervals: Degenerate quadrilateral!");
	}
	
	return false;
}


bool Math::TriangleTriangleIntersection(Triangle& triangle0, Triangle& triangle1, Vector3& P0, Vector3& P1) {
	Plane plane0;				// cutting plane
	Plane plane1;				// face plane

	float t1, t2, t3, t4;		// interval distances
	Vector3 p1, p2, p3, p4;		// interval points

	std::array<float, 3> dv1;	// distance from T1's vertices to P2
	std::array<float, 3> dv2;	// distance from T2's vertices to P1

	std::array<float, 3> pv1;	// projected triangle points
	std::array<float, 3> pv2;

	std::array<Vector3, 3> v = { triangle0.v0, triangle0.v1, triangle0.v2 };	// cutting triangle
	std::array<Vector3, 3> w = { triangle1.v0, triangle1.v1, triangle1.v2 };	// face triangle

	enum AXIS { AXIS_X, AXIS_Y, AXIS_Z } axis = AXIS_X;

	plane0.Normal((triangle0.v1-triangle0.v0).Cross(triangle0.v2-triangle0.v0));
	plane0.D(-(plane0.Normal()).Dot(triangle0.v0));

	dv2[0] = (plane0.Normal()).Dot(triangle1.v0) + plane0.D();
	dv2[1] = (plane0.Normal()).Dot(triangle1.v1) + plane0.D();
	dv2[2] = (plane0.Normal()).Dot(triangle1.v2) + plane0.D();

	if (std::fabs(dv2[0]) < EPSILON) { dv2[0] = 0; }
	if (std::fabs(dv2[1]) < EPSILON) { dv2[1] = 0; }
	if (std::fabs(dv2[2]) < EPSILON) { dv2[2] = 0; }

	// no intersection if all have same sign and do not equal zero
	if (dv2[0] * dv2[1] > 0 && dv2[0] * dv2[2] > 0) { return false; }

	plane1.Normal((triangle1.v1-triangle1.v0).Cross(triangle1.v2-triangle1.v0));
	plane1.D(-(plane1.Normal()).Dot(triangle1.v0));

	dv1[0] = (plane1.Normal()).Dot(triangle0.v0) + plane1.D();
	dv1[1] = (plane1.Normal()).Dot(triangle0.v1) + plane1.D();
	dv1[2] = (plane1.Normal()).Dot(triangle0.v2) + plane1.D();

	if (std::fabs(dv1[0]) < EPSILON) { dv1[0] = 0; }
	if (std::fabs(dv1[1]) < EPSILON) { dv1[1] = 0; }
	if (std::fabs(dv1[0]) < EPSILON) { dv1[2] = 0; }

	// no intersection if all have same sign and do not equal zero
	if (dv1[0] * dv1[1] > 0 && dv1[0] * dv1[2] > 0) { return false; }

	// Direction of the intersection line
	Vector3 direction = (plane0.Normal()).Cross(plane1.Normal());

	// Determine projection axis (which intersection segment is most closely aligned with)
	float max = 0;
	float Dx = std::fabs(direction.x);
	float Dy = std::fabs(direction.y);
	float Dz = std::fabs(direction.z);
	if (Dx > max) { max = Dx; axis = AXIS_X; }
	if (Dy > max) { max = Dy; axis = AXIS_Y; }
	if (Dz > max) { max = Dz; axis = AXIS_Z; }

	// Axis-aligned projection of vertices onto L
	switch (axis) {
		case AXIS_X:
			pv1[0] = triangle0.v0.x;
			pv1[1] = triangle0.v1.x;
			pv1[2] = triangle0.v2.x;
			pv2[0] = triangle1.v0.x;
			pv2[1] = triangle1.v1.x;
			pv2[2] = triangle1.v2.x;
			break;

		case AXIS_Y:
			pv1[0] = triangle0.v0.y;
			pv1[1] = triangle0.v1.y;
			pv1[2] = triangle0.v2.y;
			pv2[0] = triangle1.v0.y;
			pv2[1] = triangle1.v1.y;
			pv2[2] = triangle1.v2.y;
			break;

		case AXIS_Z:
			pv1[0] = triangle0.v0.z;
			pv1[1] = triangle0.v1.z;
			pv1[2] = triangle0.v2.z;
			pv2[0] = triangle1.v0.z;
			pv2[1] = triangle1.v1.z;
			pv2[2] = triangle1.v2.z;
			break;
	}

	// Compute interval and intersection points for T1
	bool coplanar = TriangleIntervals(v, pv1, dv1, t1, t2, p1, p2);

	// handle coplanar triangles separately
	if (coplanar) {
		return false;
	}

	// Compute interval and intersection points for T2
	TriangleIntervals(w, pv2, dv2, t3, t4, p3, p4);

	// Sort intervals
	if (t1 > t2) {
		std::swap(t1, t2), std::swap(p1, p2);
	}
	if (t3 > t4) {
		std::swap(t3, t4), std::swap(p3, p4);
	}

	// Intervals do not overlap
	if (t2 < t3 || t4 < t1) { return false; }

	// Determine the actual segment of intersection
	if (t3 < t1) {
		P0 = p1;
		P1 = (t4 < t2) ? p4 : p2;
	}
	else {
		P0 = p3;
		P1 = (t4 > t2) ? p2 : p4;
	}

	return true;
}


bool Math::TriangleQuadIntersection(Triangle& triangle, Quadrilateral& quad, Vector3& P0, Vector3& P1) {
	// Triangle and quadrilateral vertexes must be provided in clockwise order!

	float tt0, tt1;				// interval distances for triangle
	float qt0, qt1;				// interval distances for quadrilateral
	Vector3 tp0, tp1;			// interval points for triangle
	Vector3 qp0, qp1;			// interval points for quadrilateral

	std::array<float, 3> td;	// distance from triangle vertices to quadrilateral plane
	std::array<float, 4> qd;	// distance from quadrilateral vertices to triangle plane

	std::array<float, 3> tp;	// projected triangle points
	std::array<float, 4> qp;	// projected quadrilateral points


	// Triangle plane
	Vector3 tpn = Vector3::Cross(triangle.v1 - triangle.v0, triangle.v2 - triangle.v0); // triangle plane normal
	float tpd = -Vector3::Dot(tpn, triangle.v0); // triangle plane distance

	qd[0] = Vector3::Dot(tpn, quad.v0) + tpd;
	qd[1] = Vector3::Dot(tpn, quad.v1) + tpd;
	qd[2] = Vector3::Dot(tpn, quad.v2) + tpd;
	qd[3] = Vector3::Dot(tpn, quad.v3) + tpd;

	if (std::fabs(qd[0]) < EPSILON) { qd[0] = 0; }
	if (std::fabs(qd[1]) < EPSILON) { qd[1] = 0; }
	if (std::fabs(qd[2]) < EPSILON) { qd[2] = 0; }
	if (std::fabs(qd[3]) < EPSILON) { qd[3] = 0; }

	// No intersection if all have same sign and do not equal zero
	if (qd[0] * qd[1] > 0 && qd[0] * qd[2] > 0) { return false; }

	// Quadrilateral plane
	Vector3 qpn = Vector3::Cross(quad.v1 - quad.v0, quad.v3 - quad.v0); // quadrilateral plane normal
	float qpd = -Vector3::Dot(qpn, quad.v0);

	td[0] = Vector3::Dot(qpn, triangle.v0) + qpd;
	td[1] = Vector3::Dot(qpn, triangle.v1) + qpd;
	td[2] = Vector3::Dot(qpn, triangle.v2) + qpd;

	if (std::fabs(td[0]) < EPSILON) { td[0] = 0; }
	if (std::fabs(td[1]) < EPSILON) { td[1] = 0; }
	if (std::fabs(td[0]) < EPSILON) { td[2] = 0; }

	// No intersection if all have same sign and do not equal zero
	if (td[0] * td[1] > 0 && td[0] * td[2] > 0) { return false; }

	// Compute normalized direction of the intersection segment
	Vector3 direction = Vector3::Normalize(Vector3::Cross(tpn, qpn));

	// Determine projection axis the intersection segment is most closely aligned with
	float dX = std::fabs(direction.x);
	float dY = std::fabs(direction.y);
	float dZ = std::fabs(direction.z);

	float dMax = 0;
	enum AXIS { AXIS_X, AXIS_Y, AXIS_Z } axis{};
	if (dX > dMax) { dMax = dX, axis = AXIS_X; }
	if (dY > dMax) { dMax = dY, axis = AXIS_Y; }
	if (dZ > dMax) { dMax = dZ, axis = AXIS_Z; }

	// Axis-aligned projection of vertices onto intersection segment
	switch (axis) {
		case AXIS_X: {
			tp[0] = triangle.v0.x;
			tp[1] = triangle.v1.x;
			tp[2] = triangle.v2.x;

			qp[0] = quad.v0.x;
			qp[1] = quad.v1.x;
			qp[2] = quad.v2.x;
			qp[3] = quad.v3.x;
			break;
		}

		case AXIS_Y: {
			tp[0] = triangle.v0.y;
			tp[1] = triangle.v1.y;
			tp[2] = triangle.v2.y;

			qp[0] = quad.v0.y;
			qp[1] = quad.v1.y;
			qp[2] = quad.v2.y;
			qp[3] = quad.v3.y;
			break;
		}

		case AXIS_Z: {
			tp[0] = triangle.v0.z;
			tp[1] = triangle.v1.z;
			tp[2] = triangle.v2.z;

			qp[0] = quad.v0.z;
			qp[1] = quad.v1.z;
			qp[2] = quad.v2.z;
			qp[3] = quad.v3.z;
			break;
		}
	}

	// Compute interval and intersection points for triangle
	std::array<Vector3, 3> tv = { triangle.v0, triangle.v1, triangle.v2 };
	TriangleIntervals(tv, tp, td, tt0, tt1, tp0, tp1);

	// Compute interval and intersection points for quadrilateral
	std::array<Vector3, 4> qv = { quad.v0, quad.v1, quad.v2, quad.v3 };
	QuadIntervals(qv, qp, qd, qt0, qt1, qp0, qp1);

	// Sort intervals
	if (tt0 > tt1) {
		std::swap(tt0, tt1), std::swap(tp0, tp1);
	}

	if (qt0 > qt1) {
		std::swap(qt0, qt1), std::swap(qp0, qp1);
	}

	// Intervals do not overlap, so no intersection occurs
	if (tt1 < qt0 || qt1 < tt0) {
		return false;
	}

	// Compute the intersection segment
	if (qt0 < tt0) {
		P0 = tp0;
		P1 = (qt1 < tt1) ? qp1 : tp1;
	}
	else {
		P0 = qp0;
		P1 = (qt1 > tt1) ? tp1 : qp1;
	}

	// Order the endpoints according to the direction vector
	switch (axis) {
		case AXIS_X:
			if (direction.x < 0 && P0.x < P1.x) {
				std::swap(P0, P1);
			}
			else if (direction.x > 0 && P0.x > P1.x) {
				std::swap(P0, P1);
			}
			break;

		case AXIS_Y:
			if (direction.y < 0 && P0.y < P1.y) {
				std::swap(P0, P1);
			}
			else if (direction.y > 0 && P0.y > P1.y) {
				std::swap(P0, P1);
			}
			break;

		case AXIS_Z:
			if (direction.z < 0 && P0.z < P1.z) {
				std::swap(P0, P1);
			}
			else if (direction.z > 0 && P0.z > P1.z) {
				std::swap(P0, P1);
			}
			break;
	}

	return true;
}
