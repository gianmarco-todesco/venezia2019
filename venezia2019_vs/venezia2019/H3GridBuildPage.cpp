#include "H3GridBuildPage.h"
#include "H3GridBuildPage.h"

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

namespace {
    QMatrix4x4 htr(double kx, double ky, double kz)
    {
        return H3::KModel::translation(QVector3D(0,0,0), QVector3D(kx,ky,kz));
    }

} // namespace






H3GridBuildPage::H3GridBuildPage()
: m_theta(30)
, m_phi(-20)
, m_cameraDistance(10)
, m_rotating(true)
, m_shaderProgram(0)
, m_grid(0)
, m_status(0)
, m_panMode(0)
, m_panParameter(0)
, m_uffa(0)
, m_uffa2(0)
{
    m_grid = new H3Grid534();
    
    m_hMatrix.setToIdentity();
}

H3GridBuildPage::~H3GridBuildPage()
{
    delete m_shaderProgram; m_shaderProgram=0;
    delete m_grid; m_grid = 0;
    
}

void H3GridBuildPage::initializeGL()
{

  m_shaderProgram = loadProgram("h3grid2");
 
  m_vertexCube.makeCube(0.035,3);
  makeEdgeBox();
  
  

}

void H3GridBuildPage::start()
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
    m_statusStartTime = m_clock.elapsed();
}


