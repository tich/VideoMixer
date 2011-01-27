#include "glwidget.h"
#include <QPainter>
#include <math.h>
#include "gstvideoplayer.h"

inline void IdentityMatrix(GLfloat *m)
{
    m[0 * 4 + 0] = 1.0f;
    m[1 * 4 + 0] = 0.0f;
    m[2 * 4 + 0] = 0.0f;
    m[3 * 4 + 0] = 0.0f;
    m[0 * 4 + 1] = 0.0f;
    m[1 * 4 + 1] = 1.0f;
    m[2 * 4 + 1] = 0.0f;
    m[3 * 4 + 1] = 0.0f;
    m[0 * 4 + 2] = 0.0f;
    m[1 * 4 + 2] = 0.0f;
    m[2 * 4 + 2] = 1.0f;
    m[3 * 4 + 2] = 0.0f;
    m[0 * 4 + 3] = 0.0f;
    m[1 * 4 + 3] = 0.0f;
    m[2 * 4 + 3] = 0.0f;
    m[3 * 4 + 3] = 1.0f;
}

GLWidget::GLWidget(QWidget *parent, int video_index)
    : QGLWidget(parent)
{
    createdVertices = 0;
    createdNormals = 0;
    m_vertexNumber = 0;
    frames = 0;
    first=true;
    video_stream = video_index;
    setAttribute(Qt::WA_PaintOnScreen,true);
//    setAttribute(Qt::WA_NoSystemBackground,true);

    setAutoBufferSwap(false);
/*
#ifndef Q_WS_QWS
    setMinimumSize(300, 250);
#endif*/
}

GLWidget::~GLWidget()
{
  if (createdVertices)
      delete[] createdVertices;
  if (createdNormals)
      delete[] createdNormals;
}

void GLWidget::refresh_texture()
{
    if(gstVideoPlayer::scanning)
    {
        gstVideoPlayer::refresh_buffer(video_stream); //manually call appsink, not through the callback function

        if(gstVideoPlayer::buffer!=NULL && gstVideoPlayer::buffer[video_stream]!=NULL)
        {
            glBindTexture( GL_TEXTURE_2D, m_uiTexture );
            unsigned char* data=(unsigned char *) GST_BUFFER_DATA (gstVideoPlayer::buffer[video_stream]);
            if(first)
            {
                QString bla(gst_caps_to_string(gstVideoPlayer::buffer[video_stream]->caps));
                printf("%s",bla.toStdString().c_str());
                QString b = bla.remove(0, bla.indexOf("width=(int)")+11);
                QString width = b.left(b.indexOf(", height"));
                iwidth = width.toInt();
                b = bla.remove(0, bla.indexOf("height=(int)")+12);
                QString height = b.left(b.indexOf(", framerate"));
                iheight = height.toInt();
                first = false;
                printf("\nwidth = %d, height = %d\n", iwidth, iheight);
            }
            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, iwidth,iheight, 0, GL_RGB, GL_UNSIGNED_BYTE, data );
            gst_buffer_unref(gstVideoPlayer::buffer[video_stream]);
        }
        else
            printf("Buffer null\n");
    }
}



void GLWidget::paintTexturedCube()
{
	//refresh_texture();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_uiTexture);
	GLfloat afVertices[] = {
		-1.0,-1.0,-1.0, 1.0,-1.0,-1.0, -1.0,1.0,-1.0,
		1.0, 1.0,-1.0, -1.0,1.0,-1.0, 1.0,-1.0,-1.0,
	};
	glVertexAttribPointer(vertexAttr2, 3, GL_FLOAT, GL_FALSE, 0, afVertices);
	glEnableVertexAttribArray(vertexAttr2);


	GLfloat afTexCoord[] = {
		// My Code For Texture Vertices
		0.0f,1.0f, 1.0f,1.0f, 0.0f,0.0f,
		1.0f,0.0f, 0.0f,0.0f, 1.0f,1.0f,
	};
	glVertexAttribPointer(texCoordAttr2, 2, GL_FLOAT, GL_FALSE, 0, afTexCoord);
	glEnableVertexAttribArray(texCoordAttr2);

	GLfloat afNormals[] = {
		0,0,1, 0,0,1, 0,0,1,
		0,0,1, 0,0,1, 0,0,1,
	};
	glVertexAttribPointer(normalAttr2, 3, GL_FLOAT, GL_FALSE, 0, afNormals);
	glEnableVertexAttribArray(normalAttr2);

	glUniform1i(textureUniform2, 0);    // use texture unit 0

	glDrawArrays(GL_TRIANGLES, 0, 6);

}

static void reportCompileErrors(GLuint shader, const char *src)
{
    GLint value = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &value);
    bool compiled = (value != 0);
    value = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &value);
    if (!compiled && value > 1) {
        char *log = new char [value];
        GLint len;
        glGetShaderInfoLog(shader, value, &len, log);
        qWarning("%s\n", log);
        qWarning("when compiling:\n%s\n", src);
        delete [] log;
    }
}

