#include "Fig7bPage.h"

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


#include "Viewer.h"

// fig.7 (a) A regular Hyperbolic tessellation: {8,3} (b) the structure of Escher's "Circle Limit III"


Fig7bPage::Fig7bPage()
    : CircleLimit3Page(true)
{
}
    
Fig7bPage::~Fig7bPage()
{
}






void Fig7bPage::savePictures()
{
    Fig7bPage page;
    page.savePicture("fig7b.png");
    // savePicture("fig7b.png");
}
