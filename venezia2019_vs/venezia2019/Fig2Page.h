#pragma once


#include "Page.h"
#include "H2TessellationPage.h"

#include <QString>
#include <QMatrix4x4>


class Fig2Page : public H2TessellationPage {
  
public:
    Fig2Page(bool veryDetailed = false);
    ~Fig2Page();    
    void savePictures();
};
