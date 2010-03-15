#ifndef MESH_H
#define MESH_H

#include "types.h"
#include <QString>
#include <QVector>
#include <QFile>
#include "misc/vector.h"
#include "obj.h"
#include "program.h"

class MeshTree;

enum MeshType { MESH_TRIANGLES, MESH_TRIANGLE_STRIP };

#define  ACTIVATION          1   // set or get
#define  STANDINGMOVEORDERS  2   // set or get
#define  STANDINGFIREORDERS  3   // set or get
#define  HEALTH              4   // get (0-100%)
#define  INBUILDSTANCE       5   // set or get
#define  BUSY                6   // set or get (used by misc. special case missions like transport ships)
#define  PIECE_XZ            7   // get
#define  PIECE_Y             8   // get
#define  UNIT_XZ             9   // get
#define  UNIT_Y              10  // get
#define  UNIT_HEIGHT         11  // get
#define  XZ_ATAN             12  // get atan of packed x,z coords
#define  XZ_HYPOT            13  // get hypot of packed x,z coords
#define  ATAN                14  // get ordinary two-parameter atan
#define  HYPOT               15  // get ordinary two-parameter hypot
#define  GROUND_HEIGHT       16  // get
#define  BUILD_PERCENT_LEFT  17  // get 0 = unit is built and ready, 1-100 = How much is left to build
#define  YARD_OPEN           18  // set or get (change which plots we occupy when building opens and closes)
#define  ARMORED             20  // set or get
#define  BUGGER_OFF          19  // set or get (ask other units to clear the area)

#define  MIN_ID                      69      // returns the lowest valid unit ID number
#define  MAX_ID                      70      // returns the highest valid unit ID number
#define  MY_ID                       71      // returns ID of current unit
#define  UNIT_TEAM                   72      // returns team(player ID in TA) of unit given with parameter
#define  UNIT_BUILD_PERCENT_LEFT     73      // basically BUILD_PERCENT_LEFT, but comes with a unit parameter
#define  UNIT_ALLIED                 74      // is unit given with parameter allied to the unit of the current COB script. 0=not allied, not zero allied
#define  UNIT_IS_ON_THIS_COMP        75      // indicates if the 1st parameter(a unit ID) is local to this computer
#define  VETERAN_LEVEL               32      // gets kills * 100

#define FLAG_HIDE               0x01
#define FLAG_WAIT_FOR_TURN      0x02
#define FLAG_NEED_COMPUTE       0x04
#define FLAG_EXPLODE            0x08
#define FLAG_ANIMATE            0x10
#define FLAG_ANIMATED_TEXTURE   0x20
#define FLAG_DONT_SHADE         0x40

    // a few things needed to handle explosions properly

#define EXPLODE_SHATTER                 1               // The piece will shatter instead of remaining whole
#define EXPLODE_EXPLODE_ON_HIT          2               // The piece will explode when it hits the ground
#define EXPLODE_FALL                    4               // The piece will fall due to gravity instead of just flying off
#define EXPLODE_SMOKE                   8               // A smoke trail will follow the piece through the air
#define EXPLODE_FIRE                    16              // A fire trail will follow the piece through the air
#define EXPLODE_BITMAPONLY              32              // The piece will not fly off or shatter or anything.  Only a bitmap explosion will be rendered.

#define EXPLODE_BITMAP1                 256
#define EXPLODE_BITMAP2                 512
#define EXPLODE_BITMAP3                 1024
#define EXPLODE_BITMAP4                 2048
#define EXPLODE_BITMAP5                 4096
#define EXPLODE_BITMAPNUKE              8192

class AXE
{
public:
    float	move_speed;
    float	move_distance;
    float	rot_angle;
    float	rot_speed;
    float	rot_accel;
    float	angle;
    float	pos;
    bool	rot_limit;
    bool	rot_speed_limit;
    float	rot_target_speed;
    bool	is_moving;

    inline AXE()    {   reset();    }

    inline void reset()
    {
        move_speed = 0.0f;
        move_distance = 0.0f;
        pos = 0.0f;
        rot_angle = 0.0f;
        rot_speed = 0.0f;
        rot_accel = 0.0f;
        angle = 0.0f;
        rot_limit = true;
        rot_speed_limit = false;
        rot_target_speed = 0.0f;
        is_moving = false;
    }

    inline void reset_move() {move_speed = 0.0f;}

    inline void reset_rot()
    {
        rot_angle = 0.0f;
        rot_accel = 0.0f;
        rot_limit = true;
        rot_speed_limit = false;
        rot_target_speed = 0.0f;
    }
};

#define SURFACE_ADVANCED		0x01		// Tell it is not a 3Do surface
#define	SURFACE_REFLEC			0x02		// Reflection
#define SURFACE_LIGHTED			0x04		// Lighting
#define SURFACE_TEXTURED		0x08		// Texturing
#define SURFACE_GOURAUD			0x10		// Gouraud shading
#define SURFACE_BLENDED			0x20		// Alpha Blending
#define SURFACE_PLAYER_COLOR	0x40		// The color is the owner's color
#define SURFACE_GLSL			0x80		// Use a shader to create a surface effect
#define SURFACE_ROOT_TEXTURE    0X100       // Use only the textures of the root object (all objects share the same texture set)

