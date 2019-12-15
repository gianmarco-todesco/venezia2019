#include "Fig5Page.h"

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

// fig.5 Four decagons kept in position by six equilateral triangles. The chain does not close nicely.



Fig5Page::Fig5Page()
    : m_theta(23.5)
    , m_phi(-25.75)
    , m_cameraDistance(24.2)
    , m_rotating(false)
    , m_parameter(0.627)
{
    m_hMatrix.setToIdentity();
}
    
Fig5Page::~Fig5Page()
{
}

  
void Fig5Page::initializeGL() 
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
    
}

void Fig5Page::resizeGL(int width, int height) 
{
    double aspect = (float)width/height;
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, aspect, 1.0, 70.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Fig5Page::paintGL() 
{        
    glClearColor(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glPushMatrix();
    glTranslated(0,0,-m_cameraDistance);
    glRotated(m_theta,1,0,0);
    glRotated(m_phi,0,1,0);


    qreal viewArr[16], projArr[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, viewArr);
    glGetDoublev(GL_PROJECTION_MATRIX, projArr);
    QMatrix4x4 view(viewArr), proj(projArr);
    QMatrix4x4 projView = proj.transposed() * view.transposed();
    QMatrix4x4 identity; identity.setToIdentity();

    // m_shaderProgram->bind();
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);
    
    QMatrix4x4 globalMatrix; globalMatrix.setToIdentity();

    draw();
    

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisable(GL_CULL_FACE);
    // m_shaderProgram->release();
 
    glPopMatrix();
}




void Fig5Page::mousePressEvent(QMouseEvent *e)
{
    m_lastPos = e->pos();
    m_rotating = e->button() == Qt::RightButton;
}

void Fig5Page::mouseReleaseEvent(QMouseEvent *e)
{

}

void Fig5Page::mouseMoveEvent(QMouseEvent *e)
{
  QPoint delta = m_lastPos - e->pos();
  m_lastPos = e->pos();
  if(!m_rotating)
  {
      m_parameter += delta.x() * 0.001;
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


void Fig5Page::wheelEvent(QWheelEvent*e)
{
  m_cameraDistance = clamp(m_cameraDistance - e->delta() * 0.01, 1,50);
  updateGL();
}


void Fig5Page::keyPressEvent(QKeyEvent *e)
{
    e->ignore();
}


void Fig5Page::savePictures()
{
    Fig5Page page;
    page.m_cameraDistance = m_cameraDistance;
    page.m_theta = m_theta;
    page.m_phi = m_phi;
    page.savePicture("fig5.png");
}


void Fig5Page::draw()
{
    double unit = 3;
    double edgeLength = 2.0;
    double decagonRadius = edgeLength*0.5/sin(M_PI/10);
    
    double h = decagonRadius*cos(M_PI/10);
    
    double angle = m_parameter*100;


    QMatrix4x4 mat;
    mat.setToIdentity();  
    mat.translate(0,h,0);  
    mat.rotate(-angle, 1,0,0);
    mat.translate(0,h,0);  
    mat.rotate(-36*2, 0,0,1);        
    
    double triangleRadius = edgeLength*0.5/sin(M_PI/3);

    QVector3D bpos = -(mat*mat).map(QVector3D(0,0,0)) * 0.5;


    glPushMatrix();
    glTranslated(bpos.x(), bpos.y(), bpos.z());

    for(int i=0;i<3;i++)
    {
        // primo decagono
        drawPolygon(10, decagonRadius, Color(0.6,0.6,0.6));
    
        // due triangli    
        glPushMatrix();
        glTranslated(0,h,0);    
        glRotated(-angle/2, 1,0,0);
        for(int t=0; t<2;t++)
        {
            glPushMatrix();
            glRotated(180*t,0,0,1);
            glTranslated(- edgeLength * 0.5,0,0);
            glRotated(-20,0,1,0);
            glTranslated(-triangleRadius,0,0);
            drawPolygon(3, triangleRadius, Color(0.2,0.2,0.2));
            glPopMatrix();
        }
        glPopMatrix();

        glMultMatrixd(mat.constData());    
    }

    // ultimo decagono
    drawPolygon(10, decagonRadius, Color(0.6,0.6,0.6));
    
    glPopMatrix();


    /*


    glPushMatrix();
    glRotated(180,0,0,1);
    glTranslated(- edgeLength * 0.5,0,0);
    glRotated(-20,0,1,0);
    glTranslated(-triangleRadius,0,0);
    drawPolygon(3, triangleRadius, Color(0.2,0.2,0.2));
    glPopMatrix();


    glPushMatrix();
    glRotated(-angle, 1,0,0);
    glTranslated(0,h,0);
    // drawPolygon(10, decagonRadius, Color(0.3,0.6,0.8));
    glPopMatrix();


    glPushMatrix();
    glRotated(-angle, 1,0,0);
    glTranslated(0,h,0);
        
    glMultMatrixd(mat.constData());        
    drawPolygon(10, decagonRadius, Color(0.3,0.6,0.8));
    glMultMatrixd(mat.constData());        
    drawPolygon(10, decagonRadius, Color(0.3,0.6,0.8));
    glPopMatrix();

    */

    /*

    if(2 == 2)
    {


    }

    double phi;
    phi = 2*M_PI/10;
    QVector3D p(cos(phi)*decagonRadius, sin(phi)*decagonRadius, 0);
    QMatrix4x4 mat; 
    mat.setToIdentity();
    mat.rotate(angle, 1,0,0);
    mat.translate(0,-h,0);
    QVector3D p0 = mat.map(p);
    QVector3D p1 = p0; p1.setY(-p1.y());
    QVector3D p2 = p0, p3 = p1;
    p2.setX(-p2.x());
    p3.setX(-p3.x());

    // glColor3d(0,0.5,0.9);
    setColor(0.1,0.1,0.1);
    drawSphere(p0, 0.1);
    drawSphere(p1, 0.1);
    drawSphere(p2, 0.1);
    drawSphere(p3, 0.1);

    double length = (p1-p0).length();
    double err = length - edgeLength;
    if(err > 0.0) 
    {
        double v = qMax(0., 1.-err*50.0);
        setColor(1,v,v);
    } else if(err < 0.0)
    {
        double v = qMax(0., 1.+err*50.0);
        setColor(v,v,1);
    }
    else
    {
        setColor(.1,.1,.1);
    }

    if(fabs(err)<0.01)
        setColor(1.0,0.0,0.0);

    drawCylinder(p0,p1,0.04);
    drawCylinder(p2,p3,0.04);

    */

}


void Fig5Page::drawPolygon(int n, double r0, const Color &color)
{
    double border = 0.3;
    double h = 0.05;
    double r3 = r0 - border / cos(M_PI/n);
    double t = 0.1;
    double r1 = (1-t)*r0 + t*r3;
    double r2 = (1-t)*r3 + t*r0;
    
    QVector<QPointF> cssn;
    for(int i=0;i<=2*n+2;i++) 
    {
        double phi = M_PI*i/n;
        cssn.append(QPointF(cos(phi), sin(phi)));
    }

    Color color2(0.1,0.1,0.1);
    
    double rr[] = {r0,r1,r2,r3};

    for(int s=0; s<6; s++)
    {
        setColor( s==1 || s==4 ? color : color2);
        int v = s%3;
        int b = s>=3 ? 1 : 0;
        double ra=rr[v+1-b],rb=rr[v+b];
        double sgn = b*2-1;
        glBegin(GL_TRIANGLE_STRIP);
        glNormal3d(0,0,-sgn);
        for(int i=0; i<=2*n; i+=2) 
        {
            glVertex3d(cssn[i].x()*ra,cssn[i].y()*ra,-h*sgn);
            glVertex3d(cssn[i].x()*rb,cssn[i].y()*rb,-h*sgn);
        }
        glEnd();
    }

    // bordo
    setColor(color2);
    glBegin(GL_TRIANGLES);
    for(int i=0;i<n;i++)
    {
        glNormal3d(cssn[2*i+1].x(), cssn[2*i+1].y(), 0);

        glVertex3d(cssn[2*i].x()*r0, cssn[2*i].y()*r0, -h);
        glVertex3d(cssn[2*i+2].x()*r0, cssn[2*i+2].y()*r0,  h);
        glVertex3d(cssn[2*i].x()*r0, cssn[2*i].y()*r0,  h);
       
        glVertex3d(cssn[2*i].x()*r0, cssn[2*i].y()*r0, -h);
        glVertex3d(cssn[2*i+2].x()*r0, cssn[2*i+2].y()*r0, -h);
        glVertex3d(cssn[2*i+2].x()*r0, cssn[2*i+2].y()*r0,  h);


        glNormal3d(-cssn[2*i+1].x(), -cssn[2*i+1].y(), 0);
        
        glVertex3d(cssn[2*i].x()*r3, cssn[2*i].y()*r3, h);
        glVertex3d(cssn[2*i+2].x()*r3, cssn[2*i+2].y()*r3,  -h);
        glVertex3d(cssn[2*i].x()*r3, cssn[2*i].y()*r3,  -h);

        glVertex3d(cssn[2*i].x()*r3, cssn[2*i].y()*r3, h);
        glVertex3d(cssn[2*i+2].x()*r3, cssn[2*i+2].y()*r3, h);
        glVertex3d(cssn[2*i+2].x()*r3, cssn[2*i+2].y()*r3,  -h);

    }
    glEnd();

}
