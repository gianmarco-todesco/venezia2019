#include "Fig1Page.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>

#include <assert.h>
#include <QKeyEvent>
#include <QWheelEvent>

#include "Gutil.h"
#include "Point3.h"
#include "Polyhedra.h"
#include "H3.h"
#include "H3Grid.h"

#include <QGLShaderProgram>
#include <qmatrix4x4.h>
#include <QVector4D.h>
#include <qvector.h>
#include <qlist.h>
#include <QGLBuffer>
#include <QTIme>


#include "Viewer.h"


// fig.1 A computer rendering inspired by the Escher's lithography "Cubic Space Division (Cubic Space Filling)"

Fig1Page::Fig1Page()
    : m_theta(23.5)
    , m_phi(-25.75)
    , m_cameraDistance(24.2)
    , m_rotating(false)
    , m_gridSmallUnit(0)
    , m_gridBigUnit(1)
    , m_textureId(0)

{
    m_hMatrix.setToIdentity();
}
    
Fig1Page::~Fig1Page()
{
}


void Fig1Page::buildMesh()
{
    QTime clock;
    clock.start();
    const double r1 = 1.0;
    const double r2 = r1/3.0;
    const double d = 10;

    m_mesh.m_hasTexCoords = true;

    int n = 1;
    
    for(int ix=-n;ix<=n;ix++)
    for(int iy=-n;iy<=n;iy++)
    for(int iz=-n;iz<=n;iz++)
        makeVertexBox(m_mesh, QVector3D(ix,iy,iz)*d, r1);
      

    double q = n*d;


    for(int ix=-n;ix<=n;ix++)
    for(int iy=-n;iy<=n;iy++)
    for(int iz=-n;iz<=n;iz++)
    {
        makeEdgeBox(m_mesh, QVector3D(ix,iy,iz)*d, QVector3D(ix+1,iy,iz)*d, QVector3D(0,1,0), r2);
        makeEdgeBox(m_mesh, QVector3D(ix,iy,iz)*d, QVector3D(ix,iy+1,iz)*d, QVector3D(0,0,1), r2);
        makeEdgeBox(m_mesh, QVector3D(ix,iy,iz)*d, QVector3D(ix,iy,iz+1)*d, QVector3D(0,1,0), r2);
    }

    m_mesh.createBuffers();
    m_gridSmallUnit = d;
    m_gridBigUnit = d*2*n;

    qDebug() << "cube grid created: " << clock.elapsed();

    m_offset = QVector3D(d*0.5,d*0.5,d*0.5);
}


void Fig1Page::makeVertexBox(Mesh &mesh, const QVector3D &center, double r)
{
    int n = 3;
    double rx=r, ry=r, rz=r;
    QRectF uvRect(0.1,0.1,0.3,0.3);
    mesh.addFace(
        center + QVector3D(-rx,-ry,rz),
        QVector3D(2*rx/(n-1),0,0),
        QVector3D(0,2*ry/(n-1),0),
        n,n,
        uvRect);
    mesh.addFace(
        center + QVector3D(rx,-ry,-rz),
        QVector3D(-2*rx/(n-1),0,0),
        QVector3D(0,2*ry/(n-1),0),
        n,n,
        uvRect);
    mesh.addFace(
        center + QVector3D(-rx,-ry,-rz),
        QVector3D(0,0,2*rz/(n-1)),
        QVector3D(0,2*ry/(n-1),0),
        n,n,
        uvRect);
    mesh.addFace(
        center + QVector3D(rx,-ry,rz),
        QVector3D(0,0,-2*rz/(n-1)),
        QVector3D(0,2*ry/(n-1),0),
        n,n,
        uvRect);
    mesh.addFace(
        center + QVector3D(-rx,-ry,-rz),
        QVector3D(2*rx/(n-1),0,0),
        QVector3D(0,0,2*rz/(n-1)),
        n,n,
        uvRect);
    mesh.addFace(
        center + QVector3D(rx,ry,-rz),
        QVector3D(-2*rx/(n-1),0,0),
        QVector3D(0,0,2*rz/(n-1)),
        n,n,
        uvRect);
}

void Fig1Page::makeEdgeBox(Mesh &mesh, const QVector3D &p0, const QVector3D &p1, const QVector3D &up, double r)
{
    double h = (p1-p0).length();
    QVector3D e0 = (p1-p0) * (1.0/h);
    QVector3D e1 = (up - e0 * QVector3D::dotProduct(e0, up)).normalized();
    QVector3D e2 = QVector3D::crossProduct(e0,e1).normalized();
    QVector3D center = (p0+p1)*0.5;

    int n = 3;
    QRectF uvRect(0.6,0.1,0.3,0.3);
    mesh.addFace(
        center - e0 * (h*0.5) - (e1 + e2) * r,
        e0 * (h/(n-1)),
        e2 * (2*r/(n-1)),
        n,n,
        uvRect);
    mesh.addFace(
        center - e0 * (h*0.5) + (e1 + e2) * r,
        e0 * (h/(n-1)),
        -e2 * (2*r/(n-1)),
        n,n,
        uvRect);
    mesh.addFace(
        center - e0 * (h*0.5) + (-e1 + e2) * r,
        e0 * (h/(n-1)),
        e1 * (2*r/(n-1)),
        n,n,
        uvRect);
    mesh.addFace(
        center - e0 * (h*0.5) - (-e1 + e2) * r,
        e0 * (h/(n-1)),
        -e1 * (2*r/(n-1)),
        n,n,
        uvRect);
}

  
void Fig1Page::initializeGL() 
{
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHT0);
    // glEnable(GL_LIGHT1);
    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE,1.0);
    glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER,1.0);

    GLfloat lcolor[] =  { 0.7f, 0.7f, 0.7f, 1.0f};
    GLfloat lpos[]   = { 5, 7, 10, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, lcolor);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lcolor);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lcolor);
    glLightfv(GL_LIGHT0, GL_POSITION, lpos);

    lpos[0] = -5;
    glLightfv(GL_LIGHT1, GL_AMBIENT, lcolor);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lcolor);
    glLightfv(GL_LIGHT1, GL_SPECULAR, lcolor);
    glLightfv(GL_LIGHT1, GL_POSITION, lpos);

    // m_shaderProgram = loadProgram("h3grid");
    buildMesh();
    m_shaderProgram = loadProgram("cubeGrid");

    createTextures();
}

