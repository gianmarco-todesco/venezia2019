#include "CubeGridPage.h"


#include "Gutil.h"
#include "Mesh.h"
#include "Overlay.h"
#include <assert.h>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QGLShaderProgram>
#include <qmatrix4x4.h>
#include <qvector.h>
#include <qlist.h>

#include <QTime>
#include <qdebug.h>
#include <QPainter>


CubeGridPage::CubeGridPage()
: m_cameraDistance(15)
, m_theta(31.75)
, m_phi(-19.75)
, m_rotating(true)
, m_gridSmallUnit(0)
, m_gridBigUnit(1)
, m_status(-1)
, m_translating(true)
, m_translatingSpeed(1)
{
}

CubeGridPage::~CubeGridPage()
{
}

void CubeGridPage::buildMesh()
{
    QTime clock;
    clock.start();
    const double r1 = 1.0;
    const double r2 = 0.2;
    const double d = 10;

    int n = 4;
    for(int ix=-n;ix<=n;ix++)
    for(int iy=-n;iy<=n;iy++)
    for(int iz=-n;iz<=n;iz++)
        m_mesh.addBox(QVector3D(ix,iy,iz)*d, r1,r1,r1);

    double q = n*d;
    for(int ix=-n;ix<=n;ix++)
    for(int iy=-n;iy<=n;iy++)
    {
        m_mesh.addBox(QVector3D(ix,iy,0)*d, r2,r2,q);
        m_mesh.addBox(QVector3D(ix,0,iy)*d, r2,q,r2);
        m_mesh.addBox(QVector3D(0,ix,iy)*d, q,r2,r2);
    }
    
    m_mesh.createBuffers();
    m_gridSmallUnit = d;
    m_gridBigUnit = d*2*n;

    qDebug() << "cube grid created: " << clock.elapsed();

    m_offset = QVector3D(d*0.5,d*0.5,d*0.5);
}

void CubeGridPage::initializeGL()
{
    buildMesh();
    m_shaderProgram = loadProgram("cubeGrid");
    
}

void CubeGridPage::start()
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
  createTextures();
  setStatus(0);
}

void CubeGridPage::stop()
{
    destroyTextures();
}

void CubeGridPage::resizeGL(int width, int height)
{
    double aspect = (float)width/height;
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, aspect, 1.0, 200.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void CubeGridPage::paintGL()
{
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // drawBackground();

    glPushMatrix();
    glTranslated(0,0,-m_cameraDistance);
    glRotated(m_theta,1,0,0);
    glRotated(m_phi,0,1,0);


    GLfloat specular[] =  { 0.7f, 0.7f, 0.7f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 90.0);

    setColor(1,1,1);
    draw();
    // drawAxes();

    /*
    setColor(1,0,1);
    drawSphere(2*getCurrentDirection(), 0.2);

    setColor(0,1,1);
    drawSphere(QVector3D(0.2,0,0), 0.1);
    drawSphere(QVector3D(-0.2,0,0), 0.1);
    */

    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);




    glDisable(GL_CULL_FACE);
    
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glPopMatrix();

}

void CubeGridPage::draw()
{
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);


    m_shaderProgram->bind();
    setViewUniforms(m_shaderProgram);

    
    m_mesh.bind();

    QVector3D dir = getCurrentDirection();
    int m = 1;
    for(int ix = -m; ix <= m; ix++) 
    for(int iy = -m; iy <= m; iy++) 
    for(int iz = -m; iz <= m; iz++) 
    {
        QVector3D v = m_offset + m_gridBigUnit * QVector3D(ix,iy,iz);
        //if(QVector3D::dotProduct(v,dir)>0) 
        {
            m_shaderProgram->setUniformValue("u_offset", v);
            m_mesh.draw();
        }
    }




    m_mesh.release();

    glDisable(GL_CULL_FACE);
    m_shaderProgram->release();
    
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

  
    // moveOffset(-getCurrentDirection()*0.1);
    if(m_translating)
    {
        m_translatingSpeed = qMin(1.0, m_translatingSpeed + 0.02);
    }
    else
    {
        m_translatingSpeed = qMax(0.0, m_translatingSpeed - 0.02);
    }
    if(m_translatingSpeed>0.0)
        moveOffset(QVector3D(1,1,1)*0.05*m_translatingSpeed);
}



