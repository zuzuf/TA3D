#ifndef OBJ_H
#define OBJ_H

#include <QString>

class Material
{
public:
    QString name;
    QString textureName;

    Material() : name(), textureName()  {}
};

#endif // OBJ_H
