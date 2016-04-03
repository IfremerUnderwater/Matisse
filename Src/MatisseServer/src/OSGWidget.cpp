#include "OSGWidget.h"
//#include "PickHandler.h"

#include <osg/Camera>

#include <osg/DisplaySettings>
#include <osg/Geode>
#include <osg/Material>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/StateSet>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgGA/EventQueue>
#include <osgGA/TrackballManipulator>

#include <osgUtil/IntersectionVisitor>
#include <osgUtil/PolytopeIntersector>
#include <osgUtil/Optimizer>

#include <osgViewer/View>
#include <osgViewer/ViewerEventHandlers>

#include <cassert>

#include <stdexcept>
#include <vector>

#include <QDebug>
#include <QKeyEvent>
#include <QWheelEvent>

namespace
{

#ifdef WITH_SELECTION_PROCESSING
QRect makeRectangle( const QPoint& first, const QPoint& second )
{
    // Relative to the first point, the second point may be in either one of the
    // four quadrants of an Euclidean coordinate system.
    //
    // We enumerate them in counter-clockwise order, starting from the lower-right
    // quadrant that corresponds to the default case:
    //
    //            |
    //       (3)  |  (4)
    //            |
    //     -------|-------
    //            |
    //       (2)  |  (1)
    //            |

    if( second.x() >= first.x() && second.y() >= first.y() )
        return QRect( first, second );
    else if( second.x() < first.x() && second.y() >= first.y() )
        return QRect( QPoint( second.x(), first.y() ), QPoint( first.x(), second.y() ) );
    else if( second.x() < first.x() && second.y() < first.y() )
        return QRect( second, first );
    else if( second.x() >= first.x() && second.y() < first.y() )
        return QRect( QPoint( first.x(), second.y() ), QPoint( second.x(), first.y() ) );

    // Should never reach that point...
    return QRect();
}
#endif

}

OSGWidget::OSGWidget( QWidget* parent,
                      const QGLWidget* shareWidget,
                      Qt::WindowFlags f )
    : QGLWidget( parent,
                 shareWidget,
                 f )
    , _graphicsWindow( new osgViewer::GraphicsWindowEmbedded( this->x(),
                                                              this->y(),
                                                              this->width(),
                                                              this->height() ) )
    , _viewer( new osgViewer::CompositeViewer )
    , _selectionActive( false )
    , _selectionFinished( true )
    , _stereoActive(false)
{


    float aspectRatio = static_cast<float>( this->width() ) / static_cast<float>( this->height() );

    osg::Camera* camera = new osg::Camera;
    camera->setViewport( 0, 0, this->width() , this->height() );

    // Set clear color
    QColor clearColor = QColor(MATISSE_BLACK);
    camera->setClearColor( osg::Vec4( clearColor.redF(), clearColor.greenF(), clearColor.blueF(), clearColor.alphaF() ) );

    camera->setProjectionMatrixAsPerspective( 30.f, aspectRatio, 1.f, 1000.f );
    camera->setGraphicsContext( _graphicsWindow );

    osgViewer::View* view = new osgViewer::View;
    view->setCamera( camera );

    view->addEventHandler( new osgViewer::StatsHandler );
#ifdef WITH_PICK_HANDLER
    view->addEventHandler( new PickHandler );
#endif

    osgGA::TrackballManipulator* manipulator = new osgGA::TrackballManipulator;
    //manipulator->setAllowThrow( false );

    view->setCameraManipulator( manipulator );

    _viewer->addView( view );
    _viewer->setThreadingModel( osgViewer::CompositeViewer::SingleThreaded );
    _viewer->realize();

    this->setAutoBufferSwap( false );

    // This ensures that the widget will receive keyboard events. This focus
    // policy is not set by default. The default, Qt::NoFocus, will result in
    // keyboard events that are ignored.
    this->setFocusPolicy( Qt::StrongFocus );
    this->setMinimumSize( 100, 100 );

    // Ensures that the widget receives mouse move events even though no
    // mouse button has been pressed. We require this in order to let the
    // graphics window switch viewports properly.
    this->setMouseTracking( true );

    connect( &_timer, SIGNAL(timeout()), this, SLOT(update()) );
    _timer.start( 10 );
}

OSGWidget::~OSGWidget()
{
}

bool OSGWidget::setSceneFromFile(std::string sceneFile_p)
{
    // load the data
    setlocale(LC_ALL, "C");
    //_loadedModel = osgDB::readRefNodeFile(sceneFile_p, new osgDB::Options("noTriStripPolygons"));
    _loadedModel = osgDB::readRefNodeFile(sceneFile_p);
    if (!_loadedModel)
    {
        std::cout << "No data loaded" << std::endl;
        return false;
    }

    // optimize the scene graph, remove redundant nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(_loadedModel.get());

    osgViewer::View *view = _viewer->getView(0);

    view->setSceneData( _loadedModel.get() );

    return true;

}

