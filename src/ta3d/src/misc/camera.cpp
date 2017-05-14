/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2005  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/
#include <stdafx.h>
#include <TA3D_NameSpace.h>
#include "camera.h"
#include "math.h"
#include <QMatrix4x4>

namespace TA3D
{

    Camera* Camera::inGame = NULL;



	Camera::Camera()					// Initialise la caméra
	{
		reset();
	}

	void Camera::reset()
	{
		// Pos
		pos.x = 0.0f;
		pos.y = 0.0f;
		pos.z = 30.0f;

		// Factor
		widthFactor = 1.0f;

		shakeVector.reset();
		shakeMagnitude = 0.0f;
		shakeDuration = 0.0f;
		shakeTotalDuration = 1.0f;
		rpos  = pos;
		dir   = up = pos;
		dir.z = -1.0f; // direction
		up.y  = 1.0f; // Haut
		zfar  = 140000.0f;
		znear = 1.0f;
		side  = dir * up;
		zfar2  = zfar * zfar;
		mirror = false;
		mirrorPos = 0.0f;
		zoomFactor = 0.5f;
	}


	void Camera::setWidthFactor(const int w, const int h)
	{
		// 1280x1024 is a 4/3 mode
		widthFactor = (w * 4 == h * 5)   ? (1.0f) : (float(w) * 0.75f / float(h));
	}

	void Camera::setMatrix(const Matrix& v)
	{
		dir.reset();
		up = dir;
		dir.z = -1.0f;
		up.y = 1.0f;
		dir = dir * v;
		up = up * v;
		side = dir * up;
	}



	void Camera::setShake(const float duration, float magnitude)
	{
		magnitude *= 0.1f;
		if (shakeDuration <= 0.0f || magnitude >= shakeMagnitude)
		{
			shakeDuration = duration;
			shakeTotalDuration = duration + 1.0f;
			shakeMagnitude = magnitude;
		}
	}


	void Camera::updateShake(const float dt)
	{
		if (shakeDuration > 0.0f)
		{
			shakeDuration -= dt;
			float dt_step = 0.03f;
			for (float c_dt = 0.0f ; c_dt < dt ; c_dt += dt_step)
			{
				float rdt = Math::Min(dt_step, dt - c_dt);
				Vector3D rand_vec( float((TA3D_RAND() % 2001) - 1000) * 0.001f * shakeMagnitude,
								   float((TA3D_RAND() % 2001) - 1000) * 0.001f * shakeMagnitude,
								   float((TA3D_RAND() % 2001) - 1000) * 0.001f * shakeMagnitude );
				shakeVector += -rdt * 10.0f * shakeVector;
				shakeVector += rand_vec;
				if (shakeVector.x < -20.0f)		shakeVector.x = -20.0f;
				else if(shakeVector.x > 20.0f)	shakeVector.x = 20.0f;
				if (shakeVector.y < -20.0f )	shakeVector.y = -20.0f;
				else if(shakeVector.y > 20.0f)	shakeVector.y = 20.0f;
				if (shakeVector.z < -20.0f)		shakeVector.z = -20.0f;
				else if(shakeVector.z > 20.0f)	shakeVector.z = 20.0f;
			}
		}
		else
		{
			float dt_step = 0.03f;
			for (float c_dt = 0.0f; c_dt < dt; c_dt += dt_step)
			{
				float rdt = Math::Min(dt_step, dt - c_dt);
				shakeVector += -rdt * 10.0f * shakeVector;

				if( shakeVector.x < -20.0f )    shakeVector.x = -20.0f;
				else
					if( shakeVector.x > 20.0f)	shakeVector.x = 20.0f;
				if (shakeVector.y < -20.0f)     shakeVector.y = -20.0f;
				else
					if( shakeVector.y > 20.0f)	shakeVector.y = 20.0f;
				if (shakeVector.z < -20.0f)     shakeVector.z = -20.0f;
				else
					if( shakeVector.z > 20.0f)	shakeVector.z = 20.0f;
			}
		}
	}



	void Camera::setView(bool classic)
	{
		zfar2 = zfar * zfar;

		glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();
		if (lp_CONFIG && lp_CONFIG->ortho_camera)
			glOrtho(-0.5f * zoomFactor * float(SCREEN_W), 0.5f * zoomFactor * float(SCREEN_W), -0.5f * zoomFactor * float(SCREEN_H), 0.5f * zoomFactor * float(SCREEN_H), znear, zfar);
		else
			glFrustum(-widthFactor * znear, widthFactor * znear, -0.75f * znear, 0.75f * znear, znear, zfar);

		if (classic)
		{
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
		}

		pos = rpos;
		Vector3D FP(pos);
		FP += dir;
		FP += shakeVector;
        QMatrix4x4 M;
        M.lookAt(QVector3D(pos.x + shakeVector.x, pos.y + shakeVector.y, pos.z + shakeVector.z),
                 QVector3D(FP.x, FP.y, FP.z),
                 QVector3D(up.x, up.y, up.z));
        glMultMatrixf(M.data());

		if (!classic)
		{
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
		}

		if (mirror)
		{
			glScalef(1.0f, -1.0f, 1.0f);
			glTranslatef(0.0f, mirrorPos - 2.0f * shakeVector.y, 0.0f);
		}
	}

