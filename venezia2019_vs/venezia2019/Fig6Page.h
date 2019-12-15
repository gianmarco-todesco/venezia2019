#pragma once


#include "Page.h"
#include "H2TessellationPage.h"

#include <QString>
#include <QMatrix4x4>


class Fig6Page : public H2TessellationPage {
  
public:
    Fig6Page(bool veryDetailed = false);
    ~Fig6Page();    
    void savePictures();
};
