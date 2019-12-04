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
, m_foo(0)
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
    double edgeLength = m_edgeLength;    
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
    
    draw2();    

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisable(GL_CULL_FACE);
    m_shaderProgram->release();

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
    m_edgeLength = edgeLength;
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
            qSwap(adjVertTb[i][1], adjVertTb[i][2]);
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


    /*

    for(int i=0; i<dod->getEdgeCount(); i++)
    {
        const Polyhedron::Edge &edge = dod->getEdge(i);
        int a = edge.m_a, b = edge.m_b;
        int k = 0;
        for(k=0;k<3 && adjVertTb[a][k] != b ;k++) {}
        assert(k<3);
        QMatrix4x4 mat = m_vertexMatrices[a];
        if(k==0) { / * m_edgeMatrices.append(mat);  * / }
        else if(k==1) { QMatrix4x4 rot; rot.rotate(-90,0,0,1); / * m_edgeMatrices.append(mat*rot); * / }
        else  { QMatrix4x4 rot; rot.rotate(90,0,1,0); m_edgeMatrices.append(mat*rot);}

        break;
    }
    */
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
            int b = adjVertTb[a][j];
            QPair<int,int> edge(a,b);
            if(touchedEdges.contains(edge)) continue;
            touchedEdges.insert(edge);
            touchedEdges.insert(QPair<int,int>(b,a));
            m_edgeMatrices.append(mat*rots[j]);
        }
    }


    Polyhedron::Face face = dod->getFace(0);
    QVector3D face0Center;    
    for(int i=0;i<5;i++) face0Center += 0.2 * pts[face.m_vertices[i]].toVector3D();
    m_dodTranslate = H3::KModel::translation(-face0Center, face0Center);
    m_dodTranslate.rotate(36, 0,1,0);

    m_otherDodMatrices.append(m_dodTranslate);
    for(int i=1; i<12; i++)
    {
        QMatrix4x4 rot = getFaceMatrix(dod, i) * getFaceMatrix(dod, 0).inverted();
        m_otherDodMatrices.append(rot * m_dodTranslate);
    }

    for(int i=0;i<12;i++) m_faceCenters.append(sc * getFaceCenter(dod, i));

    delete dod;
}

void Fig12Page::makeDodMesh()
{
    for(int i=0;i<m_edgeMatrices.count();i++)
        m_dodMesh.hMerge(m_edgeBox, m_edgeMatrices[i]);
    m_dodMesh.createBuffers();
}



void Fig12Page::draw1()
{
    
    /*

    for(int dodIndex = 0; dodIndex <= 13; dodIndex++)
    {
        QMatrix4x4 dodMatrix;
        if(dodIndex == 0) dodMatrix.setToIdentity();
        else if(dodIndex<=12) dodMatrix = m_otherDodMatrices[dodIndex - 1];  
        else if(dodIndex == 13)
        {
            dodMatrix = m_otherDodMatrices[11] * m_otherDodMatrices[m_foo];  
        }
        else continue;

        m_vertexCube.bind();

        for(int i=0;i<m_vertexMatrices.count();i++)
            draw(dodMatrix * m_vertexMatrices[i], m_vertexCube);    
    
        m_vertexCube.release();


        m_edgeBox.bind();
        // draw(identity, m_edgeBox);
        for(int i=0;i<m_edgeMatrices.count();i++)
            draw(dodMatrix * m_edgeMatrices[i], m_edgeBox);

        m_edgeBox.release();
    }
    */

    QMatrix4x4 identity; identity.setToIdentity();
    m_dodMesh.bind();
    draw(identity, m_dodMesh);
    m_dodMesh.release();
    

    /*
    for(int i=0;i<m_vertexMatrices.count();i++)
    {
        QMatrix4x4 mat = m_vertexMatrices[i];
        QVector4D pos4 = mat.map(QVector4D(0,0,0,1.0));
        QVector3D pos = pos4.toVector3DAffine();
        v()->drawText(pos*5, QString::number(i));
    }
    */
    /*
    for(int i=0;i<m_faceCenters.count();i++)
    {
        v()->drawText(m_faceCenters[i]*5, QString::number(i));
    }
    */
}



class PointList {

    
public:

    struct Key {
        double r;
        int lat, lng;
    };

    class Shell {
    public:
        double r;
        QMap<QPair<int, int>, QList<QVector3D> >  table;
    };

    PointList() {
        m_shells.append(Shell());
        m_shells[0].r = 0;
    }
    
    bool add(const QVector3D &p);


    Key getKey(const QVector3D &p)
    {       
        const qreal pi = 3.14159265359;
        double r = p.length();
        if(r<1e-5)
        {
            Key key;
            key.r = 0;
            key.lat = 0;
            key.lng = 0;
            return key;
        }
        Key key;
        key.r = r;
        double y = qMax(0.0, qMin(1.0, p.y()/r));
        double lat = acos(y);
        key.lat = (int)(10.0*lat/pi);
        double phi = atan2(p.x(),p.z());
        key.lng = (int)(5.0*phi/pi);
        return key;
    }

