#include "hds_mesh.h"
#include "glutils.hpp"

HDS_Mesh::HDS_Mesh()
{
}

HDS_Mesh::~HDS_Mesh() {
    releaseMesh();
}

void HDS_Mesh::releaseMesh() {
    for(auto vit=vertSet.begin();vit!=vertSet.end();vit++)
        if( (*vit) != nullptr )
            delete (*vit);
    vertSet.clear();

    for(auto fit=faceSet.begin();fit!=faceSet.end();fit++)
        if( (*fit) != nullptr )
            delete (*fit);
    faceSet.clear();

    for(auto heit=heSet.begin();heit!=heSet.end();heit++)
        if( (*heit) != nullptr )
            delete (*heit);
    heSet.clear();
}

void HDS_Mesh::setMesh(const vector<HDS_Face *> &faces, const vector<HDS_Vertex *> &verts, const vector<HDS_HalfEdge *> &hes) {
    releaseMesh();
    faceSet.insert(faces.begin(), faces.end());
    vertSet.insert(verts.begin(), verts.end());
    heSet.insert(hes.begin(), hes.end());
}

void HDS_Mesh::draw()
{
    if( showFace )
    {
        glColor4f(0.75, 0.75, 0.75, 1);
        /// traverse the mesh and render every single face
        for(auto fit=faceSet.begin();fit!=faceSet.end();fit++)
        {
            face_t* f = (*fit);
            // render the faces
            he_t* he = f->he;
            he_t* hen = he->next;
            he_t* hep = he->prev;

            point_t v(he->v->x(), he->v->y(), he->v->z());
            point_t vp(hep->v->x(), hep->v->y(), hep->v->z());
            point_t vn(hen->v->x(), hen->v->y(), hen->v->z());

            QVector3D n = QVector3D::crossProduct(vn - v, vp - v);
            n.normalize();
            glNormal3f(n.x(), n.y(), n.z());

            he_t* curHe = he;

            glBegin(GL_POLYGON);
            do
            {
                vert_t* v = curHe->v;
                GLUtils::useVertex(v->pos);
                curHe = curHe->next;
            }while( curHe != he );
            glEnd();
        }
    }

    if( showEdge )
    {
        glColor4f(0.25, 0.25, 0.25, 1);
        GLfloat line_mat_diffuse[4] = {0.25, 0.25, 0.25, 1};
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, line_mat_diffuse);

        // render the boundaires
        for(auto eit=heSet.begin();eit!=heSet.end();eit++)
        {
            he_t* e = (*eit);
            he_t* ef = e->flip;
            GLUtils::drawLine(e->v->pos, ef->v->pos);
        }
    }

    if( showVert )
    {
        glColor4f(1, 0, 0, 1);
        GLfloat line_mat_diffuse[4] = {1, 0, 0, 1};
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, line_mat_diffuse);

        // render the boundaires
        for(auto vit=vertSet.begin();vit!=vertSet.end();vit++)
        {
            vert_t* v = (*vit);
            glPushMatrix();
            glTranslatef(v->x(), v->y(), v->z());
#if 0
            glutSolidSphere(0.125, 16, 16);
#else
            glPointSize(2.0);
            glBegin(GL_POINTS);
            glVertex3f(0, 0, 0);
            glEnd();
#endif
            glPopMatrix();
        }
    }
}
