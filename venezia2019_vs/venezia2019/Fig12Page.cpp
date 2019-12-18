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
#include <QPainter>


#include "Viewer.h"


class MyGrid {
public:
    MyGrid(int n);
    ~MyGrid();


    struct Cell {
        QMatrix4x4 m_hMatrix;
        int m_gridEdges[30];
    };

    struct GridFace {
    public:
        int dod, face;
        GridFace() : dod(-1), face(-1) {}
        GridFace(int _dod, int _face) : dod(_dod), face(_face) {}
        bool operator==(const GridFace &other) const { return dod==other.dod && face==other.face; }
    };

    class GridEdge {
    public:
        int m;
        QVector3D pos;
        double r;
        GridFace f1, f2;
        GridEdge() : m(0), r(0) {  }
        bool hasFace(const GridFace &face) const { return face == f1 || face == f2; }
        
        void addCell(int cellIndex, int cellFace, const GridFace &oldFace) {
            m++;
            Q_ASSERT(cellIndex>=0 && cellIndex != f1.dod && cellIndex != f2.dod);
            if(oldFace == f1) f1 = GridFace(cellIndex, cellFace);
            else if(oldFace == f2) f2 = GridFace(cellIndex, cellFace);
            else { Q_ASSERT(oldFace == f1 || oldFace == f2); }
        }
    };

    struct DodFace {
        QVector4D center;
        int vertices[5], edges[5];
    };
    class DodEdge {
    public:
        QVector4D center;
        QVector4D side[2];
        int va,vb,fa,fb;
        DodEdge() : va(-1), vb(-1), fa(-1), fb(-1) {}

        int getOtherFace(int face) const {
            Q_ASSERT(fa>=0 && fb>=0 && fa!=fb);
            if(face == fa) return fb;
            else if(face == fb) return fa;
            else 
            {
                Q_ASSERT(face == fa || face == fb);
                return -1;
            }
        }
    };


    int cellCount;
    QList<Cell> m_cells;
    QList<GridEdge> m_gridEdges;
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

    QList<QPair<QVector3D, int>> m_balls;

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

    void createFirstCell();
    int addCell(int srcCellIndex, int faceIndex);
    int createNewGridEdge(int cellIndex, int edgeIndex);
    void joinFaces(int cellIndex, int gridEdgeIndex);
    void buildGrid();

    void highlightEdge(int cellIndex, int edgeIndex, int type);
    int getEdge3();
    int getEdge2();

    QVector3D toBall(const QVector4D &p) const { return H3::KModel::toBall(p,10); }
};


MyGrid::MyGrid(int n)
    : cellCount(n)
{
}

MyGrid::~MyGrid()
{
}



void MyGrid::buildGrid()
{
    createFirstCell();
    for(int i=0;i<12;i++)
        addCell(0,i);

    while(m_cells.count() < cellCount)
    {
        int k = -1;
        double rmin = 1e80;
        for(int i=0;i<m_gridEdges.count();i++)
        {
            if(m_gridEdges[i].m>=4) continue;

           /*
            QVector3D p = m_gridEdges[i].pos;
            double x = p.normalized().x();
            if(x<0.8) continue;
            */
            double r =  m_gridEdges[i].r;
            if(k<0 || r<rmin)
            {
                rmin = r;
                k = i;
            }
        }
        Q_ASSERT(k>=0);
        double r = m_gridEdges[k].r;
        addCell(m_gridEdges[k].f1.dod, m_gridEdges[k].f1.face);

    }

    /*
    addCell(0,1);
    addCell(0,2);
    addCell(0,7);
    */

    /*
    QList<int> ii;
    for(int i=0; i<m_gridEdges.count();i++)
    {
        if(m_gridEdges[i].m == 3) ii.append(i);
    }
    foreach(int i, ii)
    {
        addCell(m_gridEdges[i].f1.dod, m_gridEdges[i].f1.face);
    }
    for(;;)
    {
        int i = getEdge3();
        if(i<0) break;
        addCell(m_gridEdges[i].f1.dod, m_gridEdges[i].f1.face);
    }
    for(;;)
    {
        int i = getEdge2();
        if(i<0) break;
        addCell(m_gridEdges[i].f1.dod, m_gridEdges[i].f1.face);
        break;
    }
    for(;;)
    {
        int i = getEdge3();
        if(i<0) break;
        addCell(m_gridEdges[i].f1.dod, m_gridEdges[i].f1.face);
    }
    */


    //for(int i=0;i<12;i++) addCell(0,i);
    //addCell(1+4, 6);
    //addCell(1+4, 9);
}


