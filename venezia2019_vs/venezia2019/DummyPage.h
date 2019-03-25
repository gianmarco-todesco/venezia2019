#ifndef DUMMYPAGE_H
#define DUMMYPAGE_H

#include "Page.h"


class DummyPage : public Page {
public:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();
};

#endif