void Fig1Page::resizeGL(int width, int height) 
{
    double aspect = (float)width/height;
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30, aspect, 1.0, 500.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Fig1Page::paintGL() 
{        
    glClearColor(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glPushMatrix();
    glTranslated(0,0,-m_cameraDistance);
    glRotated(m_theta,1,0,0);
    glRotated(m_phi,0,1,0);


    GLfloat specular[] =  { 0.7f, 0.7f, 0.7f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 90.0);

    setColor(1,1,1);
    draw();
    glPopMatrix();


}


void Fig1Page::draw()
{
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);


    m_shaderProgram->bind();
    setViewUniforms(m_shaderProgram);

    
    m_mesh.bind();

    QMatrix4x4 mat;
    mat.setToIdentity();
    mat.rotate(-m_phi,0,1,0);
    mat.rotate(-m_theta,1,0,0);

    QVector3D dir = mat.map(QVector3D(0,0,-1));

    int m = 0;
    
    for(int ix = -5; ix <= m; ix++) 
    for(int iy = -5; iy <= m; iy++) 
    for(int iz = -10; iz <= m; iz++) 
    {
        QVector3D v = m_offset + m_gridBigUnit * QVector3D(ix,iy,iz);
        //if(QVector3D::dotProduct(v,dir)>0) 
        {
            m_shaderProgram->setUniformValue("u_offset", v);
            m_mesh.draw();
        }
    }




    m_mesh.release();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    m_shaderProgram->release();
    
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  

}






void Fig1Page::mousePressEvent(QMouseEvent *e)
{
    m_lastPos = e->pos();
    m_rotating = e->button() == Qt::RightButton;
}

void Fig1Page::mouseReleaseEvent(QMouseEvent *e)
{

}

void Fig1Page::mouseMoveEvent(QMouseEvent *e)
{
  QPoint delta = m_lastPos - e->pos();
  m_lastPos = e->pos();
  if(!m_rotating)
  {

      // m_hOffset += QVector3D(0,0,delta.y()*0.01);
      // m_hMatrix = H3::KModel::translation(m_hOffset, QVector3D(0,0,0));
  }
  else
  {
    m_phi -= 0.25*delta.x();
    m_theta -= 0.25*delta.y();
  }
  updateGL();
}


void Fig1Page::wheelEvent(QWheelEvent*e)
{
  m_cameraDistance = clamp(m_cameraDistance - e->delta() * 0.01, 1,150);
  updateGL();
}


void Fig1Page::keyPressEvent(QKeyEvent *e)
{
    double d =  0.1 * ((e->modifiers() & Qt::ShiftModifier) == 0 ? 1 : -1);
    if(e->key() == Qt::Key_X)
    {
        m_offset.setX(m_offset.x() + d); 
    }
    else if(e->key() == Qt::Key_Y)
    {
        m_offset.setY(m_offset.y() + d); 
    }
    if(e->key() == Qt::Key_Z)
    {
        m_offset.setZ(m_offset.z() + d); 
    }
    e->ignore();
}


void Fig1Page::savePictures()
{
    Fig1Page page;
    page.m_cameraDistance = m_cameraDistance;
    page.m_theta = m_theta;
    page.m_phi = m_phi;
    page.savePicture("fig1.png", 10);
}


void Fig1Page::createTextures()
{
    QImage img(1024, 1024, QImage::Format_ARGB32);
    img.fill(QColor(120,120,120));
    QPainter pa;
    pa.begin(&img);

    int x0,y0,x1,y1;

    QColor color(200,200,200);

    // vertex
    x0 = (int)(0.5+0.1*img.width());
    x1 = (int)(0.5+0.4*img.width());
    y0 = (int)(0.5+0.1*img.height());
    y1 = (int)(0.5+0.4*img.height());
    
    QRectF rect(x0,y0,x1-x0+1,y1-y0+1);

    pa.fillRect(rect.adjusted(-5,-5,5,5), Qt::black);
    int d = 4;
    pa.fillRect(rect.adjusted(d,d,-d,-d), color);
    int q = (x1-x0)/3;
    int d1 = 2;
    pa.fillRect(rect.adjusted(q-d1,q-d1,-q+d1,-q+d1), Qt::black);

    // edge
    x0 = (int)(0.5+0.6*img.width());
    x1 = (int)(0.5+0.9*img.width());
    y0 = (int)(0.5+0.1*img.height());
    y1 = (int)(0.5+0.4*img.height());
    
    rect = QRectF(x0,y0,x1-x0+1,y1-y0+1);
    d = 10;
    pa.fillRect(rect.adjusted(-5,-5,5,5), Qt::black);
    pa.fillRect(rect.adjusted(d,d,-d,-d), color);


    // img.save("bu.png");
   

    glGenTextures(1, &m_textureId); 
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    gluBuild2DMipmaps( GL_TEXTURE_2D, 4, img.width(), img.height(),
                   GL_BGRA_EXT, GL_UNSIGNED_BYTE, img.bits() );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D, 0);
}
