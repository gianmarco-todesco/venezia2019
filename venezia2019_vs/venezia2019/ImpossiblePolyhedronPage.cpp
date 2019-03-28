#include "ImpossiblePolyhedronPage.h"

#include "Polyhedra.h"
#include "Gutil.h"
#include "Point3.h"

#include "Mesh.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>
#include <QKeyEvent>
#include <QWheelEvent>

#include <vector>

#include <QGLShaderProgram>
#include <qmatrix4x4.h>
#include <qvector.h>
#include <qlist.h>
#include <qmap.h>



void drawPolygon(int n, double r0, const Color &color) {
    double border = 0.3;
    double h = 0.01;
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


int mode = 1;
double trianglePos = 3.529;
double squarePos = 6.047;
double squareAngle = 14.986;
double squareScale = 1.029;

ImpossiblePolyhedronPage::ImpossiblePolyhedronPage()
: m_cameraDistance(15)
, m_theta(0)
, m_phi(0)
, m_rotating(true)
, m_parameter(3.752)
{
}

ImpossiblePolyhedronPage::~ImpossiblePolyhedronPage()
{
}

void ImpossiblePolyhedronPage::initializeGL()
{
}

void ImpossiblePolyhedronPage::start()
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

void ImpossiblePolyhedronPage::resizeGL(int width, int height)
{
    double aspect = (float)width/height;
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, aspect, 1.0, 70.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void ImpossiblePolyhedronPage::paintGL()
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

    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);


    glDisable(GL_CULL_FACE);
    
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

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


void ImpossiblePolyhedronPage::draw()
{
    //glEnable(GL_CULL_FACE);
    
    //glDisable(GL_CULL_FACE);

    double position = m_parameter;
    double unit = 3;

    double edgeLength = 2.0;
    double decagonRadius = edgeLength*0.5/sin(M_PI/10);
    double squareRadius = edgeLength / sqrt(2.0);
    double triangleRadius = edgeLength*0.5/sin(M_PI/3);

    // double radius = 0.85* unit*m_parameter;

    // edges (decagons)
    for(int i=0; i<12; i++)
    {
        glPushMatrix();

        // edge matrix
        int k = i/4;
        if(k==1) glRotated(90,0,0,1);
        else if(k==2) glRotated(90,1,0,0);        
        glRotated(90*(i%4),0,1,0);

        // move 
        glTranslated(position,0,position);
        glRotated(45,0,1,0);

        drawPolygon(10, decagonRadius, Color(0.3,0.6,0.8));

        glPopMatrix();
    }

    // triangoli rossi

    QVector3D e2 = QVector3D(1,1,1).normalized();
    QVector3D e0 = QVector3D(0,1,0);
    e0 = (e0 - e2*QVector3D::dotProduct(e0,e2)).normalized();
    QVector3D e1 = QVector3D::crossProduct(e2,e0);

    QMatrix4x4 mat(
        e0.x(),e0.y(),e0.z(), 0,
        e1.x(),e1.y(),e1.z(), 0,
        e2.x(),e2.y(),e2.z(), 0,
        0,0,0,1);

    for(int i=0;i<8;i++)
    {
        

        glPushMatrix();
        if(i>=4) glRotated(180,1,0,0);
        glRotated(90*i,0,1,0);
        double d = trianglePos;
        glTranslated(d,d,d);
        glMultMatrixd(mat.transposed().constData());
        drawPolygon(3, triangleRadius, Color(0.8,0.1,0.1));
        glPopMatrix();


    }
    // facce cubo
    for(int i=0;i<6;i++)
    {
        glPushMatrix();
        if(1<=i && i<=4)
        {
            glRotated(90*i,0,0,1);
            glRotated(90,1,0,0);
        }
    

        glTranslated(0,0,squarePos);
        
        // quadrato
        drawPolygon(4, squareRadius * squareScale, Color(0.8,0.8,0.1));

        // triangoli attorno al quadrato
        for(int j=0; j<4; j++)
        {
            glPushMatrix();
            glRotated(45+90*j,0,0,1);
            glTranslated(edgeLength*0.5* squareScale,0,0);
        
            glRotated(squareAngle,0,1,0);
            glTranslated(triangleRadius*0.5* squareScale,0,0);
            drawPolygon(3, triangleRadius* squareScale, Color(0.8,0.8,0.1));
            glPopMatrix();
        }
    
        glPopMatrix();
    }
    /*
    Polyhedron *ph = makeCube();
    ph->computeFaceVertices();


    double sc = 3.0 / ph->getVertex(0).m_pos.length();
    ph->scale(sc);

    double unit = fabs(ph->getVertex(0).m_pos.x());

    for(int i=0;i<ph->getVertexCount();i++)
    {
        setColor(0.5,0.8,0.1);
        drawSphere(ph->getVertex(i).m_pos, 0.01);
    }

    for(int i=0;i<ph->getEdgeCount();i++)
    {
        const Polyhedron::Edge &edge = ph->getEdge(i);
        QVector3D p0 = ph->getVertex(edge.m_a).m_pos;
        QVector3D p1 = ph->getVertex(edge.m_b).m_pos;
        setColor(0.2,0.2,0.1);
        drawCylinder(p0,p1,0.008);
    }

    double r = unit*0.9*m_parameter;

    
    for(int i=0; i<12; i++)
    {
        glPushMatrix();
        int k = i/4;
        if(k==1) glRotated(90,0,0,1);
        else if(k==2) glRotated(90,1,0,0);
        
        glRotated(90*(i%4),0,1,0);
        glTranslated(unit,0,unit);
        glRotated(45,0,1,0);

        drawPolygon(10,r);

        glPopMatrix();
    }

    QVector<QVector3D> quadPts;
    for(int i=0; i<4; i++) quadPts.append(getPos(r,unit,1,90*i));
    
    for(int i=0; i<6; i++)
    {
        glPushMatrix();
        if(1<=i && i<=4) { glRotated(90*i, 1,0,0); }
        glPopMatrix();
    }

    setColor(0.8,0.2,0.1);
    drawSphere(getPos(r,unit,0,0), 0.05);
    setColor(0.2,0.8,0.1);

    drawSphere(getPos(r,unit,1,0), 0.05);
    drawSphere(getPos(r,unit,1,90), 0.05);
    drawSphere(getPos(r,unit,1,180), 0.05);
    drawSphere(getPos(r,unit,1,270), 0.05);




    delete ph;
  
  */

}



void ImpossiblePolyhedronPage::mousePressEvent(QMouseEvent *e)
{
      m_lastPos = e->pos();
      m_rotating = e->button() == Qt::RightButton;
}

void ImpossiblePolyhedronPage::mouseReleaseEvent(QMouseEvent *e)
{

}


void ImpossiblePolyhedronPage::mouseMoveEvent(QMouseEvent *e)
{
  QPoint delta = m_lastPos - e->pos();
  m_lastPos = e->pos();
  if(!m_rotating)
  {
      if(mode==1) {
        m_parameter += 0.001*delta.x();
      } else if(mode == 2) {
          trianglePos += 0.001*delta.x();
      } else if(mode == 3) {
          squarePos += 0.001*delta.x();
      } else if(mode == 4) {
          squareAngle += 0.001*delta.x();
      } else if(mode == 5) {
          squareScale += 0.001*delta.x();
      }
  }
  else
  {
    m_phi -= 0.25*delta.x();
    m_theta -= 0.25*delta.y();

  }
  // updateGL();
}

void ImpossiblePolyhedronPage::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_1) mode = 1;
    else if(e->key() == Qt::Key_2) mode = 2;
    else if(e->key() == Qt::Key_3) mode = 3;
    else if(e->key() == Qt::Key_4) mode = 4;
    else if(e->key() == Qt::Key_5) mode = 5;
    else if(e->key() == Qt::Key_6) mode = 6;
    else    e->ignore();
}

void ImpossiblePolyhedronPage::wheelEvent(QWheelEvent*e)
{
    m_cameraDistance = clamp(m_cameraDistance - e->delta() * 0.01, 1,50);
}