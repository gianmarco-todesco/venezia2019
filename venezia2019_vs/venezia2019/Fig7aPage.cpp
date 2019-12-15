#include "Fig7aPage.h"

// fig. 7a: A regular Hyperbolic tessellation: {8,3}


Fig7aPage::Fig7aPage(bool veryDetailed)
    : H2TessellationPage(8,3,veryDetailed)
{
}

Fig7aPage::~Fig7aPage()
{
}

    
void Fig7aPage::savePictures()
{
    Fig7aPage page;
    page.savePicture("fig7a.png");
}