    void Camera::setView(QMatrix4x4 &projectionMatrix,
                         QMatrix4x4 &modelViewMatrix)
    {
        zfar2 = zfar * zfar;

        projectionMatrix = QMatrix4x4();
        if (lp_CONFIG && lp_CONFIG->ortho_camera)
            projectionMatrix.ortho(-0.5f * zoomFactor * float(SCREEN_W), 0.5f * zoomFactor * float(SCREEN_W), -0.5f * zoomFactor * float(SCREEN_H), 0.5f * zoomFactor * float(SCREEN_H), znear, zfar);
        else
            projectionMatrix.frustum(-widthFactor * znear, widthFactor * znear, -0.75f * znear, 0.75f * znear, znear, zfar);

        modelViewMatrix = QMatrix4x4();
        pos = rpos;
        Vector3D FP(pos);
        FP += dir;
        FP += shakeVector;
        modelViewMatrix.lookAt(QVector3D(pos.x + shakeVector.x, pos.y + shakeVector.y, pos.z + shakeVector.z),
                               QVector3D(FP.x, FP.y, FP.z),
                               QVector3D(up.x, up.y, up.z));

        if (mirror)
        {
            modelViewMatrix.scale(1.0f, -1.0f, 1.0f);
            modelViewMatrix.translate(0.0f, mirrorPos - 2.0f * shakeVector.y, 0.0f);
        }
    }

    Matrix Camera::getMatrix() const
	{
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();
		if (lp_CONFIG && lp_CONFIG->ortho_camera)
			glOrtho(-0.5f * zoomFactor * float(SCREEN_W), 0.5f * zoomFactor * float(SCREEN_W), -0.5f * zoomFactor * float(SCREEN_H), 0.5f * zoomFactor * float(SCREEN_H), -512.0f, zfar);
		else
			glFrustum(-widthFactor * znear, widthFactor * znear, -0.75f * znear, 0.75f * znear, znear, zfar);

		const Vector3D FP(rpos + dir + shakeVector);
        QMatrix4x4 M;
        M.lookAt(QVector3D(pos.x + shakeVector.x, pos.y + shakeVector.y, pos.z + shakeVector.z),
                 QVector3D(FP.x, FP.y, FP.z),
                 QVector3D(up.x, up.y, up.z));
        glMultMatrixf(M.data());

		if (mirror)
		{
			glScalef(1.0f, -1.0f, 1.0f);
			glTranslatef(0.0f, mirrorPos - 2.0f * shakeVector.y, 0.0f);
		}
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		Matrix mProj;
		GLfloat tmp[16];
		glGetFloatv(GL_PROJECTION_MATRIX, tmp);
		for(int i = 0 ; i < 16 ; ++i)
			mProj.E[i % 4][i / 4] = tmp[i];
		return mProj;
	}

	void Camera::getFrustum(std::vector<Vector3D>& list)
	{
		if (lp_CONFIG && lp_CONFIG->ortho_camera)
		{
			const Vector3D wside = static_cast<float>(SCREEN_W) * side;
			const Vector3D hup = static_cast<float>(SCREEN_H) * up;
			list.push_back( rpos + znear * dir + 0.5f * zoomFactor * (-wside + hup) );
			list.push_back( rpos + znear * dir + 0.5f * zoomFactor * (wside + hup) );
			list.push_back( rpos + znear * dir + 0.5f * zoomFactor * (-wside - hup) );
			list.push_back( rpos + znear * dir + 0.5f * zoomFactor * (wside - hup) );

			list.push_back( rpos + zfar * dir + 0.5f * zoomFactor * (-wside + hup) );
			list.push_back( rpos + zfar * dir + 0.5f * zoomFactor * (wside + hup) );
			list.push_back( rpos + zfar * dir + 0.5f * zoomFactor * (-wside - hup) );
			list.push_back( rpos + zfar * dir + 0.5f * zoomFactor * (wside - hup) );
		}
		else
		{
			list.push_back( rpos + znear * (-widthFactor * side + 0.75 * up + dir) );
			list.push_back( rpos + znear * (widthFactor * side + 0.75 * up + dir) );
			list.push_back( rpos + znear * (-widthFactor * side - 0.75 * up + dir) );
			list.push_back( rpos + znear * (widthFactor * side - 0.75 * up + dir) );

			list.push_back( rpos + zfar * (-widthFactor * side + 0.75 * up + dir) );
			list.push_back( rpos + zfar * (widthFactor * side + 0.75 * up + dir) );
			list.push_back( rpos + zfar * (-widthFactor * side - 0.75 * up + dir) );
			list.push_back( rpos + zfar * (widthFactor * side - 0.75 * up + dir) );
		}
	}



} // namespace TA3D
