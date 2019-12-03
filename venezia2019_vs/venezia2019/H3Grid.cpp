#include "H3Grid.h"
#include "H3.h"
#include "Polyhedra.h"
#include <assert.h>
#define _USE_MATH_DEFINES
#include <math.h>


H3Grid534::H3Grid534()
{
    Polyhedron *dod = makeDodecahedron();
    compute();
    double r = radius;
    double sc = r/dod->getVertex(0).m_pos.length();
    int vCount = dod->getVertexCount();
    for(int i=0;i<vCount;i++) {
        QVector4D p = dod->getVertex(i).m_pos * sc;
        p.setW(1.0);
        m_pts.append(p);
        m_adjVertTb.append(QVector<int>());
    }
    for(int i=0;i<dod->getEdgeCount();i++)
    {
        const Polyhedron::Edge &edge = dod->getEdge(i);
        int a = edge.m_a, b = edge.m_b;
        m_edges.append(qMakePair(a,b));
        m_adjVertTb[a].append(b);
        m_adjVertTb[b].append(a);
    }

    for(int i=0; i<vCount;i++)
    {
        QVector3D p = m_pts[i].toVector3D();
        QList<QVector3D> ds;
        assert(m_adjVertTb[i].count() == 3);
        for(int j=0;j<m_adjVertTb[i].count();j++) 
            ds.append(m_pts[m_adjVertTb[i][j]].toVector3D());
        if(QVector3D::dotProduct(p, QVector3D::crossProduct(ds[2]-ds[0], ds[1]-ds[0]))<0)
        {
            qSwap(m_adjVertTb[i][1], m_adjVertTb[i][2]);
        }
    }


    // compute dodTranslation
    dod->computeFaceVertices();
    Polyhedron::Face face = dod->getFace(0);
    QVector3D p;    
    for(int i=0;i<5;i++) p += m_pts[face.m_vertices[i]].toVector3D();
    p *= 1.0/5.0;

    
    dodTranslation = H3::KModel::translation(-p, p);

    delete dod;


}

// trovo i parametri del dodecaedro con un angolo diedro di pi/2
void H3Grid534::compute()
{
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

    radius = H3::KModel::getRadius(d2);   

    // edge length è il doppio del cateto con angolo opposto theta
    // uso la formula sin(A) = sinh(opposto) / sinh(ipotenusa)
    edgeLength = 2*asinh(sin(theta)*sinh(d2));


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
}


QMatrix4x4 H3Grid534::getCornerMatrix()
{
    int vIndex = 0;
    QMatrix4x4 mat1 = H3::KModel::translation(m_pts[vIndex].toVector3D(), QVector3D(0,0,0));

    QVector3D p0 = mat1.map(m_pts[vIndex]).toVector3D();
    assert(p0.length()<1.0e-7);

    QVector3D pp[3];
    for(int j=0;j<3;j++) {
        int k = m_adjVertTb[vIndex][j];
        QVector3D q = mat1.map(m_pts[k]).toVector3D();
        pp[j] = q.normalized();
    }

    double pds[3] = {
        QVector3D::dotProduct(pp[0],pp[1]),
        QVector3D::dotProduct(pp[0],pp[2]),
        QVector3D::dotProduct(pp[1],pp[2])
    };
    for(int i=0;i<3;i++) {
        assert(fabs(pds[i]) < 1.0e-6);
    }

    // ortonormalizzo per maggior precisione
    pp[1] = (pp[1] - QVector3D::dotProduct(pp[0],pp[1])*pp[0]).normalized();

    QVector3D pp2 = QVector3D::crossProduct(pp[0],pp[1]).normalized();
 
    assert(QVector3D::dotProduct(pp2, pp[2])>0.99999);
    pp[2] = pp2;

    QMatrix4x4 mat(
        pp[0].x(), pp[0].y(), pp[0].z(), 0,
        pp[1].x(), pp[1].y(), pp[1].z(), 0,
        pp[2].x(), pp[2].y(), pp[2].z(), 0,
        0,0,0,1
    ); 


    //assert(fabs(QVector3D::dotProduct(pp[1],pp[0]))<1.0e-7);
    //assert(fabs(QVector3D::dotProduct(pp[2],pp[0]))<1.0e-7);
    //assert(fabs(QVector3D::dotProduct(pp[1],pp[2]))<1.0e-7);
    

    return mat * mat1;

}


