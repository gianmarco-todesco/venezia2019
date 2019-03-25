#include "Gutil.h"
#include <assert.h>

const double pi = 3.14159265;

using namespace GmLib;

void drawCircle(const QPointF &center, double r, int m)
{
  glBegin(GL_LINE_STRIP);
  for(int i=0;i<m;i++)
  {
    double phi = pi * 2 * i/m;
    glVertex2d(center.x() + r * cos(phi), center.y() + r * sin(phi));
  }
  glVertex2d(center.x() + r, center.y());
  glEnd();
}

void fillCircle(const QPointF &center, double r, int m)
{
  glBegin(GL_POLYGON);
  for(int i=0;i<m;i++)
  {
    double phi = pi * 2 * i/m;
    glVertex2d(center.x() + r * cos(phi), center.y() + r * sin(phi));
  }
  glEnd();
}

void drawSphere(const GmLib::Point3 &center, double radius, int n, int m)
{
  int vertexCount = (m*n+2);
  assert(vertexCount<65536);
  GLfloat *vertices = new GLfloat[vertexCount*8];
  GLfloat *vertex = vertices;
  for(int i=0;i<n;i++)
  {
    double u = (double)(i+1)/(double)(n+1);
    double theta = pi * u;
    double csTheta = cos(theta);
    double snTheta = sin(theta);
    for(int j=0;j<m;j++)
    {
      double v = (double)(j-i*0.5)/(double)m;
      double phi = 2*pi*v;
      double csPhi = cos(phi);
      double snPhi = sin(phi);
      vertex[3] = csPhi * snTheta;
      vertex[4] = -csTheta;
      vertex[5] = snPhi * snTheta;
      vertex[0] = vertex[3] * radius;
      vertex[1] = vertex[4] * radius;
      vertex[2] = vertex[5] * radius;
      vertex[6] = u;
      vertex[7] = v;
      vertex += 8;
    }
  }
  vertex[0] = vertex[2] = 0; vertex[1] = -radius;
  vertex[3] = vertex[5] = 0; vertex[4] = -1;
  vertex[6] = 0; vertex[7] = 0;
  vertex += 8;
  vertex[0] = vertex[2] = 0; vertex[1] = radius;
  vertex[3] = vertex[5] = 0; vertex[4] = 1;
  vertex[6] = 1; vertex[7] = 0;
  vertex += 8;
  assert(vertices + vertexCount*8 == vertex);

  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glNormalPointer(GL_FLOAT, sizeof(GL_FLOAT)*8, vertices+3);
  glVertexPointer(3, GL_FLOAT, sizeof(GL_FLOAT)*8, vertices);
  glTexCoordPointer(2, GL_FLOAT, sizeof(GL_FLOAT)*8, vertices+6);

  int faceCount = 2*m*(n-1)+ 2*m;
  GLushort *indices = new GLushort[faceCount*3];
  GLushort *index = indices;
  for(int i=0;i+1<n;i++)
  {
    int i1 = i+1;
    for(int j=0;j<m;j++)
    {
      int j1 = (j+1)%m;
      *index++ = i*m+j;
      *index++ = i1*m+j1;
      *index++ = i*m+j1;

      *index++ = i*m+j;
      *index++ = i1*m+j;
      *index++ = i1*m+j1;
    }
  }

  for(int j=0;j<m;j++)
  {
    int j1 = (j+1)%m;
    *index++ = n*m;
    *index++ = j;
    *index++ = j1;
    *index++ = n*m+1;
    *index++ = (n-1)*m+j1;
    *index++ = (n-1)*m+j;
  }
  assert(index == indices + faceCount * 3);
  glPushMatrix();
  glTranslated(center.x(),center.y(),center.z());
  glDrawElements(GL_TRIANGLES, 3*faceCount, GL_UNSIGNED_SHORT, indices);
  glPopMatrix();
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  delete [] indices;
  delete [] vertices;
}

void drawCone(const GmLib::Point3 &c0, double r0, const GmLib::Point3 &c1, double r1, int n, int m)
{
  Point3 c01 = c1-c0;
  double height = c01.getNorm();
  Point3 u1 = c01 * (1.0/height);
  Point3 u0;
  double fx = fabs(u1.x()), fy = fabs(u1.y()), fz = fabs(u1.z());
  if(fx<fy) u0 = fx<fz ? Point3(1,0,0) : Point3(0,0,1);
  else u0 = fy<fz ? Point3(0,1,0) : Point3(0,0,1);
  u0 = cross(u0,u1).normalized();
  Point3 u2 = cross(u0,u1);
  double ny = r0-r1, nr = height;
  double tmp = 1.0/sqrt(ny*ny+nr*nr); ny*=tmp; nr*=tmp;

  int vertexCount = (m*n);
  assert(vertexCount<65536);
  GLfloat *vertices = new GLfloat[vertexCount*8];
  GLfloat *vertex = vertices;
  for(int i=0;i<n;i++)
  {
    double u = (double)(i+1)/(double)(n+1);
    double phi = 2 * pi * u;
    Point3 ur = u0*cos(phi) + u2*sin(phi);
    Point3 nrm = nr*ur+ny*u1;

    for(int j=0;j<m;j++)
    {
      double v = (double)j/(double)(m-1);
      Point3 p = c0 + c01*v + ((1-v)*r0+v*r1)*ur;

      vertex[0] = p.x();
      vertex[1] = p.y();
      vertex[2] = p.z();
      vertex[3] = nrm.x();
      vertex[4] = nrm.y();
      vertex[5] = nrm.z();
      vertex[6] = u;
      vertex[7] = v;
      vertex += 8;
    }
  }

  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glNormalPointer(GL_FLOAT, sizeof(GL_FLOAT)*8, vertices+3);
  glVertexPointer(3, GL_FLOAT, sizeof(GL_FLOAT)*8, vertices);
  glTexCoordPointer(2, GL_FLOAT, sizeof(GL_FLOAT)*8, vertices+6);

  int faceCount = 2*n*(m-1);
  GLushort *indices = new GLushort[faceCount*3];
  GLushort *index = indices;
  for(int i=0;i<n;i++)
  {
    int i1 = (i+1)%n;
    for(int j=0;j+1<m;j++)
    {
      int j1 = j+1;
      *index++ = i*m+j;
      *index++ = i*m+j1;
      *index++ = i1*m+j1;

      *index++ = i*m+j;
      *index++ = i1*m+j1;
      *index++ = i1*m+j;
    }
  }

  glDrawElements(GL_TRIANGLES, 3*faceCount, GL_UNSIGNED_SHORT, indices);
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  delete [] indices;
  delete [] vertices;
}

void drawCylinder(const GmLib::Point3 &c0, const GmLib::Point3 &c1, double radius, int n, int m)
{
  drawCone(c0,radius,c1,radius,n,m);
}

void drawTriangle(const GmLib::Point3 &p0, const GmLib::Point3 &p1, const GmLib::Point3 &p2)
{
  Point3 nrm = cross(p1-p0,p2-p0).normalized();
  glBegin(GL_TRIANGLES);
  glNormal(nrm);
  glTexCoord2d(0,0);
  glVertex(p0);
  glTexCoord2d(1,0);
  glVertex(p1);
  glTexCoord2d(1,1);
  glVertex(p2);
  glEnd();
}

void setColor(double r, double g, double b, double m)
{
  GLfloat c[4] = {(GLfloat)r,(GLfloat)g,(GLfloat)b,(GLfloat)m};
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, c);
}