void MyGrid::createFirstCell()
{
    m_cells.clear();
    m_cells.append(Cell());
    Cell *cell = &m_cells.last();
    cell->m_hMatrix.setToIdentity();
    for(int i=0;i<30;i++) 
    {
        cell->m_gridEdges[i] = createNewGridEdge(0, i);
    }
}


int MyGrid::addCell(int srcCellIndex, int srcFaceIndex)
{
    int newCellIndex = m_cells.count();
    m_cells.append(Cell());
    Cell *cell = &m_cells.last();
    QMatrix4x4 srcCellMatrix = m_cells[srcCellIndex].m_hMatrix;
    cell->m_hMatrix = srcCellMatrix * m_otherDodMatrices[srcFaceIndex];
    for(int i=0;i<30;i++) cell->m_gridEdges[i] = -1;

    // get src cell grid edges along the face
    for(int j=0;j<5;j++)
    {
        int dodEdgeIndex = m_dodFaces[0].edges[j]; 
        int tmp = m_otherDodEdgeMap[srcFaceIndex][j]; 
        int srcDodEdgeIndex = m_dodFaces[srcFaceIndex].edges[tmp];
        
        QVector3D uff1 = toBall(cell->m_hMatrix.map(m_dodEdges[dodEdgeIndex].center));
        QVector3D uff2 = toBall(srcCellMatrix.map(m_dodEdges[srcDodEdgeIndex].center));


        //QVector3D cellCenter = toBall(cell->m_hMatrix.map(QVector3D(0,0,0)));
        //QVector3D uff1 = cellCenter * 0.1 + toBall(cell->m_hMatrix.map(m_dodEdges[dodEdgeIndex].center)) * 0.9;
        //m_balls.append(qMakePair(uff1, j));
        //m_balls.append(qMakePair(uff2, j));

        double ufflength = (uff1-uff2).length();
        Q_ASSERT(ufflength < 5e-3);

        // add the new dod to the grid edge

        const DodEdge &dodEdge = m_dodEdges[dodEdgeIndex];
        int otherFace = dodEdge.getOtherFace(0);

        int gridEdgeIndex = cell->m_gridEdges[dodEdgeIndex] = m_cells[srcCellIndex].m_gridEdges[srcDodEdgeIndex];
        GridEdge &gridEdge = m_gridEdges[gridEdgeIndex];
        gridEdge.addCell(newCellIndex, otherFace, GridFace(srcCellIndex, srcFaceIndex));
        
    }

    // join faces
    for(int j=0;j<5;j++)
    {
        int dodEdgeIndex = m_dodFaces[0].edges[j]; 
        int gridEdgeIndex = cell->m_gridEdges[dodEdgeIndex];
        Q_ASSERT(gridEdgeIndex>=0);
        GridEdge &gridEdge = m_gridEdges[gridEdgeIndex];
        if(gridEdge.m == 4) 
            joinFaces(newCellIndex, gridEdgeIndex);
    }

    // create the new grid edges
    for(int i=0;i<30;i++)
    {
        if(cell->m_gridEdges[i]<0) cell->m_gridEdges[i] = createNewGridEdge(newCellIndex, i);
    }

    return newCellIndex;
}


int MyGrid::createNewGridEdge(int cellIndex, int edgeIndex)
{
    int index = m_gridEdges.count();
    m_gridEdges.append(GridEdge());
    GridEdge &gridEdge = m_gridEdges.last();
    gridEdge.f1 = GridFace(cellIndex,m_dodEdges[edgeIndex].fa);
    gridEdge.f2 = GridFace(cellIndex,m_dodEdges[edgeIndex].fb);
    gridEdge.m = 1;
    gridEdge.pos = toBall(m_cells[cellIndex].m_hMatrix.map(m_dodEdges[edgeIndex].center));
    gridEdge.r = gridEdge.pos.length();
    return index;
}

