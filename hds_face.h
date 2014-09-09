#ifndef HDS_FACE_H
#define HDS_FACE_H

#include <QVector3D>

class HDS_HalfEdge;

class HDS_Face
{
public:
    HDS_Face();
    ~HDS_Face();

    HDS_Face(const HDS_Face &other);
    HDS_Face operator=(const HDS_Face &other);

    QVector3D normal;
    HDS_HalfEdge *he;
};

#endif // HDS_FACE_H
