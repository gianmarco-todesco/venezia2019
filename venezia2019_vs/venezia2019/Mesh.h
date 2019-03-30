#ifndef MESH_INCLUDED
#define MESH_INCLUDED

#include <qglbuffer.h>

class Mesh {
public:
    QVector<GLfloat> m_vertexData;
    QVector<GLushort> m_indexData;
    int m_vCount;
    QGLBuffer m_vertexBuffer, m_indexBuffer;
    bool m_hasTexCoords;

    Mesh();
    ~Mesh();

    
    void createBuffers();
    void bind();
    void draw() const;
    void release();

    GLushort addVertex(const QVector3D &pos, const QVector3D &normal);
    GLushort addVertex(const QVector3D &pos, const QVector3D &normal, const QPointF &uv);
    void addTriangle(int a, int b, int c);
    void addQuad(int a, int b, int c, int d);
    void addTriangleGrid(int n, int m, int firstIndex = 0);

    void addFace(const QVector3D &p, const QVector3D &du, const QVector3D &dv, int n, int m);

    void addBox(const QVector3D &center, double rx, double ry, double rz, int nx=2, int ny=2, int nz=2);
    void addCube(const QVector3D &center, double r, int n=2) { addBox(center, r,r,r, n,n,n); }


    void makeSphere(double r, int n, int m);
    void makeCube(double r, int n);
    void makeBox(double rx, double ry, double rz, int nx, int ny, int nz);

    void makePrism(double r, double h, int n);
    void makePolydronPiece(double r, double h, int n);

};

#endif

