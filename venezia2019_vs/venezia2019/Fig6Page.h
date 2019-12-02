#pragma once


#include "Page.h"
#include "PDiskPage.h"

#include <QString>
#include <QMatrix4x4>


class Fig6Page : public PDiskPage {
  
public:
    Fig6Page();
    ~Fig6Page();
    
    void savePictures();

    void resizeGL(int width, int height);
    
    void paintGL();

};
