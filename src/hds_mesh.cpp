#include "hds_mesh.h"
#include "glutils.hpp"
#include "mathutils.hpp"
#include "utils.hpp"
#include "meshviewer.h"
#include <GL/glu.h>
#include<iostream>
using namespace std;

HDS_Mesh::HDS_Mesh()
{
    showFace = true;
    showVert = true;
    showEdge = true;
    showNormals = true;

}

HDS_Mesh::HDS_Mesh(const HDS_Mesh &other)
{
    /// need a deep copy
    showFace = other.showFace;
    showEdge = other.showEdge;
    showVert = other.showVert;
    showNormals = other.showNormals;

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

    /// fill in the pointers
    for( auto &he : heSet ) {
        auto he_ref = other.heMap.at(he->index);
        //cout << he_ref->index << endl;
        he->flip = heMap.at(he_ref->flip->index);
        he->prev = heMap.at(he_ref->prev->index);
        he->next = heMap.at(he_ref->next->index);

        if (he_ref->twin != nullptr)
            he->twin = heMap.at(he_ref->twin->index);

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

bool HDS_Mesh::validateEdge(he_t *e) {
    if( heMap.find(e->index) == heMap.end() ) return false;
    if( e->flip->flip != e ) return false;
    if( e->next->prev != e ) return false;
    if( e->prev->next != e ) return false;
    if( e->f == nullptr ) return false;
    if( e->v == nullptr ) return false;
    if( faceSet.find(e->f) == faceSet.end() ) return false;
    if( vertSet.find(e->v) == vertSet.end() ) return false;
    return true;
}

bool HDS_Mesh::validateFace(face_t *f) {
    if( faceMap.find(f->index) == faceMap.end() ) return false;

    int maxEdges = 100;
    he_t *he = f->he;
    he_t *curHe = he;
    int edgeCount = 0;
    do {
        curHe = curHe->next;
        ++edgeCount;
        if( edgeCount > maxEdges ) return false;
    } while( curHe != he );
    return true;
}

bool HDS_Mesh::validateVertex(vert_t *v) {
    if( vertMap.find(v->index) == vertMap.end() ) return false;

    int maxEdges =100;
    he_t *he = v->he;
    he_t *curHe = he;
    int edgeCount = 0;
    do {
        curHe = curHe->flip->next;
        ++edgeCount;
        if( edgeCount > maxEdges ) return false;
    } while( curHe != he );
    return true;
}

void HDS_Mesh::validate() {
    /// verify that the mesh has good topology
    for( auto v : vertSet ) {
        if( !validateVertex(v) ) {
            cout << "vertex #" << v->index << " is invalid." << endl;
        }
    }

    for( auto f : faceSet ) {
        if( !validateFace(f) ) {
            cout << "face #" << f->index << " is invalid." << endl;
        }
    }

    for( auto e : heSet ) {
        if( !validateEdge(e) ) {
            cout << "half edge #" << e->index << " is invalid." << endl;
        }
    }
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

void HDS_Mesh::printMesh(const string &msg)
{
    if( !msg.empty() ) {
        cout << msg << endl;
    }
    for(auto v : vertSet) {
        cout << *v << endl;
    }

    for(auto f : faceSet) {
        cout << *f << endl;
    }

    for(auto he : heSet) {
        cout << *he << endl;
    }
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

    // reset the UIDs, hack
    HDS_Face::resetIndex();
    HDS_Vertex::resetIndex();
    HDS_HalfEdge::resetIndex();

    for (auto &f : faces) {
        int faceIdx = HDS_Face::assignIndex();
        faceMap[faceIdx] = f;
        f->index = faceIdx;
        faceSet.insert(f);
    }

    for (auto &v : verts) {
        int vertIdx = HDS_Vertex::assignIndex();
        vertMap[vertIdx] = v;
        v->index = vertIdx;
        vertSet.insert(v);
    }

    heSet.insert(hes.begin(), hes.end());
    for(auto &he : heSet) {
        if( he->index >= 0 ) continue;

        int heIdx = HDS_HalfEdge::assignIndex();
        heMap[heIdx] = he;
        he->index = heIdx;

        int hefIdx = HDS_HalfEdge::assignIndex();
        he->flip->index = hefIdx;
        heMap[hefIdx] = he->flip;
    }

    /// sort the face set
    sortedFaces.assign(faceSet.begin(), faceSet.end());
    std::sort(sortedFaces.begin(), sortedFaces.end(), [](const face_t *fa, const face_t *fb) {
        auto ca = fa->corners();
        auto cb = fb->corners();
        float minZa = 1e9, minZb = 1e9;
        for (auto va : ca) {
            minZa = std::min(va->pos.z(), minZa);
        }
        for (auto vb : cb) {
            minZb = std::min(vb->pos.z(), minZb);
        }
        return minZa < minZb;
    });
}



// below newly added;

#define MAX_CHAR        128

void drawString(const char* str, int numb) {
    static int isFirstCall = 1;
    static GLuint lists;

    if( isFirstCall ) { // 如果是第一次调用，执行初始化
        // 为每一个ASCII字符产生一个显示列表
        isFirstCall = 0;

        // 申请MAX_CHAR个连续的显示列表编号
        lists = glGenLists(MAX_CHAR);

        // 把每个字符的绘制命令都装到对应的显示列表中
        wglUseFontBitmaps(wglGetCurrentDC(), 0, MAX_CHAR, lists);
    }
    // 调用每个字符对应的显示列表，绘制每个字符

    //for(int i=0; i<numb; i++){

    glCallList(lists+*str);
    cout<<"ok"<<*str<<endl;

    //}
}
void display(int num) {
    //     glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(1.0f, 1.0f, 1.0f);
    //    glRasterPos2f(0.0f, 0.0f);
    //     glScalef(100,100,100);
    char ss[20];
    itoa(num,ss,10);
    drawString(ss, num);
    num=0;

}



//   above newly adde;


void HDS_Mesh::draw(ColorMap cmap)
{
    QGLWidget  aaa;
    if( showFace )
    {
        /// traverse the mesh and render every single face
        //  cout << "sorted faces = " << sortedFaces.size() << endl;
        MeshViewer dd;

        for (auto fit = sortedFaces.begin(); fit != sortedFaces.end(); fit++)
        {
            face_t* f = (*fit);
            if( f->isCutFace ) continue;

            // render the faces
            he_t* he = f->he;
            he_t* hen = he->next;
            he_t* hep = he->prev;

            he_t* curHe = he;

            if( f->isPicked ) {
                glColor4f(0.95, 0.75, 0.75, 0.5);
            }
            else if (f->isConnector) {
                glColor4f(0.75, 0.75, 0.95, 0.5);
            }
            else {
                glColor4f(0.75, 0.75, 0.95, 0.5);
            }

            if (f->isFlap) {
                glColor4f(0.75, 0.95, 0.75, 0.5);
            }

            int vcount = 0;
            glBegin(GL_POLYGON);
            do
            {
                ++vcount;
                vert_t* v = curHe->v;

                /// interpolation
                if (!f->isConnector) {
                    QColor clr = cmap.getColor_discrete(v->colorVal);
                    GLUtils::setColor(clr, 0.5);
                }
                GLUtils::useNormal(v->normal);
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
        //glLineWidth(2.0);
        // render the boundaires
        for(auto eit=heSet.begin();eit!=heSet.end();eit++)
        {
            he_t* e = (*eit);
            he_t* en = e->next;

            QColor c = Qt::black;
            if( e->isPicked )
                c = Qt::red;
            else if( e->isCutEdge ) {
                c = Qt::green;
            }

            GLUtils::drawLine(e->v->pos, en->v->pos, c);
        }
    }

if(showVert || showNormals) {
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
            glPointSize(14.0);
            if( v->isPicked ){
                glColor4f(1, 1, 0, 1);
                //       display(v->index);

            }


            else {
                double c = .5 - clamp(v->curvature, -Pi/2, Pi/2) / Pi; //[0, 1]
                //cout << v->index << ":" << v->curvature << ", " << c << endl;
                //        if( c < 0.5 ) {
                //          glColor4f(0.0, (0.5 - c)*2, c*2.0, 1.0);
                //        }
                //        else {
                //          glColor4f(c, (c-0.5, 0.0, 1.0);
                //        }
                glColor4f(c, 1-c, 1-c, 1.0);
            }

            if( showVert )
    {
            glBegin(GL_POINTS);
            glVertex3f(0, 0, 0);
            glEnd();
    }
            if(showNormals)
            {
                glBegin(GL_LINES);


                GLUtils::setColor(Qt::green);
                GLUtils::useVertex(QVector3D(0, 0, 0));
                GLUtils::useVertex(v->normal);

                glEnd();
            }
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

void HDS_Mesh::flipShowEdges()
{
    showEdge = !showEdge;
}

void HDS_Mesh::flipShowFaces()
{
    showFace = !showFace;
}

void HDS_Mesh::flipShowVertices()
{
    showVert = !showVert;
}
void HDS_Mesh:: flipShowNormals()
{
    showNormals = !showNormals;
}

vector<HDS_Mesh::face_t *> HDS_Mesh::incidentFaces(vert_t *v)
{
    he_t *he = v->he;
    he_t *curHe = he;
    vector<face_t*> faces;
    do {
        faces.push_back(curHe->f);
        curHe = curHe->flip->next;
    } while( curHe != he );
    return faces;
}

/// all outgoing half edges of vertex v
vector<HDS_Mesh::he_t *> HDS_Mesh::incidentEdges(vert_t *v)
{
    he_t *he = v->he;
    he_t *curHe = he;
    vector<he_t*> hes;
    do {
        hes.push_back(curHe);
        curHe = curHe->flip->next;
    } while( curHe != he );
    return hes;
}

vector<HDS_Mesh::face_t *> HDS_Mesh::incidentFaces(HDS_Mesh::face_t *f)
{
    he_t *he = f->he;
    he_t *curHe = he;
    vector<face_t*> faces;
    do {
        faces.push_back(curHe->flip->f);
        curHe = curHe->next;
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

    if( it != m.end() ) {
        it->second->setPicked( !it->second->isPicked );
    }
}

void HDS_Mesh::selectFace(int idx)
{
    flipSelectionState(idx, faceMap);
}

void HDS_Mesh::selectEdge(int idx)
{
    flipSelectionState(idx, heMap);
}

void HDS_Mesh::selectVertex(int idx)
{
    flipSelectionState(idx, vertMap);
}

unordered_set<HDS_Mesh::vert_t*> HDS_Mesh::getReebPoints(const vector<double> &funcval, const QVector3D &normdir)
{

    auto moorseFunc = [&](vert_t* v, double a, double b, double c) -> double{
        if (!funcval.empty()) {
            // assign the function value to the vertex
            v->morseFunctionVal = funcval[v->index];
            return (funcval[v->index]);
            cout<<"v->index="<<v->index<<endl;
        }
        else {
            return a * v->pos.x() + b * v->pos.y() + c * v->pos.z();
            cout<<"a="<<a<<endl;      //later added;
        }
    };

    const int n = 3;
    vector<tuple<double, double, double>> randvals;
    for (int i = 0; i < 10; ++i) {
        double a = (rand() / (double)RAND_MAX - 0.5) * 1e-8 + normdir.x();
        double b = (rand() / (double)RAND_MAX - 0.5) * 1e-8 + normdir.y();
        double c = (rand() / (double)RAND_MAX - 0.5) * 1e-8 + normdir.z();
        randvals.push_back(make_tuple(a, b, c));



    }

    int s11=0,s22=0,s33=0;
    auto isReebPoint = [&](vert_t* v) {


        int s1=0,s2=0,s3=0;
        for (int tid = 0; tid < n; ++tid) {

            // perform n tests

#if 1
            double a = std::get<0>(randvals[tid]);
            double b = std::get<1>(randvals[tid]);
            double c = std::get<2>(randvals[tid]);
#else
            double a = 0, b = 0, c = 0;
#endif




            auto neighbors = v->neighbors();

            // if all neighbors have smaller z-values
            bool allSmaller = std::all_of(neighbors.begin(), neighbors.end(), [&](vert_t* n) {

                    //    cout<<"moorseFunc(n"<<n->index<<", a, b, c) = "<<moorseFunc(n, a, b, c)<<endl;
                    //    cout<<"moorseFunc(v"<<v->index<<", a, b, c) = "<<moorseFunc(v, a, b, c)<<endl;
                    return moorseFunc(n, a, b, c) > moorseFunc(v, a, b, c);

        });

            // if all neighbors have larger z-values
            bool allLarger = std::all_of(neighbors.begin(), neighbors.end(), [&](vert_t* n) {
                    return moorseFunc(n, a, b, c) < moorseFunc(v, a, b, c);
        });
            //   cout<<" moorseFunc("<<v->index<<", a, b, c) = "<< moorseFunc(v, a, b, c);
            // if this is a saddle point
            bool isSaddle = false;
            vector<double> diffs;

            if(!allSmaller&&!allLarger)                 //later added;
            {

                for (auto n : neighbors) {
                    double st= moorseFunc(n, a, b, c) - moorseFunc(v, a, b, c);
                    //     if(abs(st)<1e-5) st=0;                                             //later changed;
                    diffs.push_back(st);
                    //   diffs[n->index]=st;
                    if(v->index==6||v->index==7||v->index==5||v->index==8||v->index==100){
                        //          cout<<"moorseFunc("<<n->index<<", a, b, c) - moorseFunc("<<v->index<<", a, b, c) = "<<st<<endl;
                    }

                }
                //      cout<<"diffs[0]="<<diffs[0]<<"diffs[1]="<<diffs[1]<<"diffs[2]="<<diffs[2]<<endl; //later added;

                //      cout<<"diffs.size()"<<diffs.size()<<endl;                                    //later added;
                //      cout<<"diffs.front()"<<diffs.front()<<endl;                                   //later added;
                int ngroups = 1, sign = diffs.front() > 0 ? 1 : -1;

                for (int i = 1; i < diffs.size(); ++i) {
                    if (diffs[i] * sign >= 0) continue;
                    else {
                        sign = -sign;
                        ++ngroups;
                        if (i == diffs.size() - 1 && sign == diffs.front()) {
                            --ngroups;
                        }
                    }

                }


                if (ngroups % 2 == 1) {
                    //cout << "error in computing groups!" << endl;
                    ngroups = ngroups - 1;
                }
                //   cout<<v->index<<"ngroups = "<<ngroups<<endl;
                isSaddle = (ngroups >= 4 && ngroups % 2 == 0);

                if (isSaddle) {
                    v->rtype = HDS_Vertex::Saddle;
                    v->sdegree = (ngroups - 2) / 2;
                    //     cout << "Saddle: " << v->index << ",moorseFunc(v, a, b, c) =  " << moorseFunc(v, a, b, c) << endl;
                    //for (auto neighbor : neighbors) {
                    //  cout << moorseFunc(neighbor, a, b, c) << " ";
                    //}
                    //cout << endl;
                    s1+=1;
                    //    cout<<"saddle !!!!!!!!!"<<s1<<endl;
                }

            }

            if (allSmaller) {
                v->rtype = HDS_Vertex::Minimum;
                //       cout << "Minimum: " << v->index << ",moorseFunc(v, a, b, c) =  " << moorseFunc(v, a, b, c) << endl;
                //for (auto neighbor : neighbors) {
                //  cout << moorseFunc(neighbor, a, b, c) << " ";
                //}
                //cout << endl;
                s2+=1;
                //    cout<<"minimum  !!!!!!!!!"<<s2<<endl;
            }


            if (allLarger) {
                v->rtype = HDS_Vertex::Maximum;
                //     cout << "Maximum: " << v->index << ",moorseFunc(v, a, b, c) =  " << moorseFunc(v, a, b, c) << endl;
                //for (auto neighbor : neighbors) {
                //  cout << moorseFunc(neighbor, a, b, c) << " ";
                //}
                //cout << endl;
                s3+=1;
                //    cout<<"maximum  !!!!!!!!!"<<s3<<endl;

            }


            if (allSmaller || allLarger || isSaddle) {}
            else {
                v->rtype = HDS_Vertex::Regular;
                return false;
            }


        }
        if(s1==3)s11+=v->sdegree;
        if(s2==3)s22+=1;
        if(s3==3)s33+=1;
        cout<<"maximum = "<<s33<<endl;
        cout<<"minimum = "<<s22<<endl;

        cout<<"saddle = "<< s11<<endl;
        return true;
    };



    return Utils::filter_set(vertSet, isReebPoint);

}

void HDS_Mesh::colorVertices(const vector<double> &val)
{
#if 1
    int nverts = vertSet.size();
    for (int i = 0; i < nverts; ++i) {
        vertMap[i]->colorVal = val[i];
    }

#else
    int i = 0;
    for (auto v : vertSet) {
        v->colorVal = val[i++];
    }
#endif
}

void HDS_Mesh::save(const string &filename)
{
    // the vertices and faces are saved in the same order they are loaded in
    stringstream ss;

    // save the vertices first
    for (int i = 0; i < vertMap.size(); ++i) {
        auto v = vertMap[i];
        ss << "v " << v->pos.x() << ' ' << v->pos.y() << ' ' << v->pos.z() << endl;
    }

    // save the faces
    for (int i = 0; i < faceMap.size(); ++i) {
        auto corners = faceMap[i]->corners();
        ss << "f ";
        for (auto v : corners) {
            ss << v->index + 1 << ' ';
        }
        ss << endl;
    }

    ofstream fout(filename);
    fout << ss.str();
    fout.close();
}
