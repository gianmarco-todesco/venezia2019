#include "Mesh.h"
#include <qvector3d.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>
#include <QMatrix4x4>

Mesh::Mesh()
    : m_vertexBuffer( QGLBuffer::VertexBuffer )
    , m_indexBuffer( QGLBuffer::IndexBuffer )
    , m_vCount(0)
    , m_hasTexCoords(false)
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
    int m = m_hasTexCoords ? 8 : 6;
    m_vertexBuffer.bind();
    glVertexPointer(3, GL_FLOAT, sizeof(GL_FLOAT)*m, 0);
    glNormalPointer(GL_FLOAT, sizeof(GL_FLOAT)*m, (GLubyte*) NULL + sizeof(GL_FLOAT)*3);
    if(m_hasTexCoords)
        glTexCoordPointer(2, GL_FLOAT, sizeof(GL_FLOAT)*m, (GLubyte*) NULL + sizeof(GL_FLOAT)*6);
    m_indexBuffer.bind();
}

void Mesh::draw() const
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
    assert(!m_hasTexCoords);
    m_vertexData 
        << pos.x() << pos.y() << pos.z() 
        << normal.x() << normal.y() << normal.z();
    return m_vCount++;
}

GLushort Mesh::addVertex(const QVector3D &pos, const QVector3D &normal, const QPointF &uv)
{
    assert(m_hasTexCoords);
    m_vertexData 
        << pos.x() << pos.y() << pos.z() 
        << normal.x() << normal.y() << normal.z()
        << uv.x() << uv.y();
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

void Mesh::hMerge(const Mesh &other, const QMatrix4x4 &hMatrix)
{
    assert(m_hasTexCoords == other.m_hasTexCoords);
    int oldVCount = m_vCount;
    int vertexSize = other.m_hasTexCoords ? 8 : 6;
    const QVector<GLfloat> &vv = other.m_vertexData;
    int k = 0;
    for(int i=0; i<other.m_vCount; i++)
    {
        QVector3D pos(vv[k],vv[k+1],vv[k+2]);
        QVector3D normal(vv[k+3],vv[k+4],vv[k+5]);
        QPointF uv;
        if(m_hasTexCoords) uv = QPointF(vv[k+6], vv[k+7]);
        k += vertexSize;
        QVector3D pos2 = hMatrix.map(pos);
        const double epsilon = 1.0e-5;
        QVector3D normal2 = (hMatrix.map(pos + normal*epsilon) - pos2).normalized();
        m_vertexData 
            << pos2.x() << pos2.y() << pos2.z() 
            << normal2.x() << normal2.y() << normal2.z();
        if(m_hasTexCoords) 
            m_vertexData << uv.x() << uv.y();
        m_vCount++;
    }
    foreach(GLushort q, other.m_indexData) m_indexData.append(q + oldVCount);
}


void Mesh::makeSphere(double r, int n, int m)
{
    for(int i=0; i<n; i++)
    {
        double u = (double)(i+1)/(n+1);
        double theta = M_PI*u;
        double csTheta = cos(theta), snTheta = sin(theta);
        for(int j=0;j<m;j++)
        {
            double v = (double)j/m;
            double phi = 2*M_PI*v;
            double csPhi = cos(phi), snPhi = sin(phi);
            QVector3D norm(csPhi*snTheta, csTheta, snPhi*snTheta);
            if(m_hasTexCoords) addVertex(norm*r, norm, QPointF(u,v));
            else addVertex(norm*r, norm);
        }
    }
    int k = m_vCount;

    if(m_hasTexCoords)
    {
        addVertex(QVector3D(0,r,0), QVector3D(0,1,0), QPointF(0,0));
        addVertex(QVector3D(0,-r,0), QVector3D(0,-1,0), QPointF(1,0));
    }
    else
    {
        addVertex(QVector3D(0,r,0), QVector3D(0,1,0));
        addVertex(QVector3D(0,-r,0), QVector3D(0,-1,0));
    }

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

void Mesh::addFace(const QVector3D &p, const QVector3D &du, const QVector3D &dv, int n, int m, const QRectF &uvRect)
{
    int k0 = m_vCount;
    QVector3D norm = QVector3D::crossProduct(du,dv).normalized();
    for(int i=0;i<n;i++)
    {
        for(int j=0;j<m;j++)
        {
            if(m_hasTexCoords)
            {
                double u = uvRect.left() + uvRect.width() * (double)i/(n-1);
                double v = uvRect.top() + uvRect.height() * (double)j/(m-1);

                addVertex(p+du*i+dv*j,norm, QPointF(u,v));
            }
            else
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
            // addTriangle(k1,k1+m,k1+1+m); // per debug: triangoli invece di quadrati
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
    addBox(QVector3D(0,0,0), rx,ry,rz,nx,ny,nz);    
    createBuffers();
}

void Mesh::addBox(const QVector3D &center, double rx, double ry, double rz, int nx, int ny, int nz)
{
   addFace(
        center + QVector3D(-rx,-ry,rz),
        QVector3D(2*rx/(nx-1),0,0),
        QVector3D(0,2*ry/(ny-1),0),
        nx,ny);
    addFace(
        center + QVector3D(rx,-ry,-rz),
        QVector3D(-2*rx/(nx-1),0,0),
        QVector3D(0,2*ry/(ny-1),0),
        nx,ny);
    addFace(
        center + QVector3D(-rx,-ry,-rz),
        QVector3D(0,0,2*rz/(nz-1)),
        QVector3D(0,2*ry/(ny-1),0),
        nz,ny);
    addFace(
        center + QVector3D(rx,-ry,rz),
        QVector3D(0,0,-2*rz/(nz-1)),
        QVector3D(0,2*ry/(ny-1),0),
        nz,ny);
    addFace(
        center + QVector3D(-rx,-ry,-rz),
        QVector3D(2*rx/(nx-1),0,0),
        QVector3D(0,0,2*rz/(nz-1)),
        nx,nz);
    addFace(
        center + QVector3D(rx,ry,-rz),
        QVector3D(-2*rx/(nx-1),0,0),
        QVector3D(0,0,2*rz/(nz-1)),
        nx,nz);
}



/*
void Mesh::makeCube(double r, int n, int m)
{
}
*/



void Mesh::makePrism(double r, double h, int n)
{
    QVector<QVector3D> pts;
    for(int i=0;i<n;i++) {
        double phi = 2*M_PI*i/n;
        double cs = cos(phi), sn = sin(phi);
        pts.append(QVector3D(r*cs,r*sn,-h));
        pts.append(QVector3D(r*cs,r*sn, h));
    }
    QVector3D norm;
    int k = m_vCount;
    // up face
    addVertex(QVector3D(0,0,h), QVector3D(0,0,1));
    for(int i=0;i<n;i++) addVertex(pts[2*i+1], QVector3D(0,0,1));
    for(int i=0;i+1<n;i++) addTriangle(k,k+1+i,k+2+i);
    addTriangle(k,k+n,k+1);
    // dn face
    k = m_vCount;
    addVertex(QVector3D(0,0,-h), QVector3D(0,0,-1));
    for(int i=0;i<n;i++) addVertex(pts[2*i], QVector3D(0,0,-1));
    for(int i=0;i+1<n;i++) addTriangle(k,k+2+i,k+1+i);
    addTriangle(k,k+1,k+n);
    // side faces
    for(int i=0;i<n;i++)
    {
        int i1 = (i+1)%n;
        k = m_vCount;
        QVector3D norm = QVector3D::crossProduct(pts[2*i1]-pts[2*i], QVector3D(0,0,1)).normalized();
        addVertex(pts[2*i], norm);
        addVertex(pts[2*i1], norm);
        addVertex(pts[2*i1+1], norm);
        addVertex(pts[2*i+1], norm);
        addQuad(k,k+1,k+2,k+3);
    }

    createBuffers();
}


void Mesh::addTriangleGrid(int n, int m, int firstIndex)
{
    for(int i=0; i+1<n; i++)
    {
        for(int j=0;j+1<m;j++)
        {
            int k = firstIndex + i*m + j;
            addQuad(k, k+1, k+1+m, k+m);
        }
    }
}



void Mesh::makePolydronPiece(double r, double h, int n)
{
    m_hasTexCoords = true;
    double d = 0.5;
    double r1 = r - d/cos(M_PI/n);
    QVector<QPointF> cssn;
    for(int i=0;i<=n;i++) {
        double phi = 2*M_PI*i/n;
        double cs = cos(phi), sn = sin(phi);
        cssn.append(QPointF(cs,sn));
    }
    QVector3D norm;
    int k;
    // up face
    norm = QVector3D(0,0,1);
    k = m_vCount;
    for(int i=0;i<=n;i++) 
    {
        double v = (double)i/n;
        addVertex(QVector3D(cssn[i].x()*r1, cssn[i].y()*r1, h), norm, QPointF(v,0));
        addVertex(QVector3D(cssn[i].x()*r, cssn[i].y()*r, h), norm, QPointF(v,0.25));
        if(i<n)
            addQuad(k + i*2, k + i*2 + 1, k + i*2 + 3, k + i*2 + 2);
    }

    // dn page
    norm = QVector3D(0,0,-1);
    k = m_vCount;
    for(int i=0;i<=n;i++) 
    {
        double v = (double)i/n;
        addVertex(QVector3D(cssn[i].x()*r, cssn[i].y()*r, -h), norm, QPointF(v,0.25));
        addVertex(QVector3D(cssn[i].x()*r1, cssn[i].y()*r1, -h), norm, QPointF(v,0.5));
        
        if(i<n)
           addQuad(k + i*2, k + i*2 + 1, k + i*2 + 3, k + i*2 + 2);
    }

    // side faces
    for(int i=0;i<n;i++)
    {
        int i1 = (i+1)%n;
        k = m_vCount;
        QPointF q = (cssn[i] + cssn[i1])*0.5;
        QVector3D norm = QVector3D(q.x(),q.y(),0.0).normalized();
        addVertex(QVector3D(cssn[i].x()*r, cssn[i].y()*r, -h), norm, QPointF(0,0.5));
        addVertex(QVector3D(cssn[i1].x()*r, cssn[i1].y()*r, -h), norm, QPointF(1,0.5));
        addVertex(QVector3D(cssn[i1].x()*r, cssn[i1].y()*r, h), norm, QPointF(1,0.75));
        addVertex(QVector3D(cssn[i].x()*r, cssn[i].y()*r, h), norm, QPointF(0,0.75));        
        addQuad(k,k+1,k+2,k+3);
    }
    for(int i=0;i<n;i++)
    {
        int i1 = (i+1)%n;
        k = m_vCount;
        QPointF q = (cssn[i] + cssn[i1])*0.5;
        QVector3D norm = QVector3D(q.x(),q.y(),0.0).normalized();
        addVertex(QVector3D(cssn[i].x()*r1, cssn[i].y()*r1, h), norm, QPointF(0,0.5));
        addVertex(QVector3D(cssn[i1].x()*r1, cssn[i1].y()*r1, h), norm, QPointF(1,0.5));
        addVertex(QVector3D(cssn[i1].x()*r1, cssn[i1].y()*r1, -h), norm, QPointF(1,0.75));
        addVertex(QVector3D(cssn[i].x()*r1, cssn[i].y()*r1, -h), norm, QPointF(0,0.75));        
        addQuad(k,k+1,k+2,k+3);
    }

    createBuffers();
}
