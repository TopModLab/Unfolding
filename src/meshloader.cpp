#include "meshloader.h"
#include "stringutils.h"

#include <QFile>
#include <QString>
#include <QTextStream>


ui32s_t* MeshLoader::getTriangulatedIndices() const
{
	ui32s_t* ret = new ui32s_t;
	ret->reserve(m_polys.size() * 3);
	for (auto Fi : m_polys)
	{
		if (Fi->size > 3)
		{
			// triangulate this face
			for (size_t j = 1; j < Fi->size - 1; j++)
			{
				ret->push_back(Fi->v[0]);
				ret->push_back(Fi->v[j]);
				ret->push_back(Fi->v[j + 1]);
			}
		}
		else
		{
			ret->insert(ret->end(), Fi->v.begin(), Fi->v.end());
		}
	}
	ret->shrink_to_fit();
	return ret;
}

void MeshLoader::clear() {
	/*verts.clear();
	faces.clear();
	texcoords.clear();
	normals.clear();

	verts.reserve(131076);
	normals.reserve(131076);
	texcoords.reserve(131076);
	faces.reserve(131076*2);*/
	m_verts.clear();
	m_uvs.clear();
	m_norms.clear();
	for (auto poly : m_polys)
	{
		delete poly;
	}
	m_polys.clear();
}

void MeshLoader::shrink_to_fit()
{
	m_verts.shrink_to_fit();
	m_uvs.shrink_to_fit();
	m_norms.shrink_to_fit();
	m_polys.shrink_to_fit();
}

void MeshLoader::triangulate()
{
	cout << "Triangulating the mesh ..." << endl;
	vector<PolyIndex*> newFaces, usedFases;

	usedFases.reserve(m_polys.size());
	for (auto Fi : m_polys)
	{
		if( Fi->v.size() > 3 )
		{
			// triangulate this face
			for(size_t j=1;j<Fi->v.size()-1;j++)
			{
				PolyIndex* f = new PolyIndex;
				f->v.push_back(Fi->v[0]);
				f->v.push_back(Fi->v[j]);
				f->v.push_back(Fi->v[j+1]);

				f->uv.push_back(Fi->uv[0]);
				f->uv.push_back(Fi->uv[j]);
				f->uv.push_back(Fi->uv[j+1]);

				f->n.push_back(Fi->n[0]);
				f->n.push_back(Fi->n[j]);
				f->n.push_back(Fi->n[j+1]);

				newFaces.push_back(f);
			}
			usedFases.push_back(Fi);
		}
		else
		{
			newFaces.push_back(Fi);
		}
	}
	// Clear Non-Triangular Faces
	for (auto used : usedFases)
	{
		delete used;
	}
	usedFases.clear();

	m_polys = newFaces;

	triangulated = true;

	hasVertexNormal = false;
	hasVertexTexCoord = false;

	cout << "done.";
}
/*
void MeshLoader::estimateNormals()
{
	if( hasVertexNormal )
	{
		cout << "already has vertex normal ..." << endl;
		// only estimate face normal
		for(size_t i=0;i<faces.size();i++)
		{
			QVector3D p0, p1, p2;

			p0 = verts[faces[i].v[0]];
			p1 = verts[faces[i].v[1]];
			p2 = verts[faces[i].v[2]];

			norm_t n = QVector3D::crossProduct(p0 - p1, p2 - p1);
			n.normalize();

			faces[i].normal = n;
		}
	}
	else
	{
		normals.resize(verts.size());
		for(size_t i=0;i<verts.size();i++)
			normals[i] = norm_t(0, 0, 0);

		// for each face, compute its normal
		// add the contribution to its vertices
		for(size_t i=0;i<faces.size();i++)
		{

			QVector3D p0, p1, p2;

			p0 = verts[faces[i].v[0]];
			p1 = verts[faces[i].v[1]];
			p2 = verts[faces[i].v[2]];

			norm_t n = QVector3D::crossProduct(p0 - p1, p2 - p1);
			n.normalize();

			faces[i].normal = n;

			for(size_t j=0;j<faces[i].v.size();j++)
			{
				int pidx, nidx, idx;
				idx = j;
				pidx = j-1;
				if( pidx < 0 ) pidx += faces[i].v.size();
				nidx = j+1;
				if( nidx > faces[i].v.size() - 1 ) nidx -= faces[i].v.size();

				QVector3D vp, vc, vn;
				vp = verts[faces[i].v[pidx]];
				vc = verts[faces[i].v[idx]];
				vn = verts[faces[i].v[nidx]];

				QVector3D e1 = vp - vc, e2 = vn - vc;

				float theta = QVector3D::dotProduct(e1, e2) / (e1.length() * e2.length());

				normals[faces[i].v[idx]] += theta * n;
			}
		}

		for(size_t i=0;i<normals.size();i++)
			normals[i].normalize();
	}
}
*/
istream& operator>>(istream& is, QVector3D& v) {
	qreal x, y, z;
	is >> x >> y >> z;
	v.setX(x); v.setY(y); v.setZ(z);
	return is;
}