void MyGrid::joinFaces(int cellIndex, int gridEdgeIndex)
{
    GridEdge &gridEdge = m_gridEdges[gridEdgeIndex];
    Q_ASSERT(gridEdge.m == 4);
    Q_ASSERT(gridEdge.f1.dod == cellIndex || gridEdge.f2.dod == cellIndex);
    GridFace gridFaces[2] = {gridEdge.f1, gridEdge.f2};
    if(gridFaces[0].dod != cellIndex)
    {
        qSwap(gridFaces[0], gridFaces[1]);
    }
    int ee[2][5];
    for(int i=0; i<2; i++)
    {
        const int dod = gridFaces[i].dod;
        const int face = gridFaces[i].face;
        int k = -1;
        for(int j=0; j<5; j++)
        {
            int t = m_dodFaces[face].edges[j];
            if(t>=0 && m_cells[dod].m_gridEdges[t] == gridEdgeIndex)
            {
                Q_ASSERT(k<0);
                k = j;
            }
        }
        Q_ASSERT(k>=0);
        for(int j=0; j<5; j++)
        {
            int j2 = i == 0 ? (j+k)%5 :  (5-j+k)%5;
            ee[i][j] = m_dodFaces[face].edges[j2];
        }
    }
    for(int i=0; i<2; i++)
    {
        const int dod = gridFaces[i].dod;
        QVector3D dodCenter = toBall(m_cells[dod].m_hMatrix.map(QVector4D(0,0,0,1)));
        for(int j=0;j<5;j++)
        {
            QVector3D uff1 = toBall(m_cells[dod].m_hMatrix.map(m_dodEdges[ee[i][j]].center));
            // m_balls.append(qMakePair(dodCenter * 0.1 + uff1 * 0.9, j));
        }
    }
    for(int j=0;j<5; j++)
    {
        Q_ASSERT(m_cells[gridFaces[1].dod].m_gridEdges[ee[1][j]]>=0);
    }
    Q_ASSERT(gridFaces[0].dod == cellIndex);
    Cell *cell = &m_cells[cellIndex];
    Cell *otherCell = &m_cells[gridFaces[1].dod];
    for(int j=0;j<5; j++)
    {
        if(cell->m_gridEdges[ee[0][j]]<0)
        {
            int gridEdgeIndex = otherCell->m_gridEdges[ee[1][j]]; 
            cell->m_gridEdges[ee[0][j]] = gridEdgeIndex;

            const DodEdge &dodEdge = m_dodEdges[ee[0][j]];
            int otherFace = dodEdge.getOtherFace(gridFaces[0].face);

            m_gridEdges[gridEdgeIndex].addCell(cellIndex, otherFace, gridFaces[1]);
        }
    }

}


int MyGrid::getEdge3()
{
    for(int i=0; i<m_gridEdges.count();i++)
        if(m_gridEdges[i].m == 3)
            return i;
    return -1;
}

int MyGrid::getEdge2()
{
    for(int i=0; i<m_gridEdges.count();i++)
        if(m_gridEdges[i].m == 2)
            return i;
    return -1;
}


