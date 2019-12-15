#include "Fig4Page.h"

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
#include <QVector3D>
#include <qvector.h>
#include <qlist.h>
#include <QGLBuffer>
#include <QTIme>


#include "Viewer.h"

// fig.4 A computer rendering of the Denis' polyhedron and a cube. The twelve decagons are placed in correspondence with the cube edges.


const double magicRadius = 3.752;


Fig4Page::Fig4Page()
    : m_theta(36.5)
    , m_phi(-31.25)
    , m_cameraDistance(18.2)
    , m_rotating(false)
{
    m_hMatrix.setToIdentity();
}
    
Fig4Page::~Fig4Page()
{
}

  
void Fig4Page::initializeGL() 
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

void Fig4Page::resizeGL(int width, int height) 
{
    double aspect = (float)width/height;
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, aspect, 1.0, 70.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Fig4Page::paintGL() 
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




void Fig4Page::mousePressEvent(QMouseEvent *e)
{
    m_lastPos = e->pos();
    m_rotating = e->button() == Qt::RightButton;
}

void Fig4Page::mouseReleaseEvent(QMouseEvent *e)
{

}

void Fig4Page::mouseMoveEvent(QMouseEvent *e)
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


void Fig4Page::wheelEvent(QWheelEvent*e)
{
  m_cameraDistance = clamp(m_cameraDistance - e->delta() * 0.01, 1,50);
  updateGL();
}


void Fig4Page::keyPressEvent(QKeyEvent *e)
{
    e->ignore();
}


void Fig4Page::savePictures()
{
    Fig4Page page;
    page.m_cameraDistance = m_cameraDistance;
    page.m_theta = m_theta;
    page.m_phi = m_phi;
    page.savePicture("fig4.png");
}


void Fig4Page::draw()
{

    drawCube();

    const double trianglePos = 3.529;
    const double squarePos = 6.047;
    const double squareAngle = 14.986;
    const double squareScale = 1.029;

    

    double position = magicRadius + 0.0;
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

    // if(m_showAllFaces)
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
            else if(i==5)
            {
                glRotated(180,0,1,0);
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

void Fig4Page::drawCube()
{
    Polyhedron *ph = makeCube();
    ph->computeFaceVertices();

    double position = magicRadius + 0.0;
    
    double thickness = 0.05;

    double sc = position*sqrt(3.0) / ph->getVertex(0).m_pos.length();
    sc*=1.02;
    ph->scale(sc);

    double unit = fabs(ph->getVertex(0).m_pos.x());

    for(int i=0;i<ph->getVertexCount();i++)
    {
        setColor(0.4,0.2,0.4);
        drawSphere(ph->getVertex(i).m_pos, thickness*1.5);
    }

    glDisable(GL_LIGHTING);
    glCullFace(GL_FRONT);
    glColor3d(0,0,0);
    for(int i=0;i<ph->getEdgeCount();i++)
    {
        const Polyhedron::Edge &edge = ph->getEdge(i);
        QVector3D p0 = ph->getVertex(edge.m_a).m_pos;
        QVector3D p1 = ph->getVertex(edge.m_b).m_pos;
        drawCylinder(p0,p1,0.05);
    }
    glCullFace(GL_BACK);
    glColor3d(1,1,1);
    for(int i=0;i<ph->getEdgeCount();i++)
    {
        const Polyhedron::Edge &edge = ph->getEdge(i);
        QVector3D p0 = ph->getVertex(edge.m_a).m_pos;
        QVector3D p1 = ph->getVertex(edge.m_b).m_pos;
        drawCylinder(p0,p1,0.02);
    }
    glEnable(GL_LIGHTING);
}


void Fig4Page::drawPolygon(int n, double r0, const Color &color)
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


