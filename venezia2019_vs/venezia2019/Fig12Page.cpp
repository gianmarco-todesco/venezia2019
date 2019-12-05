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


class MyGrid {
public:
    MyGrid();
    ~MyGrid();

    struct Cell {
        QMatrix4x4 m_hMatrix;
    };

    struct Edge {
    };

    QList<Cell> m_cells;
    double m_edgeLength;
    QList<QVector4D> m_pts; // dod vertices
    QList<QVector4D> m_faceCenters;
    QList<QVector<int>> m_adjVertTb; // table : vertex => adjacent vertices

    QList<QMatrix4x4> m_vertexMatrices;
    QList<QMatrix4x4> m_edgeMatrices;
    QMatrix4x4 m_dodTranslate;
    QList<QMatrix4x4> m_otherDodMatrices;
    QList<QPair<int, int> > m_edges;

    void build();
    void drawGadgets();

    Polyhedron *createDod(double radius);
    void computePts(Polyhedron *dod);
    void computeAdjacentVerticesTable(Polyhedron *dod);
    void computeVertexMatrices(Polyhedron *dod);
    void computeEdgeMatrices(Polyhedron *dod);
    void computeDodTranslateMatrix(Polyhedron *dod);
    void computeOtherDodMatrices(Polyhedron *dod);
    void computeFaceCenters(Polyhedron *dod);

    int addCell(int srcCellIndex, int faceIndex);
    void buildGrid();

    void highlightEdge(int cellIndex, int edgeIndex, int type);

};


MyGrid::MyGrid()
{
}

MyGrid::~MyGrid()
{
}


void MyGrid::buildGrid()
{
    // create first cell
    m_cells.append(Cell());
    m_cells.last().m_hMatrix.setToIdentity();


    addCell(0,5);
    addCell(1,6);
    // addCell(0,0);
    //addCell(1,0);
}


void MyGrid::drawGadgets()
{

}

void MyGrid::highlightEdge(int cellIndex, int edgeIndex, int type)
{
    QVector4D pa = m_pts[m_edges[edgeIndex].first];
    QVector4D pb = m_pts[m_edges[edgeIndex].second];

    QVector4D pm;
    QVector3D p;
    if(type == 0)
    {
        pm = 0.5*(pa+pb);    
        p = H3::KModel::toBall(m_cells[cellIndex].m_hMatrix.map(pm),10.0);
        setColor(1,1,0);
        drawSphere(p, 0.5);
    }
    else
    {
        double t = 0.4;
        pm = (1-t)*pa + t*pb;
        p = H3::KModel::toBall(m_cells[cellIndex].m_hMatrix.map(pm),10.0);
        setColor(1,0,0);
        drawSphere(p, 0.5);
        pm = (1-t)*pb + t*pa;
        p = H3::KModel::toBall(m_cells[cellIndex].m_hMatrix.map(pm),10.0);
        setColor(1,0,0);
        drawSphere(p, 0.5);
    }

}

int MyGrid::addCell(int srcCellIndex, int faceIndex)
{
    const QMatrix4x4 &srcMatrix = m_cells[srcCellIndex].m_hMatrix;
    Cell cell;
    cell.m_hMatrix = srcMatrix * m_otherDodMatrices[faceIndex];
    int index = m_cells.count();
    m_cells.append(cell);
    return index;
}



Polyhedron *MyGrid::createDod(double radius)
{
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
    for(int i=0; i<dod->getVertexCount(); i++)
    {
        dod->getVertex(i).m_pos *= sc;
    }

    return dod;
}

void MyGrid::computePts(Polyhedron *dod)
{
    m_pts.clear();
    int vCount = dod->getVertexCount();
    for(int i=0;i<vCount;i++) 
    {
        QVector4D p = dod->getVertex(i).m_pos;
        p.setW(1.0);
        m_pts.append(p);
    }
}

void MyGrid::computeAdjacentVerticesTable(Polyhedron *dod)
{
    m_adjVertTb.clear();
    int vCount = dod->getVertexCount();
    for(int i=0;i<vCount;i++) 
    {
        m_adjVertTb.append(QVector<int>());
    }
    for(int i=0;i<dod->getEdgeCount();i++)
    {
        const Polyhedron::Edge &edge = dod->getEdge(i);
        int a = edge.m_a, b = edge.m_b;
        m_adjVertTb[a].append(b);
        m_adjVertTb[b].append(a);
    }
}