void CubeGridPage::mousePressEvent(QMouseEvent *e)
{
      m_lastPos = e->pos();
      m_rotating = e->button() == Qt::RightButton;
}

void CubeGridPage::mouseReleaseEvent(QMouseEvent *e)
{

}

void CubeGridPage::mouseMoveEvent(QMouseEvent *e)
{
  QPoint delta = m_lastPos - e->pos();
  m_lastPos = e->pos();
  if(!m_rotating)
  {
  }
  else
  {
    m_phi -= 0.25*delta.x();
    m_theta -= 0.25*delta.y();
  }
  updateGL();
}

void CubeGridPage::keyPressEvent(QKeyEvent *e)
{
    /*
    if(e->key() == Qt::Key_1) m_offset += QVector3D(m_gridSmallUnit,0,0);
    else if(e->key() == Qt::Key_2) m_offset -= QVector3D(m_gridSmallUnit,0,0);
    else if(e->key() == Qt::Key_W) moveOffset(-getCurrentDirection());
    else if(e->key() == Qt::Key_S) moveOffset(getCurrentDirection());
    //else if(e->key() == Qt::Key_3) m_offset += QVector3D(m_gridBigUnit,0,0);
    //else if(e->key() == Qt::Key_4) m_offset -= QVector3D(m_gridBigUnit,0,0);
    else 
    */
    if(e->key() == Qt::Key_P) {
        m_translating = !m_translating;
    } 
    else if(e->key() == Qt::Key_Right) setStatus(m_status+1);
    else if(e->key() == Qt::Key_Left) setStatus(m_status-1);
    e->ignore();
    updateGL();
}

void CubeGridPage::wheelEvent(QWheelEvent*e)
{
    m_cameraDistance = clamp(m_cameraDistance - e->delta() * 0.01, 1,80);
    updateGL();
}

QVector3D CubeGridPage::getCurrentDirection()
{
    QMatrix4x4 mat;
    mat.setToIdentity();
    mat.rotate(-m_phi,0,1,0);
    mat.rotate(-m_theta,1,0,0);

    return mat.map(QVector3D(0,0,-1));
}

double recenter(double x, double u) {
    double t = x/u + 0.5;
    return (t - floor(t) - 0.5)*u;
}


void CubeGridPage::moveOffset(const QVector3D &delta)
{
    const double u = m_gridSmallUnit;
    m_offset += delta;
    m_offset = QVector3D(
        recenter(m_offset.x(), u),
        recenter(m_offset.y(), u),
        recenter(m_offset.z(), u));
}