istream& operator>>(istream& is, QVector2D& v) {
	qreal x, y;
	is >> x >> y;
	v.setX(x); v.setY(y);
	return is;
}

bool OBJLoader::load(const string &filename)
{
	try{
		if (filename[0] == ':')// resource file
		{
			cout << "Using file from qrc\n";
			clear();
			load_from_string(filename);
		}
		else
		{
			clear();
			load_from_file(filename);
		}
		m_filename = filename;
		shrink_to_fit();

		cout << "finish loading file " << m_filename << endl;
		cout << "# faces = " << m_polys.size() << endl;
		cout << "# vertices = " << m_verts.size() << endl;

		return true;
	}
	catch( exception e )
	{
		cerr << e.what() << endl;
		return false;
	}
}

OBJLoader::index_t OBJLoader::facetype(const char * str, uint32_t * val)
{
	int argv = sscanf(str, "%d/%d/%d", val, val + 1, val + 2);
	switch (argv)
	{
	case 3:// V/T/N
		return VTN;//111
	case 2:// V/T
		return VT;//011
	case 1:
		argv = sscanf(str, "%d//%d", val, val + 2);
		if (argv == 2)// V//N
		{
			return VN;//101
		}
		else// V
		{
			return V;//001
		}
	}
}

bool OBJLoader::load_from_string(const string &filename)
{
	// Read Entire File Into String
	QFile f(filename.c_str());
	if (!f.open(QFile::ReadOnly))
		return false;
	auto str = QTextStream(&f).readAll().toUtf8().toStdString();
	size_t strLen = str.length();
	const char* srcStr = str.c_str();
	f.close();
	// Parsing String
	if (srcStr != nullptr)
	{
		auto subStr = srcStr;
		int err;
		int offset = 0;
		char trash[255] = {};
		char lineHeader[3] = {};
		double val[3] = {};
		uint32_t indices[3];

		while (subStr - srcStr <= strLen)
		{
			// Skip end of line
			if (*subStr == '\n' || *subStr == '\r')
			{
				subStr++;
				continue;
			}
			lineHeader[0] = lineHeader[1] = 0;
			err = sscanf(subStr, "%s%n", &lineHeader, &offset);
			subStr += offset;
			// Vertex
			if (strcmp(lineHeader, "v") == 0)
			{
				sscanf(subStr, "%lf %lf %lf%n", val, val + 1, val + 2, &offset);
				m_verts.insert(m_verts.end(), val, val + 3);

				subStr += offset + 1;
			}
			// Texture Coordinate
			else if (strcmp(lineHeader, "vt") == 0)
			{
				err = sscanf(subStr, "%lf %lf%n", val, val + 1, &offset); m_uvs.insert(m_uvs.end(), val, val + 2);
				subStr += offset + 1;
			}
			// Vertex Normal
			else if (strcmp(lineHeader, "vn") == 0)
			{
				//float val[3];
				err = sscanf(subStr, "%lf %lf %lf%n", val, val + 1, val + 2, &offset);
				m_norms.insert(m_norms.end(), val, val + 3);

				subStr += offset + 1;
			}
			// Face Index
			else if (strcmp(lineHeader, "f") == 0)
			{
				PolyIndex* fid = new PolyIndex;
				err = sscanf(subStr, "%s%n", &trash, &offset);
				indices[1] = indices[2] = 0;
				index_t ft = facetype(trash, indices);
				fid->push_back(indices);
				subStr += offset;

				switch (ft)
				{
				case VTN://111
					while (*subStr != '\n' && *subStr != '\r' && *subStr != '\0')
					{
						err = sscanf(subStr, "%s%n", &trash, &offset);
						sscanf(trash, "%d/%d/%d", indices, indices + 1, indices + 2);
						fid->push_back(indices);
						subStr += offset;
					}
					break;
				case VT://011
					while (*subStr != '\n' && *subStr != '\r' && *subStr != '\0')
					{
						err = sscanf(subStr, "%s%n", &trash, &offset);
						sscanf(trash, "%d/%d", indices, indices + 1);
						fid->push_back(indices);
						subStr += offset;
					}
					break;
				case VN://101
					while (*subStr != '\n' && *subStr != '\r' && *subStr != '\0')
					{
						err = sscanf(subStr, "%s%n", &trash, &offset);
						sscanf(trash, "%d//%d", indices, indices + 2);
						fid->push_back(indices);
						subStr += offset;
					}
					break;
				case V://001
					while (*subStr != '\n' && *subStr != '\r' && *subStr != '\0')
					{
						err = sscanf(subStr, "%s%n", &trash, &offset);
						sscanf(trash, "%d", indices);
						fid->push_back(indices);
						subStr += offset;
					}
					break;
				default:
					break;
				}
				fid->size = fid->v.size();
				m_polys.push_back(fid);
				subStr++;
			}
			// Comment
			else if (strcmp(lineHeader, "#") == 0)
			{
				err = sscanf(subStr, "%[^\r\n]%n", &trash, &offset);
				subStr += offset + 1;
			}
			// Others
			else
			{
				// skip everything except \n and \r
				err = sscanf(subStr, "%[^\r\n]%n", &trash, &offset);
				subStr += offset + 1;
			}

		};
	}
}