void H3GridBuildPage::resizeGL(int width, int height)
{
  double aspect = (float)width/height;
  glViewport(0,0,width,height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45, aspect, 1.0, 70.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}



void H3GridBuildPage::makeEdgeBox()
{
    double d = 0.05;
    double edgeLength = m_grid->edgeLength;    
    int n = 10;
    QVector4D base[4] = {
        QVector4D(-d,0,-d,1.0),
        QVector4D( d,0,-d,1.0),
        QVector4D( d,0, d,1.0),
        QVector4D(-d,0, d,1.0)
    };
    QVector<QVector4D> pts;
    for(int i=0; i<=2*n; i++) 
    {
        double hz = 0.5*edgeLength*(1+(double)(i-n)/(double)n);
        QMatrix4x4 tr = H3::KModel::translation(
            QVector3D(0,0,0), 
            QVector3D(0,H3::KModel::getRadius(hz), 0));
        for(int j=0; j<4; j++) pts.append(tr.map(base[j]));
    }
    for(int i=0;i<pts.count();i++) {
        pts[i] *= 1.0/pts[i].w();
    }
    const QVector3D norms[4] = {
        QVector3D(0,0,-1),
        QVector3D(1,0,0),
        QVector3D(0,0,1),
        QVector3D(-1,0,0)
    };
    Mesh &mesh = m_edgeBox;
    for(int j=0; j<4; j++) {
        int j1 = (j+1)%4;
        int k = mesh.m_vCount;
        for(int i=0; i<=2*n; i++) {
            mesh.addVertex(pts[i*4+j].toVector3D(), norms[j]);
            mesh.addVertex(pts[i*4+j1].toVector3D(), norms[j]);
        }
        for(int i=0; i<2*n; i++) {
            mesh.addQuad(k,k+2,k+3,k+1); k+= 2;
        }
    }
    mesh.createBuffers();
}


void H3GridBuildPage::paintGL()
{
    QTime localClock;
    localClock.start();
    

    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawBackground();

    glPushMatrix();
    glTranslated(0,0,-m_cameraDistance);
    glRotated(m_theta,1,0,0);
    glRotated(m_phi,0,1,0);


    GLfloat specular[] =  { 0.7f, 0.7f, 0.7f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 90.0);
    // glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, colors[0]);

    if(m_uffa > 0)
        draw4();
    else
        draw2();


    glPopMatrix();
}


void H3GridBuildPage::draw(const QMatrix4x4 &mat, const Mesh &mesh)
{
    m_shaderProgram->setUniformValue("hMatrix", m_hMatrix * mat );
    mesh.draw();
}





void H3GridBuildPage::mousePressEvent(QMouseEvent *e)
{
  m_lastPos = e->pos();
  m_rotating = e->button() == Qt::RightButton;
}

void H3GridBuildPage::mouseReleaseEvent(QMouseEvent *e)
{

}

void H3GridBuildPage::mouseMoveEvent(QMouseEvent *e)
{
  QPoint delta = m_lastPos - e->pos();
  m_lastPos = e->pos();
  if(!m_rotating)
  {
      if(m_panMode == 0 && m_uffa==0)
      {
          m_hOffset += 0.001 * QVector3D(-delta.x(), delta.y(), 0.0);
          m_hMatrix = htr(m_hOffset.x(), m_hOffset.y(), m_hOffset.z());
      }
      else
      {
          m_panParameter = qMax(0., qMin(1., m_panParameter + 0.01*delta.x()));

          QVector3D p = m_grid->getCornerMatrix().map(QVector3D(0,0,0));
          m_hMatrix = H3::KModel::translation(p * m_panParameter, QVector3D(0,0,0));
      }
  }
  else
  {
    m_phi -= 0.25*delta.x();
    m_theta -= 0.25*delta.y();
  }
  updateGL();
}


void H3GridBuildPage::wheelEvent(QWheelEvent*e)
{
  m_cameraDistance = clamp(m_cameraDistance - e->delta() * 0.005, 1,50);
  updateGL();
}


void H3GridBuildPage::keyPressEvent(QKeyEvent *e)
{
    
    if(e->key() == Qt::Key_Right) { m_status++;  m_statusStartTime = m_clock.elapsed(); }
    else if(e->key() == Qt::Key_Left) m_status--;
    else if(e->key() == Qt::Key_P) { m_panMode = 1-m_panMode; }
    else if(e->key() == Qt::Key_1) { m_uffa = 0; }
    else if(e->key() == Qt::Key_2) { m_uffa = 1; }
    else if(e->key() == Qt::Key_3) { m_uffa = 3; }


    else if(e->key() == Qt::Key_7) { m_uffa2 = 0; }
    else if(e->key() == Qt::Key_8) { m_uffa2 = 1; }
    else if(e->key() == Qt::Key_9) { m_uffa2 = 2; }
    else if(e->key() == Qt::Key_0) { m_hOffset = QVector3D(0,0,0); m_hMatrix.setToIdentity(); }
    else
      e->ignore();
}


//------------------------
//
// draw
//
//------------------------

void H3GridBuildPage::draw2()
{
    drawAxes();


    m_shaderProgram->bind();
    setViewUniforms(m_shaderProgram);    
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);
    
    QMatrix4x4 identity; identity.setToIdentity();

    m_vertexCube.bind();
    
    double hEdgeLength = m_grid->edgeLength;
    double kEdgeLength = H3::KModel::getRadius(hEdgeLength);

    int m = 14; 
    QList<double> qs; 
    for(int i=0;i<m; i++) qs.append(H3::KModel::getRadius(hEdgeLength * i / (m-1)));


    QVector<QMatrix4x4> Sx,Sy,Sz,S_x,S_y,S_z;
    for(int i=0;i<m;i++)
    {
        Sx.append(htr(qs[i],0,0));
        Sy.append(htr(0,qs[i],0));
        Sz.append(htr(0,0,qs[i]));
        S_x.append(htr(-qs[i],0,0));
        S_y.append(htr(0,-qs[i],0));
        S_z.append(htr(0,0,-qs[i]));
    }

    const QMatrix4x4 
        Mx = htr(kEdgeLength,0,0),
        My = htr(0,kEdgeLength,0),
        Mz = htr(0,0,kEdgeLength),
        M_x = htr(-kEdgeLength,0,0),
        M_y = htr(0,-kEdgeLength,0),
        M_z = htr(0,0,-kEdgeLength),
        Mxy = Mx*My,
        Mxz = Mx*Mz,
        Myx = My*Mx,
        Myz = My*Mz,
        Mzx = Mz*Mx,
        Mzy = Mz*My,
        Mxyz = Mxy*Mz,
        Myxz = Myx*Mz,
        Mxzy = Mxz*My,
        Mxyzx = Mxyz*Mx;
        

    int k = (m_clock.elapsed() - m_statusStartTime)/10;

    // il primo cubo. lo faccio sempre
    draw(identity, m_vertexCube);

    if(m_status >= 1) {
        // aggiungo un cubo
        draw(Sy[1], m_vertexCube);
    }
    if(m_status >= 2) {
        // prima colonna
        int mm = m_status == 2 ? qMin(m-2,k)+2 : m;
        for(int i = 2; i<mm; i++) draw(Sy[i], m_vertexCube);
    } 
    
    if(m_status >= 3) {
        // colonne 0y/0x
        int mm = m_status == 3 ? qMin(m-1,k)+1 : m;
        for(int i = 1; i<mm; i++) draw(Sx[i], m_vertexCube);
    } 
    
    if(m_status >= 4) {
        // quattro lati
        int mm = m_status == 4 ? qMin(m-1,k) +1 : m;        
        for(int i = 1; i<mm; i++) 
        {
            draw(Mx*Sy[i], m_vertexCube);
            draw(My*Sx[i], m_vertexCube);
        }
    } 
    
    if(m_status >= 5) {
        // chiude
        int mm1, mm2;
        if(m_status == 5) { mm1 = qMin(m/2-1, k) + 1; mm2 = qMin((m+1)/2-1, k) + 1; }
        else { mm1 = m/2; mm2 = (m+1)/2; }
        for(int i = 1; i<mm1; i++) draw(Mx*My*S_x[i], m_vertexCube);
        for(int i = 1; i<mm2; i++) draw(My*Mx*S_y[i], m_vertexCube);
    } 
    
    if(m_status>=6) {

        int mm = m_status == 6 ? qMin(m-1,k)+1 : m;
        for(int i=1;i<mm; i++) 
        {
            draw(Sz[i], m_vertexCube); 
            draw(Mx*Sz[i], m_vertexCube); 
            draw(My*Sz[i], m_vertexCube); 
            draw(Mxy*Sz[i], m_vertexCube); 
            draw(Myx*Sz[i], m_vertexCube); 
        }

    }

    if(m_status >= 7)
    {
        int mm = m_status == 7 ? qMin(m-1,k)+1 : m;
        for(int i=1;i<mm; i++) 
        {
            draw(Mz * Sx[i], m_vertexCube); 
            draw(Mz * Sy[i], m_vertexCube); 
            draw(Mxz * S_x[i], m_vertexCube); 
            draw(Mxz * Sy[i], m_vertexCube); 
            draw(Myz * Sx[i], m_vertexCube); 
            draw(Myz * S_y[i], m_vertexCube); 
            draw(Mxyz * S_x[i], m_vertexCube); 
            draw(Mxyz * S_y[i], m_vertexCube); 
            draw(Myxz * S_x[i], m_vertexCube); 
            draw(Myxz * S_y[i], m_vertexCube); 
        }
    }

    if(m_status >= 8)
    {
        int mm = m_status == 8 ? qMin(m-1,k)+1 : m;
        for(int i=1;i<mm; i++) 
        {
            draw(Mxzy * S_x[i], m_vertexCube); 
            draw(Mzx * Sy[i], m_vertexCube); 
            draw(Mzy * Sx[i], m_vertexCube); 
            draw(Myz * Mx * S_y[i], m_vertexCube); 
            draw(Mxyz * M_x * S_y[i], m_vertexCube); 
        }
    }

    if(m_status >= 9)
    {
        int mm = m_status == 9 ? qMin(m-1,k)+1 : m;
        for(int i=1;i<mm; i++) 
        {
            draw(Mzx * My * S_x[i], m_vertexCube); 
            draw(Mzy * Mx * S_z[i], m_vertexCube); 
            draw(Myxz * M_y * M_x * S_z[i], m_vertexCube); 
            draw(Mxz * My * M_x * S_y[i], m_vertexCube); 

            draw(Myz * Mx * M_y * S_z[i], m_vertexCube); 

        }
    }

    m_vertexCube.release();

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisable(GL_CULL_FACE);
    m_shaderProgram->release();

}


