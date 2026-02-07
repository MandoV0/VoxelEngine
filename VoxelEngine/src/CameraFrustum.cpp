#include "CameraFrustum.h"

CameraFrustum::CameraFrustum()
{
}

CameraFrustum::~CameraFrustum()
{
}

void CameraFrustum::SetCamInternals(float angle, float ratio, float nearD, float farD)
{
	this->ratio = ratio;
	this->angle = angle;
	this->nearD = nearD;
	this->farD = farD;

	// compute width and height of the near and far plane sections
	tang = (float)tan(ANG2RAD * angle * 0.5);
	nh = nearD * tang;
	nw = nh * ratio;
	fh = farD * tang;
	fw = fh * ratio;
}

/*
* Set the camera definition
* 
* p: camera position
* l: look at point
* u: up vector
*/
void CameraFrustum::SetCamDef(const glm::vec3& p, const glm::vec3& l, const glm::vec3& u) {

	glm::vec3 dir, nc, fc, X, Y, Z;

	// compute the Z axis of camera
	// this axis points in the opposite direction from 
	// the looking direction
	Z = p - l;
	Z = glm::normalize(Z);

	// X axis of camera with given "up" vector and Z axis
	X = glm::normalize(glm::cross(u, Z));

	// the real "up" vector is the cross product of Z and X
	Y = glm::cross(Z, X);

	// compute the centers of the near and far planes
	nc = p - Z * nearD;
	fc = p - Z * farD;

	// compute the 4 corners of the frustum on the near plane
	ntl = nc + Y * nh - X * nw;
	ntr = nc + Y * nh + X * nw;
	nbl = nc - Y * nh - X * nw;
	nbr = nc - Y * nh + X * nw;

	// compute the 4 corners of the frustum on the far plane
	ftl = fc + Y * fh - X * fw;
	ftr = fc + Y * fh + X * fw;
	fbl = fc - Y * fh - X * fw;
	fbr = fc - Y * fh + X * fw;

	// compute the six planes
	// the function set3Points assumes that the points
	// are given in counter clockwise order
	pl[TOP].set3Points(ntr, ntl, ftl);
	pl[BOTTOM].set3Points(nbl, nbr, fbr);
	pl[LEFT].set3Points(ntl, nbl, fbl);
	pl[RIGHT].set3Points(nbr, ntr, fbr);
	pl[NEAR].set3Points(ntl, ntr, nbr);
	pl[FAR].set3Points(ftr, ftl, fbl);
}

int CameraFrustum::BoxInFrustum(const AABox& box) const
{
	for (int i = 0; i < 6; i++)
	{
		const glm::vec3& n = pl[i].normal;

		// Choose the “positive vertex” along the plane normal
		glm::vec3 pv = {
			n.x >= 0 ? box.max.x : box.min.x,
			n.y >= 0 ? box.max.y : box.min.y,
			n.z >= 0 ? box.max.z : box.min.z
		};

		// outside if positive vertex is behind plane
		if (glm::dot(n, pv) + pl[i].d < 0)
			return false;
	}
	return true; // Intersect

	/*
	* In the method from the paper we would do:
	* 6 Planes * 8 Corners = 48 Dot Products per Box.
	* 
	* With this method
	*/
}