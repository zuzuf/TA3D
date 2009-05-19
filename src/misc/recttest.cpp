#include "../stdafx.h"
#include "recttest.h"

namespace TA3D
{

    RectTest::RectTest (Camera& cam, const Rect<int>& pos) : cam(cam)
    {
        cam.setView();
        MATRIX_4x4 modelView;
        MATRIX_4x4 project;

        int	viewportCoords[4] = {0, 0, 0, 0};
        glGetIntegerv(GL_VIEWPORT, viewportCoords);
        glGetFloatv(GL_MODELVIEW_MATRIX,  (float*)modelView.E);
        glGetFloatv(GL_PROJECTION_MATRIX, (float*)project.E);

        modelView = Transpose(modelView);
        project = Transpose(project);

        VW =  (viewportCoords[2] - viewportCoords[0]) * 0.5f;
        VH = -(viewportCoords[3] - viewportCoords[1]) * 0.5f;

        T = modelView * project; // Matrice de transformation

        X1 = Math::Min(pos.x1, pos.x2);
        Y1 = Math::Min(pos.y1, pos.y2);
        X2 = Math::Max(pos.x1, pos.x2);
        Y2 = Math::Max(pos.y1, pos.y2);
    }

    bool RectTest::contains(const Vector3D &point) const
    {
        Vector3D Vec(point - cam.pos);
        float d = Vec.sq();

        if (d > 16384.0f && (Vec % cam.dir) <= 0.0f)
            return false;

        Vector3D UPos (glNMult(point, T));
        UPos.x = UPos.x * VW + VW;
        UPos.y = UPos.y * VH - VH;
        return X1 <= UPos.x && X2 >= UPos.x && Y1 <= UPos.y && Y2 >= UPos.y;
    }

} // namespace TA3D
