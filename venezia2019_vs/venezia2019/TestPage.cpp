#include "TestPage.h"
#include "Texture.h"


#include <math.h>
#include <assert.h>
#include <QKeyEvent>
#include <QWheelEvent>

#include <vector>

#include "Gutil.h"
#include "Point3.h"

#include <QGLShaderProgram>
#include <qmatrix4x4.h>
#include <QVector4D.h>
#include <qvector.h>
#include <qlist.h>
#include "Polyhedra.h"
#include "H3.h"

// #include "vector.h"


QVector3D toBall(const QVector4D &p) 
{
    double iw = 1.0/p.w();
    double x = p.x()*iw;
    double y = p.y()*iw;
    double z = p.z()*iw;
    QVector3D q = p.toVector3DAffine();

    double s2 = qMin(1.0, q.lengthSquared()); 
    double k = 1.0 / (1.0 + sqrt(1.0 - s2));        
    return 5*QVector3D(k*x,k*y,k*z);
}

QVector3D toBall(const QVector3D &p) 
{
    return toBall(QVector4D(p.x(),p.y(),p.z(),1.0));
}



class GeometryBuilder {
public:

    class Transformation {
    public:
    virtual QVector3D transform(const QVector3D &pos) = 0; 
    virtual QPair<QVector3D, QVector3D> transform(
        const QVector3D &pos,  
        const QVector3D &e0, 
        const QVector3D &e1) {
            QVector3D pos1 = transform(pos);
            QVector3D norm = QVector3D::crossProduct(
                transform(pos + 0.1*e0) - pos1,
                transform(pos + 0.1*e1) - pos1).normalized();
            return qMakePair(pos1,norm);
        }
    };

    Transformation &transf;

    QVector<GLushort> indices;
    QVector<GLfloat> buffer;
    int vCount;

    GeometryBuilder(Transformation &tr) : vCount(0), transf(tr) {}

    int addVertex(const QPair<QVector3D, QVector3D> &posAndNorm) {
        const QVector3D &pos = posAndNorm.first;
        const QVector3D &norm = posAndNorm.second;
        int index = vCount++;
        buffer.append(pos.x());
        buffer.append(pos.y());
        buffer.append(pos.z());
        buffer.append(norm.x());
        buffer.append(norm.y());
        buffer.append(norm.z());
        return index;
    }

    void addFace(int a, int b, int c) {
        indices.append(a);
        indices.append(b);
        indices.append(c);
    }



    void addPolyhedronFace(Polyhedron *ph, int faceIndex) {
        const int n = 5;
        
        const Polyhedron::Face &face = ph->getFace(faceIndex);
        const int m = face.m_vertices.size();
        QVector<QVector3D> pts;
        for(int j=0;j<m;j++) 
        {
            pts.append(ph->getVertex(face.m_vertices[j]).m_pos);
        }
        QVector3D faceCenter = getFaceCenter(ph, faceIndex);

        QVector3D e0 = (pts[0]-faceCenter).normalized();
        QVector3D e1 = pts[1]-faceCenter;
        e1 = (e1 - QVector3D::dotProduct(e1,e0)*e0).normalized();

        QList<QList<int> > tb; 
        int k0 = addVertex(transf.transform(faceCenter, e0,e1));
        tb.append(QList<int>());
        for(int j=0;j<m;j++) tb[0].append(k0);

        for(int i=1; i<n; i++) // anelli concentrici
        {
            tb.append(QList<int>());
            double t = (double)i/(double)(n-1);
            for(int j=0; j<m; j++) // spicchi
            {
                tb[i].append(vCount);
                QVector3D p0 = pts[j]*t + faceCenter*(1-t);
                QVector3D p1 = pts[(j+1)%m]*t + faceCenter*(1-t);
                for(int h=0; h<i; h++)
                {
                    double s = 0.5*(1-cos(M_PI*(double)h/(double)i));
                    QVector3D p = p0*(1-s)+p1*s;

                    addVertex(transf.transform(p, e0,e1));
                }
            }
        }

        int q = k0+1;
        for(int i=1; i<n; i++)
        {
            for(int j=0; j<m; j++) 
            {
                QVector<int> qq0, qq1;
                int q;
                q = tb[i-1][j];
                for(int h=0; h+1<i; h++) qq0.append(q++);
                qq0.append(tb[i-1][(j+1)%m]);
                q = tb[i][j];
                for(int h=0; h<i; h++) qq1.append(q++);
                qq1.append(tb[i][(j+1)%m]);
                for(int h=0; h<i; h++) addFace(qq1[h],qq1[h+1],qq0[h]);
                for(int h=0; h+1<i; h++) addFace(qq1[h+1],qq0[h+1],qq0[h]);
            }

        }
        
    }

};


