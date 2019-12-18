#include "Fig2Page.h"

// fig.2 A regular tessellation of the Hyperbolic plane represented on the Poincaré disk model. 
// The tessellation is named {5,4}: four pentagons meet at each vertex.


Fig2Page::Fig2Page(bool veryDetailed)
    : H2TessellationPage(5,4,true)
{
}

Fig2Page::~Fig2Page()
{
}

void Fig2Page::savePictures()
{
    Fig2Page page(true);
    page.savePicture("Fig2.png");
}
