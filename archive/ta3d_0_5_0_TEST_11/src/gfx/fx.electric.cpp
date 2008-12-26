
#include "fx.electric.h"
#include "../EngineClass.h"
#include "fx.manager.h"
#include "../3do.h"
#include "../ingame/players.h"

namespace TA3D
{


    FXElectric::FXElectric(const Vector3D& P)
        : Pos(P), life(1.0f)
    {
        U.x = (Math::RandFromTable() % 201) * 0.01f - 1.0f;
        U.y = (Math::RandFromTable() % 201) * 0.01f - 1.0f;
        U.z = (Math::RandFromTable() % 201) * 0.01f - 1.0f;
        
        V.x = 1.0f; V.y = 0.0f; V.z = 0.0f;
        V = V * U;
        V.unit();
        
        U = V * U;
        U.unit();
    }



    bool FXElectric::move(const float dt)
    {
        life -= dt;
        // When it shoud die, return true
        return (life <= 0.0f);		
    }



    void FXElectric::draw()
    {
        glBegin( GL_LINE_STRIP );
        float start = 2.0f * life * PI;
        float end = start + 1.0f;
        float step = 0.1f;
        for( float i = start ; i <= end ; i += step )
        {
            glColor4ub(0x70+(Math::RandFromTable() & 0x1F),0x70+(Math::RandFromTable() & 0x1F),0xFF-((int)Math::RandFromTable() & 0xF),0xFF);

            Vector3D P = Pos + (cos(i) * 2.0f) * V + (sin(i) * 2.0f) * U;
            P.x += (Math::RandFromTable() % 61) * 0.01f - 0.3f;
            P.y += (Math::RandFromTable() % 61) * 0.01f - 0.3f;
            P.z += (Math::RandFromTable() % 61) * 0.01f - 0.3f;
            glVertex3fv( (GLfloat*)&P );
        }
        glEnd();
    }



} // namespace TA3D

