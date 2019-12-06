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
        int m_edges[30];
    };

    struct Edge {
        struct { int dod, face; } f1, f2;
    };

    struct DodFace {
        QVector4D center;
        int vertices[5], edges[5];
    };
    struct DodEdge {
        QVector4D center;
        QVector4D side[2];
        int va,vb,fa,fb;
    };


    QList<Cell> m_cells;
    QList<Edge> m_edges;
    double m_edgeLength;
    QList<QVector4D> m_pts; // dod vertices
    QList<DodFace> m_dodFaces;
    QList<DodEdge> m_dodEdges;
    QList<QVector<int>> m_adjVertTb; // table : vertex => adjacent vertices

    QList<QMatrix4x4> m_vertexMatrices;
    QList<QMatrix4x4> m_edgeMatrices;
    QMatrix4x4 m_dodTranslate;
    QList<QMatrix4x4> m_otherDodMatrices;
    QList<QVector<int>> m_otherDodEdgeMap; 
    // corrispondenza fra gli edge della faccia 0 del nuovo dod e quelli della faccia i-esima del vecchio


    void build();
    void drawGadgets();

    Polyhedron *createDod(double radius);
    void computePts(Polyhedron *dod);
    void computeAdjacentVerticesTable(Polyhedron *dod);
    void computeVertexMatrices(Polyhedron *dod);
    void computeEdgeMatrices(Polyhedron *dod);
    void computeDodTranslateMatrix(Polyhedron *dod);
    void computeOtherDodMatrices(Polyhedron *dod);
    void computeDodFaces(Polyhedron *dod);
    void computeDodEdges(Polyhedron *dod);
    void computeOtherDodEdgeMap();

    int addCell(int srcCellIndex, int faceIndex);
    void buildGrid();

    void highlightEdge(int cellIndex, int edgeIndex, int type);

    QVector3D toBall(const QVector4D &p) const { return H3::KModel::toBall(p,10); }
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
    Cell *cell = &m_cells.last();
    for(int i=0;i<30;i++)
    {
        const DodEdge &dodEdge = m_dodEdges.at(i);
        cell->m_edges[i] = m_edges.count();
        Edge edge;
        edge.f1.dod = edge.f2.dod = 0;
        edge.f1.face = dodEdge.fa;
        edge.f2.face = dodEdge.fb;
        m_edges.append(edge);
    }

    for(int i=0;i<12;i++)
    {
        addCell(0,i);
        continue;
        cell = &m_cells[1+i];
        for(int j=0;j<5;j++)
        {
            int j1 = m_dodFaces[0].edges[j]; // il j1-esimo edge del nuovo dod e il j-esimo edge della faccia 0
            int j2 = m_otherDodEdgeMap[i][j1]; // e corrisponde al j2-esimo edge del vecchio dod

            QVector3D uff1 = toBall(cell->m_hMatrix.map(m_dodEdges[j1].center));
            QVector3D uff2 = toBall(m_cells[0].m_hMatrix.map(m_dodEdges[j2].center));
            double ufflength = (uff1-uff2).length();
            Q_ASSERT(ufflength < 1e-6);

            int j3 = m_cells[0].m_edges[j2];
            cell->m_edges[j1] = j3;
            const DodEdge dodEdge = m_dodEdges[j1];
            Q_ASSERT(dodEdge.fa == 0 ||  dodEdge.fb == 0);
            int otherFace = m_dodEdges[j1].fa == 0 ? m_dodEdges[j1].fb : m_dodEdges[j1].fa;
            Q_ASSERT(otherFace != 0);
            Edge &edge = m_edges[j3];
            if(edge.f1.dod == 0 && edge.f1.face == i)
            {
                edge.f1.dod = i+1;
                edge.f1.face = otherFace;
            }
            else if(edge.f2.dod == 0 && edge.f2.face == i)
            {
                edge.f2.dod = i+1;
                edge.f2.face = otherFace;
            }
            else
            {
                // Q_ASSERT(false);
            }
        }
    }

    //addCell(1,6);
    // addCell(0,0);
    //addCell(1,0);
}


void MyGrid::drawGadgets()
{
    Color colors[5] = {Color(1,0,0), Color(0,1,0), Color(0,0,1), Color(0,1,1), Color(1,0,1) };

    int i = 4;

    for(int j=0;j<5;j++)
    {
        QVector3D p = toBall(m_cells[i+1].m_hMatrix.map(m_dodEdges[m_dodFaces[0].edges[j]].center));
        setColor(colors[j]);
        drawSphere(p, 0.5);

        int j1 = m_dodFaces[i].edges[m_otherDodEdgeMap[i][m_dodFaces[0].edges[j]]];

        p = toBall(m_dodEdges[j1].side[0]);
        
        drawSphere(p, 0.4);
    }

    /*


    QVector4D p = m_otherDodMatrices[0].map(m_dodEdges[m_dodFaces[0].edges[0]].center);
    setColor(1,0,0);
    drawSphere(toBall(p), 0.5);

    p = m_dodEdges[m_dodFaces[0].edges[1]].center;
    setColor(0,1,0);
    drawSphere(toBall(p), 0.5);
    */

    /*
    int cellIndex = 0;
    int faceIndex = 5;
    setColor(1,0,0);
    highlightEdge(cellIndex, m_dodFaces[faceIndex].edges[0], 0);
    setColor(0,1,0);
    highlightEdge(cellIndex, m_dodFaces[faceIndex].edges[1], 0);
    setColor(0,0,1);
    highlightEdge(cellIndex, m_dodFaces[faceIndex].edges[2], 0);
    */
}

