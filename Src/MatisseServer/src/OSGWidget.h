#ifndef OSGWidget_h__
#define OSGWidget_h__

#include "GraphicalCharter.h"
#include <QPoint>
#include <QtOpenGL>
#include <QTimer>

#include <osg/ref_ptr>

#include <osgViewer/GraphicsWindow>
#include <osgViewer/CompositeViewer>

class OSGWidget : public QGLWidget
{
    Q_OBJECT

public:
    OSGWidget( QWidget* parent = 0,
               const QGLWidget* shareWidget = 0,
               Qt::WindowFlags f = 0 );

    virtual ~OSGWidget();

    ///
    /// \brief setSceneFromFile load a scene from a 3D file
    /// \param sceneFile_p path to any 3D file supported by osg
    /// \return true if loading succeded
    ///
    bool setSceneFromFile(char* sceneFile_p);

    ///
    /// \brief setClearColor set the clear color for all cameras
    /// \param r_p red [0..1]
    /// \param g_p green [0..1]
    /// \param b_p blue [0..1]
    /// \param alpha_p transparency [0..1]
    ///
    void setClearColor(double r_p, double g_p, double b_p, double alpha_p=1.0);

    ///
    /// \brief clearSceneData removes scene data
    ///
    void clearSceneData();

protected:

    virtual void paintEvent( QPaintEvent* paintEvent );
    virtual void paintGL();
    virtual void resizeGL( int width, int height );

    virtual void keyPressEvent( QKeyEvent* event );
    virtual void keyReleaseEvent( QKeyEvent* event );

    virtual void mouseMoveEvent( QMouseEvent* event );
    virtual void mousePressEvent( QMouseEvent* event );
    virtual void mouseReleaseEvent( QMouseEvent* event );
    virtual void wheelEvent( QWheelEvent* event );

    virtual bool event( QEvent* event );

private:

    virtual void onHome();
    virtual void onResize( int width, int height );

    osgGA::EventQueue* getEventQueue() const;

    osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> graphicsWindow_;
    osg::ref_ptr<osgViewer::CompositeViewer> viewer_;

    QPoint selectionStart_;
    QPoint selectionEnd_;

    bool selectionActive_;
    bool selectionFinished_;
    bool _stereoActive;

    osg::ref_ptr<osg::Node> _loadedModel;

    void processSelection();

protected:
    QTimer _timer;

};

#endif