bool OSGWidget::setSceneData(osg::ref_ptr<osg::Node> sceneData_p)
{
    if (!sceneData_p)
    {
        std::cout << "No data loaded" << std::endl;
        return false;
    }

    _loadedModel = sceneData_p;

    osgViewer::View *view = _viewer->getView(0);

    view->setSceneData( _loadedModel.get() );

    return true;
}

void OSGWidget::setClearColor(double r_p, double g_p, double b_p, double alpha_p)
{
    std::vector<osg::Camera*> cameras;
    _viewer->getCameras( cameras );

    for (unsigned int i=0; i<cameras.size(); i++){
        cameras[i]->setClearColor( osg::Vec4( r_p, g_p, b_p, alpha_p ));
    }

}

void OSGWidget::clearSceneData()
{
    osgViewer::View *view = _viewer->getView(0);
    view->setSceneData( 0 );

    _loadedModel = NULL;

}

void OSGWidget::paintEvent( QPaintEvent* /* paintEvent */ )
{
    this->makeCurrent();

    QPainter painter( this );
    painter.setRenderHint( QPainter::Antialiasing );

    this->paintGL();

#ifdef WITH_SELECTION_PROCESSING
    if( selectionActive_ && !selectionFinished_ )
    {
        painter.setPen( Qt::black );
        painter.setBrush( Qt::transparent );
        painter.drawRect( makeRectangle( selectionStart_, selectionEnd_ ) );
    }
#endif

    painter.end();

    this->swapBuffers();
    this->doneCurrent();
}

void OSGWidget::paintGL()
{
    _viewer->frame();
}

void OSGWidget::resizeGL( int width, int height )
{
    this->getEventQueue()->windowResize( this->x(), this->y(), width, height );
    _graphicsWindow->resized( this->x(), this->y(), width, height );

    this->onResize( width, height );
}

void OSGWidget::keyPressEvent( QKeyEvent* event )
{
    QString keyString   = event->text();
    const char* keyData = keyString.toLocal8Bit().data();

    if( event->key() == Qt::Key_S )
    {
        /*#ifdef WITH_SELECTION_PROCESSING
        selectionActive_ = !selectionActive_;
#endif*/

        // Further processing is required for the statistics handler here, so we do
        // not return right away.
        if (_stereoActive){
            osg::DisplaySettings::instance()->setStereo(false);
            _stereoActive=false;
        }
        else{
            osg::DisplaySettings::instance()->setStereo(true);
            _stereoActive=true;
        }
    }
    else if( event->key() == Qt::Key_D )
    {
        osgDB::writeNodeFile( *_viewer->getView(0)->getSceneData(),
                              "/tmp/sceneGraph.osg" );

        return;
    }
    else if( event->key() == Qt::Key_H )
    {
        this->onHome();
        return;
    }

    this->getEventQueue()->keyPress( osgGA::GUIEventAdapter::KeySymbol( *keyData ) );
}

void OSGWidget::keyReleaseEvent( QKeyEvent* event )
{
    QString keyString   = event->text();
    const char* keyData = keyString.toLocal8Bit().data();

    this->getEventQueue()->keyRelease( osgGA::GUIEventAdapter::KeySymbol( *keyData ) );
}

void OSGWidget::mouseMoveEvent( QMouseEvent* event )
{
    // Note that we have to check the buttons mask in order to see whether the
    // left button has been pressed. A call to `button()` will only result in
    // `Qt::NoButton` for mouse move events.
    if( _selectionActive && event->buttons() & Qt::LeftButton )
    {
        _selectionEnd = event->pos();

        // Ensures that new paint events are created while the user moves the
        // mouse.
        this->update();
    }
    else
    {
        this->getEventQueue()->mouseMotion( static_cast<float>( event->x() ),
                                            static_cast<float>( event->y() ) );
    }
}

void OSGWidget::mousePressEvent( QMouseEvent* event )
{
    // Selection processing
    if( _selectionActive && event->button() == Qt::LeftButton )
    {
        _selectionStart    = event->pos();
        _selectionEnd      = _selectionStart; // Deletes the old selection
        _selectionFinished = false;           // As long as this is set, the rectangle will be drawn
    }

    // Normal processing
    else
    {
        // 1 = left mouse button
        // 2 = middle mouse button
        // 3 = right mouse button

        unsigned int button = 0;

        switch( event->button() )
        {
        case Qt::LeftButton:
            button = 1;
            break;

        case Qt::MiddleButton:
            button = 2;
            break;

        case Qt::RightButton:
            button = 3;
            break;

        default:
            break;
        }

        this->getEventQueue()->mouseButtonPress( static_cast<float>( event->x() ),
                                                 static_cast<float>( event->y() ),
                                                 button );
    }
}

