/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2010 Interactive Visualization and Data Analysis Group.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

//!    File   : Mesh.cpp
//!    Author : Jens Krueger
//!             IVCI & DFKI & MMCI, Saarbruecken
//!             SCI Institute, University of Utah
//!    Date   : July 2010
//
//!    Copyright (C) 2010 DFKI, MMCI, SCI Institute

#include <algorithm>
#include "Mesh.h"
#include "KDTree.h"

using namespace tuvok;

Mesh::Mesh() :
  m_KDTree(0),
  m_DefColor(1,1,1,1),
  m_MeshDesc("Generic Triangle Mesh"),
  m_meshType(MT_TRIANGLES)
{
  m_VerticesPerPoly = (m_meshType == Mesh::MT_TRIANGLES) ? 3 : 2;
}

Mesh::Mesh(const VertVec& vertices, const NormVec& normals,
           const TexCoordVec& texcoords, const ColorVec& colors,
           const IndexVec& vIndices, const IndexVec& nIndices,
           const IndexVec& tIndices, const IndexVec& cIndices,
           bool bBuildKDTree, bool bScaleToUnitCube,
           const std::string& desc, EMeshType meshType) :
  m_KDTree(0),
  m_vertices(vertices),
  m_normals(normals),
  m_texcoords(texcoords),
  m_colors(colors),
  m_VertIndices(vIndices),
  m_NormalIndices(nIndices),
  m_TCIndices(tIndices),
  m_COLIndices(cIndices),
  m_DefColor(1,1,1,1),
  m_MeshDesc(desc),
  m_meshType(meshType)
{
  m_VerticesPerPoly = (m_meshType == Mesh::MT_TRIANGLES) ? 3 : 2;

  ComputeAABB();
  if (bScaleToUnitCube) ScaleToUnitCube(); 
  if (bBuildKDTree) m_KDTree = new KDTree(this);
}


void Mesh::ComputeAABB() {
  if (m_vertices.empty()) return;

  m_Bounds[0] = m_vertices[0];
  m_Bounds[1] = m_vertices[0];

  for (VertVec::iterator i = m_vertices.begin()+1;i<m_vertices.end();i++) {
    if (i->x < m_Bounds[0].x) m_Bounds[0].x = i->x;
    if (i->x > m_Bounds[1].x) m_Bounds[1].x = i->x;
    if (i->y < m_Bounds[0].y) m_Bounds[0].y = i->y;
    if (i->y > m_Bounds[1].y) m_Bounds[1].y = i->y;
    if (i->z < m_Bounds[0].z) m_Bounds[0].z = i->z;
    if (i->z > m_Bounds[1].z) m_Bounds[1].z = i->z;
  }
}

void Mesh::ComputeUnitCubeScale(FLOATVECTOR3& scale, 
                                FLOATVECTOR3& translation) {
  if (m_vertices.empty()) {
    translation = FLOATVECTOR3(0,0,0);
    scale = FLOATVECTOR3(1,1,1);
    return;
  }

  FLOATVECTOR3 maxExtensionV = (m_Bounds[1]-m_Bounds[0]);

  float maxExtension = (maxExtensionV.x > maxExtensionV.y)
                          ? ((maxExtensionV.x > maxExtensionV.z) ?
                              maxExtensionV.x : maxExtensionV.z)
                          : ((maxExtensionV.y > maxExtensionV.z) ? 
                              maxExtensionV.y : maxExtensionV.z);

  scale = 1.0f/maxExtension;
  translation = - (m_Bounds[1]+m_Bounds[0])/(2*maxExtension);
}

void Mesh::Transform(const FLOATMATRIX4& m) {
  for (VertVec::iterator i = m_vertices.begin();i<m_vertices.end();i++) {
    *i = (FLOATVECTOR4((*i),1)*m).xyz();
  }
  GeometryHasChanged(true, true);
}


void Mesh::ScaleAndBias(const FLOATVECTOR3& scale,
                        const FLOATVECTOR3& translation) {

  for (VertVec::iterator i = m_vertices.begin();i<m_vertices.end();i++)
	  *i = (*i*scale) + translation;

  m_Bounds[0] = (m_Bounds[0] * scale) + translation;
  m_Bounds[1] = (m_Bounds[1] * scale) + translation;

  GeometryHasChanged(false, true);
}

void Mesh::GeometryHasChanged(bool bUpdateAABB, bool bUpdateKDtree) {
  if (bUpdateAABB) ComputeAABB();
  if (bUpdateKDtree && m_KDTree) ComputeKDTree();
}

