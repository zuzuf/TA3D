
#include "fx.electric.h"
#include "../EngineClass.h"
#include "fx.manager.h"
#include "../3do.h"
#include "../ingame/players.h"

namespace TA3D
{


    FXElectric::FXElectric(const Vector3D& P)
        :Pos(P), life(3.0f)
    {
        for( int i = 0 ; i < 5 ; i++ )
        {
            Dir[i].x = (Math::RandFromTable() % 2001) * 0.001f - 1.0f;
            Dir[i].y = (Math::RandFromTable() % 2001) * 0.001f - 1.0f;
            Dir[i].z = (Math::RandFromTable() % 2001) * 0.001f - 1.0f;
            Dir[i].unit();
        }
    }



    bool FXElectric::move(const float dt)
    {
        life -= dt;
        // When it shoud die, return true
        return (life <= 0.0f);		
    }



    void FXElectric::draw()
    {
        glColor4ub(0x7F,0x7F,0xFF,0xFF);
        glBegin( GL_LINES );
        for( int i = 0 ; i < 5 ; i++ )
        {
            Vector3D P = Pos + (3.0f * (3.0f - life)) * Dir[i];
            glVertex3fv( (GLfloat*)&P );
            P = P + Dir[i];
            glVertex3fv( (GLfloat*)&P );
        }
        glEnd();
    }



} // namespace TA3D