QMatrix4x4 rotation(double degree, int x, int y, int z) 
{
    QMatrix4x4 rot;rot.setToIdentity();
    rot.rotate(degree,x,y,z);
    return rot;
}

void H3GridBuildPage::draw3()
{
    drawAxes();
    glDisable(GL_LIGHTING);

    Polyhedron *ph = makeDodecahedron();
    ph->computeFaceVertices();
    ph->scale(m_grid->radius / ph->getVertex(0).m_pos.length());
    
    QMatrix4x4 cornerMatrix = m_grid->getCornerMatrix();

    QMatrix4x4 faceRotation;
    faceRotation = 
        rotation(180*0.5*atan(2.0)/M_PI, 1,0,0) *
        // rotation(36, 0,0,1) *
        rotation(-180*0.5*atan(2.0)/M_PI, 1,0,0);
    
    QMatrix4x4 matrices[8];
    QMatrix4x4 rot;     
    for(int i=0;i<4;i++) 
    {
        rot.setToIdentity(); 
        rot.rotate(90*i,1,0,0);
        matrices[i] = rot * cornerMatrix;
        rot.setToIdentity(); 
        rot.rotate(180,0,1,0);
        rot.rotate(90*i,1,0,0);
        matrices[4+i] = rot * cornerMatrix;
    }

    for(int i=0;i<8;i++)
    {
        matrices[i] = cornerMatrix.inverted() * matrices[i];
    }

    m_hMatrix.setToIdentity();
    m_hMatrix = cornerMatrix;

    glColor3d(1,0,1);
    for(int i=0;i<8;i++)
    {
        drawHPolyhedron(ph, matrices[i]);
    }

    for(int i=0;i<ph->getVertexCount();i++)
    {
        QVector3D p = H3::KModel::toBall(m_hMatrix.map(QVector4D(ph->getVertex(i).m_pos,1)));
        v()->drawText(p, QString::number(i), 0.2);
    }
    int edges[][2] = {
        {0,17}, {17,14}, {14,9}, {9,0}, 

        {0,13}, {17,10}, {14, 7}, {9,18},
        {13,18}, {18,7},  {7,10}, {10,13}, 
    };
    glColor3d(0,1,1);
    for(int i=0;i<9;i++) 
        drawHLine(ph->getVertex(edges[i][0]).m_pos, ph->getVertex(edges[i][1]).m_pos);

    QMatrix4x4 base = m_hMatrix;

    QMatrix4x4 uff(-1,0,0,0,0,-1,0,0,0,0,-1,0,0,0,0,1);
    m_hMatrix = uff * base;
    for(int i=0;i<12;i++) 
        drawHLine(ph->getVertex(edges[i][0]).m_pos, ph->getVertex(edges[i][1]).m_pos);

    delete(ph);
    glEnable(GL_LIGHTING);
}