#define ROTATION				0x01
#define ROTATION_PERIODIC		0x02
#define ROTATION_COSINE			0x04		// Default calculation is linear
#define TRANSLATION				0x10
#define TRANSLATION_PERIODIC	0x20
#define TRANSLATION_COSINE		0x40

class ANIMATION				// Class used to set default animation to a model, this animation will play if no ANIMATION_DATA is provided (ie for map features)
{
public:
    byte        type;
    Vector3D    angle_0;
    Vector3D    angle_1;
    float       angle_w;
    Vector3D    translate_0;
    Vector3D    translate_1;
    float       translate_w;

    inline ANIMATION() : type(0), angle_w(0.), translate_w(0.) {}
    void animate( double &t, Vector3D &R, Vector3D& T);
};

class Mesh : public QObject
{
    Q_OBJECT;
    friend class GeometryGraph;
    friend class TextureViewer;
    friend class TreeWidget;
    friend class MeshManip;
    friend class MainWindow;
    friend class SurfaceProperties;
    friend class ShaderEditor;
    friend class Gfx;
    friend class MeshTree;
    friend class AmbientOcclusionThread;
    friend class SpringModelLoader;
    friend class Animation;
public:
    Mesh();
    ~Mesh();

    void destroy();

    void draw(int id = -1, Mesh *root = NULL);
	void drawOcclusion(int id = -1);
	void drawOcclusionMap(int id = -1);

    void load(const QString &filename);
    void save(const QString &filename);

    void load3DM(const QString &filename);
    void load3SO(const QString &filename);
    void load3DO(const QString &filename);
    void loadASC(const QString &filename, float size = 10.0f);
    void load3DS(const QString &filename, float scale = 10.0f);
    void loadOBJ(const QString &filename);

	void saveOBJ(const QString &filename);
	void saveS3O(const QString &filename);

    void computeNormals();
    void computeSize();

    bool isEmpty();

    inline float getSize()  {   return size;    }
    inline float getSize2() {   return size2;   }
    inline int getID()   {   return ID;  }
    inline QString getName()    {   return name;    }

    int hit(const Vec &pos, const Vec &dir, Vec &p);
    Mesh *getMesh(int id);
    Mesh *getMeshByScriptID(int id);
    Mesh *getMesh(const QString &name);
    Vec getRelativePosition(int id);
    Vec getRelativePositionAnim(int id);
    int getDepth(int id);
    int getParent(int id);
    sint32 nbSubObjects();
    void deleteMesh(int id);
    void invertOrientation();

    // Several functions related to UV mapping
    void autoComputeUVcoordinates();
    void sphericalMapping();
    void basicMapping();

    // Simplifies geometry
    void mergeSimilarVertices();
    void toTriangleSoup();
    void splitGeometry();               // Isolate connex components
	void computeAmbientOcclusion(int w, int h, int precision = 25);     // This is recursive only if SURFACE_ROOT_TEXTURE is set !!
    Mesh *toSingleMesh();               // Builds a single mesh containing all the geometry

    QList<Mesh*> getSubList();
    void move(const float dt);
    void resetAnimData();
    void resetScriptData();

signals:
    void loaded();

private:
    sint32 computeID(sint32 id = 0);
    void computeInfo();
    void load3DMrec(QFile &file);
	void load3DOrec(QFile &file);
	void save3DMrec(QFile &file);
    void obj_finalize(QVector<int> &face, QVector<Vec> &vertex, QVector<Vector2D> &tcoord, Material* mtl = NULL);
    void copy(const Mesh *src);
    void clear();           // Like destroy but doesn't free memory and it's not recursive

protected:
    QString             name;
    Vec                 pos;
    Mesh                *child;
    Mesh                *next;
    QVector<Vec>        vertex;
    QVector<Vec>        normal;
    QVector<GLuint>     index;
    QVector<GLuint>     tex;
    QVector<GLfloat>    tcoord;
    int                 type;
    uint32              flag;
    uint32              color;
    uint32              rColor;
    float               size;
    float               size2;
    Program             shader;
    QString             fragmentProgram;
    QString             vertexProgram;
    sint32              ID;
    sint32              nbSubObj;
    ANIMATION           defaultAnim;
public:
    AXE                 axe[3];
    bool                explode;
    float               explode_time;
    uint32              anim_flag;
    uint32              explosion_flag;
    sint32              scriptID;

public:
    static bool whiteSurface;
    static bool animated;
    static Mesh *instance();
    static bool hitTriangle(const Vec &a, const Vec &b, const Vec &c, const Vec &pos, const Vec &dir, Vec &p);
    static Mesh *createSphere(float r, int dw, int dh);
    static Mesh *createCube(float size);
    static Mesh *createCylinder(float r, float h, int d, bool capped);
    static Mesh *createCone(float r, float h, int d, bool capped);
    static Mesh *createTorus(float R, float r, int D, int d);
    static Mesh *merge(const QList<Mesh*> &list);

private:
    static Mesh *pInstance;
};

#endif // MESH_H