void OSGWidget::mouseReleaseEvent(QMouseEvent* event)
{
    // Selection processing: Store end position and obtain selected objects
    // through polytope intersection.
    if( _selectionActive && event->button() == Qt::LeftButton )
    {
        _selectionEnd      = event->pos();
        _selectionFinished = true; // Will force the painter to stop drawing the
        // selection rectangle

        this->processSelection();
    }

    // Normal processing
    else
    {
        // 1 = left mouse button
        // 2 = middle mouse button
        // 3 = right mouse button

        unsigned int button = 0;

        switch( event->button() )
        {
        case Qt::LeftButton:
            button = 1;
            break;

        case Qt::MiddleButton:
            button = 2;
            break;

        case Qt::RightButton:
            button = 3;
            break;

        default:
            break;
        }

        this->getEventQueue()->mouseButtonRelease( static_cast<float>( event->x() ),
                                                   static_cast<float>( event->y() ),
                                                   button );
    }
}

void OSGWidget::wheelEvent( QWheelEvent* event )
{
    // Ignore wheel events as long as the selection is active.
    if( _selectionActive )
        return;

    event->accept();
    int delta = event->delta();

    osgGA::GUIEventAdapter::ScrollingMotion motion = delta > 0 ?   osgGA::GUIEventAdapter::SCROLL_UP
                                                                 : osgGA::GUIEventAdapter::SCROLL_DOWN;

    this->getEventQueue()->mouseScroll( motion );
}

bool OSGWidget::event( QEvent* event )
{
    bool handled = QGLWidget::event( event );

    // This ensures that the OSG widget is always going to be repainted after the
    // user performed some interaction. Doing this in the event handler ensures
    // that we don't forget about some event and prevents duplicate code.
    switch( event->type() )
    {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
    case QEvent::Wheel:
        this->update();
        break;

    default:
        break;
    }

    return handled;
}

void OSGWidget::onHome()
{
    osgViewer::ViewerBase::Views views;
    _viewer->getViews( views );

    for( std::size_t i = 0; i < views.size(); i++ )
    {
        osgViewer::View* view = views.at(i);
        view->home();
    }
}

void OSGWidget::onResize( int width, int height )
{
    std::vector<osg::Camera*> cameras;
    _viewer->getCameras( cameras );

    cameras[0]->setViewport( 0, 0, width, height );
    //cameras[1]->setViewport( this->width() / 2, 0, this->width() / 2, this->height() );
}

osgGA::EventQueue* OSGWidget::getEventQueue() const
{
    osgGA::EventQueue* eventQueue = _graphicsWindow->getEventQueue();

    if( eventQueue )
        return eventQueue;
    else
        throw std::runtime_error( "Unable to obtain valid event queue");
}

void OSGWidget::processSelection()
{
#ifdef WITH_SELECTION_PROCESSING
    QRect selectionRectangle = makeRectangle( selectionStart_, selectionEnd_ );
    int widgetHeight         = this->height();

    double xMin = selectionRectangle.left();
    double xMax = selectionRectangle.right();
    double yMin = widgetHeight - selectionRectangle.bottom();
    double yMax = widgetHeight - selectionRectangle.top();

    osgUtil::PolytopeIntersector* polytopeIntersector
            = new osgUtil::PolytopeIntersector( osgUtil::PolytopeIntersector::WINDOW,
                                                xMin, yMin,
                                                xMax, yMax );

    // This limits the amount of intersections that are reported by the
    // polytope intersector. Using this setting, a single drawable will
    // appear at most once while calculating intersections. This is the
    // preferred and expected behaviour.
    polytopeIntersector->setIntersectionLimit( osgUtil::Intersector::LIMIT_ONE_PER_DRAWABLE );

    osgUtil::IntersectionVisitor iv( polytopeIntersector );

    for( unsigned int viewIndex = 0; viewIndex < viewer_->getNumViews(); viewIndex++ )
    {
        osgViewer::View* view = viewer_->getView( viewIndex );

        if( !view )
            throw std::runtime_error( "Unable to obtain valid view for selection processing" );

        osg::Camera* camera = view->getCamera();

        if( !camera )
            throw std::runtime_error( "Unable to obtain valid camera for selection processing" );

        camera->accept( iv );

        if( !polytopeIntersector->containsIntersections() )
            continue;

        auto intersections = polytopeIntersector->getIntersections();

        for( auto&& intersection : intersections )
            qDebug() << "Selected a drawable:" << QString::fromStdString( intersection.drawable->getName() );
    }
#endif
}
