#pragma once


#include "Page.h"
#include "CircleLimit3Page.h"

#include <QString>
#include <QMatrix4x4>


class Fig7bPage : public CircleLimit3Page {
  
public:
    Fig7bPage(bool figureMode = false);
    ~Fig7bPage();
    void paintGL();
    
    void savePictures();
};