void Mesh::ScaleToUnitCube() {
  FLOATVECTOR3 scale, translation;
  ComputeUnitCubeScale(scale,translation);
  ScaleAndBias(scale,translation);
}

Mesh::~Mesh() {
  delete m_KDTree;
}

void Mesh::RecomputeNormals() {
  if (m_meshType != MT_TRIANGLES) return;

  m_normals.resize(m_vertices.size());
  for(size_t i = 0;i<m_normals.size();i++) m_normals[i] = FLOATVECTOR3();

  for (size_t i = 0;i<m_VertIndices.size();i+=3) {
    UINTVECTOR3 indices(m_VertIndices[i], m_VertIndices[i+1], m_VertIndices[i+2]);

    FLOATVECTOR3 tang = m_vertices[indices.x]-m_vertices[indices.y];
    FLOATVECTOR3 bin  = m_vertices[indices.x]-m_vertices[indices.z];

    FLOATVECTOR3 norm = bin % tang;
  	
    m_normals[indices.x] = m_normals[indices.x]+norm;
    m_normals[indices.y] = m_normals[indices.y]+norm;
    m_normals[indices.z] = m_normals[indices.z]+norm;
  }
  for(size_t i = 0;i<m_normals.size();i++) {
    float l = m_normals[i].length();
    if (l > 0) m_normals[i] = m_normals[i] / l;;
  }

  m_NormalIndices = m_VertIndices;
}

bool Mesh::Validate(bool bDeepValidation) {
  // make sure the sizes of the vectors match
  if (!m_NormalIndices.empty() && 
      m_NormalIndices.size() != m_VertIndices.size()) return false;
  if (!m_TCIndices.empty() && 
      m_TCIndices.size() != m_VertIndices.size()) return false;
  if (!m_COLIndices.empty() && 
      m_COLIndices.size() != m_VertIndices.size()) return false;

  if (!bDeepValidation) return true;

  // in deep validation mode check if all the indices are within range
  size_t count = m_vertices.size();
  for (IndexVec::iterator i = m_VertIndices.begin();
       i != m_VertIndices.end();
       i++) {
    if ((*i) >= count) return false;
  }
  count = m_normals.size();
  for (IndexVec::iterator i = m_NormalIndices.begin();
       i != m_NormalIndices.end();
       i++) {
    if ((*i) >= count) return false;
  }
  count = m_texcoords.size();
  for (IndexVec::iterator i = m_TCIndices.begin();
       i != m_TCIndices.end();
       i++) {
    if ((*i) >= count) return false;
  }
  count = m_colors.size();
  for (IndexVec::iterator i = m_COLIndices.begin();
       i != m_COLIndices.end();
       i++) {
    if ((*i) >= count) return false;
  }

  return true;
}

double Mesh::IntersectInternal(const Ray& ray, FLOATVECTOR3& normal,
                               FLOATVECTOR2& tc, FLOATVECTOR4& color, 
                               double tmin, double tmax) const {
  
  if (m_meshType != MT_TRIANGLES) return noIntersection;

  if (m_KDTree) {
    return m_KDTree->Intersect(ray, normal, tc, color, tmin, tmax);
  } else {
    double t = noIntersection;
    FLOATVECTOR3 _normal;
    FLOATVECTOR2 _tc;
    FLOATVECTOR4 _color;

    for (size_t i = 0;i<m_VertIndices.size();i+=3) {
      double currentT = IntersectTriangle(i, ray, _normal, _tc, _color);

      if (currentT < t) {
        normal = _normal;
        t = currentT;
        tc = _tc;
        color = _color;
      }

    }
    return t;
  }
}

