#ifndef MESH_H
#define MESH_H

#include "types.h"
#include <QString>
#include <QVector>
#include <QFile>
#include "misc/vector.h"
#include "obj.h"
#include "program.h"

enum MeshType { MESH_TRIANGLES, MESH_TRIANGLE_STRIP };

#define SURFACE_ADVANCED		0x01		// Tell it is not a 3Do surface
#define	SURFACE_REFLEC			0x02		// Reflection
#define SURFACE_LIGHTED			0x04		// Lighting
#define SURFACE_TEXTURED		0x08		// Texturing
#define SURFACE_GOURAUD			0x10		// Gouraud shading
#define SURFACE_BLENDED			0x20		// Alpha Blending
#define SURFACE_PLAYER_COLOR	0x40		// The color is the owner's color
#define SURFACE_GLSL			0x80		// Use a shader to create a surface effect

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
public:
    Mesh();
    ~Mesh();

    void destroy();

    void draw(int id = -1);

    void load(const QString &filename);
    void save(const QString &filename);

    void load3DM(const QString &filename);
    void load3DO(const QString &filename);
    void loadASC(const QString &filename, float size = 10.0f);
    void load3DS(const QString &filename, float scale = 10.0f);
    void loadOBJ(const QString &filename);

    void computeNormals();
    void computeSize();

    bool isEmpty();

    inline float getSize()  {   return size;    }
    inline float getSize2() {   return size2;   }
    inline int getID()   {   return ID;  }
    inline QString getName()    {   return name;    }

    int hit(const Vec &pos, const Vec &dir, Vec &p);
    Mesh *getMesh(int id);
    Vec getRelativePosition(int id);
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
    void computeAmbientOcclusion(int w, int h, Mesh *base = NULL);     // This is not recursive !!

signals:
    void loaded();

private:
    sint32 computeID(sint32 id = 0);
    void computeInfo();
    void load3DMrec(QFile &file);
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

public:
    static bool whiteSurface;
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
