#include "DummyPage.h"
#include "Gutil.h"

void DummyPage::initializeGL() 
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
}

void DummyPage::resizeGL(int width, int height) 
{
    double aspect = (float)width/height;
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, aspect, 1.0, 70.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void DummyPage::paintGL() 
{        
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawBackground();

    glPushMatrix();
    glTranslated(0,0,-10);
    glRotated(20,1,0,0);
    glRotated(30,0,1,0);


    GLfloat specular[] =  { 0.7f, 0.7f, 0.7f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 90.0);

    // draw();
    drawAxes();

    setColor(1,0,0);
    drawSphere(QVector3D(0,0,0), 1);

    glPopMatrix();
}
