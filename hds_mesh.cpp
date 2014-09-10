#include "hds_mesh.h"
#include "glutils.hpp"
#include "mathutils.hpp"

HDS_Mesh::HDS_Mesh()
{
  showFace = true;
  showVert = true;
  showEdge = true;
}

HDS_Mesh::HDS_Mesh(const HDS_Mesh &other)
{
  /// need a deep copy
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
  int faceIdx = 0;
  for(auto &f : faceSet) {
    faceMap[faceIdx] = f;
    f->index = faceIdx;
    ++faceIdx;
  }

  vertSet.insert(verts.begin(), verts.end());
  int vertIdx = 0;
  for(auto &v : vertSet) {
    vertMap[vertIdx] = v;
    v->index = vertIdx;
    ++vertIdx;
  }

  heSet.insert(hes.begin(), hes.end());
  int heIdx = 0;
  for(auto &he : heSet) {
    heMap[heIdx] = he;
    if( he->index >= 0 ) continue;
    he->index = heIdx;
    he->flip->index = ++heIdx;
    heMap[heIdx] = he->flip;
    ++heIdx;
  }
}

void HDS_Mesh::draw()
{
  if( showFace )
  {
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

      if( f->isPicked ) {
        glColor4f(0.95, 0.75, 0.75, 0.5);
      }
      else {
        glColor4f(0.75, 0.75, 0.95, 0.5);
      }
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
    glLineWidth(2.0);
    // render the boundaires
    for(auto eit=heSet.begin();eit!=heSet.end();eit++)
    {
      he_t* e = (*eit);
      he_t* en = e->next;

      if( e->isPicked )
        glColor4f(0.25, 0.25, 0.25, 1);
      else
        glColor4f(0.25, 0.75, 0.25, 1);

      GLUtils::drawLine(e->v->pos, en->v->pos, e->isPicked?Qt::red:Qt::black);
    }
  }

  if( showVert )
  {
    glColor4f(0, 0, 1, 1);
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
      glPointSize(10.0);
      if( v->isPicked )
        glColor4f(1, 0, 0, 1);
      else
        glColor4f(0, 0, 1, 1);

      glBegin(GL_POINTS);
      glVertex3f(0, 0, 0);
      glEnd();
#endif
      glPopMatrix();
    }
  }
}

void HDS_Mesh::drawFaceIndices()
{
  for(auto &f : faceSet) {
    float r, g, b;
    encodeIndex<float>(f->index, r, g, b);
    glColor4f(r, g, b, 1.0);

    he_t* he = f->he;
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

void HDS_Mesh::drawEdgeIndices()
{
  for(auto eit=heSet.begin();eit!=heSet.end();eit++)
  {
    he_t* e = (*eit);
    he_t* en = e->next;

    // draw only odd index half edges
    if( e->index & 0x1 ) continue;

    float r, g, b;
    encodeIndex<float>(e->index, r, g, b);
    glLineWidth(2.0);
    GLUtils::drawLine(e->v->pos, en->v->pos, QColor::fromRgbF(r, g, b));
  }
}

void HDS_Mesh::drawVertexIndices()
{
  glColor4f(0, 0, 1, 1);
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
    glPointSize(10.0);

    float r, g, b;
    encodeIndex<float>(v->index, r, g, b);
    glColor4f(r, g, b, 1.0);

    glBegin(GL_POINTS);
    glVertex3f(0, 0, 0);
    glEnd();
#endif
    glPopMatrix();
  }
}

template <typename T>
void HDS_Mesh::flipSelectionState(int idx, unordered_map<int, T> &m) {
  auto it = m.find(idx);
  auto flip = [=](bool &v) {
    v = !v;
  };

  if( it != m.end() ) {
    flip(it->second->isPicked);
  }
}

void HDS_Mesh::selectFace(int idx)
{
  flipSelectionState(idx, faceMap);
}

void HDS_Mesh::selectEdge(int idx)
{
  flipSelectionState(idx, heMap);
  flipSelectionState(idx+1, heMap);
}

void HDS_Mesh::selectVertex(int idx)
{
  flipSelectionState(idx, vertMap);
}
