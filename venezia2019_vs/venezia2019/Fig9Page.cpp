#include "Fig9Page.h"

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

// fig.9 Geometric objects of the Hyperbolic space represented in the Poincar� ball: 
// (a) points and lines (b) many squares perpendicular to the same line (c) a Hyperbolic regular dodecahedron


Fig9Page::Fig9Page()
{
}
    
Fig9Page::~Fig9Page()
{
}

void Fig9Page::paintGL() 
{        
    glClearColor(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(true);

    // drawBackground();

    
    
    glPushMatrix();
    glTranslated(0,0,-m_cameraDistance);
    glRotated(m_theta, 1,0,0);
    glRotated(m_phi, 0,1,0);

    GLfloat specular[] =  { 0.7f, 0.7f, 0.7f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 90.0);

    draw();
    drawOutSphere2();

    /*

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    setColor(0.4,0.8,0.4);
    drawSphere(QVector3D(0.5,0,0.8),0.6);
    setColor(0.8,0.4,0.4);
    drawSphere(QVector3D(-0.5,0,0.8),0.6);
    */


    glPopMatrix();
}




void Fig9Page::savePictures()
{
    Fig9Page *page = clone();
    //page.m_cameraDistance = m_cameraDistance;
    //page.m_theta = m_theta;
    //page.m_phi = m_phi;
    page->savePicture(getFigureName());
    delete page;
    v()->makeCurrent();
    resizeGL(width(), height());
}


extern QVector3D rainbow(double t);
extern QMatrix4x4 makeTranslation(double x, double y, double z);





Fig9aPage::Fig9aPage()
{
    m_theta = 36.5;
    m_phi = -41.25;
}

void Fig9aPage::draw()
{
    QGLShaderProgram *prog;

    m_hMatrix = makeTranslation(0,0,m_parameter);
    // diversi punti e diverse rette

    m_hMatrix = makeTranslation(m_hPan.x(),0,m_hPan.y());
        

    QVector3D p0(0.7,-0.1,0.1), p1(0.1,0.07,0.5);
    QVector3D p(0.2,0.8,0.3);
    setColor(0,0.5,0.9);
    drawHPoint(p0);
    drawHPoint(p1);
    drawHPoint(p);
    drawHLine2(p0,p);
    drawHLine2(p1,p);
    drawHLine2(p0,p1);

    assert(glGetError() == GL_NO_ERROR);
}

Fig9bPage::Fig9bPage()
{
    m_theta = 34;
    m_phi = -62.5;
}

void Fig9bPage::draw()
{
    QGLShaderProgram *prog;

    m_hMatrix = makeTranslation(0,0,m_parameter);
    glDepthMask(true);
    glDepthFunc(GL_LESS);
    // draw faces
        
    m_texture2.bind();    
    prog = m_h3ShaderProgram;
    prog->bind();
    // setViewUniforms(prog);
    prog->setUniformValue("u_texture", 0);
    prog->setUniformValue("u_texScale", QPointF(1,1));

    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    m_mesh.bind();

    int m = 10;
    for(int i=0; i<=m;i++)
    {

        prog->setUniformValue("u_color", rainbow(0.5*(double)i/m));
        double z = 0.9*(i-5.0)/5.0;
        QMatrix4x4 mat = H3::KModel::translation(QVector3D(0,0,0), QVector3D(0,0,z));
        prog->setUniformValue("hMatrix", m_hMatrix * mat);
        m_mesh.draw();
    }
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);    
    m_mesh.release();
    prog->release();
    m_texture2.release();

}



Fig9cPage::Fig9cPage()
{
    m_theta = 36.5;
    m_phi = -41.25;
}

void Fig9cPage::draw()
{
    QGLShaderProgram *prog;

    m_hMatrix = makeTranslation(0,0,0);

    // hpolyhedron 

    m_texture1.bind();    
    prog = m_h3ShaderProgram;
    prog->bind();
    // setViewUniforms(prog);
    prog->setUniformValue("u_texture", 0);
    prog->setUniformValue("u_texScale", QPointF(1,1));   
    prog->setUniformValue("hMatrix", m_hMatrix);
    prog->setUniformValue("u_color", QVector3D(1,1,1));

    double radius = 0.99; // qMax(0.1, qMin(0.99, 0.4 + m_parameter));
    setColor(1,0,1);
    Polyhedron *ph = makeDodecahedron();
    ph->computeFaceVertices();
    ph->scale(radius / ph->getVertex(0).m_pos.length());
    drawHPolyhedron(ph);

    m_texture1.release();
    prog->release();
    /*
    int m_status = 5;
    if(m_status == 5)
    {
        // vertex angle
        const Polyhedron::Face &face = ph->getFace(0);

        QVector3D p0 = ph->getVertex(face.m_vertices[1]).m_pos;
        QVector3D p1 = ph->getVertex(face.m_vertices[0]).m_pos;
        QVector3D p2 = ph->getVertex(face.m_vertices[2]).m_pos;
        drawHLine(p0,p1);
        drawHLine(p0,p2);

        drawHAngle(p0,p0-p1,p0-p2);

    } else if(m_status == 6)
    {
        int n = ph->getVertexCount();
        for(int i=0;i<n;i++)
        {
            const QVector3D p1 = ph->getVertex(i).m_pos;
            QVector3D p2;
            double maxDist = 0;
            int k = -1;
            for(int j=0; j<n; j++)
            {
                const QVector3D q = ph->getVertex(j).m_pos;
                double d = (p1-q).length();
                if(d>maxDist) { k= j; maxDist = d; p2 = q; }
            }
            if(i<k) {
                drawHLine(p1,p2);
            }
        }
    }
    else if(m_status == 7)
    {
        // dihedral angle
        const Polyhedron::Edge &edge = ph->getEdge(0);
        QVector3D midPoint = (
            ph->getVertex(edge.m_a).m_pos + 
            ph->getVertex(edge.m_b).m_pos)*0.5;
        QList<int> fis;
        for(int i=0;i<ph->getFaceCount();i++)
        {
            bool found = false;
            const Polyhedron::Face &face = ph->getFace(i);
            for(int j=0;j<face.m_edges.size();j++) 
                if(face.m_edges[j]==0)
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

    } else if(m_status == 6)
    */


    delete ph;
}