void MyGrid::computeVertexMatrices(Polyhedron *dod)
{
    int vCount = dod->getVertexCount();
    for(int i=0;i<vCount;i++)
    {
        double qq;

        // traslazione iperbolica: vertice vIndex-esimo => origine
        QMatrix4x4 mat1 = H3::KModel::translation(m_pts[i].toVector3D(), QVector3D(0,0,0));

        // check!
        qq = mat1.map(m_pts[i]).toVector3DAffine().length();
        assert(qq<5.0e-7);

        // pp[] <= i tre vertici adiacenti h-traslati attorno all'origine 
        // e normalizzati
        QVector3D pp[3];
        for(int j=0;j<3;j++) 
        {
            int k = m_adjVertTb[i][j];
            pp[j] = mat1.map(m_pts[k]).toVector3DAffine().normalized();
        }

        // check!
        for(int j=0;j<3;j++) 
        {
            double pd = QVector3D::dotProduct(pp[j],pp[(j+1)%3]);
            assert(fabs(pd) < 1.0e-6);
        }

        // ortonormalizzo per maggior precisione
        pp[1] = (pp[1] - QVector3D::dotProduct(pp[0],pp[1])*pp[0]).normalized();
        QVector3D pp2 = QVector3D::crossProduct(pp[0],pp[1]).normalized(); 
        if(QVector3D::dotProduct(pp2, pp[2])<0)
        {
            qSwap(m_adjVertTb[i][1], m_adjVertTb[i][2]);
            qSwap(pp[1], pp[2]);
            pp2 = QVector3D::crossProduct(pp[0],pp[1]).normalized(); 
        }
        assert(QVector3D::dotProduct(pp2, pp[2])>0.99999);
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
}

void MyGrid::computeEdgeMatrices(Polyhedron *dod)
{
    int vCount = dod->getVertexCount();
    QMatrix4x4 rots[3];
    rots[0].rotate(-90,0,0,1);
    rots[1].setToIdentity();
    rots[2].rotate( 90,1,0,0);
    QSet<QPair<int, int> > touchedEdges;
    for(int a=0;a<vCount;a++)
    {
        QMatrix4x4 mat = m_vertexMatrices[a];
        for(int j=0; j<3; j++)
        {
            int b = m_adjVertTb[a][j];
            QPair<int,int> edge(a,b);
            if(touchedEdges.contains(edge)) continue;
            touchedEdges.insert(edge);
            touchedEdges.insert(QPair<int,int>(b,a));
            m_edgeMatrices.append(mat*rots[j]);
        }
    }
}

void MyGrid::computeDodTranslateMatrix(Polyhedron *dod)
{
    Polyhedron::Face face = dod->getFace(0);
    QVector3D face0Center;    
    for(int i=0;i<5;i++) face0Center += 0.2 * m_pts[face.m_vertices[i]].toVector3D();
    m_dodTranslate = H3::KModel::translation(face0Center, -face0Center);
    m_dodTranslate.rotate(36, 0,1,0);
}

void MyGrid::computeOtherDodMatrices(Polyhedron *dod)
{
    m_otherDodMatrices.clear();
    m_otherDodMatrices.append(m_dodTranslate);
    for(int i=1; i<12; i++)
    {
        QMatrix4x4 rot = getFaceMatrix(dod, i) * getFaceMatrix(dod, 5).inverted();
        m_otherDodMatrices.append(rot * m_dodTranslate);
    }
}

void MyGrid::computeFaceCenters(Polyhedron *dod)
{
    for(int i=0; i<dod->getFaceCount(); i++)
    {
        QVector4D c;
        const Polyhedron::Face &face = dod->getFace(i);
        for(int j=0;j<(int)face.m_vertices.size(); j++) 
            c += m_pts[face.m_vertices[j]];
        c *= 1.0/face.m_vertices.size();
        m_faceCenters.append(c);
    }
}


void MyGrid::build()
{
   // trovo i parametri del dodecaedro con un angolo diedro di pi/2
    double phi = atan(2.0)/2; 
    // atan(2.0) � il supplementare dell'angolo diedro
    // phi � l'angolo COM, dove C=centro faccia, O=centro dod, M=punto medio di un lato
    
    double theta = asin(2.0/(sqrt(3.0)*(1+sqrt(5.0))));
    // sqrt(3.0)*(1+sqrt(5.0)) / 4 � il rapporto fra raggio e lato
    // theta � l'angolo POM, dove P=vertice, O=centro dod, M=punto medio di un lato    
    
    double d1 = H3::getHypotenuseLengthFromAngles(phi, M_PI/4);
    // distanza iperbolica fra il centro del poliedro e il punto medio di uno spigolo

    double d2 = H3::getHypotenuseLengthFromAdjancentCathetus(theta, d1);
    // distanza (iperbolica) fra il centro e un vertice

    const double radius = H3::KModel::getRadius(d2);   

    // edge length � il doppio del cateto con angolo opposto theta
    // uso la formula sin(A) = sinh(opposto) / sinh(ipotenusa)
    const double edgeLength = 2*asinh(sin(theta)*sinh(d2));
    m_edgeLength = edgeLength;
    

    Polyhedron *dod = createDod(radius);
    dod->computeFaceVertices();
    computePts(dod);
    computeAdjacentVerticesTable(dod);
    computeVertexMatrices(dod);
    computeEdgeMatrices(dod);
    computeDodTranslateMatrix(dod);
    computeOtherDodMatrices(dod);
    computeFaceCenters(dod);

    for(int i=0;i<dod->getEdgeCount();i++)
    {
        const Polyhedron::Edge &edge = dod->getEdge(i);
        m_edges.append(qMakePair(edge.m_a, edge.m_b));
    }

    delete dod;

    buildGrid();
}



Fig12Page::Fig12Page()
: m_theta(10)
, m_phi(20)
, m_cameraDistance(15)
, m_rotating(true)
, m_shaderProgram(0)
, m_foo(0)
, m_grid(new MyGrid())
{
    m_hMatrix.setToIdentity();
    build();
    m_grid->build();
}

Fig12Page::~Fig12Page()
{
    delete m_grid;
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

  makeEdgeBox(m_edgeBox, 10);
  makeEdgeBox(m_edgeBoxLow, 1);
  
  makeDodMesh();

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


void Fig12Page::makeEdgeBox(Mesh &mesh, int n)
{
    double d = 0.05;
    double edgeLength = m_grid->m_edgeLength;    
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


    qreal viewArr[16], projArr[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, viewArr);
    glGetDoublev(GL_PROJECTION_MATRIX, projArr);
    QMatrix4x4 view(viewArr), proj(projArr);
    QMatrix4x4 projView = proj.transposed() * view.transposed();

    m_shaderProgram->bind();
    // setViewUniforms(m_shaderProgram);    
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);
    
    draw3();    

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisable(GL_CULL_FACE);
    m_shaderProgram->release();

    drawGadgets();

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
    if(e->key() == Qt::Key_D) { m_foo = (m_foo+1)%12; }
    else if(e->key() == Qt::Key_A) { m_foo = (m_foo+11)%12; }
    else 
      e->ignore();
}


void Fig12Page::build()
{
 
}

void Fig12Page::makeDodMesh()
{
    for(int i=0;i<m_grid->m_edgeMatrices.count();i++)
        m_dodMesh.hMerge(m_edgeBox, m_grid->m_edgeMatrices[i]);
    m_dodMesh.createBuffers();
}



void Fig12Page::draw1()
{
}




void Fig12Page::draw2()
{
 
}

void Fig12Page::draw3()
{
    m_dodMesh.bind();
    for(int i=0;i<m_grid->m_cells.size();i++)
    {
        draw(m_grid->m_cells[i].m_hMatrix, m_dodMesh);
    }
    m_dodMesh.release();


}


void Fig12Page::drawGadgets()
{
    for(int i=0; i<12; i++)
    {
        QVector4D p = m_grid->m_cells[1].m_hMatrix.map(m_grid->m_faceCenters[i]);
        QVector3D p3 = H3::KModel::toBall(p, 10);
        v()->drawText(p3, QString("F%1").arg(i));

    }

    m_grid->drawGadgets();
}


void Fig12Page::savePictures()
{
    Fig12Page page;
    page.m_cameraDistance = m_cameraDistance;
    page.m_theta = m_theta;
    page.m_phi = m_phi;
    page.savePicture("fig12.png", 10);
}