bool OBJLoader::load_from_file(const string &filename)
{
	FILE* fp = fopen(filename.c_str(), "r");
	if (fp == nullptr)
	{
		return false;
	}
	int err;
	char buff[255] = {};
	char lineHeader[3] = {};
	double val[3] = {};
	uint32_t indices[3];
	char endflg;

	while (true)
	{
		lineHeader[0] = lineHeader[1] = 0;
		err = fscanf(fp, "%2s", &lineHeader);
		if (err == EOF)
		{
			break;
		}
		// Vertex
		if (strcmp(lineHeader, "v") == 0)
		{
			fscanf(fp, "%lf %lf %lf\n", val, val + 1, val + 2);
			m_verts.insert(m_verts.end(), val, val + 3);
		}
		// Texture Coordinate
		else if (strcmp(lineHeader, "vt") == 0)
		{
			fscanf(fp, "%lf %lf\n", val, val + 1);
			m_uvs.insert(m_uvs.end(), val, val + 2);
		}
		// Vertex Normal
		else if (strcmp(lineHeader, "vn") == 0)
		{
			//float val[3];
			fscanf(fp, "%lf %lf %lf\n", val, val + 1, val + 2);
			m_norms.insert(m_norms.end(), val, val + 3);
		}
		// Face Index
		else if (strcmp(lineHeader, "f") == 0)
		{
			PolyIndex* fid = new PolyIndex;
			err = fscanf(fp, "%s", &buff);
			indices[1] = indices[2] = 0;
			index_t ft = facetype(buff, indices);
			fid->push_back(indices);
			endflg = fgetc(fp);
			switch (ft)
			{
			case VTN://111
				while (endflg != '\n' && endflg != '\r' && endflg != '\0')
				{
					ungetc(endflg, fp);
					fscanf(fp, "%d/%d/%d", indices, indices + 1, indices + 2);
					fid->push_back(indices);
					endflg = fgetc(fp);
				}
				break;
			case VT://011
				while (endflg != '\n' && endflg != '\r' && endflg != '\0')
				{
					ungetc(endflg, fp);
					fscanf(fp, "%d/%d", indices, indices + 1);
					fid->push_back(indices);
					endflg = fgetc(fp);
				}
				break;
			case VN://101
				while (endflg != '\n' && endflg != '\r' && endflg != '\0')
				{
					ungetc(endflg, fp);
					fscanf(fp, "%d//%d", indices, indices + 2);
					fid->push_back(indices);
					endflg = fgetc(fp);
				}
				break;
			case V://001
				while (endflg != '\n' && endflg != '\r' && endflg != EOF)
				{
					ungetc(endflg, fp);
					fscanf(fp, "%d", indices);
					fid->push_back(indices);
					endflg = fgetc(fp);
				}
				break;
			default:
				break;
			}
			fid->size = fid->v.size();
			m_polys.push_back(fid);
		}
		// Comment
		else if (strcmp(lineHeader, "#") == 0)
		{
			fscanf(fp, "%[^\r\n]", &buff);
		}
		// Others
		else
		{
			// skip everything except \n or \r
			fscanf(fp, "%[^\r\n]", &buff);
		}
	}
	fclose(fp);
}

bool PLYLoader::load(const string& filename) {
	try{
		return true;
	}
	catch( exception e )
	{
		cerr << e.what() << endl;
		return false;
	}
}
