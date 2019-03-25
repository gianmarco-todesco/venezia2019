#ifndef TEXTURE_H
#define TEXTURE_H

#include <QtOpenGL>
#include <map>

class Texture
{
public:
  Texture(const QString& fileName);
  virtual ~Texture();

  void load();
  virtual void bind();
  virtual void unbind();
  virtual bool failed() const {return m_failed;}

  static Texture *get(QString fileName);
  static void discard(Texture *texture);

protected:
  GLuint m_texture;
  bool m_failed;
  bool m_loaded;
  QString m_fileName;

private:
  static std::map<QString, Texture *> m_textures;

};

#endif // TEXTURE_H
