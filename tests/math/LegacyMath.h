/*
 * Copyright 2011-2014 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARX_TESTS_GRAPHICS_LEGACYMATH_H
#define ARX_TESTS_GRAPHICS_LEGACYMATH_H

#include "graphics/Math.h"

//! Transforms a Vertex by a quaternion
Vec3f TransformVertexQuat(const glm::quat & quat, const Vec3f & vertexin) {
	
	float rx = vertexin.x * quat.w - vertexin.y * quat.z + vertexin.z * quat.y;
	float ry = vertexin.y * quat.w - vertexin.z * quat.x + vertexin.x * quat.z;
	float rz = vertexin.z * quat.w - vertexin.x * quat.y + vertexin.y * quat.x;
	float rw = vertexin.x * quat.x + vertexin.y * quat.y + vertexin.z * quat.z;
	
	return Vec3f(
		quat.w * rx + quat.x * rw + quat.y * rz - quat.z * ry,
		quat.w * ry + quat.y * rw + quat.z * rx - quat.x * rz,
		quat.w * rz + quat.z * rw + quat.x * ry - quat.y * rx);
}

//! Invert-Transform of vertex by a quaternion
void TransformInverseVertexQuat(const glm::quat & quat, const Vec3f & vertexin, Vec3f & vertexout) {
	
	glm::quat rev_quat = glm::inverse(quat);
	
	float x = vertexin.x;
	float y = vertexin.y;
	float z = vertexin.z;
	
	float qx = rev_quat.x;
	float qy = rev_quat.y;
	float qz = rev_quat.z;
	float qw = rev_quat.w;
	
	float rx = x * qw - y * qz + z * qy;
	float ry = y * qw - z * qx + x * qz;
	float rz = z * qw - x * qy + y * qx;
	float rw = x * qx + y * qy + z * qz;
	
	vertexout.x = qw * rx + qx * rw + qy * rz - qz * ry;
	vertexout.y = qw * ry + qy * rw + qz * rx - qx * rz;
	vertexout.z = qw * rz + qz * rw + qx * ry - qy * rx;
}

//! Invert Multiply of a quaternion by another quaternion
void Quat_Divide(glm::quat * dest, const glm::quat * q1, const glm::quat * q2) {
	dest->x = q1->w * q2->x - q1->x * q2->w - q1->y * q2->z + q1->z * q2->y;
	dest->y = q1->w * q2->y - q1->y * q2->w - q1->z * q2->x + q1->x * q2->z;
	dest->z = q1->w * q2->z - q1->z * q2->w - q1->x * q2->y + q1->y * q2->x;
	dest->w = q1->w * q2->w + q1->x * q2->x + q1->y * q2->y + q1->z * q2->z;
}

//! Inverts a Quaternion
void Quat_Reverse(glm::quat * q) {
	glm::quat qw, qr;
	Quat_Divide(&qr, q, &qw);
	*q = qr;
}


glm::quat Quat_Multiply(const glm::quat & q1, const glm::quat & q2) {
	/*
	Fast multiplication

	There are some schemes available that reduces the number of internal
	multiplications when doing quaternion multiplication. Here is one:

	   q = (a, b, c, d), p = (x, y, z, w)

	   tmp_00 = (d - c) * (z - w)
	   tmp_01 = (a + b) * (x + y)
	   tmp_02 = (a - b) * (z + w)
	   tmp_03 = (c + d) * (x - y)
	   tmp_04 = (d - b) * (y - z)
	   tmp_05 = (d + b) * (y + z)
	   tmp_06 = (a + c) * (x - w)
	   tmp_07 = (a - c) * (x + w)
	   tmp_08 = tmp_05 + tmp_06 + tmp_07
	   tmp_09 = 0.5 * (tmp_04 + tmp_08)

	   q * p = (tmp_00 + tmp_09 - tmp_05,
	            tmp_01 + tmp_09 - tmp_08,
	            tmp_02 + tmp_09 - tmp_07,
	            tmp_03 + tmp_09 - tmp_06)

	With this method You get 7 less multiplications, but 15 more
	additions/subtractions. Generally, this is still an improvement.
	  */

	return glm::quat(
		q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z,
		q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
		q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z,
		q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x
	);
}

//! Converts a unit quaternion into a rotation matrix.
void MatrixFromQuat(glm::mat4x4 & m, const glm::quat & quat) {
	float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

	// calculate coefficients
	x2 = quat.x + quat.x;
	y2 = quat.y + quat.y;
	z2 = quat.z + quat.z;
	xx = quat.x * x2;
	xy = quat.x * y2;
	xz = quat.x * z2;
	yy = quat.y * y2;
	yz = quat.y * z2;
	zz = quat.z * z2;
	wx = quat.w * x2;
	wy = quat.w * y2;
	wz = quat.w * z2;

	m[0][0] = 1.0F - (yy + zz);
	m[1][0] = xy - wz;
	m[2][0] = xz + wy;
	m[3][0] = 0.0F;

	m[0][1] = xy + wz;
	m[1][1] = 1.0F - (xx + zz);
	m[2][1] = yz - wx;
	m[3][1] = 0.0F;

	m[0][2] = xz - wy;
	m[1][2] = yz + wx;
	m[2][2] = 1.0F - (xx + yy);
	m[3][2] = 0.0F;
}