void H3GridBuildPage::drawHPolyhedron(Polyhedron *ph, const QMatrix4x4 &matrix)
{
    QMatrix4x4 old = m_hMatrix; m_hMatrix = m_hMatrix * matrix;
    for(int i=0;i<ph->getEdgeCount();i++)
    {
        const Polyhedron::Edge &edge = ph->getEdge(i);
        QVector3D p0 = ph->getVertex(edge.m_a).m_pos;
        QVector3D p1 = ph->getVertex(edge.m_b).m_pos;
        drawHLine(p0,p1);
    }
    m_hMatrix = old;
}


void H3GridBuildPage::drawHLine(const QVector3D &p0, const QVector3D &p1)
{
    glBegin(GL_LINE_STRIP);
    int n = 30;
    for(int i=0;i<n;i++)
    {
        double t = (double)i/(n-1);
        QVector3D p = (1-t)*p0 + t*p1;
        QVector3D q = H3::KModel::toBall(this->m_hMatrix.map(QVector4D(p,1.0)));
        glVertex(q);
    }
    glEnd();
}






void H3GridBuildPage::draw4()
{
    drawAxes();

    QMatrix4x4 cornerMatrix = m_grid->getCornerMatrix();

    m_shaderProgram->bind();
    setViewUniforms(m_shaderProgram);    
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);
    
    QMatrix4x4 identity; identity.setToIdentity();

    m_vertexCube.bind();
    
    double hEdgeLength = m_grid->edgeLength;
    double kEdgeLength = H3::KModel::getRadius(hEdgeLength);

    //QMatrix4x4 baseMats[] = {
    //};

    int m = 16; 
    QList<double> qs; 
    for(int i=0;i<m; i++) qs.append(H3::KModel::getRadius(hEdgeLength * i / (m-1)));


    QVector<QMatrix4x4> Sx,Sy,Sz,S_x,S_y,S_z;
    for(int i=0;i<m;i++)
    {
        Sx.append(htr(qs[i],0,0));
        Sy.append(htr(0,qs[i],0));
        Sz.append(htr(0,0,qs[i]));
        S_x.append(htr(-qs[i],0,0));
        S_y.append(htr(0,-qs[i],0));
        S_z.append(htr(0,0,-qs[i]));
    }

    const QMatrix4x4 
        Mx = htr(kEdgeLength,0,0),
        My = htr(0,kEdgeLength,0),
        Mz = htr(0,0,kEdgeLength),
        M_x = htr(-kEdgeLength,0,0),
        M_y = htr(0,-kEdgeLength,0),
        M_z = htr(0,0,-kEdgeLength),
        Mxy = Mx*My,
        Mxz = Mx*Mz,
        Myx = My*Mx,
        Myz = My*Mz,
        Mzx = Mz*Mx,
        Mzy = Mz*My,
        Mxyz = Mxy*Mz,
        Myxz = Myx*Mz,
        Mxzy = Mxz*My,
        Mxyzx = Mxyz*Mx;
    

    QMatrix4x4 matrices[8];
    QMatrix4x4 rot;     
    for(int i=0;i<4;i++) 
    {
        rot.setToIdentity(); 
        rot.rotate(90*i,1,0,0);
        matrices[i] = rot;
        rot.setToIdentity(); 
        rot.rotate(180,0,1,0);
        rot.rotate(90*i,1,0,0);
        matrices[4+i] = rot;
    }

    for(int ii=0;ii<(m_uffa==1 ? 1 : 8);ii++)
    {
        QMatrix4x4 oldMatrix = m_hMatrix; m_hMatrix = m_hMatrix * matrices[ii];


        //--------------


        draw(identity, m_vertexCube);
        for(int i = 1; i<m; i++) draw(Sy[i], m_vertexCube);
        for(int i = 1; i<m; i++) draw(Sx[i], m_vertexCube);
        for(int i = 1; i<m; i++) 
        {
            draw(Mx*Sy[i], m_vertexCube);
            draw(My*Sx[i], m_vertexCube);
        }
        for(int i = 1; i<m; i++) draw(Mx*My*S_x[i], m_vertexCube);
        for(int i = 1; i<m; i++) draw(My*Mx*S_y[i], m_vertexCube);
        for(int i=1;i<m; i++) 
        {
            draw(Sz[i], m_vertexCube); 
            draw(Mx*Sz[i], m_vertexCube); 
            draw(My*Sz[i], m_vertexCube); 
            draw(Mxy*Sz[i], m_vertexCube); 
            draw(Myx*Sz[i], m_vertexCube); 
        }

        for(int i=1;i<m; i++) 
        {
            draw(Mz * Sx[i], m_vertexCube); 
            draw(Mz * Sy[i], m_vertexCube); 
            draw(Mxz * S_x[i], m_vertexCube); 
            draw(Mxz * Sy[i], m_vertexCube); 
            draw(Myz * Sx[i], m_vertexCube); 
            draw(Myz * S_y[i], m_vertexCube); 
            draw(Mxyz * S_x[i], m_vertexCube); 
            draw(Mxyz * S_y[i], m_vertexCube); 
            draw(Myxz * S_x[i], m_vertexCube); 
            draw(Myxz * S_y[i], m_vertexCube); 
        }

        for(int i=1;i<m; i++) 
        {
            draw(Mxzy * S_x[i], m_vertexCube); 
            draw(Mzx * Sy[i], m_vertexCube); 
            draw(Mzy * Sx[i], m_vertexCube); 
            draw(Myz * Mx * S_y[i], m_vertexCube); 
            draw(Mxyz * M_x * S_y[i], m_vertexCube); 
        }

        for(int i=1;i<m; i++) 
        {
            draw(Mzx * My * S_x[i], m_vertexCube); 
            draw(Mzy * Mx * S_z[i], m_vertexCube); 
            draw(Myxz * M_y * M_x * S_z[i], m_vertexCube); 
            draw(Mxz * My * M_x * S_y[i], m_vertexCube); 

            draw(Myz * Mx * M_y * S_z[i], m_vertexCube); 
        }

        //--------------

        m_hMatrix = oldMatrix;

    }





    m_vertexCube.release();


    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisable(GL_CULL_FACE);
    m_shaderProgram->release();

    if(m_uffa2>=1)
    {
        glDisable(GL_LIGHTING);
        Polyhedron *ph = makeDodecahedron();
        ph->computeFaceVertices();
        ph->scale(m_grid->radius / ph->getVertex(0).m_pos.length());
    
        // QMatrix4x4 cornerMatrix = m_grid->getCornerMatrix();

        int edges[][2] = {
            {0,17}, {17,14}, {14,9}, {9,0}, 

            {0,13}, {17,10}, {14, 7}, {9,18},
            {13,18}, {18,7},  {7,10}, {10,13}, 
        };
        glColor3d(1,0,1);
        
        glLineWidth(3.0);
        
        QMatrix4x4 old;
        
        
        old = m_hMatrix;
        m_hMatrix = m_hMatrix * cornerMatrix;
        for(int i=0;i<12;i++) {
            QVector3D p0 = ph->getVertex(edges[i][0]).m_pos;
            QVector3D p1 = ph->getVertex(edges[i][1]).m_pos;
            drawHLine(p0,p1);
        }
        m_hMatrix = old;

        if(m_uffa2>=2)
        {
            glColor3d(0,0,1.0);
            old = m_hMatrix;
            m_hMatrix = m_hMatrix * QMatrix4x4(-1,0,0,0, 0,-1,0,0, 0,0,-1,0, 0,0,0,1) * cornerMatrix;
            for(int i=0;i<12;i++) {
                QVector3D p0 = ph->getVertex(edges[i][0]).m_pos;
                QVector3D p1 = ph->getVertex(edges[i][1]).m_pos;
                drawHLine(p0,p1);
            }
            m_hMatrix = old;
        }


        glLineWidth(1.0);

        glEnable(GL_LIGHTING);
        delete ph;
    }

}