    int findClosestShell(double r) const;
    int touchShell(double r);
    void dump();

private:
    QList<Shell> m_shells;
};


bool PointList::add(const QVector3D &p)
{
    Key key = getKey(p);
    Shell &shell = m_shells[touchShell(key.r)];
    const double infinite = 1e40;
    double minDist = infinite;
    for(int i=key.lat-1;i<=key.lat+1;i++)
    {
        for(int j=key.lng-1;j<=key.lng+1;j++)
        {
            QMap<QPair<int, int>, QList<QVector3D>>::iterator it = shell.table.find(qMakePair(i,j));
            if(it == shell.table.end()) continue;
            QList<QVector3D> &lst = it.value();
            foreach(QVector3D p2, lst) 
            {
                double dist = (p-p2).length();
                if(dist<minDist) minDist = dist;
            }
        }
    }
    if(minDist < 1e-5) return false;
    QMap<QPair<int, int>, QList<QVector3D>>::iterator it = shell.table.find(qMakePair(key.lat,key.lng));
    if(it == shell.table.end())
    {
        QList<QVector3D> lst;
        lst.append(p);
        shell.table[qMakePair(key.lat,key.lng)] = lst;
    }
    else
    {
        it.value().append(p);
    }
    return true;
}


int PointList::findClosestShell(double r) const
{
    if(m_shells.isEmpty()) return -1;
    else
    {
        if(r>=m_shells.last().r) return m_shells.count()-1;
        else if(r<=m_shells.first().r) return 0;
        int a = 0, b = m_shells.count()-1;
        assert(m_shells[a].r <= r && r < m_shells[b].r);
        while(b-a>1)
        {
            int c = (a+b)/2;
            if(m_shells[c].r <= r) a=c; else b=c;
        }
        assert(m_shells[a].r <= r && r < m_shells[b].r);
        return (r-m_shells[a].r < m_shells[b].r - r) ? a : b;
    }    
}

int PointList::touchShell(double r)
{
    if(r<1.e-3) return 0;
    int k = findClosestShell(r);
    if(fabs(r - m_shells[k].r) > 0.001)
    {
        Shell shell;
        shell.r = r;
        if(r>m_shells[k].r) { m_shells.insert(k+1, shell); return k+1; }
        else { m_shells.insert(k,shell); return k; }
    }
    else return k;
}



void PointList::dump()
{
    qDebug() << "-------------------------------";
    foreach(Shell shell, m_shells)
    {
        int m = 0;
        QList<QVector3D> lst;
        foreach(lst, shell.table.values()) m += lst.count();
        qDebug() << " r=" << shell.r << " tb " << shell.table.count() << " total=" << m;
    }
}




void Fig12Page::draw2()
{
    QMatrix4x4 identity; identity.setToIdentity();
    m_dodMesh.bind();

    QList<QMatrix4x4> stack;
    stack.append(identity);

    int count = 0;

    qDebug() << "----------------------------------------";
    PointList positions;
    while(stack.count()>0)
    {
        QMatrix4x4 dodMatrix = stack.last();
        stack.pop_back();
        QVector3D q = dodMatrix.map(QVector4D(0,0,0,1)).toVector3DAffine();
        qreal qq = q.length();
        if(qq > 0.1 && q.x()/qq < 0.5) continue;
        if(qq>0.99995) // 0.9993) 
            continue;
        if(!positions.add(q)) continue;
        draw(dodMatrix, m_dodMesh);
        count++;
        if (count>2000) 
            break;

        for(int i=1;i<12;i++) stack.append(dodMatrix * m_otherDodMatrices[i]);
    }

    // positions.dump();

    m_dodMesh.release();

}

void Fig12Page::draw3()
{

    QMatrix4x4 identity; identity.setToIdentity();
    m_dodMesh.bind();

    for(int dodIndex = 0; dodIndex <= 13; dodIndex++)
    {
        QMatrix4x4 dodMatrix;
        if(dodIndex == 0) dodMatrix.setToIdentity();
        else if(dodIndex<=12) dodMatrix = m_otherDodMatrices[dodIndex - 1];  
        else if(dodIndex == 13)
        {
            dodMatrix = m_otherDodMatrices[11] * m_otherDodMatrices[m_foo];  
        }
        else continue;
        draw(dodMatrix, m_dodMesh);
    }

    m_dodMesh.release();

}



void Fig12Page::savePictures()
{
    Fig12Page page;
    page.m_cameraDistance = m_cameraDistance;
    page.m_theta = m_theta;
    page.m_phi = m_phi;
    page.savePicture("fig12.png", 10);
}