void CubeGridPage::createTextures()
{
    QImage img;
    QPainter pa;
    OverlayPanel *panel;

    // title
    panel = &m_panels.title;
    img = QImage(1200,300,QImage::Format_ARGB32);
    img.fill( Qt::transparent);
    pa.begin(&img);
    pa.setBrush(QColor(100,100,100,127));
    pa.setPen(Qt::NoPen);
    pa.drawRoundedRect(50,0,1100,200,20,20);
    pa.setFont(QFont("Calibri", 80, QFont::Bold));
    pa.setPen(Qt::white);
    pa.drawText(QRect(0,0,1200,100), Qt::AlignCenter, "Hyperbolic Honeycomb");
    pa.setFont(QFont("Calibri", 40, QFont::Bold));
    pa.drawText(QRect(0,100,1200,100), Qt::AlignCenter, "Gian Marco Todesco");
    pa.end();
    panel->createTexture(img);
    panel->setSize(35);
    panel->setPosition(0.5,0.8);

    // title2
    panel = &m_panels.title2;
    img = QImage(1200,400,QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    pa.begin(&img);
    pa.setBrush(QColor(100,100,100,127));
    pa.setPen(Qt::NoPen);
    pa.drawRoundedRect(50,0,1100,600,20,20);
    pa.setFont(QFont("Calibri", 80, QFont::Bold));
    pa.setPen(Qt::white);
    pa.drawText(QRect(0,20,1200,100), Qt::AlignCenter, "Tale of two errors");
    pa.setFont(QFont("Calibri", 40, QFont::Bold));
    pa.drawText(QRect(0,100,1200,300), Qt::AlignCenter, "How I lost my way and stumbled upon\nthe Hyperbolic Space");
    pa.end();
    panel->createTexture(img);
    panel->setSize(51);
    panel->setPosition(0.5,0.65);

    // escher
    panel = &m_panels.escher;
    panel->createTexture(QImage("images/cubic-space-division.jpg"));
    panel->setSize(60);
    panel->setPosition(0.3,0.4);


    // escher caption
    panel = &m_panels.escherCaption;
    img = QImage(600,160,QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    pa.begin(&img);
    pa.setFont(QFont("Calibri", 30,-1, true));
    pa.setPen(Qt::white);
    pa.drawText(QRect(0,0,600,40), Qt::AlignLeft, "Cubic space division");
    pa.drawText(QRect(0,40,600,40), Qt::AlignLeft, "1953 M. C. Escher");
    pa.end();
    panel->createTexture(img);
    panel->setSize(30);
    panel->setPosition(0.75,0.7);

    // grid
    panel = &m_panels.grids;
    panel->createTexture(QImage("images/h2grid.png"));
    panel->setSize(60);
    panel->setPosition(0.3,0.4);

}

void CubeGridPage::destroyTextures()
{
    OverlayPanel *panels[] = {
        &m_panels.title,
        &m_panels.title2,
        &m_panels.escher,
        &m_panels.escherCaption,
        &m_panels.grids
    };
    for(int i=0;i<sizeof(panels)/sizeof(panels[0]);i++) panels[i]->deleteTexture();
}

void CubeGridPage::setStatus(int status)
{
    if(status<0) status=0;
    else if(status>5) status=5;
    int oldStatus = m_status;
    if(oldStatus==status) return;
    m_status = status;
    Overlay *ov = getOverlay();
    QPointF titlePos(0.5,0.8);

    if(status == 0)
    {
        // Solo titolo1
        ov->removeAll();
        ov->add(&m_panels.title, titlePos);
    } 
    else if(status == 1)
    {
        // escher
        QPointF escherPos(0.3, 0.4);
        QPointF captionPos(0.94, 0.5);
        if(oldStatus != 0) 
        {
            ov->removeAll();
            ov->add(&m_panels.title, titlePos);
        }

        ov->addAndMove(&m_panels.escher, 
            escherPos + QPointF(-1, 0), escherPos, 1,0);
        ov->addAndMove(&m_panels.escherCaption, 
            captionPos + QPointF(1, 0), captionPos, 1,0);        
    }
    else if(status == 2)
    {
        // tolgo escher
        if(oldStatus == 1)
        {
            ov->moveAndRemove(&m_panels.escher, 
                m_panels.escher.getPosition(), 
                m_panels.escher.getPosition() + QPointF(-1, 0), 
                1,0);
            ov->moveAndRemove(&m_panels.escherCaption, 
                m_panels.escherCaption.getPosition(), 
                m_panels.escherCaption.getPosition() + QPointF(1, 0), 
                1,0);
        }
        else 
        {
            ov->removeAll();
            ov->add(&m_panels.title, titlePos);
        }
    }
    else if(status == 3)
    {
        // hgrid
        QPointF pos(0.5,0.4);
        if(oldStatus != 2)
        {
            ov->removeAll();
            ov->add(&m_panels.title, titlePos);

        }
        ov->addAndMove(&m_panels.grids, pos+QPointF(-1,0), pos, 1,0);
    }
    else if(status == 4)
    {
        // tolgo hgrid
        if(oldStatus == 3)
        {
            ov->moveAndRemove(&m_panels.grids, 
                m_panels.grids.getPosition(), 
                m_panels.grids.getPosition() + QPointF(2,0),            
                1,0);
        }
        else
        {
            ov->removeAll();
            ov->add(&m_panels.title, titlePos);
        }
    }
    else if(status == 5)
    {
        // scambio titoli
        QPointF pos(0.5,0.7);

        ov->moveAndRemove(&m_panels.title, 
            m_panels.title.getPosition(), 
            m_panels.title.getPosition() + QPointF(0, 1), 
            1,0);
        ov->addAndMove(&m_panels.title2, 
            pos + QPointF(-1.0,0), pos, 
            1,0.2);
    }


}
