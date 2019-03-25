#include "Mesh.h"
#include <qvector3d.h>
#define _USE_MATH_DEFINES
#include <math.h>
Mesh::Mesh()
    : m_vertexBuffer( QGLBuffer::VertexBuffer )
    , m_indexBuffer( QGLBuffer::IndexBuffer )
    , m_vCount(0)
{
}

Mesh::~Mesh()
{
}

void Mesh::createBuffers()
{
    m_indexBuffer.create();
    m_indexBuffer.bind();
    m_indexBuffer.allocate(m_indexData.count() * sizeof(GLushort));
    m_indexBuffer.setUsagePattern(QGLBuffer::StaticDraw);
    m_indexBuffer.write(0, m_indexData.constData(), m_indexData.count() * sizeof(GLushort));
    m_indexBuffer.release();

    m_vertexBuffer.create();
    m_vertexBuffer.bind();
    m_vertexBuffer.allocate(m_vertexData.count() * sizeof(GLfloat));
    m_vertexBuffer.setUsagePattern(QGLBuffer::StaticDraw);

    m_vertexBuffer.write(0, m_vertexData.constData(), m_vertexData.count() * sizeof(GLfloat));
    m_vertexBuffer.release();  
}

void Mesh::bind()
{
    m_vertexBuffer.bind();
    glVertexPointer(3, GL_FLOAT, sizeof(GL_FLOAT)*6, 0);
    glNormalPointer(GL_FLOAT, sizeof(GL_FLOAT)*6, (GLubyte*) NULL + sizeof(GL_FLOAT)*3);
    m_indexBuffer.bind();
}

void Mesh::draw()
{
    glDrawElements(GL_TRIANGLES, m_indexData.count(), GL_UNSIGNED_SHORT, 0);    
}

void Mesh::release()
{
    m_vertexBuffer.release();
    m_indexBuffer.release();
}




GLushort Mesh::addVertex(const QVector3D &pos, const QVector3D &normal)
{
    m_vertexData 
        << pos.x() << pos.y() << pos.z() 
        << normal.x() << normal.y() << normal.z();
    return m_vCount++;
}

void Mesh::addTriangle(int a, int b, int c)
{
    m_indexData << a << b << c;
}

void Mesh::addQuad(int a, int b, int c, int d)
{
    m_indexData << a << b << c;
    m_indexData << a << c << d;

}



void Mesh::makeSphere(double r, int n, int m)
{
    for(int i=0; i<n; i++)
    {
        double theta = M_PI*(double)(i+1)/(n+1);
        double csTheta = cos(theta), snTheta = sin(theta);
        for(int j=0;j<m;j++)
        {
            double phi = 2*M_PI*j/m;
            double csPhi = cos(phi), snPhi = sin(phi);
            QVector3D norm(csPhi*snTheta, csTheta, snPhi*snTheta);
            addVertex(norm*r, norm);
        }
    }
    int k = m_vCount;

    addVertex(QVector3D(0,r,0), QVector3D(0,1,0));
    addVertex(QVector3D(0,-r,0), QVector3D(0,-1,0));

    for(int j=0;j<m;j++)
    {
        int j1= (j+1)%m;
        addTriangle(k,j1,j);
        addTriangle(k+1,k-1-j1,k-1-j);
        for(int i=0;i+1<n;i++)
        {
           addTriangle(i*m+j,i*m+j1,(i+1)*m+j);
           addTriangle((i+1)*m+j,i*m+j1,(i+1)*m+j1);
        }
    }
    createBuffers();
}

void Mesh::addFace(const QVector3D &p, const QVector3D &du, const QVector3D &dv, int n, int m)
{
    int k0 = m_vCount;
    QVector3D norm = QVector3D::crossProduct(du,dv).normalized();
    for(int i=0;i<n;i++)
    {
        for(int j=0;j<m;j++)
        {
            addVertex(p+du*i+dv*j,norm);
        }
    }
    for(int i=0;i+1<n;i++)
    {
        int k1 = k0 + i*m;
        for(int j=0;j+1<m;j++)
        {
            // if((i^j)&1) // per debug: uno si e uno no, a scacchiera
            addQuad(k1,k1+m,k1+1+m,k1+1);
            k1++;
        }
    }
}

void Mesh::makeCube(double r, int n)
{
    makeBox(r,r,r,n,n,n);
}

void Mesh::makeBox(double rx, double ry, double rz, int nx, int ny, int nz)
{
    addFace(
        QVector3D(-rx,-ry,rz),
        QVector3D(2*rx/(nx-1),0,0),
        QVector3D(0,2*ry/(ny-1),0),
        nx,ny);
    addFace(
        QVector3D(rx,-ry,-rz),
        QVector3D(-2*rx/(nx-1),0,0),
        QVector3D(0,2*ry/(ny-1),0),
        nx,ny);
    addFace(
        QVector3D(-rx,-ry,-rz),
        QVector3D(0,0,2*rz/(nz-1)),
        QVector3D(0,2*ry/(ny-1),0),
        nz,ny);
    addFace(
        QVector3D(rx,-ry,rz),
        QVector3D(0,0,-2*rz/(nz-1)),
        QVector3D(0,2*ry/(ny-1),0),
        nz,ny);
    addFace(
        QVector3D(-rx,-ry,-rz),
        QVector3D(2*rx/(nx-1),0,0),
        QVector3D(0,0,2*rz/(nz-1)),
        nx,nz);
    addFace(
        QVector3D(rx,ry,-rz),
        QVector3D(-2*rx/(nx-1),0,0),
        QVector3D(0,0,2*rz/(nz-1)),
        nx,nz);

    
    createBuffers();
}

/*
void Mesh::makeCube(double r, int n, int m)
{
}
*/

