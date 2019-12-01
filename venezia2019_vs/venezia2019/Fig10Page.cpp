#include "Fig10Page.h"

// #define _USE_MATH_DEFINES
// #include <math.h>
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

extern QVector3D rainbow(double t);
extern QMatrix4x4 makeTranslation(double x, double y, double z);


Fig10Page::Fig10Page(double radius)
    : m_radius(radius)
    , m_edgeIndex(8)
{
}
    
Fig10Page::~Fig10Page()
{
}

void Fig10Page::paintGL() 
{        
    glClearColor(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // drawBackground();
    
    glPushMatrix();
    glTranslated(0,0,-m_cameraDistance);
    glRotated(m_theta, 1,0,0);
    glRotated(m_phi, 0,1,0);

    GLfloat specular[] =  { 0.7f, 0.7f, 0.7f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 90.0);

    draw();
    drawOutSphere();

    glPopMatrix();
}

Fig10aPage::Fig10aPage()
    : Fig10Page(0.5)
{
}

Fig10bPage::Fig10bPage()
    : Fig10Page(0.99)
{
}

void Fig10Page::draw()
{
    QGLShaderProgram *prog;

    m_hMatrix = makeTranslation(0,0,0);

    // hpolyhedron 
    m_texture1.bind();    
    prog = m_h3ShaderProgram;
    prog->bind();
    // setViewUniforms(prog);
    prog->setUniformValue("u_texture", 0);
    prog->setUniformValue("u_texScale", QPointF(20,20));   
    prog->setUniformValue("hMatrix", m_hMatrix);
    prog->setUniformValue("u_color", QVector3D(1,1,1));

    double radius = m_radius;
    
    setColor(1,0,1);
    Polyhedron *ph = makeDodecahedron();
    ph->computeFaceVertices();
    ph->scale(radius / ph->getVertex(0).m_pos.length());
    drawHPolyhedron(ph);

    m_texture1.release();
    prog->release();

    // dihedral angle
    int edgeIndex = m_edgeIndex;
    if(edgeIndex<0) edgeIndex=0;
    else if(edgeIndex>ph->getEdgeCount()-1) edgeIndex = ph->getEdgeCount()-1;
    const Polyhedron::Edge &edge = ph->getEdge(edgeIndex);
    QVector3D midPoint = (
        ph->getVertex(edge.m_a).m_pos + 
        ph->getVertex(edge.m_b).m_pos)*0.5;
    QList<int> fis;
    for(int i=0;i<ph->getFaceCount();i++)
    {
        bool found = false;
        const Polyhedron::Face &face = ph->getFace(i);
        for(int j=0;j<face.m_edges.size();j++) 
            if(face.m_edges[j]==edgeIndex)
            {
                found=true;
                break;
            }
        if(found)
            fis.append(i);
    }
    if(fis.count() == 2)
    {
        const QVector3D p0 = getFaceCenter(ph, fis[0]);
        const QVector3D p1 = getFaceCenter(ph, fis[1]);
        drawHLine(midPoint,p0);
        drawHLine(midPoint,p1);
        drawHAngle(midPoint,midPoint-p0,midPoint-p1);
    }


    delete ph;
}


void Fig10Page::savePictures()
{
    Fig10Page *page = clone();
    //page.m_cameraDistance = m_cameraDistance;
    //page.m_theta = m_theta;
    //page.m_phi = m_phi;
    page->savePicture(getFigureName());
    delete page;
}


void Fig10Page:: keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Plus) m_edgeIndex++;
    else if(e->key() == Qt::Key_Minus) m_edgeIndex--;
}