QMatrix4x4 H3Grid534::getCellVertexMatrix(int vIndex)
{
    // traslazione iperbolica: vertice vIndex-esimo => origine
    QMatrix4x4 mat1 = H3::KModel::translation(
            m_pts[vIndex].toVector3D(), 
            QVector3D(0,0,0));

    // check!
    assert(mat1.map(m_pts[vIndex]).toVector3DAffine().length()<1.0e-7);

    // pp[] <= i tre vertici adiacenti h-traslati attorno all'origine 
    // e normalizzati
    QVector3D pp[3];
    for(int j=0;j<3;j++) {
        int k = m_adjVertTb[vIndex][j];
        pp[j] = mat1.map(m_pts[k]).toVector3DAffine().normalized();
    }

    // check!
    #ifndef  NDEBUG
    for(int i=0;i<3;i++) {
        double pd = QVector3D::dotProduct(pp[i],pp[(i+1)%3]);
        assert(fabs(pd) < 2.0e-7);
    }
    #endif

    // ortonormalizzo per maggior precisione
    pp[1] = (pp[1] - QVector3D::dotProduct(pp[0],pp[1])*pp[0]).normalized();
    QVector3D pp2 = QVector3D::crossProduct(pp[0],pp[1]).normalized(); 
    assert(QVector3D::dotProduct(pp2, pp[2])>0.99999);
    pp[2] = pp2;


    // mat : pp[] => x,y,z
    QMatrix4x4 mat(
        pp[0].x(), pp[0].y(), pp[0].z(), 0,
        pp[1].x(), pp[1].y(), pp[1].z(), 0,
        pp[2].x(), pp[2].y(), pp[2].z(), 0,
        0,0,0,1
    ); 
    
    return (mat * mat1).inverted();
}




QMatrix4x4 H3Grid534::getEdgeMatrix()
{
    QVector3D p(0, H3::KModel::getRadius(edgeLength), 0);
    return H3::KModel::translation(p,QVector3D(0,0,0));
}


void H3Grid534::addDodVertices(GridMatrices &matrices)
{
    for(int i=0;i<20;i++)
        matrices.m_vertexMatrices.append(getCellVertexMatrix(i));
}

void H3Grid534::addEdgeAndVertex(GridMatrices &matrices, const QMatrix4x4 &mat)
{
    matrices.m_edgeMatrices.append(mat);
    matrices.m_vertexMatrices.append(mat * dirMatrices[2]);
}

void H3Grid534::flower(GridMatrices &matrices, const QMatrix4x4 &mat, int level)
{
    addEdgeAndVertex(matrices, mat);
    if(level>0)
    {
        QMatrix4x4 rot1;
        rot1.setToIdentity();
        rot1.rotate(90, 0,0,1);
        for(int i=0;i<4;i++) {
            QMatrix4x4 rot2;
            rot2.setToIdentity();
            rot2.rotate(90*i, 0,1,0);
            flower(matrices, mat * dirMatrices[2] * rot2 * rot1, level - 1);    
        }
    }
}


//=============================================================================

void H3Grid::createFirstVertex()
{
    clear();
    Vertex v;
    v.matrix.setToIdentity();
    for(int i=0;i<6;i++) v.links[i] = -1;
    for(int i=0;i<24;i++) v.channels[i] = -1;
    m_vertices.append(v);
}

const int channelTable[] = {
    4,8,12,16, 
    0,19,22,9,
    1,7,21,13,
    2,11,20,17,
    3,15,23,5,
    14,10,6,18
};

int H3Grid::innerLink(int j)
{
    assert(0<=j && j<24);
    return channelTable[j];
}


void H3Grid::addVertex(int srcVertexIndex, int direction)
{
    assert(0<=direction && direction<6);
    assert(0<=srcVertexIndex && srcVertexIndex<m_vertices.count());

    Vertex &srcVertex = m_vertices[srcVertexIndex];
    if(srcVertex.links[direction]!= -1) return;

    // index of the new vertex
    int vertexIndex = m_vertices.count();

    // old vertex => new vertex
    srcVertex.links[direction] = vertexIndex;

    Vertex vertex;
    for(int i=0;i<6;i++) vertex.links[i] = -1;
    for(int j=0;j<24;j++) vertex.channels[j] = -1;

    // new vertex => old vertex
    vertex.links[5] = srcVertexIndex;

    const int linkTable[6][4] = {
        { 22 ,21 ,20 ,23 },
        { 20 ,23 ,22 ,21 },
        { 23 ,22 ,21 ,20 },
        { 22 ,21 ,20 ,23 },
        { 21 ,20 ,23 ,22 },
        { 22 ,21 ,20 ,23 }
    };

    for(int j=0;j<4;j++) 
    {
        int a = direction * 4 + j;
        int b = linkTable[direction][j];
        srcVertex.channels[a] = b;
        vertex.channels[b] = a;
    }

    QMatrix4x4 edgeMatrix = srcVertex.matrix * m_rotations[direction];

    m_edgeMatrices.append(edgeMatrix);

    vertex.matrix = edgeMatrix * m_gridData.dirMatrices[2];

    m_vertices.append(vertex);

}