void MyGrid::drawGadgets()
{

    Color colors[5] = {Color(0.5,0,0), Color(0,0.5,0), Color(0,0,0.5), Color(0,0.5,0.5), Color(0.5,0,0.5) };

    glDisable(GL_LIGHTING);
    glColor3d(1,0,1);
    for(int i=0;i<m_dodEdges.count();i++)
    {
        int va = m_dodEdges[i].va;
        int vb = m_dodEdges[i].vb;
        QVector4D pa = m_pts[m_dodEdges[i].va];
        QVector4D pb = m_pts[m_dodEdges[i].vb];
        int m = 20;
        glBegin(GL_LINE_STRIP);
        for(int j=0;j<m;j++)
        {
            double t = (double)j/(double)(m-1);
            QVector3D p = toBall((1-t)*pa + t*pb);
            glVertex(p);
        }

        glEnd();
    }

    glEnable(GL_LIGHTING);

    /*
    for(int i=0;i<m_gridEdges.count();i++)
    {
        drawSphere(m_gridEdges[i].pos,0.1);
    }
    */
    return;

    QSet<int> touched;
    for(int i=0;i<m_cells.count();i++)
    {
        for(int j=0;j<30;j++)
        {
            int gridEdgeIndex = m_cells[i].m_gridEdges[j];
            if(gridEdgeIndex<0) continue;
            if(touched.contains(gridEdgeIndex)) continue;
            touched.insert(gridEdgeIndex);
            const GridEdge &gridEdge = m_gridEdges[gridEdgeIndex];
            int m = gridEdge.m;
            Q_ASSERT(0<=m && m<=4);
            if(m>2)
            {

                setColor(colors[m]);
                QVector3D p = toBall(m_cells[i].m_hMatrix.map(m_dodEdges[j].center));
                drawSphere(p,0.1);
            }

            if(false && m<4)
            {
                QVector3D p;
                const double t = 0.5;
                QVector3D p1 = toBall(m_cells[gridEdge.f1.dod].m_hMatrix.map(m_dodFaces[gridEdge.f1.face].center));
                QVector3D p2 = toBall(m_cells[gridEdge.f2.dod].m_hMatrix.map(m_dodFaces[gridEdge.f2.face].center));

                glDisable(GL_LIGHTING);
                glBegin(GL_LINE_STRIP);
                glColor3d(1,0,1);
                glVertex(p1*t+p*(1-t)); 
                glVertex(p);
                glVertex(p2*t+p*(1-t));    
                glEnd();
                glEnable(GL_LIGHTING);
            }
        }
    }

    for(int i=0;i<m_balls.count();i++)
    {
        setColor(colors[m_balls[i].second]);
        drawSphere(m_balls[i].first, 0.2);
    }
    /*
    
    for(int j=0;j<5;j++)
    {
        QVector3D p = toBall(m_cells[i+1].m_hMatrix.map(m_dodEdges[m_dodFaces[0].edges[j]].center));
        setColor(colors[j]);
        drawSphere(p, 0.5);

        int j1 = m_dodFaces[i].edges[m_otherDodEdgeMap[i][m_dodFaces[0].edges[j]]];

        p = toBall(m_dodEdges[j1].side[0]);
        
        drawSphere(p, 0.4);
    }
    */

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
    rots[0].rotate(-90, 0,0,1);
    rots[0].rotate(-90, 0,1,0); 
    rots[1].setToIdentity();
    rots[2].rotate( 90,1,0,0);
    rots[2].rotate(90, 0,1,0); 
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



Fig12Page::Fig12Page(int cellCount)
: m_theta(-20)
, m_phi(41.5)
, m_cameraDistance(5.2) // 25
, m_rotating(true)
, m_shaderProgram(0)
, m_foo(0)
, m_grid(new MyGrid(50000)) // cellCount))
, m_textureId(0)
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
 
  // m_sphere.makeSphere(0.3,10,10);  
  // m_vertexCube.makeCube(0.1,8);

  makeVertexBox(m_vertexBox, 10);
  makeEdgeBox(m_edgeBox, 10);
  makeEdgeBox(m_edgeBoxLow, 1);
  
  makeDodMesh();

  createTextures();
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


void Fig12Page::makeVertexBox(Mesh &mesh, int n)
{
    const QRectF uvRect(11.0/1024.0, 10.0/1024.0, (502.-12.)/1024.0, (502.-12.0)/1024.0);
    double r = 0.2;
    mesh.m_hasTexCoords = true;
    mesh.addFace(QVector3D(r,0,0), QVector3D(0,r/4,0), QVector3D(0,0,r/4), 5, 5, uvRect);
    mesh.addFace(QVector3D(0,r,0), QVector3D(0,0,r/4), QVector3D(r/4,0,0), 5, 5, uvRect);
    mesh.addFace(QVector3D(0,0,r), QVector3D(r/4,0,0), QVector3D(0,r/4,0), 5, 5, uvRect);
    mesh.createBuffers();
}


void Fig12Page::makeEdgeBox(Mesh &mesh, int n)
{
    double d = 0.07;
    mesh.m_hasTexCoords = true;
    double edgeLength = m_grid->m_edgeLength;    
    QVector4D base[3] = {
        QVector4D( d, 0, 0, 1.0),
        QVector4D( d, 0, d, 1.0),
        QVector4D( 0, 0, d, 1.0)
    };
    QVector<QVector4D> pts;
    for(int i=0; i<=2*n; i++) 
    {
        double hz = 0.5*edgeLength*(1+ 0.62 * (double)(i-n)/(double)n) ;
        QMatrix4x4 tr = H3::KModel::translation(
            QVector3D(0,0,0), 
            QVector3D(0,H3::KModel::getRadius(hz), 0));
        for(int j=0; j<3; j++) pts.append(tr.map(base[j]));
    }
    for(int i=0;i<pts.count();i++) {
        pts[i] *= 1.0/pts[i].w();
    }
    const QVector3D norms[2] = {
        QVector3D(1,0,0),
        QVector3D(0,0,1)
    };
    double u0 = (512.0+10.0)/1024.0, u1 = (1024.0-10.0)/1024.0, v0 = 10.0/1024.0, v1 = 502.0/1024.0;
    for(int j=0; j<2; j++) {
        int k = mesh.m_vCount;
        for(int i=0; i<=2*n; i++) {
            double u = (double)i/(double)(2*n); u = 1-u; u = (1-u)*u0 + u*u1;
            mesh.addVertex(pts[i*3+j].toVector3D(), norms[j], QPointF(u, v0));
            mesh.addVertex(pts[i*3+j+1].toVector3D(), norms[j], QPointF(u, v1));
        }
        for(int i=0; i<2*n; i++) {
            mesh.addQuad(k,k+2,k+3,k+1); k+= 2;
        }
        qSwap(v0,v1);
    }
    mesh.createBuffers();
}



void Fig12Page::makeDodMesh()
{
    m_dodMesh.m_hasTexCoords = true;
    for(int i=0;i<m_grid->m_edgeMatrices.count();i++)
        m_dodMesh.hMerge(m_edgeBox, m_grid->m_edgeMatrices[i]);
    for(int i=0;i<m_grid->m_vertexMatrices.count();i++)
        m_dodMesh.hMerge(m_vertexBox, m_grid->m_vertexMatrices[i]);
    
    m_dodMesh.createBuffers();
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

    // drawAxes();


    qreal viewArr[16], projArr[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, viewArr);
    glGetDoublev(GL_PROJECTION_MATRIX, projArr);
    QMatrix4x4 view(viewArr), proj(projArr);
    QMatrix4x4 projView = proj.transposed() * view.transposed();

    m_shaderProgram->bind();
    // setViewUniforms(m_shaderProgram);    
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_CULL_FACE);
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_textureId);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    m_dodMesh.bind();
    for(int i=0;i<m_grid->m_cells.size();i++)
    {
        draw(m_grid->m_cells[i].m_hMatrix, m_dodMesh);
    }
    m_dodMesh.release(); 

    glDisable(GL_TEXTURE_2D);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_CULL_FACE);
    m_shaderProgram->release();

    drawGadgets();

    glPopMatrix();
}

