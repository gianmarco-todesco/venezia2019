#ifndef MESH_INCLUDED
#define MESH_INCLUDED

#include <qglbuffer.h>

class Mesh {
public:
    QVector<GLfloat> m_vertexData;
    QVector<GLushort> m_indexData;
    int m_vCount;
    QGLBuffer m_vertexBuffer, m_indexBuffer;
    
    Mesh();
    ~Mesh();

    
    void createBuffers();
    void bind();
    void draw();
    void release();

    GLushort addVertex(const QVector3D &pos, const QVector3D &normal);
    void addTriangle(int a, int b, int c);
    void addQuad(int a, int b, int c, int d);

    void addFace(const QVector3D &p, const QVector3D &du, const QVector3D &dv, int n, int m);

    void makeSphere(double r, int n, int m);
    void makeCube(double r, int n);
    void makeBox(double rx, double ry, double rz, int nx, int ny, int nz);


};

#endif

