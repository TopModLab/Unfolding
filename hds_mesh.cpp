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
  showFace = other.showFace;
  showEdge = other.showEdge;
  showVert = other.showVert;

  /// copy the vertices set
  vertSet.clear();
  vertMap.clear();
  for( auto v : other.vertSet ) {
    /// he is not set for this vertex
    vert_t *nv = new vert_t(*v);
    vertSet.insert(nv);
    vertMap.insert(make_pair(v->index, nv));
  }

  faceSet.clear();
  faceMap.clear();
  for( auto f : other.faceSet ) {
    /// he is not set for this vertex
    face_t *nf = new face_t(*f);
    faceSet.insert(nf);
    faceMap.insert(make_pair(f->index, nf));
  }

  heSet.clear();
  heMap.clear();
  for( auto he : other.heSet ) {
    /// face, vertex, prev, next, and flip are not set yet
    he_t *nhe = new he_t(*he);
    heSet.insert(nhe);
    heMap.insert(make_pair(he->index, nhe));
  }

  /// connect the half edges
  for( auto &he : heSet ) {
    auto he_ref = other.heMap.at(he->index);
    he->flip = heMap.at(he_ref->flip->index);
    he->prev = heMap.at(he_ref->prev->index);
    he->next = heMap.at(he_ref->next->index);

    he->f = faceMap.at(he_ref->f->index);
    he->v = vertMap.at(he_ref->v->index);
  }

  /// set the half edges for faces
  for( auto &f : faceSet ) {
    auto f_ref = other.faceMap.at(f->index);
    f->he = heMap.at(f_ref->he->index);
  }

  /// set the half edges for vertices
  for( auto &v : vertSet ) {
    auto v_ref = other.vertMap.at(v->index);
    v->he = heMap.at(v_ref->he->index);
  }
}

HDS_Mesh::~HDS_Mesh() {
  releaseMesh();
}

void HDS_Mesh::printInfo(const string& msg)
{
  if( !msg.empty() ) {
    cout << msg << endl;
  }
  cout << "#vertices = " << vertSet.size() << endl;
  cout << "#faces = " << faceSet.size() << endl;
  cout << "#half edges = " << heSet.size() << endl;
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
    if( he->index >= 0 ) continue;

    heMap[heIdx] = he;
    he->index = heIdx;
    ++heIdx;

    he->flip->index = heIdx;
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

set<HDS_Mesh::face_t *> HDS_Mesh::incidentFaces(vert_t *v)
{
  he_t *he = v->he;
  he_t *curHe = he;
  set<face_t*> faces;
  do {
    faces.insert(curHe->f);
    curHe = curHe->flip->next;
  } while( curHe != he );
  return faces;
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