//! Converts a rotation matrix into a unit quaternion.
void QuatFromMatrix(glm::quat & quat, const glm::mat4x4 & mat) {
	float m[4][4];
	m[0][0] = mat[0][0];
	m[0][1] = mat[0][1];
	m[0][2] = mat[0][2];
	m[0][3] = mat[0][3];
	m[1][0] = mat[1][0];
	m[1][1] = mat[1][1];
	m[1][2] = mat[1][2];
	m[1][3] = mat[1][3];
	m[2][0] = mat[2][0];
	m[2][1] = mat[2][1];
	m[2][2] = mat[2][2];
	m[2][3] = mat[2][3];
	m[3][0] = mat[3][0];
	m[3][1] = mat[3][1];
	m[3][2] = mat[3][2];
	m[3][3] = mat[3][3];
	float  tr, s, q[4];

	int nxt[3] = {1, 2, 0};

	tr = m[0][0] + m[1][1] + m[2][2];

	// check the diagonal
	if (tr > 0.0f)
	{
		s = sqrt(tr + 1.0f);
		quat.w = s * ( 1.0f / 2 );
		s = 0.5f / s;
		quat.x = (m[1][2] - m[2][1]) * s;
		quat.y = (m[2][0] - m[0][2]) * s;
		quat.z = (m[0][1] - m[1][0]) * s;
	}
	else
	{
		// diagonal is negative
		int i = 0;

		if (m[1][1] > m[0][0]) i = 1;

		if (m[2][2] > m[i][i]) i = 2;

		int j = nxt[i];
		int k = nxt[j];

		s = sqrt((m[i][i] - (m[j][j] + m[k][k])) + 1.0f);

		q[i] = s * 0.5f;

		if (s != 0.0) s = 0.5f / s;

		q[3] = (m[j][k] - m[k][j]) * s;
		q[j] = (m[i][j] + m[j][i]) * s;
		q[k] = (m[i][k] + m[k][i]) * s;


		quat.x = q[0];
		quat.y = q[1];
		quat.z = q[2];
		quat.w = q[3];
	}
}


inline void ZRotatePoint(Vec3f * in, Vec3f * out, float c, float s) {
	*out = Vec3f(in->x * c + in->y * s, in->y * c - in->x * s, in->z);
}

inline void YRotatePoint(Vec3f * in, Vec3f * out, float c, float s) {
	*out = Vec3f(in->x * c + in->z * s, in->y, in->z * c - in->x * s);
}

inline void XRotatePoint(Vec3f * in, Vec3f * out, float c, float s) {
	*out = Vec3f(in->x, in->y * c - in->z * s, in->y * s + in->z * c);
}

glm::quat toNonNpcRotation(const Anglef & src) {
	Anglef ang = src;
	ang.setYaw(360 - ang.getYaw());
	
	glm::mat4x4 mat;
	Vec3f vect(0, 0, 1);
	Vec3f up(0, 1, 0);
	vect = VRotateY(vect, ang.getPitch());
	vect = VRotateX(vect, ang.getYaw());
	vect = VRotateZ(vect, ang.getRoll());
	up = VRotateY(up, ang.getPitch());
	up = VRotateX(up, ang.getYaw());
	up = VRotateZ(up, ang.getRoll());
	MatrixSetByVectors(mat, vect, up);
	return glm::toQuat(mat);
}

Vec3f camEE_RT(const Vec3f & in, const EERIE_TRANSFORM & trans) {
	const Vec3f temp1 = in - trans.pos;
	Vec3f temp2;
	Vec3f temp3;
	
	temp2.x = (temp1.x * trans.ycos) + (temp1.z * trans.ysin);
	temp2.z = (temp1.z * trans.ycos) - (temp1.x * trans.ysin);
	temp3.z = (temp1.y * trans.xsin) + (temp2.z * trans.xcos);
	temp3.y = (temp1.y * trans.xcos) - (temp2.z * trans.xsin);
	temp2.y = (temp3.y * trans.zcos) - (temp2.x * trans.zsin);
	temp2.x = (temp2.x * trans.zcos) + (temp3.y * trans.zsin);
	
	return Vec3f(temp2.x, temp2.y, temp3.z);
}

#endif // ARX_TESTS_GRAPHICS_LEGACYMATH_H