void Fig12Page::draw(const QMatrix4x4 &mat, const Mesh &mesh)
{
    m_shaderProgram->setUniformValue("texture", 0 );
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




void Fig12Page::drawGadgets()
{
    for(int i=0; i<0; i++)
    {
        QVector4D p = m_grid->m_cells[0].m_hMatrix.map(m_grid->m_dodFaces[i].center);
        QVector3D p3 = H3::KModel::toBall(p, 10);
        v()->drawText(p3, QString("F%1").arg(i));

    }

    m_grid->drawGadgets();
}


void Fig12Page::savePictures()
{
    Fig12Page page(30000);
    page.m_cameraDistance = m_cameraDistance;
    page.m_theta = m_theta;
    page.m_phi = m_phi;
    page.savePicture("fig12.png", 10);
}


void Fig12Page::createTextures()
{
    // QImage img; img.load("bu.png");

    
    QImage img(1024, 1024, QImage::Format_ARGB32);
    img.fill(QColor(120,120,120));
    QPainter pa;
    pa.begin(&img);

    int x0,y0,x1,y1;

    // vertex
    x0=10;y0=10;x1=502;y1=502;

    pa.fillRect(x0,y1-10,x1-x0+1,10, Qt::black);
    pa.fillRect(x1-10,y0, 10, y1-y0+1, Qt::black);
    //pa.fillRect(x0,y1-5,x1-x0+1,5, Qt::white);
    //pa.fillRect(x1-5,y0, 5, y1-y0+1, Qt::white);

    int d = 160;
    pa.fillRect(x0,y0+d,d,5, Qt::black);
    pa.fillRect(x0+d,y0,5,d, Qt::black);


    // edge
    x0=512+10;y0=8;x1=1024-10;y1=502;

    // bordo laterale edges
    pa.fillRect(x0,y1-10,x1-x0+1,10, Qt::black);
    //pa.fillRect(x0,y1-5,x1-x0+1,5, Qt::white); 
    
    //pa.fillRect(x0,y1-10,x1-x0+1,10, Qt::black);
    //pa.fillRect(x0,y1-5,x1-x0+1,5, Qt::white); // bordo laterale edges
    
    // altro bordo
    // pa.fillRect(x0,y0-5,5, y1-y0+1+10, Qt::white);
    // pa.fillRect(x1-4,y0-5,5, y1-y0+1+10, Qt::white);

    pa.end();

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
    //glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D, 0);


}

void Fig12Page::destroyTextures()
{
    glDeleteTextures(1, &m_textureId);
}
