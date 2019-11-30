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

const double magicRadius = 3.752;

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
: m_cameraDistance(20)
, m_theta(0)
, m_phi(0)
, m_rotating(true)
, m_parameter(0)
, m_status(0)
, m_showCube(false)
, m_showAllFaces(true)
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

void ImpossiblePolyhedronPage::draw()
{
    if(m_status == 0) draw1();
    else draw2();
    
}

void ImpossiblePolyhedronPage::draw1()
{
    if(m_showCube) drawCube();

    

    double position = magicRadius + m_parameter;
    double unit = 3;

    double edgeLength = 2.0;
    double decagonRadius = edgeLength*0.5/sin(M_PI/10);
    double squareRadius = edgeLength / sqrt(2.0);
    double triangleRadius = edgeLength*0.5/sin(M_PI/3);

    
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

    if(m_showAllFaces)
    {
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
    }
}


void ImpossiblePolyhedronPage::drawCube()
{
    Polyhedron *ph = makeCube();
    ph->computeFaceVertices();

    double position = magicRadius + m_parameter;
    
    double thickness = 0.05;

    double sc = position*sqrt(3.0) / ph->getVertex(0).m_pos.length();
    ph->scale(sc);

    double unit = fabs(ph->getVertex(0).m_pos.x());

    for(int i=0;i<ph->getVertexCount();i++)
    {
        setColor(0.4,0.2,0.4);
        drawSphere(ph->getVertex(i).m_pos, thickness*1.5);
    }

    for(int i=0;i<ph->getEdgeCount();i++)
    {
        const Polyhedron::Edge &edge = ph->getEdge(i);
        QVector3D p0 = ph->getVertex(edge.m_a).m_pos;
        QVector3D p1 = ph->getVertex(edge.m_b).m_pos;
        setColor(0.8,0.2,0.8);
        drawCylinder(p0,p1,thickness);
    }
}


void ImpossiblePolyhedronPage::draw2()
{
    double unit = 3;
    double edgeLength = 2.0;
    double decagonRadius = edgeLength*0.5/sin(M_PI/10);
    
    double h = decagonRadius*cos(M_PI/10);
    
    double angle = m_parameter*100;

    glPushMatrix();
    glRotated(angle, 1,0,0);
    glTranslated(0,-h,0);
    drawPolygon(10, decagonRadius, Color(0.3,0.6,0.8));
    glPopMatrix();

    glPushMatrix();
    glRotated(-angle, 1,0,0);
    glTranslated(0,h,0);
    drawPolygon(10, decagonRadius, Color(0.3,0.6,0.8));
    glPopMatrix();


    if(m_status == 2)
    {
        QMatrix4x4 mat;
        mat.setToIdentity();

        /*
        mat.translate(0,h,0);
        mat.rotate(angle,1,0,0);
        mat.translate(0,h,0);

        */
        mat.rotate(-36*2, 0,0,1);        
        mat.translate(0,h,0);
        mat.rotate(-angle*2,1,0,0);
        mat.translate(0,h,0);

        glPushMatrix();
        glRotated(-angle, 1,0,0);
        glTranslated(0,h,0);
        
        glMultMatrixd(mat.constData());        
        drawPolygon(10, decagonRadius, Color(0.3,0.6,0.8));
        glMultMatrixd(mat.constData());        
        drawPolygon(10, decagonRadius, Color(0.3,0.6,0.8));
        glPopMatrix();

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
        setColor(1,1,1);
    }

    drawCylinder(p0,p1,0.04);
    drawCylinder(p2,p3,0.04);


}

void ImpossiblePolyhedronPage::draw_old()
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
      m_parameter += 0.001*delta.x();
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
    if(e->key() == Qt::Key_1) { m_status = 0; m_parameter = 0; }
    else if(e->key() == Qt::Key_2) { m_status = 1; m_parameter = 0; }
    else if(e->key() == Qt::Key_3) m_status = 2;
    else if(e->key() == Qt::Key_C) m_showCube = !m_showCube;
    else if(e->key() == Qt::Key_F) m_showAllFaces = !m_showAllFaces;
    else    e->ignore();
}

void ImpossiblePolyhedronPage::wheelEvent(QWheelEvent*e)
{
    m_cameraDistance = clamp(m_cameraDistance - e->delta() * 0.01, 1,50);
}
