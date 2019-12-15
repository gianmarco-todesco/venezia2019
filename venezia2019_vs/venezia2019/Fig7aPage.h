#pragma once


#include "Page.h"
#include "H2TessellationPage.h"

#include <QString>
#include <QMatrix4x4>


class Fig7aPage : public H2TessellationPage {
  
public:
    Fig7aPage(bool veryDetailed = false);
    ~Fig7aPage();    
    void savePictures();
};