static void reportLinkErrors(GLuint program, const char *vsrc, const char *fsrc)
{
    GLint value = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &value);
    bool linked = (value != 0);
    value = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &value);
    if (!linked && value > 1) {
        char *log = new char [value];
        GLint len;
        glGetProgramInfoLog(program, value, &len, log);
        qWarning("%s\n", log);
        qWarning("when linking:\n%s\nwith:\n%s\n", vsrc, fsrc);
        delete [] log;
    }
}

void GLWidget::initializeGL ()
{
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &m_uiTexture);
   // m_uiTexture = bindTexture(QImage(":/qt.png"));
    glewInit();
    GLuint vshader1 = glCreateShader(GL_VERTEX_SHADER);
    const char *vsrc1[1] = {
        "attribute highp vec4 vertex;\n"
        "attribute mediump vec3 normal;\n"
        "uniform mediump mat4 matrix;\n"
        "varying mediump vec4 color;\n"
        "void main(void)\n"
        "{\n"
        "    vec3 toLight = normalize(vec3(0.0, 0.3, 1.0));\n"
        "    float angle = max(dot(normal, toLight), 0.0);\n"
        "    vec3 col = vec3(0.40, 1.0, 0.0);\n"
        "    color = vec4(col * 0.2 + col * 0.8 * angle, 1.0);\n"
        "    color = clamp(color, 0.0, 1.0);\n"
        "    gl_Position = matrix * vertex;\n"
        "}\n"
    };
    glShaderSource(vshader1, 1, vsrc1, 0);
    glCompileShader(vshader1);
    reportCompileErrors(vshader1, vsrc1[0]);

    GLuint fshader1 = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fsrc1[1] = {
        "varying mediump vec4 color;\n"
        "void main(void)\n"
        "{\n"
        "    gl_FragColor = color;\n"
        "}\n"
    };
    glShaderSource(fshader1, 1, fsrc1, 0);
    glCompileShader(fshader1);
    reportCompileErrors(fshader1, fsrc1[0]);

    program1 = glCreateProgram();
    glAttachShader(program1, vshader1);
    glAttachShader(program1, fshader1);
    glLinkProgram(program1);
    reportLinkErrors(program1, vsrc1[0], fsrc1[0]);

    vertexAttr1 = glGetAttribLocation(program1, "vertex");
    normalAttr1 = glGetAttribLocation(program1, "normal");
    matrixUniform1 = glGetUniformLocation(program1, "matrix");

    GLuint vshader2 = glCreateShader(GL_VERTEX_SHADER);
    const char *vsrc2[1] = {
        "attribute highp vec4 vertex;\n"
        "attribute highp vec4 texCoord;\n"
        "attribute mediump vec3 normal;\n"
        "uniform mediump mat4 matrix;\n"
        "varying highp vec4 texc;\n"
        "varying mediump float angle;\n"
        "void main(void)\n"
        "{\n"
        "    vec3 toLight = normalize(vec3(0.0, 0.3, 1.0));\n"
        "    angle = max(dot(normal, toLight), 0.0);\n"
        "    gl_Position = matrix * vertex;\n"
        "    texc = texCoord;\n"
        "}\n"
    };
    glShaderSource(vshader2, 1, vsrc2, 0);
    glCompileShader(vshader2);
    reportCompileErrors(vshader2, vsrc2[0]);

    GLuint fshader2 = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fsrc2[1] = {
        "varying highp vec4 texc;\n"
        "uniform sampler2D tex;\n"
        "varying mediump float angle;\n"
        "void main(void)\n"
        "{\n"
        "    highp vec3 color = texture2D(tex, texc.st).rgb;\n"
        "    color = color * 0.2 + color * 0.8 * angle;\n"
        "    gl_FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);\n"
        "}\n"
    };
    glShaderSource(fshader2, 1, fsrc2, 0);
    glCompileShader(fshader2);
    reportCompileErrors(fshader2, fsrc2[0]);

    program2 = glCreateProgram();
    glAttachShader(program2, vshader2);
    glAttachShader(program2, fshader2);
    glLinkProgram(program2);
    reportLinkErrors(program2, vsrc2[0], fsrc2[0]);

    vertexAttr2 = glGetAttribLocation(program2, "vertex");
    normalAttr2 = glGetAttribLocation(program2, "normal");
    texCoordAttr2 = glGetAttribLocation(program2, "texCoord");
    matrixUniform2 = glGetUniformLocation(program2, "matrix");
    textureUniform2 = glGetUniformLocation(program2, "tex");

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    m_fAngle = 0;
    m_fScale = 1;


}

void GLWidget::paintEvent (QPaintEvent *event)
{
    paintGL();
}

void GLWidget::paintGL()
{
    QPainter painter;
    painter.begin(this);

    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    refresh_texture();
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    GLfloat modelview[16];
    IdentityMatrix(modelview);


    glUseProgram(program2);
    glUniformMatrix4fv(matrixUniform2, 1, GL_FALSE, modelview);
    paintTexturedCube();
    glUseProgram(0);

    QString framesPerSecond;
    framesPerSecond.setNum(frames /(time.elapsed() / 1000.0), 'f', 2);

    painter.setPen(Qt::white);

    painter.drawText(20, 40, framesPerSecond + " fps");
    painter.end();

    swapBuffers();

    if (!(frames % 100)) {
        time.start();
        frames = 0;
    }
    m_fAngle += 1.0f;
    frames ++;
 }