void MyGrid::highlightEdge(int cellIndex, int edgeIndex, int type)
{
    if(type == 0)
    {
        QVector4D pm = m_dodEdges[edgeIndex].center;    
        QVector3D p = H3::KModel::toBall(m_cells[cellIndex].m_hMatrix.map(pm),10.0);
        drawSphere(p, 0.5);
    }
    else
    {
        for(int j=0;j<2;j++)
        {
            QVector4D pm = m_dodEdges[edgeIndex].side[j];    
            QVector3D p = H3::KModel::toBall(m_cells[cellIndex].m_hMatrix.map(pm),10.0);
            drawSphere(p, 0.5);
        }
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
    for(int i=0; i<12; i++)
    {
        QMatrix4x4 rot = getFaceMatrix(dod, i) * getFaceMatrix(dod, 5).inverted();
        m_otherDodMatrices.append(rot * m_dodTranslate);
    }
}

void MyGrid::computeDodFaces(Polyhedron *dod)
{
    for(int i=0; i<dod->getFaceCount(); i++)
    {
        QVector4D c;
        const Polyhedron::Face &face = dod->getFace(i);
        for(int j=0;j<(int)face.m_vertices.size(); j++) 
            c += m_pts[face.m_vertices[j]];
        c *= 1.0/face.m_vertices.size();
        DodFace dodFace;
        dodFace.center = c;
        for(int j=0;j<5;j++)
        {
            dodFace.edges[j] = face.m_edges[j];
            dodFace.vertices[j] = face.m_vertices[j];
        }
        m_dodFaces.append(dodFace);

        // check
        //   vertici consecutivi e sempre nello stesso verso rispetto al vettore centrodod -> centrofaccia
        //   l'edge i-esimo collega il vertice i-esimo con il (i+1)-esimo
        for(int j=0;j<5;j++)
        {
            int v0 = dodFace.vertices[j];
            int v1 = dodFace.vertices[(j+1)%5];

            QVector3D p0 = m_pts[v0].toVector3D();
            QVector3D p1 = m_pts[v1].toVector3D();
            QVector3D w = QVector3D::crossProduct(p0, p0 - c.toVector3D());
            double pd = QVector3D::dotProduct(w, p1-p0);
            assert(pd>0.0);
            const Polyhedron::Edge &edge = dod->getEdge(dodFace.edges[j]);
            assert(edge.m_a == v0 && edge.m_b == v1 || edge.m_a == v1 && edge.m_b == v0);
        }
    }
}

void MyGrid::computeDodEdges(Polyhedron *dod)
{
    double t = 0.4;
    for(int i=0;i<dod->getEdgeCount();i++)
    {
        const Polyhedron::Edge &edge = dod->getEdge(i);
        DodEdge dodEdge;
        dodEdge.va = edge.m_a;
        dodEdge.vb = edge.m_b;
        dodEdge.fa = dodEdge.fb = -1;
        QVector4D pa = m_pts[dodEdge.va];
        QVector4D pb = m_pts[dodEdge.vb];

        dodEdge.center = 0.5*(pa+pb);
        dodEdge.side[0] = (1-t)*pa+t*pb;
        dodEdge.side[1] = (1-t)*pb+t*pa;
        m_dodEdges.append(dodEdge);
    }
    for(int i=0; i<dod->getFaceCount(); i++)
    {
        const Polyhedron::Face &face = dod->getFace(i);
        for(int j=0;j<5;j++)
        {
            DodEdge &dodEdge = m_dodEdges[face.m_edges[j]];
            if(dodEdge.fa == -1) dodEdge.fa = i;
            else dodEdge.fb = i;
        }
    }
    for(int i=0;i<dod->getEdgeCount();i++)
    {
        const Polyhedron::Edge &edge = dod->getEdge(i);
        Q_ASSERT(edge.m_a>=0);
        Q_ASSERT(edge.m_b>=0);

    }

}

void MyGrid::computeOtherDodEdgeMap()
{
    for(int i=0;i<m_otherDodMatrices.count();i++)
    {
        QMatrix4x4 matrix = m_otherDodMatrices[i];
        QVector<int> ee;
        for(int j=0;j<5;j++)
        {
            QVector4D p = matrix.map(m_dodEdges[m_dodFaces[0].edges[j]].center);
            double minDist = 0;
            int foundj = -1;
            for(int j2=0;j2<5;j2++)
            {
               QVector4D p2 = m_dodEdges[m_dodFaces[i].edges[j2]].center;
               double dist = (p2-p).length();
               if(foundj<0 || dist < minDist) { foundj = j2; minDist = dist;}
            }
            assert(foundj>=0 && minDist < 1e-6);
            assert(!ee.contains(foundj));
            ee.append(foundj);
        }
        m_otherDodEdgeMap.append(ee);
    }
}



void MyGrid::build()
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
    m_edgeLength = edgeLength;
    

    Polyhedron *dod = createDod(radius);
    dod->computeFaceVertices();
    computePts(dod);
    computeAdjacentVerticesTable(dod);
    computeVertexMatrices(dod);
    computeEdgeMatrices(dod);
    computeDodTranslateMatrix(dod);
    computeOtherDodMatrices(dod);
    computeDodFaces(dod);
    computeDodEdges(dod);
    computeOtherDodEdgeMap();

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
        QVector4D p = m_grid->m_cells[0].m_hMatrix.map(m_grid->m_dodFaces[i].center);
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
