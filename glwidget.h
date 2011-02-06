/* ***********************************************************************************************************
 * THE GLWIDGET CODE TAKEN FROM OPENGLES EXAMPLE EXAMPLE
 * http://qt.gitorious.org/qt/qt/trees/6900cf8d7cb8d1e98f3af2ac7ce909dc3b5fc2ef/examples/opengl/hellogl_es2
 * ************************************************************************************************************/#ifndef GLWIDGET_H
#define GLWIDGET_H
#include <glew.h>
#include <QGLWidget>
#include <QTime>

class GLWidget : public QGLWidget {

    Q_OBJECT
public:
    GLWidget(QWidget *parent = 0, int video_index=0);
    ~GLWidget();
    void refresh_texture();
    void paintTexturedCube();
    QImage image;
protected:
    void paintGL ();
    void initializeGL ();
    void paintEvent (QPaintEvent *);
private:
    int video_stream;
    GLuint  m_uiTexture;
    qreal   m_fAngle;
    qreal   m_fScale;
    GLfloat *createdVertices;
    GLfloat *createdNormals;
    int m_vertexNumber;
    int frames;
    int iwidth, iheight;
    bool first;
    QTime time;
    GLuint program1;
    GLuint program2;
    GLuint vertexAttr1;
    GLuint normalAttr1;
    GLuint matrixUniform1;
    GLuint vertexAttr2;
    GLuint normalAttr2;
    GLuint texCoordAttr2;
    GLuint matrixUniform2;
    GLuint textureUniform2;
signals:
    void processMe(QImage);
};
#endif
