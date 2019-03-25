#ifndef CIRCLELIMIT3VIEWER_H
#define CIRCLELIMIT3VIEWER_H

#include "HTessellationViewer.h"

class CircleLimit3Tessellation;

class CircleLimit3Viewer : public HTessellationViewer
{
  CircleLimit3Tessellation *m_tess;
  Texture *m_fish[4];
  int m_status;
  int m_maxFaceCount;

public:
  CircleLimit3Viewer();
  ~CircleLimit3Viewer();

  // Complex m_hPan;

  //void hPan(double dx, double dy);
  bool onKeyDown(int key);

  void foo();

  void draw();
  void drawTess();

  void loadTextures();
};

#endif // CIRCLELIMIT3VIEWER_H