double Mesh::IntersectTriangle(size_t i, const Ray& ray, 
                               FLOATVECTOR3& normal, 
                               FLOATVECTOR2& tc, FLOATVECTOR4& color) const {

  double t = std::numeric_limits<double>::max();

  FLOATVECTOR3 vert0 = m_vertices[m_VertIndices[i]];
  FLOATVECTOR3 vert1 = m_vertices[m_VertIndices[i+1]];
  FLOATVECTOR3 vert2 = m_vertices[m_VertIndices[i+2]];

  // find vectors for two edges sharing vert0
  DOUBLEVECTOR3 edge1 = DOUBLEVECTOR3(vert1 - vert0);
  DOUBLEVECTOR3 edge2 = DOUBLEVECTOR3(vert2 - vert0);
   
  // begin calculating determinant - also used to calculate U parameter
  DOUBLEVECTOR3 pvec = ray.direction % edge2;

  // if determinant is near zero, ray lies in plane of triangle
  double det = edge1 ^ pvec;

  if (det > -0.00000001 && det < 0.00000001) return t;
  double inv_det = 1.0 / det;

  // calculate distance from vert0 to ray origin
  DOUBLEVECTOR3 tvec = ray.start - DOUBLEVECTOR3(vert0);

  // calculate U parameter and test bounds
  double u = tvec ^pvec * inv_det;
  if (u < 0.0 || u > 1.0) return t;

  // prepare to test V parameter
  DOUBLEVECTOR3 qvec = tvec % edge1;

  // calculate V parameter and test bounds
  double v = (ray.direction ^ qvec) * inv_det;
  if (v < 0.0 || u + v > 1.0) return t;

  // calculate t, ray intersects triangle
  t = (edge2 ^ qvec) * inv_det;

  if (t<0) return std::numeric_limits<double>::max();

  // interpolate normal
  if (m_NormalIndices.size()) {
    FLOATVECTOR3 normal0 = m_normals[m_NormalIndices[i]];
    FLOATVECTOR3 normal1 = m_normals[m_NormalIndices[i+1]];
    FLOATVECTOR3 normal2 = m_normals[m_NormalIndices[i+1]];

    FLOATVECTOR3 du = normal1 - normal0;
    FLOATVECTOR3 dv = normal2 - normal0;
    
    normal = normal0 + du * float(u) + dv * float(v);
  } else {
    // compute face normal if no normals are given
    normal = FLOATVECTOR3(edge1 % edge2);
  }
  normal.normalize();

  if ((FLOATVECTOR3(ray.direction) ^ normal) > 0) normal = normal *-1; 

  // interpolate texture coordinates
  if (m_TCIndices.size()) {
    FLOATVECTOR2 tc0 = m_texcoords[m_TCIndices[i]];
    FLOATVECTOR2 tc1 = m_texcoords[m_TCIndices[i+1]];
    FLOATVECTOR2 tc2 = m_texcoords[m_TCIndices[i+2]];

    double dtu1 = tc1.x - tc0.x;
    double dtu2 = tc2.x - tc0.x;
    double dtv1 = tc1.y - tc0.y;
    double dtv2 = tc2.y - tc0.y;
    tc.x = float(tc0.x  + u * dtu1 + v * dtu2);
    tc.y = float(tc0.y + u * dtv1 + v * dtv2);
  } else {
    tc.x = 0;
    tc.y = 0;
  }

  // interpolate color
  if (m_COLIndices.size()) {
    FLOATVECTOR4 col0 = m_colors[m_TCIndices[i]];
    FLOATVECTOR4 col1 = m_colors[m_TCIndices[i+1]];
    FLOATVECTOR4 col2 = m_colors[m_TCIndices[i+2]];

    double dtu1 = col1.x - col0.x;
    double dtu2 = col2.x - col0.x;
    double dtv1 = col1.y - col0.y;
    double dtv2 = col2.y - col0.y;
    color.x = float(col0.x  + u * dtu1 + v * dtu2);
    color.y = float(col0.y + u * dtv1 + v * dtv2);
  } else {
    color.x = 0;
    color.y = 0;
  }

  return t;
}


void Mesh::ComputeKDTree() {
  delete m_KDTree;
  m_KDTree = new KDTree(this);
}

const KDTree* Mesh::GetKDTree() const {
  return m_KDTree;
}

bool Mesh::AABBIntersect(const Ray& r, double& tmin, double& tmax) {
  double tymin, tymax, tzmin, tzmax;

  DOUBLEVECTOR3 inv_direction(1.0/r.direction.x, 
                          1.0/r.direction.y, 
                          1.0/r.direction.z);

  int sign[3]  = {inv_direction.x < 0,
                  inv_direction.y < 0,
                  inv_direction.z < 0};

  tmin  = (m_Bounds[sign[0]].x - r.start.x)   * inv_direction.x;
  tmax  = (m_Bounds[1-sign[0]].x - r.start.x) * inv_direction.x;
  tymin = (m_Bounds[sign[1]].y - r.start.y)   * inv_direction.y;
  tymax = (m_Bounds[1-sign[1]].y - r.start.y) * inv_direction.y;

  if ( (tmin > tymax) || (tymin > tmax) )  return false;
  if (tymin > tmin) tmin = tymin;
  if (tymax < tmax) tmax = tymax;
  tzmin = (m_Bounds[sign[2]].z - r.start.z) * inv_direction.z;
  tzmax = (m_Bounds[1-sign[2]].z - r.start.z) * inv_direction.z;
  if ( (tmin > tzmax) || (tzmin > tmax) )  return false;
  if (tzmin > tmin)
    tmin = tzmin;
  if (tzmax < tmax)
    tmax = tzmax;
  return tmax > 0;
}
