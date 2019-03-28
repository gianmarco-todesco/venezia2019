#include "HyperbolicPolyhedronPage.h"

#include "Polyhedra.h"
#include "Gutil.h"
#include "Point3.h"

#include "Mesh.h"
#include "H3.h"

#include <assert.h>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QPainter>

#include <vector>

#include <QGLShaderProgram>
#include <qmatrix4x4.h>
#include <qvector.h>
#include <qlist.h>
#include <qmap.h>



HyperbolicPolyhedronPage::HyperbolicPolyhedronPage()
: m_cameraDistance(15)
, m_theta(0)
, m_phi(0)
, m_rotating(true)
, m_parameter(0)
{
}

HyperbolicPolyhedronPage::~HyperbolicPolyhedronPage()
{
}

void HyperbolicPolyhedronPage::initializeGL()
{
    // m_mesh.makeSphere(1,20,20);
    double d = 0.7;
    int m = 40;
    double q = d / (m-1);
    m_mesh.m_hasTexCoords = true;
    m_mesh.addFace(QVector3D(-d,-d, 0), QVector3D(2,0,0)*q, QVector3D(0.0,2,0)*q, m,m);
    m_mesh.createBuffers();

    m_outSphereMesh.makeSphere(5.0, 50,50);

    createHLineMesh();


    m_h3ShaderProgram = loadProgram("h3basic");
    m_stdShaderProgram = loadProgram("basic");


    QImage img(256,256, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);
    QPainter pa;
    pa.begin(&img);
    pa.setFont(QFont("Arial",80,QFont::Bold));
    pa.setPen(Qt::black);
    pa.drawRect(0,0,255,255);
    pa.drawText(QRect(0,0,256,256), Qt::AlignCenter, "Hello!");
    pa.end();   
    m_texture1.createTexture(img);
}

void HyperbolicPolyhedronPage::start()
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

  
    m_clock.start();

}

void HyperbolicPolyhedronPage::resizeGL(int width, int height)
{
    double aspect = (float)width/height;
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, aspect, 1.0, 70.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void HyperbolicPolyhedronPage::createHLineMesh()
{
    m_hlineMesh.m_hasTexCoords = true;
    int m = 5, n = 100;
    double r = 0.1;
    QList<QVector4D> pts;
    QList<QVector3D> nrms;
    for(int i=0; i<m; i++)
    {
        double phi = 2*M_PI*i/(m-1);
        double cs = cos(phi), sn = sin(phi);
        nrms.append(QVector3D(cs, 0, sn));
        pts.append(QVector4D(cs*r, 0, sn*r, 1.0));
    }
    for(int i= 0; i<n; i++)
    {
        double y = 0.99999 * (-1.0+2.0*i/(n-1));
        QMatrix4x4 mat = H3::KModel::translation(QVector3D(0,0,0), QVector3D(0,y,0));
        for(int j=0; j<m; j++)
        {
            m_hlineMesh.addVertex(mat.map(pts[j]).toVector3DAffine(), nrms[j], QPointF( (double)i/(n-1), (double)j/(m-1)));
        }
    }
    m_hlineMesh.addTriangleGrid(n,m);
    m_hlineMesh.createBuffers();
}


void HyperbolicPolyhedronPage::paintGL()
{
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawBackground();
    
    glPushMatrix();
    glTranslated(0,0,-m_cameraDistance);
    glRotated(m_theta, 1,0,0);
    glRotated(m_phi, 0,1,0);


    GLfloat specular[] =  { 0.7f, 0.7f, 0.7f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 90.0);

    draw();
    // drawAxes();
    drawOutSphere();

    /*
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);


    glDisable(GL_CULL_FACE);
    
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    */

    glPopMatrix();
}
/*
void drawPolygon(int n, double r)
{
    QVector<QVector3D> pts;
    for(int i=0;i<n; i++)
    {
        double phi = 2*M_PI*i/n;
        pts.append(QVector3D(r*cos(phi), r*sin(phi), 0));
    }
    
    for(int i=0;i<n; i++)
    {
        setColor(0.2,0.3,0.9);
        drawSphere(pts[i], 0.03);
        setColor(0.2,0.5,0.9);
        drawCylinder(pts[i], pts[(i+1)%n], 0.02);
    }
}

QVector3D getPos(double r, double unit, int vertexIndex, double xAngle)
{
    QMatrix4x4 mat;
    mat.setToIdentity();
    // 
    mat.rotate(xAngle,1,0,0);
    mat.translate(unit,0,unit);
    mat.rotate(45,0,1,0);
    mat.rotate(vertexIndex * 36, 0,0,1);
    return mat.map(QVector3D(r,0,0));
}
*/


void HyperbolicPolyhedronPage::draw()
{
    QMatrix4x4 globalMat; 
    double z0 = m_parameter / sqrt(1.0 + m_parameter*m_parameter);
    globalMat = H3::KModel::translation(QVector3D(0,0,0), QVector3D(0,0,z0));

    m_texture1.bind();
    //glEnable(GL_TEXTURE_2D);
    
    QGLShaderProgram *prog = m_h3ShaderProgram;
    prog->bind();
    setViewUniforms(prog);
    prog->setUniformValue("u_texture", 0);
    setColor(0.8,0.3,0.04);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    // glEnable(GL_CULL_FACE);
    m_mesh.bind();

    for(int i=0; i<=10;i++)
    {
        double z = 0.8*(i-5.0)/5.0;
        QMatrix4x4 mat = H3::KModel::translation(QVector3D(0,0,0), QVector3D(0,0,z));
        prog->setUniformValue("hMatrix", globalMat * mat);
        m_mesh.draw();
    }
    m_mesh.release();


    m_hlineMesh.bind();
    prog->setUniformValue("hMatrix", globalMat);
    m_hlineMesh.draw();
    prog->setUniformValue("hMatrix", H3::KModel::translation(QVector3D(0,0,0), QVector3D(0.1,0,0)));
    m_hlineMesh.draw();
    m_hlineMesh.release();
    
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    prog->release();
    // glDisable(GL_TEXTURE_2D);

    m_texture1.release();

}

void HyperbolicPolyhedronPage::drawOutSphere()
{

    QGLShaderProgram *prog = m_stdShaderProgram;
    prog->bind();
    setViewUniforms(prog);
    setColor(0.05,0.08,0.08);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);
    m_outSphereMesh.bind();

    m_outSphereMesh.draw();
    m_outSphereMesh.release();
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);

    prog->release();
}


void HyperbolicPolyhedronPage::mousePressEvent(QMouseEvent *e)
{
      m_lastPos = e->pos();
      m_rotating = e->button() == Qt::RightButton;
}

void HyperbolicPolyhedronPage::mouseReleaseEvent(QMouseEvent *e)
{

}


void HyperbolicPolyhedronPage::mouseMoveEvent(QMouseEvent *e)
{
  QPoint delta = m_lastPos - e->pos();
  m_lastPos = e->pos();
  if(!m_rotating)
  {
      m_parameter += 0.001*delta.x();
  }
  else
  {
    m_phi -= 0.25*delta.x();
    m_theta -= 0.25*delta.y();

  }
  // updateGL();
}

void HyperbolicPolyhedronPage::keyPressEvent(QKeyEvent *e)
{
    e->ignore();
}

void HyperbolicPolyhedronPage::wheelEvent(QWheelEvent*e)
{
    m_cameraDistance = clamp(m_cameraDistance - e->delta() * 0.01, 1,50);
}
