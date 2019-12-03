#include "Fig12Page.h"



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
#include <assert.h>


#include "Viewer.h"

Fig12Page::Fig12Page()
: m_theta(10)
, m_phi(20)
, m_cameraDistance(5)
, m_rotating(true)
, m_shaderProgram(0)
{
    m_hMatrix.setToIdentity();
    build();
}

Fig12Page::~Fig12Page()
{
    delete m_shaderProgram; m_shaderProgram=0;
}

void Fig12Page::initializeGL()
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

  m_shaderProgram = loadProgram("h3grid");
 
  m_sphere.makeSphere(0.3,10,10);  
  m_vertexCube.makeCube(0.1,8);

  //makeEdgeBox(m_edgeBox, 10);
  //makeEdgeBox(m_edgeBoxLow, 1);
  
  m_clock.start();
}


void Fig12Page::resizeGL(int width, int height)
{
  double aspect = (float)width/height;
  glViewport(0,0,width,height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(50, aspect, 1.0, 60.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

/*
void Fig12Page::makeEdgeBox(Mesh &mesh, int n)
{
    double d = 0.05;
    double edgeLength = m_grid->edgeLength;    
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
*/


void Fig12Page::paintGL()
{
    QTime localClock;
    localClock.start();
    

    glClearColor(1.0,1.0,1.0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // drawBackground();

    glPushMatrix();
    glTranslated(0,0,-m_cameraDistance);
    glRotated(m_theta,1,0,0);
    glRotated(m_phi,0,1,0);


    GLfloat specular[] =  { 0.7f, 0.7f, 0.7f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 90.0);
    // glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, colors[0]);

    drawAxes();

    draw();


    glPopMatrix();
}

void Fig12Page::draw(const QMatrix4x4 &mat, const Mesh &mesh)
{
    m_shaderProgram->setUniformValue("hMatrix", m_hMatrix * mat );
    mesh.draw();
}



void Fig12Page::mousePressEvent(QMouseEvent *e)
{
  m_lastPos = e->pos();
  m_rotating = e->button() == Qt::RightButton;
}

void Fig12Page::mouseReleaseEvent(QMouseEvent *e)
{

}

void Fig12Page::mouseMoveEvent(QMouseEvent *e)
{
  QPoint delta = m_lastPos - e->pos();
  m_lastPos = e->pos();
  if(!m_rotating)
  {
      // m_hOffset += QVector3D(delta.x()*0.01, delta.y()*0.01, 0.0);
      // m_hMatrix = H3::KModel::translation(m_hOffset, QVector3D(0,0,0));

      m_hOffset += QVector3D(0,0,delta.y()*0.01);
      m_hMatrix = H3::KModel::translation(m_hOffset, QVector3D(0,0,0));
  }
  else
  {
    m_phi -= 0.25*delta.x();
    m_theta -= 0.25*delta.y();
  }
  updateGL();
}


void Fig12Page::wheelEvent(QWheelEvent*e)
{
  m_cameraDistance = clamp(m_cameraDistance - e->delta() * 0.01, 1,50);
  updateGL();
}


void Fig12Page::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Z) { 
    }
    else 
      e->ignore();
}

/*
void Fig12Page::buildGrid()
{
    QTime clock;
    clock.start();
    m_grid2->createFirstVertex();

    // 7
    for(int step=0; step<4; step++)
    {
        QList<QPair<int, int> > todo;
        for(int i=0;i<m_grid2->m_vertices.count();i++)
        {
            for(int j=0;j<6;j++)
            {
                if(m_grid2->m_vertices[i].links[j]== -1) todo.append(qMakePair(i,j));
            }
        }
        qDebug() << "todo size = " << todo.count();
        for(int i=0;i<todo.count();i++)
        {
            int vIndex = todo[i].first;
            int dir = todo[i].second;
            m_grid2->closeIfNeeded(vIndex, dir);
            if(m_grid2->m_vertices[vIndex].links[dir]>=0) continue;
            m_grid2->addVertex(vIndex, dir);
        }
        qDebug() << "Step finished. vertex count = " << m_grid2->m_vertices.count();
    }

    // chiudo tutto
    for(int i=0;i<m_grid2->m_vertices.count();i++)
    {
        for(int j=0;j<6;j++)
        {
            m_grid2->closeIfNeeded(i, j);
        }
    }
    qDebug() << "GRID BUILD: "<< clock.elapsed();
    
}
*/


//------------------------
//
// draw
//
//------------------------

/*
inline bool check(const QMatrix4x4 &mpv, const QMatrix4x4 &hMatrix, const QMatrix4x4 &mat)
{
    QVector4D p4 = (hMatrix * mat).map(QVector4D(0,0,0,1));
    QVector3D p = H3::KModel::toBall(p4, 10);
    QVector4D q4 = mpv.map(QVector4D(p, 1.0));
    QVector3D q = q4.toVector3DAffine();

    / *
    QVector3D p1 = H3::KModel::toBall((hMatrix * mat).map(QVector4D(0.1,0,0,1)), 10);
    QVector3D p2 = H3::KModel::toBall((hMatrix * mat).map(QVector4D(-0.1,0,0,1)), 10);
    if((p1-p2).length()<0.1)
    * /

    if(p4.toVector3DAffine().length()>0.95)
    {
        return -1.4<=q.x() && q.x()<=1.4 && 
               -1.4<=q.y() && q.y()<=1.4 && 
               q.z() > 0.0;
    }
    else return true;

    return  -4.0<=q.x() && q.x()<=4.0 && 
            -4.0<=q.y() && q.y()<=4.0; //  &&
            //-0.5 <= q.z() && q.z() <= 0.9;
    
}
*/


void Fig12Page::build()
{
    // trovo i parametri del dodecaedro con un angolo diedro di pi/2
    double phi = atan(2.0)/2; 
    // atan(2.0) è il supplementare dell'angolo diedro
    // phi è l'angolo COM, dove C=centro faccia, O=centro dod, M=punto medio di un lato
    
    double theta = asin(2.0/(sqrt(3.0)*(1+sqrt(5.0))));
    // sqrt(3.0)*(1+sqrt(5.0)) / 4 è il rapporto fra raggio e lato
    // theta è l'angolo POM, dove P=vertice, O=centro dod, M=punto medio di un lato    
    
    double d1 = H3::getHypotenuseLengthFromAngles(phi, M_PI/4);
    // distanza iperbolica fra il centro del poliedro e il punto medio di uno spigolo

    double d2 = H3::getHypotenuseLengthFromAdjancentCathetus(theta, d1);
    // distanza (iperbolica) fra il centro e un vertice

    const double radius = H3::KModel::getRadius(d2);   

    // edge length è il doppio del cateto con angolo opposto theta
    // uso la formula sin(A) = sinh(opposto) / sinh(ipotenusa)
    const double edgeLength = 2*asinh(sin(theta)*sinh(d2));

    /*

    QVector3D dirs[6] = {
        QVector3D(1,0,0),
        QVector3D(-1,0,0),
        QVector3D(0,1,0),
        QVector3D(0,-1,0),
        QVector3D(0,0,1),
        QVector3D(0,0,-1)
    };
    for(int i=0;i<6;i++)
    {
        QVector3D p = dirs[i] * H3::KModel::getRadius(edgeLength);
        dirMatrices.append(H3::KModel::translation(QVector3D(0,0,0), p));
    }    

    */
    QList<QVector4D> pts;
    QList<QVector<int>> adjVertTb; // vert-index => adjacent vertices indices

    Polyhedron *dod = makeDodecahedron();
    dod->computeFaceVertices();
    QVector3D fc = getFaceCenter(dod, 0);
    QVector3D e1 = -fc.normalized();
    QVector3D e0 = dod->getVertex(dod->getFace(0).m_vertices[0]).m_pos-fc;
    e0 = (e0 - e1*QVector3D::dotProduct(e1,e0)).normalized();
    QVector3D e2 = QVector3D::crossProduct(e0,e1).normalized();
    QMatrix4x4 dodMatrix(
            e0.x(), e0.y(), e0.z(), 0,
            e1.x(), e1.y(), e1.z(), 0,
            e2.x(), e2.y(), e2.z(), 0,
            0,0,0,1
        ); 
    transform(dod, dodMatrix);



    double sc = radius/dod->getVertex(0).m_pos.length();
    int vCount = dod->getVertexCount();

    for(int i=0;i<vCount;i++) 
    {
        QVector4D p = dod->getVertex(i).m_pos * sc;
        p.setW(1.0);
        pts.append(p);
        adjVertTb.append(QVector<int>());
    }
    for(int i=0;i<dod->getEdgeCount();i++)
    {
        const Polyhedron::Edge &edge = dod->getEdge(i);
        int a = edge.m_a, b = edge.m_b;
        // m_edges.append(qMakePair(a,b));
        adjVertTb[a].append(b);
        adjVertTb[b].append(a);
    }

    for(int i=0;i<vCount;i++)
    {
        double qq;

        // traslazione iperbolica: vertice vIndex-esimo => origine
        QMatrix4x4 mat1 = H3::KModel::translation(pts[i].toVector3D(), QVector3D(0,0,0));

        // check!
        qq = mat1.map(pts[i]).toVector3DAffine().length();
        assert(qq<5.0e-7);

        // pp[] <= i tre vertici adiacenti h-traslati attorno all'origine 
        // e normalizzati
        QVector3D pp[3];
        for(int j=0;j<3;j++) 
        {
            int k = adjVertTb[i][j];
            pp[j] = mat1.map(pts[k]).toVector3DAffine().normalized();
        }

        // check!
        for(int i=0;i<3;i++) 
        {
            double pd = QVector3D::dotProduct(pp[i],pp[(i+1)%3]);
            assert(fabs(pd) < 1.0e-6);
        }

        // ortonormalizzo per maggior precisione
        pp[1] = (pp[1] - QVector3D::dotProduct(pp[0],pp[1])*pp[0]).normalized();
        QVector3D pp2 = QVector3D::crossProduct(pp[0],pp[1]).normalized(); 
        qq = QVector3D::dotProduct(pp2, pp[2]);
        // assert(QVector3D::dotProduct(pp2, pp[2])>0.99999);
        pp[2] = pp2;


        // mat : pp[] => x,y,z
        QMatrix4x4 mat(
            pp[0].x(), pp[0].y(), pp[0].z(), 0,
            pp[1].x(), pp[1].y(), pp[1].z(), 0,
            pp[2].x(), pp[2].y(), pp[2].z(), 0,
            0,0,0,1
        ); 
    
        m_vertexMatrices.append((mat * mat1).inverted());
    }

    Polyhedron::Face face = dod->getFace(0);
    QVector3D face0Center;    
    for(int i=0;i<5;i++) face0Center += 0.2 * pts[face.m_vertices[i]].toVector3D();
    m_dodTranslate = H3::KModel::translation(-face0Center, face0Center);
    m_dodTranslate.rotate(36, 0,1,0);

    delete dod;
}


void Fig12Page::draw()
{
    // drawAxes();

    qreal viewArr[16], projArr[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, viewArr);
    glGetDoublev(GL_PROJECTION_MATRIX, projArr);
    QMatrix4x4 view(viewArr), proj(projArr);

    QMatrix4x4 projView = proj.transposed() * view.transposed();

    QMatrix4x4 identity; identity.setToIdentity();


    m_shaderProgram->bind();
    // setViewUniforms(m_shaderProgram);    
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);
    
    QMatrix4x4 globalMatrix; globalMatrix.setToIdentity();

    

    
    m_vertexCube.bind();

    for(int i=0;i<m_vertexMatrices.count();i++)
        draw(m_vertexMatrices[i], m_vertexCube);

    for(int i=0;i<m_vertexMatrices.count();i++)
        draw(m_dodTranslate * m_vertexMatrices[i], m_vertexCube);
    

    m_vertexCube.release();


    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisable(GL_CULL_FACE);
    m_shaderProgram->release();
}


void Fig12Page::savePictures()
{
    Fig12Page page;
    page.m_cameraDistance = m_cameraDistance;
    page.m_theta = m_theta;
    page.m_phi = m_phi;
    page.savePicture("fig12.png", 10);
}