class BallTransform : public GeometryBuilder::Transformation {
public:
    QMatrix4x4 hMatrix;
    BallTransform() {
        hMatrix.setToIdentity();
    }
    QVector3D transform(const QVector3D &pos)
    {
        QVector4D p4(pos.x(),pos.y(),pos.z(),1.0);        
        return toBall(hMatrix.map(p4));
    }
};


#define PI 3.141592653

using namespace GmLib;

TestPage::TestPage()
: m_theta(0)
, m_phi(0)
, m_pp(10,0)
, m_rotating(true)
, m_shader0(0)
, m_shaderBG(0)
, m_texture(0)
, m_cameraDistance(10)
, m_timerId(0)
, m_scaleFactor(0.99)
{
}

TestPage::~TestPage()
{
    /*
  delete m_shader0;
  delete m_shaderBG;
  delete m_texture;
  */
}

void TestPage::initializeGL()
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

  /*
  m_shader0 = loadProgram("rollingSphere");
  m_shaderBG = loadProgram("rollingSphereBg");
  m_texture = Texture::get(":resources/checkboard.png");
  */
}

void TestPage::resizeGL(int width, int height)
{
  double aspect = (float)width/height;
  glViewport(0,0,width,height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45, aspect, 1.0, 70.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void TestPage::paintGL()
{
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


    Polyhedron *ph = makeDodecahedron();
    ph->computeFaceVertices();
    double sc = m_scaleFactor / ph->getVertex(0).m_pos.length();
    for(int i=0;i<ph->getVertexCount();i++) 
        ph->getVertex(i).m_pos *= sc;



    BallTransform transform;


    setColor(0.8,0.1,0.1);
    QVector3D pa = ph->getVertex(0).m_pos;
    QVector3D pb = ph->getVertex(3).m_pos;

    drawSphere(transform.transform(pa), 0.05);
    drawSphere(transform.transform(pb), 0.05);

    QVector3D p = H3::KModel::midPoint(pa,pb);
    qDebug() << H3::KModel::distance(p,pa);
    qDebug() << H3::KModel::distance(p,pb);




    setColor(0.8,0.6,0.1);
    drawSphere(transform.transform(p), 0.02);


    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);

    for(int k=0; k<2; k++) {
        if(k==0) {
            setColor(0.1,0.6,0.8);
            transform.hMatrix.setToIdentity();
        } else {
            transform.hMatrix = H3::KModel::reflection(pa);
            setColor(0.9,0.2,0.8);
        }
        GeometryBuilder gb(transform);
        for(int i=0;i<ph->getFaceCount();i++)
        {
            gb.addPolyhedronFace(ph,i);
        }
        // glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glNormalPointer(GL_FLOAT, sizeof(GL_FLOAT)*6, &gb.buffer[0]+3);
        glVertexPointer(3, GL_FLOAT, sizeof(GL_FLOAT)*6, &gb.buffer[0]);

        //if(k==1) glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        glDrawElements(GL_TRIANGLES, gb.indices.size(), GL_UNSIGNED_SHORT, &gb.indices[0]);
        //if(k==1) glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);

 
    drawBoundary();


    glPopMatrix();
  
}

void TestPage::drawBoundary()
{
    glDisable(GL_LIGHTING);
    glColor3d(1,0,0);
    int m = 50;
    double r = 5.0;
    QVector<QPair<double, double> > cssn;
    for(int i=0;i<m;i++) {
        double phi = M_PI * 2 * i / m;
        cssn.append(qMakePair(cos(phi)*r, sin(phi)*r));
    }
    glBegin(GL_LINES);
    for(int i=0;i<m;i++) 
    {
        int i1 = (i+1)%m;
        double a = cssn[i].first, b = cssn[i].second, c = cssn[i1].first, d = cssn[i1].second;
        glVertex3d(a,b,0); glVertex3d(c,d,0);
        glVertex3d(a,0,b); glVertex3d(c,0,d);
        glVertex3d(0,a,b); glVertex3d(0,c,d);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}



void TestPage::mousePressEvent(QMouseEvent *e)
{
  m_lastPos = e->pos();
  m_rotating = e->button() == Qt::RightButton;
}

void TestPage::mouseReleaseEvent(QMouseEvent *e)
{

}

void TestPage::mouseMoveEvent(QMouseEvent *e)
{
  QPoint delta = m_lastPos - e->pos();
  m_lastPos = e->pos();
  if(!m_rotating)
  {
 
    m_scaleFactor = qMin(0.9999, m_scaleFactor + 0.001*delta.x());
  }
  else
  {
    m_phi -= 0.25*delta.x();
    m_theta -= 0.25*delta.y();
  }
  updateGL();
}

void TestPage::keyPressEvent(QKeyEvent *e)
{
  e->ignore();
}

void TestPage::wheelEvent(QWheelEvent*e)
{
  m_cameraDistance = clamp(m_cameraDistance - e->delta() * 0.01, 1,50);
  updateGL();
}