int H3Grid::getOtherEnd(int &vIndex, int &channelIndex)
{
    int i0 = vIndex;
    int i = vIndex;
    int c = channelTable[channelIndex];
    int m = 1;
    for(;;)
    {
        const Vertex &vertex = m_vertices[i];
        vIndex = i;
        channelIndex = c;
        i = vertex.links[c>>2];
        if(i<0 || i==vIndex) break;
        m++;
        c = vertex.channels[c];
        assert(c>=0);
        c = channelTable[c];
    }
    return m;
}

void H3Grid::closeIfNeeded(int vIndex, int dir)
{
    assert(0<=vIndex && vIndex<m_vertices.count());
    Vertex &vertex = m_vertices[vIndex];
    // already linked: nothing to do
    if(vertex.links[dir] != -1) 
        return;

    // search for full faces
    int channel = -1;
    int otherIndex = -1;
    int otherChannel = -1;
    for(int j=0;j<4;j++)
    {
        int c0 = dir*4+j;
        int i = vIndex;
        int c = c0;
        int m = getOtherEnd(i,c);
        if(m==5)
        {
            otherIndex = i;
            otherChannel = c;
            channel = c0;
            break;
        }
    }

    if(otherIndex<0) // nothing to do 
        return; 

    Vertex &otherVertex = m_vertices[otherIndex];
    int direction = channel/4;

    // set links
    vertex.links[direction] = otherIndex;
    int otherDirection = otherChannel/4;
    assert(otherVertex.links[otherDirection] == -1);
    otherVertex.links[otherDirection] = vIndex;

    // link channels
    int j0 = channel%4;
    int j1 = otherChannel%4;
    for(int j=0;j<4;j++) {
        int a = direction * 4 + ((j0+j)%4);
        int b = otherDirection * 4 + ((j1+4-j)%4);
        assert(vertex.channels[a] == -1);
        assert(otherVertex.channels[b] == -1);
        vertex.channels[a] = b;
        otherVertex.channels[b] = a;
    }

    // create the edge 

    QMatrix4x4 rot;
    QMatrix4x4 edgeMatrix = vertex.matrix * m_rotations[direction];
    m_edgeMatrices.append(edgeMatrix);
}

// OBSOLETO
void H3Grid::foo(int vIndex)
{
    assert(0<=vIndex && vIndex<m_vertices.count());
    Vertex &vertex = m_vertices[vIndex];

    for(int dir=0;dir<6;dir++)
    {
        if(vertex.links[dir] != -1) continue;

        int channel = -1;
        int otherIndex = -1;
        int otherChannel = -1;
        for(int j=0;j<4;j++)
        {
            int c0 = dir*4+j;
            int i = vIndex;
            int c = c0;
            int m = getOtherEnd(i,c);
            if(m==5)
            {
                otherIndex = i;
                otherChannel = c;
                channel = c0;
                break;
            }
        }

        if(otherIndex>=0)
        {
            Vertex &otherVertex = m_vertices[otherIndex];
            int direction = channel/4;

            vertex.links[direction] = otherIndex;
            int otherDirection = otherChannel/4;
            assert(otherVertex.links[otherDirection] == -1);
            otherVertex.links[otherDirection] = vIndex;

            int j0 = channel%4;
            int j1 = otherChannel%4;
            for(int j=0;j<4;j++) {
                int a = direction * 4 + ((j0+j)%4);
                int b = otherDirection * 4 + ((j1+4-j)%4);
                assert(vertex.channels[a] == -1);
                assert(otherVertex.channels[b] == -1);
                vertex.channels[a] = b;
                otherVertex.channels[b] = a;
            }


            QMatrix4x4 rot;
            QMatrix4x4 edgeMatrix = vertex.matrix * m_rotations[direction];
            m_edgeMatrices.append(edgeMatrix);



            break;
        }
    }

}


void H3Grid::initRotations()
{
    const double angles[] = {0,-90,90,90,-90,180};
    for(int i=0;i<6;i++)
    {
        QMatrix4x4 &rot = m_rotations[i];
        rot.setToIdentity();
        rot.rotate(angles[i], (i&1)==1 ? QVector3D(0,0,1) : QVector3D(1,0,0));    
    }
}


