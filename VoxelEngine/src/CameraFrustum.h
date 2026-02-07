#pragma once
#include <glm.hpp>

#define ANG2RAD 3.14159265358979323846/180.0

// Used for our chunks
struct AABox
{
	glm::vec3 min;
	glm::vec3 max;

	// Returns one of the 8 corners of the box
	glm::vec3 getVertex(int index) const
	{
		return glm::vec3(
			(index & 1) ? max.x : min.x,
			(index & 2) ? max.y : min.y,
			(index & 4) ? max.z : min.z
		);
	}
};

// The Camera Frustum Plane
struct AAPlane
{
	glm::vec3 normal;	// Direction the Plane is facing
	float d;			// Distance from origin

	void set3Points(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3)
	{
		// Create two vectors from the points
		glm::vec3 aux1 = v1 - v2;
		glm::vec3 aux2 = v3 - v2;

		// Cross product for the normal
		glm::vec3 n = glm::cross(aux2, aux1);
		float len2 = glm::dot(n, n);

		// Degenerate plane check
		if (len2 < 1e-6f) // vectors too small => plane invalid
		{
			normal = glm::vec3(0.0f, 1.0f, 0.0f); // default up
			d = -glm::dot(normal, v2);
			return;
		}

		normal = n / sqrt(len2);
		d = -glm::dot(normal, v2);
	}


	float distance(const glm::vec3& p) const
	{
		return glm::dot(normal, p) + d;
	}
};

// https://cgvr.cs.uni-bremen.de/teaching/cg_literatur/lighthouse3d_view_frustum_culling/index.html
class CameraFrustum
{

private:
	enum {
		TOP = 0,
		BOTTOM = 1,
		LEFT = 2,
		RIGHT = 3,
		NEAR = 4,
		FAR = 5,
	};

public:
	enum { OUTSIDE, INSIDE, INTERSECT };

	AAPlane pl[6];

	glm::vec3 ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr;
	float nearD, farD, ratio, angle, tang;
	float nw, nh, fw, fh;

	CameraFrustum();
	~CameraFrustum();

	void SetCamInternals(float angle, float ratio, float nearD, float farD);
	void SetCamDef(const glm::vec3& p, const glm::vec3& l, const glm::vec3& u);
	int BoxInFrustum(const AABox& b) const;
};