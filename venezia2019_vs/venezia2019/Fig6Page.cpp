#include "Fig6Page.h"

// fig.6 A regular Hyperbolic tessellation: {8,4}

Fig6Page::Fig6Page(bool veryDetailed)
    : H2TessellationPage(8,4,veryDetailed)
{
}

Fig6Page::~Fig6Page()
{
}

void Fig6Page::savePictures()
{
    Fig6Page page(true);
    page.savePicture("fig6.png");
}

