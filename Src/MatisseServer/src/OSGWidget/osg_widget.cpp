#include "osg_widget.h"

#include <QDebug>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QFileInfo>
#include <QString>
#include <QDir>

#include <osg/Camera>
#include <osg/MatrixTransform>
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
// too slow
//#include <osgUtil/DelaunayTriangulator>

#include <osgViewer/View>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/GUIEventAdapter>

#include <cassert>

#include <stdexcept>
#include <vector>


#include <osg/Referenced>
#include <osg/LineSegment>
#include <osg/Geometry>
#include <osg/Point>
#include <osg/LineWidth>

#include <osg/PolygonMode>
#include <QProcess>
#include <math.h>
#include <limits>

#include <osg/AlphaFunc>
#include <osg/BlendFunc>

#include "box_visitor.h"
#include "minmax_computation_visitor.h"
#include "geometry_type_count_visitor.h"
#include "shader_color.h"

struct SnapImage : public osg::Camera::DrawCallback {
    SnapImage(osg::GraphicsContext* _gc,const std::string& _filename, QPointF &_ref_lat_lon,osg::BoundingBox _box, double _pixel_size) :
        m_filename( _filename ),
        m_ref_lat_lon( _ref_lat_lon ),
        m_box( _box ),
        m_pixel_size( _pixel_size )
    {
        m_image = new osg::Image;
        if (_gc->getTraits()) {
            int width = _gc->getTraits()->width;
            int height = _gc->getTraits()->height;
            m_image->allocateImage(width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE);
        }
    }

    virtual void operator () (osg::RenderInfo& renderInfo) const {
        osg::Camera* camera = renderInfo.getCurrentCamera();

        osg::GraphicsContext* gc = camera->getGraphicsContext();
        if (gc->getTraits() && m_image.valid()) {

            // get the image
            int width = gc->getTraits()->width;
            int height = gc->getTraits()->height;
            m_image->readPixels( 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE );

            // Variable for the command line "gdal_translate"
            double lat_0  = m_ref_lat_lon.x();
            double lon_0 = m_ref_lat_lon.y();
            double x_min = m_box.xMin();
            double y_max = m_box.yMax();

            std::string tiff_name = m_filename+".tif";
            GDALAllRegister();
            CPLPushErrorHandler(CPLQuietErrorHandler);
            GDALDataset *geotiff_dataset;
            GDALDriver *driver_geotiff;

            driver_geotiff = GetGDALDriverManager()->GetDriverByName("GTiff");
            geotiff_dataset = driver_geotiff->Create(tiff_name.c_str(),width,height,4,GDT_Byte,NULL);

            int size = height*width;
			unsigned char *buffer_R = new unsigned char[width];
            unsigned char *buffer_G = new unsigned char[width];
            unsigned char *buffer_B = new unsigned char[width];
            unsigned char *buffer_A = new unsigned char[width];
			
            for(int i=0; i<height; i++) {
                for(int j=0; j<(width); j++) {
                    buffer_R[width-j] = m_image->data(size - ((width*i)+j))[0];
                    buffer_G[width-j] = m_image->data(size - ((width*i)+j))[1];
                    buffer_B[width-j] = m_image->data(size - ((width*i)+j))[2];
                    buffer_A[width-j] = m_image->data(size - ((width*i)+j))[3];

                }
                // CPLErr GDALRasterBand::RasterIO( GDALRWFlag eRWFlag, int nXOff, int nYOff, int nXSize, int nYSize, void * pData, int nBufXSize, int nBufYSize, GDALDataType eBufType, int nPixelSpace, int nLineSpace )

                geotiff_dataset->GetRasterBand(1)->RasterIO(GF_Write,0,i,width,1,buffer_R,width,1,GDT_Byte,0,0);
                geotiff_dataset->GetRasterBand(2)->RasterIO(GF_Write,0,i,width,1,buffer_G,width,1,GDT_Byte,0,0);
                geotiff_dataset->GetRasterBand(3)->RasterIO(GF_Write,0,i,width,1,buffer_B,width,1,GDT_Byte,0,0);
                geotiff_dataset->GetRasterBand(4)->RasterIO(GF_Write,0,i,width,1,buffer_A,width,1,GDT_Byte,0,0);
            }

			delete buffer_R;
            delete buffer_G;
            delete buffer_B;
            delete buffer_A;

            // Setup output coordinate system.
            double geo_transform[6] = { x_min, m_pixel_size, 0, y_max, 0, -m_pixel_size };
            geotiff_dataset->SetGeoTransform(geo_transform);
            char *geo_reference = NULL;
            OGRSpatialReference o_SRS;
            o_SRS.SetTM(lat_0,lon_0,0.9996,0,0);
            o_SRS.SetWellKnownGeogCS( "WGS84" );
            o_SRS.exportToWkt( &geo_reference );

            geotiff_dataset->SetProjection(geo_reference);
            CPLFree( geo_reference );
            GDALClose(geotiff_dataset) ;

            GDALDestroyDriverManager();
        }
    }

    std::string m_filename;
    osg::ref_ptr<osg::Image> m_image;
    QPointF m_ref_lat_lon;
    osg::BoundingBox m_box;
    double m_pixel_size;
};

class KeyboardEventHandler : public osgGA::GUIEventHandler
{
public:

    KeyboardEventHandler(osg::StateSet* stateset):
        _stateset(stateset)
    {
        _point = new osg::Point;
        _point->setDistanceAttenuation(osg::Vec3(0.0,0.0000,0.05f));
        _point->setSize(30);
        _stateset->setAttribute(_point.get());

        _line_width = new osg::LineWidth();
        _line_width->setWidth(4.0);
        _stateset->setAttribute(_line_width.get());

    }

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
    {
        switch(ea.getEventType())
        {
        case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            if (ea.getKey()=='+' || ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Add)
            {
                changePointSize(1.0f);
                changeLineWidth(1.0f);
                return true;
            }
            else if (ea.getKey()=='-' || ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Subtract)
            {
                changePointSize(-1.0f);
                changeLineWidth(-1.0f);
                return true;
            }
            else if (ea.getKey()=='<')
            {
                changePointAttenuation(1.1f);
                return true;
            }
            else if (ea.getKey()=='>')
            {
                changePointAttenuation(1.0f/1.1f);
                return true;
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_L)
            {
                if (_stateset->getMode(GL_LIGHTING) == osg::StateAttribute::OFF)
                    _stateset->setMode(GL_LIGHTING, osg::StateAttribute::ON);
                else
                    _stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
                return true;
            }
            break;
        }
        default:
            break;
        }
        return false;
    }


    float getPointSize() const
    {
        return _point->getSize();
    }

    float getLineWidth() const
    {
        return _line_width->getWidth();
    }

    void setPointSize(float psize)
    {
        if (psize>0.0)
        {
            _point->setSize(psize);
        }
        std::cout<<"Point size "<<psize<<std::endl;
    }

    void setLineWidth(float pwidth)
    {
        if (pwidth>0.0)
        {
            _line_width->setWidth(pwidth);
        }
        std::cout<<"Line width "<<pwidth<<std::endl;
    }

    void changePointSize(float delta)
    {
        setPointSize(getPointSize()+delta);
    }

    void changeLineWidth(float delta)
    {
        setLineWidth(getLineWidth()+delta);
    }

    void changePointAttenuation(float scale)
    {
        _point->setDistanceAttenuation(_point->getDistanceAttenuation()*scale);
    }

    osg::ref_ptr<osg::StateSet> _stateset;
    osg::ref_ptr<osg::Point>    _point;
    osg::ref_ptr<osg::LineWidth> _line_width;

};


const char *const OSGWidget::MEASUREMENT_NAME = "3DMeasurement";

OSGWidget::OSGWidget(QWidget* parent)
    : QOpenGLWidget( parent)
    , m_graphicsWindow( new osgViewer::GraphicsWindowEmbedded( this->x(),
                                                               this->y(),
                                                               this->width(),
                                                               this->height() ) )
    , m_viewer( new osgViewer::CompositeViewer )
    , m_ctrl_pressed(false)
    , m_fake_middle_click_activated(false)
    , m_zScale(1.0)
{

    m_ref_lat_lon.setX(INVALID_VALUE);
    m_ref_lat_lon.setY(INVALID_VALUE);
    m_ref_alt = INVALID_VALUE;


    //osgDB::Registry::instance()->setLibraryFilePathList("/Users/tim/code/3DMetricWorkspace/Run/Release/3DMetrics.app/Contents/osgPlugins");

    float aspectRatio = static_cast<float>( this->width() ) / static_cast<float>( this->height() );

    osg::Camera* camera = new osg::Camera;
    camera->setViewport( 0, 0, this->width() , this->height() );

    // tweak unique point not drawing
    osg::CullStack::CullingMode cullingMode = camera->getCullingMode();
    cullingMode &= ~(osg::CullStack::SMALL_FEATURE_CULLING);
    camera->setCullingMode(cullingMode);

    // Set clear color
    QColor clearColor = QColor(0,0,0);
    camera->setClearColor( osg::Vec4( clearColor.redF(), clearColor.greenF(), clearColor.blueF(), clearColor.alphaF() ) );

    camera->setGraphicsContext( m_graphicsWindow );
    camera->setProjectionMatrixAsPerspective( 30.0f, aspectRatio, 1.f, 1000.f );

    osgViewer::View* view = new osgViewer::View;
    view->setCamera( camera );

    view->addEventHandler( new osgViewer::StatsHandler );
    view->addEventHandler(new KeyboardEventHandler(view->getCamera()->getOrCreateStateSet()));


    osgGA::TrackballManipulator* manipulator = new osgGA::TrackballManipulator;
    manipulator->setAllowThrow( false );

    view->setCameraManipulator( manipulator );

    m_viewer->addView( view );
    m_viewer->setThreadingModel( osgViewer::CompositeViewer::SingleThreaded );
    m_viewer->realize();

    // This ensures that the widget will receive keyboard events. This focus
    // policy is not set by default. The default, Qt::NoFocus, will m_image in
    // keyboard events that are ignored.
    this->setFocusPolicy( Qt::StrongFocus );
    this->setMinimumSize( 100, 100 );

    // Ensures that the widget receives mouse move events even though no
    // mouse button has been pressed. We require this in order to let the
    // graphics window switch viewports properly.
    this->setMouseTracking( true );

    connect( &m_timer, SIGNAL(timeout()), this, SLOT(update()) );
    m_timer.start( 40 ); //10 );

    // Create group that will contain measurement geode and 3D model
    m_globalGroup = new osg::Group;

    m_modelsGroup = new osg::Group;
    m_geodesGroup = new osg::Group;

    m_globalGroup->addChild(m_modelsGroup);
    m_globalGroup->addChild(m_geodesGroup);

    // for Z scale management
    m_matrixTransform = new osg::MatrixTransform;
    m_matrixTransform->setMatrix(osg::Matrix::scale(1.0, 1.0, m_zScale));
    m_matrixTransform->addChild(m_globalGroup);

    // use models' min max as default
    m_useDisplayZMinMax = false;

    // don't show zscale by default
    m_showZScale = false;

    m_modelsZMin = 0;
    m_modelsZMax= 0;

    m_displayZMin = 0;
    m_displayZMax = 0;

    m_colorPalette = ShaderColor::Rainbow;
    m_overlay = new OverlayWidget(this);
    m_overlay->setColorPalette(m_colorPalette);
    m_overlay->setMinMax(m_displayZMin, m_displayZMax);
    m_overlay->show();
}

OSGWidget::~OSGWidget()
{
}

bool OSGWidget::setSceneFromFile(std::string _scene_file)
{
    osg::ref_ptr<osg::Node> node = createNodeFromFile(_scene_file);
    if(!node)
        return false;

    return addNodeToScene(node);
}

///
/// \brief createNodeFromFile load a scene from a 3D file
/// \param _sceneFile path to any 3D file supported by osg
/// \return node if loading succeded
///
osg::ref_ptr<osg::Node> OSGWidget::createNodeFromFile(std::string _scene_file)
{
    osg::ref_ptr<osg::MatrixTransform> model_transform;
    // load the data
    setlocale(LC_ALL, "C");

    QFileInfo scene_info(QString::fromStdString(_scene_file));
    std::string scene_file;

    QPointF local_lat_lon;
    double local_alt;

    if (scene_info.suffix()==QString("kml")){
        m_kml_handler.readFile(_scene_file);
        scene_file = scene_info.absoluteDir().filePath(QString::fromStdString(m_kml_handler.getModelPath())).toStdString();
        local_lat_lon.setX(m_kml_handler.getModelLat());
        local_lat_lon.setY(m_kml_handler.getModelLon());
        local_alt = m_kml_handler.getModelAlt();
    }else{
        scene_file = _scene_file;
        local_lat_lon.setX(0);
        local_lat_lon.setY(0);
        local_alt = 0;
    }

    osg::ref_ptr<osg::Node> model_node=osgDB::readRefNodeFile(scene_file, new osgDB::Options("noRotation"));

    if (!model_node)
    {
        std::cout << "No data loaded" << std::endl;
        return model_transform;

    }

    // Transform model
    model_transform = new osg::MatrixTransform;
    if (m_ref_alt == INVALID_VALUE){
        m_ref_lat_lon = local_lat_lon;
        m_ref_alt = local_alt;
        m_ltp_proj.Reset(m_ref_lat_lon.x(), m_ref_lat_lon.y(),m_ref_alt);

        osg::Matrix matrix = osg::Matrix::identity();
        //matrix.postMultScale(osg::Vec3f(1.0,1.0,m_zScale));
        model_transform->setMatrix(matrix);
    }else{
        double N,E,U;
        m_ltp_proj.Forward(local_lat_lon.x(), local_lat_lon.y(), local_alt, E, N, U);

        // TODO
        model_transform->setMatrix(osg::Matrix::translate(E,N,U));
    }

    model_transform->addChild(model_node);

    return model_transform;
}


///
/// \brief createNodeFromFile load a scene from a 3D file
/// \param _sceneFile path to any 3D file supported by osg
/// \return node if loading succeded
///
osg::ref_ptr<osg::Node> OSGWidget::createNodeFromFileWithGDAL(std::string _scene_file, LoadingMode _mode)
{
    osg::ref_ptr<osg::MatrixTransform> model_transform;

    QPointF local_lat_lon;
    double local_alt;

    GDALAllRegister();
    GDALDataset *dataset = (GDALDataset *) GDALOpen( _scene_file.c_str(), GA_ReadOnly );
    if(dataset != NULL)
    {
        double        adfGeoTransform[6];
        //                adfGeoTransform[0] /* top left x */
        //                adfGeoTransform[1] /* w-e pixel resolution */
        //                adfGeoTransform[2] /* 0 */
        //                adfGeoTransform[3] /* top left y */
        //                adfGeoTransform[4] /* 0 */
        //                adfGeoTransform[5] /* n-s pixel resolution (negative value) */
        printf( "Driver: %s/%s\n",
                dataset->GetDriver()->GetDescription(),
                dataset->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) );
        printf( "Size is %dx%dx%d\n",
                dataset->GetRasterXSize(), dataset->GetRasterYSize(),
                dataset->GetRasterCount() );
        if( dataset->GetProjectionRef()  != NULL )
            printf( "Projection is `%s'\n", dataset->GetProjectionRef() );
        if( dataset->GetGeoTransform( adfGeoTransform ) == CE_None )
        {
            printf( "Origin = (%.6f,%.6f)\n",
                    adfGeoTransform[0], adfGeoTransform[3] );
            printf( "Pixel Size = (%.6f,%.6f)\n",
                    adfGeoTransform[1], adfGeoTransform[5] );
        }

        GDALRasterBand  *poBand;
        int             nBlockXSize, nBlockYSize;
        int             bGotMin, bGotMax;
        double          adfMinMax[2];
        poBand = dataset->GetRasterBand( 1 );
        poBand->GetBlockSize( &nBlockXSize, &nBlockYSize );
        printf( "Block=%dx%d Type=%s, ColorInterp=%s\n",
                nBlockXSize, nBlockYSize,
                GDALGetDataTypeName(poBand->GetRasterDataType()),
                GDALGetColorInterpretationName(
                    poBand->GetColorInterpretation()) );
        adfMinMax[0] = poBand->GetMinimum( &bGotMin );
        adfMinMax[1] = poBand->GetMaximum( &bGotMax );
        if( ! (bGotMin && bGotMax) )
            GDALComputeRasterMinMax((GDALRasterBandH)poBand, TRUE, adfMinMax);
        printf( "Min=%.3fd, Max=%.3f\n", adfMinMax[0], adfMinMax[1] );
        if( poBand->GetOverviewCount() > 0 )
            printf( "Band has %d overviews.\n", poBand->GetOverviewCount() );
        if( poBand->GetColorTable() != NULL )
            printf( "Band has a color table with %d entries.\n",
                    poBand->GetColorTable()->GetColorEntryCount() );

        // read data
        float *pafScanline;
        float *pafScanline2;
        int   nXSize = poBand->GetXSize();
        int   nYSize = poBand->GetYSize();
        float noData = poBand->GetNoDataValue();

        // projection
        GeographicLib::LocalCartesian proj(adfGeoTransform[3],  adfGeoTransform[0]);
        local_lat_lon.setX(adfGeoTransform[3]);
        local_lat_lon.setY(adfGeoTransform[0]);
        local_alt = 0;

        double deltaz = adfMinMax[1] - adfMinMax[0];

        osg::ref_ptr<osg::Group> group = new osg::Group;

        if(_mode == LoadingModePoint)
        {

            pafScanline = (float *) CPLMalloc(sizeof(float)*nXSize);

            osg::ref_ptr<osg::Geode> geode = new osg::Geode;
            osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
            osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;

            for(int y = 0; y < nYSize; y++)
            {

                // read line
                poBand->RasterIO( GF_Read, 0, y, nXSize, 1,
                                  pafScanline, nXSize, 1, GDT_Float32,
                                  0, 0 );

                // build points

                // create point in geode
                // point
                for(int x=0; x<nXSize; x++)
                {
                    if( pafScanline[x] == noData) // check NAN
                        continue;

                    osg::Vec3f point;
                    double lon = adfGeoTransform[0] + adfGeoTransform[1]*x;
                    double lat = adfGeoTransform[3] + adfGeoTransform[5]*y;
                    double h = pafScanline[x];
                    double px, py, pz;
                    proj.Forward(lat, lon, h, px, py, pz);

                    point[0] = px;
                    point[1] = py;
                    point[2] = pz;

                    vertices->push_back(point);

                    // z color
                    double dh = (h - adfMinMax[0]) / deltaz;
                    float r = dh > 0.5 ? (dh - 0.5)*2: 0;
                    float g = dh > 0.5 ? (1.0 - dh) + 0.5 : (dh*2);
                    float b = 1.0 -dh;

                    // add a white color, colors take the form r,g,b,a with 0.0 off, 1.0 full on.
                    osg::Vec4 color(r, g, b,1.0f);
                    colors->push_back(color);
                }
            }

            // points
            osg::ref_ptr<osg::Geometry> shape_point_drawable = new osg::Geometry();

            // pass the created vertex array to the points geometry object.
            shape_point_drawable->setVertexArray(vertices);

            shape_point_drawable->setColorArray(colors, osg::Array::BIND_PER_VERTEX);

            // create and add a DrawArray Primitive (see include/osg/Primitive).  The first
            // parameter passed to the DrawArrays constructor is the Primitive::Mode which
            // in this case is POINTS (which has the same value GL_POINTS), the second
            // parameter is the index position into the vertex array of the first point
            // to draw, and the third parameter is the number of points to draw.
            shape_point_drawable->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS,0,vertices->size()));

            // fixed size points
            shape_point_drawable->getOrCreateStateSet()->setAttribute(new osg::Point(1.f), osg::StateAttribute::ON);

            geode->addDrawable(shape_point_drawable);
            group->addChild(geode);

            // TOO SLOW!!!!!!!!
            //                osg::Geometry* geometry = new osg::Geometry();

            //                osg::ref_ptr<osgUtil::DelaunayTriangulator> dt = new
            //                osgUtil::DelaunayTriangulator(vertices);
            //                dt->triangulate(); // Generate the triangles
            //                geometry->setVertexArray(vertices);
            //                geometry->addPrimitiveSet(dt->getTriangles());
            //                geode->addDrawable(geometry);
            //                group->addChild(geode);

            CPLFree(pafScanline);
        }
        else if(_mode == LoadingModeTriangle || _mode == LoadingModeTriangleNormals)
        {
            // triangles

            pafScanline = (float *) CPLMalloc(sizeof(float)*nXSize);
            pafScanline2 = (float *) CPLMalloc(sizeof(float)*nXSize);

            osg::ref_ptr<osg::Geode> geode = new osg::Geode;

            // read first line
            poBand->RasterIO( GF_Read, 0, 0, nXSize, 1,
                              pafScanline, nXSize, 1, GDT_Float32,
                              0, 0 );

            for(int y = 1; y < nYSize; y++)
            {

                // read second line
                poBand->RasterIO( GF_Read, 0, y, nXSize, 1,
                                  pafScanline2, nXSize, 1, GDT_Float32,
                                  0, 0 );

                // create triangles in geode
                // AD
                // BC
                //  triangle 1 = ABC
                //  triangle 2 = ACD


                osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;

                // for LoadingModeTriangleNormals
                osg::ref_ptr<osg::Vec3Array> normals;
                if(_mode == LoadingModeTriangleNormals)
                {
                    normals = new osg::Vec3Array;
                }

                osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;

                // point
                for(int x=0; x<nXSize-1; x++)
                {
                    // check if 1 triangle is incomplète
                    if( pafScanline2[x] == noData) // check NAN
                        continue;
                    if( pafScanline2[x+1] == noData) // check NAN
                        continue;
                    if( pafScanline[x] == noData) // check NAN
                        continue;
                    if( pafScanline[x+1] == noData) // check NAN
                        continue;

                    // build triangle
                    osg::Vec3f pointA;
                    osg::Vec3f pointB;
                    osg::Vec3f pointC;
                    osg::Vec3f pointD;

                    // B
                    double lon = adfGeoTransform[0] + adfGeoTransform[1]*x;
                    double lat = adfGeoTransform[3] + adfGeoTransform[5]*(y+1);
                    double h = pafScanline2[x];
                    double px, py, pz;
                    proj.Forward(lat, lon, h, px, py, pz);
                    pointB[0] = px;
                    pointB[1] = py;
                    pointB[2] = pz;

                    // C
                    lon = adfGeoTransform[0] + adfGeoTransform[1]*(x+1);
                    lat = adfGeoTransform[3] + adfGeoTransform[5]*(y+1);
                    h = pafScanline2[x+1];
                    proj.Forward(lat, lon, h, px, py, pz);
                    pointC[0] = px;
                    pointC[1] = py;
                    pointC[2] = pz;

                    // A
                    lon = adfGeoTransform[0] + adfGeoTransform[1]*x;
                    lat = adfGeoTransform[3] + adfGeoTransform[5]*y;
                    h = pafScanline[x];
                    proj.Forward(lat, lon, h, px, py, pz);

                    pointA[0] = px;
                    pointA[1] = py;
                    pointA[2] = pz;


                    // D
                    lon = adfGeoTransform[0] + adfGeoTransform[1]*(x+1);
                    lat = adfGeoTransform[3] + adfGeoTransform[5]*y;
                    h = pafScanline[x+1];
                    proj.Forward(lat, lon, h, px, py, pz);

                    pointD[0] = px;
                    pointD[1] = py;
                    pointD[2] = pz;


                    // triangles
                    vertices->push_back(pointA);
                    vertices->push_back(pointB);
                    vertices->push_back(pointC);

                    if(_mode == LoadingModeTriangleNormals)
                    {
                        osg::Vec3f N1 = (pointB - pointA) ^ (pointC - pointB);
                        normals->push_back(N1);
                        normals->push_back(N1);
                        normals->push_back(N1);
                    }

                    vertices->push_back(pointA);
                    vertices->push_back(pointC);
                    vertices->push_back(pointD);


                    if(_mode == LoadingModeTriangleNormals)
                    {
                        osg::Vec3f N2 = (pointC - pointA) ^ (pointD - pointC);
                        normals->push_back(N2);
                        normals->push_back(N2);
                        normals->push_back(N2);
                    }

                }

                // triangles
                osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();

                // pass the created vertex array to the points geometry object.
                geometry->setVertexArray(vertices);

                if(_mode == LoadingModeTriangleNormals)
                {
                    geometry->setNormalArray(normals, osg::Array::BIND_PER_VERTEX); //BIND_PER_PRIMITIVE_SET);
                }

                osg::Vec4 color(1.0,1.0,1.0,1.0);
                colors->push_back(color);
                geometry->setColorArray(colors, osg::Array::BIND_OVERALL); //BIND_PER_VERTEX);

                // create and add a DrawArray Primitive (see include/osg/Primitive).  The first
                // parameter passed to the DrawArrays constructor is the Primitive::Mode which
                // in this case is POINTS (which has the same value GL_POINTS), the second
                // parameter is the index position into the vertex array of the first point
                // to draw, and the third parameter is the number of points to draw.
                geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES,0,vertices->size()));

                // fixed size points
                //shape_point_drawable->getOrCreateStateSet()->setAttribute(new osg::Point(1.f), osg::StateAttribute::ON);

                geode->addDrawable(geometry);

                // swap line ponters
                float * tmp = pafScanline;
                pafScanline = pafScanline2;
                pafScanline2 = tmp;
            }

            group->addChild(geode);

            CPLFree(pafScanline);
            CPLFree(pafScanline2);

        }
        else if(_mode == LoadingModeTrianglePoint)
        {
            // triangles + points

            pafScanline = (float *) CPLMalloc(sizeof(float)*nXSize);
            pafScanline2 = (float *) CPLMalloc(sizeof(float)*nXSize);


            osg::ref_ptr<osg::Geode> geode = new osg::Geode;

            // read first line
            poBand->RasterIO( GF_Read, 0, 0, nXSize, 1,
                              pafScanline, nXSize, 1, GDT_Float32,
                              0, 0 );

            for(int y = 1; y < nYSize; y++)
            {

                // read second line
                poBand->RasterIO( GF_Read, 0, y, nXSize, 1,
                                  pafScanline2, nXSize, 1, GDT_Float32,
                                  0, 0 );

                // create triangles in geode
                // AD
                // BC
                //  triangle 1 = ABC
                //  triangle 2 = ACD


                osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
                osg::ref_ptr<osg::Vec3Array> verticesPoint = new osg::Vec3Array;
                osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
                osg::ref_ptr<osg::Vec4Array> colorst = new osg::Vec4Array;

                // point
                for(int x=0; x<nXSize-1; x++)
                {
                    // check if 1 triangle is incomplète
                    if( pafScanline2[x] == noData) // check NAN
                        continue;
                    if( pafScanline2[x+1] == noData) // check NAN
                        continue;
                    if( pafScanline[x] == noData) // check NAN
                        continue;
                    if( pafScanline[x+1] == noData) // check NAN
                        continue;

                    // build triangle
                    osg::Vec3f pointA;
                    osg::Vec3f pointB;
                    osg::Vec3f pointC;
                    osg::Vec3f pointD;
                    osg::Vec4 colorA;

                    // B
                    double lon = adfGeoTransform[0] + adfGeoTransform[1]*x;
                    double lat = adfGeoTransform[3] + adfGeoTransform[5]*(y+1);
                    double h = pafScanline2[x];
                    double px, py, pz;
                    proj.Forward(lat, lon, h, px, py, pz);
                    pointB[0] = px;
                    pointB[1] = py;
                    pointB[2] = pz;

                    // C
                    lon = adfGeoTransform[0] + adfGeoTransform[1]*(x+1);
                    lat = adfGeoTransform[3] + adfGeoTransform[5]*(y+1);
                    h = pafScanline2[x+1];
                    proj.Forward(lat, lon, h, px, py, pz);
                    pointC[0] = px;
                    pointC[1] = py;
                    pointC[2] = pz;

                    // A
                    lon = adfGeoTransform[0] + adfGeoTransform[1]*x;
                    lat = adfGeoTransform[3] + adfGeoTransform[5]*y;
                    h = pafScanline[x];
                    proj.Forward(lat, lon, h, px, py, pz);

                    pointA[0] = px;
                    pointA[1] = py;
                    pointA[2] = pz;

                    // z color
                    float dh = (h - adfMinMax[0]) / deltaz;
                    float r = dh > 0.5 ? (dh - 0.5)*2: 0;
                    float g = dh > 0.5 ? (1.0 - dh) + 0.5 : (dh*2);
                    float b = 1.0 -dh;

                    colorA = {r, g, b, 1.0f};

                    // D
                    lon = adfGeoTransform[0] + adfGeoTransform[1]*(x+1);
                    lat = adfGeoTransform[3] + adfGeoTransform[5]*y;
                    h = pafScanline[x+1];
                    proj.Forward(lat, lon, h, px, py, pz);

                    pointD[0] = px;
                    pointD[1] = py;
                    pointD[2] = pz;

                    // triangles
                    vertices->push_back(pointA);
                    vertices->push_back(pointB);
                    vertices->push_back(pointC);

                    vertices->push_back(pointA);
                    vertices->push_back(pointC);
                    vertices->push_back(pointD);

                    // Warning : Last row & last column ommitted
                    // points
                    verticesPoint->push_back(pointA);
                    colors->push_back(colorA);
                }

                // triangles
                osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();

                // pass the created vertex array to the points geometry object.
                geometry->setVertexArray(vertices);

                //osg::Vec4 color(0.3,0.1,0.3,0.3);
                osg::Vec4 color(1.0,1.0,1.0,1.0);
                colorst->push_back(color);
                geometry->setColorArray(colorst, osg::Array::BIND_OVERALL); //BIND_PER_VERTEX);

                // create and add a DrawArray Primitive (see include/osg/Primitive).  The first
                // parameter passed to the DrawArrays constructor is the Primitive::Mode which
                // in this case is POINTS (which has the same value GL_POINTS), the second
                // parameter is the index position into the vertex array of the first point
                // to draw, and the third parameter is the number of points to draw.
                geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES,0,vertices->size()));

                // fixed size points
                //shape_point_drawable->getOrCreateStateSet()->setAttribute(new osg::Point(1.f), osg::StateAttribute::ON);

                geode->addDrawable(geometry);

                // points
                osg::ref_ptr<osg::Geometry> geometryP = new osg::Geometry();

                // pass the created vertex array to the points geometry object.
                geometryP->setVertexArray(verticesPoint);
                geometryP->setColorArray(colors, osg::Array::BIND_PER_VERTEX);
                geometryP->getOrCreateStateSet()->setAttribute(new osg::Point(1.f), osg::StateAttribute::ON);
                geometryP->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS,0,verticesPoint->size()));
                geode->addDrawable(geometryP);


                // swap line poniters
                float * tmp = pafScanline;
                pafScanline = pafScanline2;
                pafScanline2 = tmp;
            }

            group->addChild(geode);

            CPLFree(pafScanline);
            CPLFree(pafScanline2);

        }

        GDALClose(dataset);

        GDALDestroyDriverManager();

        model_transform = new osg::MatrixTransform;
        if (m_ref_alt == INVALID_VALUE)
        {
            m_ref_lat_lon = local_lat_lon;
            m_ref_alt = local_alt;
            m_ltp_proj.Reset(m_ref_lat_lon.x(), m_ref_lat_lon.y(),m_ref_alt);

            model_transform->setMatrix(osg::Matrix::identity()); //translate(0,0,0));
        }else{
            double N,E,U;
            m_ltp_proj.Forward(local_lat_lon.x(), local_lat_lon.y(), local_alt, E, N, U);

            model_transform->setMatrix(osg::Matrix::translate(E,N,U));
        }

        model_transform->addChild(group);


        return  model_transform;

    }
    else
    {
        std::cout << "GDAL error ; No data loaded" << std::endl;
        return model_transform;
    }
}

///
/// \brief addNodeToScene add a node to the scene
/// \param _node node to be added
/// \return true if loading succeded
///
bool OSGWidget::addNodeToScene(osg::ref_ptr<osg::Node> _node, double _transparency)
{
    // Add model
    m_models.push_back(_node);
    osg::StateSet* state_set = _node->getOrCreateStateSet();
    state_set->setMode( GL_DEPTH_TEST, osg::StateAttribute::ON );
//    state_set->setMode( GL_BLEND, osg::StateAttribute::ON);

//    // Add the possibility of modifying the transparence
//    osg::ref_ptr<osg::Material> material = new osg::Material;
//    // Put the 3D model totally opaque
//    material->setAlpha( osg::Material::FRONT, 1.0 );
//    state_set->setAttributeAndModes ( material, osg::StateAttribute::ON );

//    osg::ref_ptr<osg::BlendFunc> bf = new osg::BlendFunc(osg::BlendFunc::ONE_MINUS_SRC_ALPHA,osg::BlendFunc::SRC_ALPHA );
//    state_set->setAttributeAndModes(bf);

    m_modelsGroup->insertChild(0, _node.get()); // put at the beginning to be drawn first

    // optimize the scene graph, remove redundant nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(_node.get(), osgUtil::Optimizer::ALL_OPTIMIZATIONS  | osgUtil::Optimizer::TESSELLATE_GEOMETRY);

    // compute z min/max of 3D model
    MinMaxComputationVisitor minmax;
    _node->accept(minmax);
    float zmin = minmax.getMin();
    float zmax = minmax.getMax();

    GeometryTypeCountVisitor geomcount;
    _node->accept(geomcount);

    // save original translation
    osg::ref_ptr<osg::MatrixTransform> model_transform =  dynamic_cast<osg::MatrixTransform*>(_node.get());

    osg::ref_ptr<NodeUserData> data = new NodeUserData();
    data->useShader = false;
    data->zmin = zmin;
    data->zmax = zmax;
    data->zoffset = 0; // will be changed on z offset changed
    data->originalZoffset = model_transform->getMatrix().getTrans().z();
    data->hasMesh = geomcount.getNbTriangles() > 0;
    _node->setUserData(data);

    //configureShaders( _node->getOrCreateStateSet() );
    _node->getOrCreateStateSet()->addUniform( new osg::Uniform( "zmin", zmin));
    _node->getOrCreateStateSet()->addUniform( new osg::Uniform( "deltaz", zmax - zmin));
    _node->getOrCreateStateSet()->addUniform( new osg::Uniform( "hasmesh", data->hasMesh));

    setCameraOnNode(_node);

    home();

    // set transparency
    setNodeTransparency(_node, _transparency);

    return true;
}

void OSGWidget::setCameraOnNode(osg::ref_ptr<osg::Node> _node)
{
    osgViewer::View *view = m_viewer->getView(0);

    view->setSceneData( m_matrixTransform); //m_group );
    // get the translation in the  node
    osg::MatrixTransform *matrix_transform = dynamic_cast <osg::MatrixTransform*> (_node.get());
    osg::Vec3d translation = matrix_transform->getMatrix().getTrans();
    BoxVisitor boxVisitor;
    _node->accept(boxVisitor);

    osg::BoundingBox box = boxVisitor.getBoundingBox();
    double x_max = box.xMax();
    double x_min = box.xMin();
    double y_max = box.yMax();
    double y_min = box.yMin();
    double cam_center_x = (x_max+x_min)/2 +  translation.x();
    double cam_center_y = (y_max+y_min)/2 +  translation.y();
    double cam_center_z;
    if( (x_max-x_min)/(2*tan(((30/2)* M_PI )/ 180.0 )) > (y_max-y_min)/(2*tan(((30* M_PI )/ 180.0 )/2)) )
    {
        cam_center_z = (x_max-x_min)/(2*tan(((30/2)* M_PI )/ 180.0 ));
    }
    else
    {
        cam_center_z = (y_max-y_min)/(2*tan(((30/2)* M_PI )/ 180.0 ));
    }

    osg::Vec3d eye(cam_center_x,
                   cam_center_y,
                   (box.zMin() + cam_center_z)*m_zScale);
    osg::Vec3d target( cam_center_x,
                       cam_center_y,
                       box.zMin()*m_zScale);
    osg::Vec3d normal(0,0,-1);

    view->getCameraManipulator()->setHomePosition(eye,target,normal);
}


///
/// \brief removeNodeFromScene remove a node from the scene
/// \param _node node to be removed
/// \return true if remove succeded
///
bool OSGWidget::removeNodeFromScene(osg::ref_ptr<osg::Node> _node)
{
    // remove model
    std::vector<osg::ref_ptr<osg::Node>>::iterator position = std::find(m_models.begin(), m_models.end(), _node);
    if (position != m_models.end()) // == myVector.end() means the element was not found
        m_models.erase(position);

    m_modelsGroup->removeChild(_node.get());

    // optimize the scene graph, remove redundant nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(this->m_globalGroup.get());

    osgViewer::View *view = m_viewer->getView(0);

    view->setSceneData( m_matrixTransform);

    return true;
}

//bool OSGWidget::setSceneData(osg::ref_ptr<osg::Node> _sceneData)
//{
//    if (!_sceneData)
//    {
//        std::cout << "No data loaded" << std::endl;
//        return false;
//    }

//    m_models.push_back(_sceneData);


//    osgViewer::View *view = m_viewer->getView(0);

//    view->setSceneData( m_models.back().get() );


//    return true;
//}

void OSGWidget::setClearColor(double _r, double _g, double _b, double _alpha)
{
    std::vector<osg::Camera*> cameras;
    m_viewer->getCameras( cameras );

    for (unsigned int i=0; i<cameras.size(); i++){
        cameras[i]->setClearColor( osg::Vec4( _r, _g, _b, _alpha ));
    }

}

// To rewrite /////////////////////////////////////////////////////////////////////////////////////
void OSGWidget::clearSceneData()
{

    osgViewer::View *view = m_viewer->getView(0);
    view->setSceneData( 0 );
    view->getDatabasePager()->cancel();
    view->getDatabasePager()->clear();

    // remove all nodes from group
    for (unsigned int i=0; i<m_models.size(); i++){
        m_modelsGroup->removeChild(m_models[i]);
        //m_models[i] = NULL; useless
    }

    m_models.clear();

    // remove all drawables
    // m_measurement_geode->removeDrawables(0,m_measurement_geode->getNumDrawables());
    for (unsigned int i=0; i<m_geodes.size(); i++)
    {
        m_geodes[i]->removeDrawables(0,m_geodes[i]->getNumDrawables());
        m_geodesGroup->removeChild(m_geodes[i]);
        //m_models[i] = NULL; useless
    }
    m_geodes.clear();

    // reinit georef
    m_ref_lat_lon.setX(INVALID_VALUE);
    m_ref_lat_lon.setY(INVALID_VALUE);
    m_ref_alt = INVALID_VALUE;

    this->initializeGL();
}

void OSGWidget::initializeGL(){

    // Init properties
    osg::StateSet* state_set = m_modelsGroup->getOrCreateStateSet();
    osg::Material* material = new osg::Material;
    material->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE );
    state_set->setAttributeAndModes( material, osg::StateAttribute::ON );
    state_set->setMode(GL_BLEND, osg::StateAttribute::ON);
    state_set->setMode(GL_LINE_SMOOTH, osg::StateAttribute::OFF);
    state_set->setMode( GL_DEPTH_TEST, osg::StateAttribute::ON );

    // to show measures too
    state_set = m_geodesGroup->getOrCreateStateSet();
    // material needed to show colors in measure without light
    material = new osg::Material;
    material->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE );
    state_set->setAttributeAndModes( material, osg::StateAttribute::ON );
    state_set->setMode(GL_BLEND, osg::StateAttribute::ON);
    state_set->setMode(GL_LINE_SMOOTH, osg::StateAttribute::ON);
    // if selected : only parts on top of all madels are shown
    //state_set->setMode( GL_DEPTH_TEST, osg::StateAttribute::ON );
}

void OSGWidget::paintGL()
{
    m_viewer->frame();

    paintOverlayGL();
}

void OSGWidget::paintOverlayGL()
{
    if(!m_showZScale)
    {
        m_overlay->hide();
        return;
    }

    QPainter painter(this);
    painter.beginNativePainting();
    painter.setViewTransformEnabled(false);
    painter.setWorldMatrixEnabled(false);

    QPen pen(Qt::gray, 1, Qt::SolidLine);
    painter.setPen(pen);
    QFont font = painter.font();
    font.setPixelSize(12);
    painter.setFont(font);

    float z_offset = m_ref_alt;
    if(m_ref_alt == INVALID_VALUE)
        z_offset = 0;

    float minval = m_modelsZMin + z_offset;
    float maxval = m_modelsZMax + z_offset;

    if(m_useDisplayZMinMax)
    {
        minval = m_displayZMin + z_offset;
        maxval = m_displayZMax + z_offset;
    }

    m_overlay->setMinMax(minval, maxval);
    m_overlay->setColorPalette(m_colorPalette);
    m_overlay->show();
    m_overlay->update();
}


void OSGWidget::resizeGL( int _width, int _height )
{
    this->getEventQueue()->windowResize( this->x(), this->y(), _width, _height );
    m_graphicsWindow->resized( this->x(), this->y(), _width, _height );

    this->onResize( _width, _height );
}

void OSGWidget::keyPressEvent( QKeyEvent* _event )
{
    QString key_string   = _event->text();
    const char* key_data = key_string.toLocal8Bit().data();

    if( _event->key() == Qt::Key_Control )
    {
        m_ctrl_pressed = true;
    }

    this->getEventQueue()->keyPress( osgGA::GUIEventAdapter::KeySymbol( *key_data ) );
}

void OSGWidget::keyReleaseEvent( QKeyEvent* _event )
{
    QString key_string   = _event->text();
    const char* key_data = key_string.toLocal8Bit().data();

    if( _event->key() == Qt::Key_Control )
    {
        m_ctrl_pressed =  false;
    }

    this->getEventQueue()->keyRelease( osgGA::GUIEventAdapter::KeySymbol( *key_data ) );
}

void OSGWidget::mouseMoveEvent( QMouseEvent* _event )
{
    emit signal_onMouseMove(_event->x(), _event->y());

    this->getEventQueue()->mouseMotion( static_cast<float>( _event->x() ),
                                        static_cast<float>( _event->y() ) );
}

void OSGWidget::mousePressEvent( QMouseEvent* _event )
{

    // for tools
    if( m_ctrl_pressed == true && _event->button()==Qt::LeftButton)
    {
        emit signal_onMousePress(Qt::MiddleButton, _event->x(), _event->y());
    }
    else
        emit signal_onMousePress(_event->button(), _event->x(), _event->y());

    // 1 = left mouse button
    // 2 = middle mouse button
    // 3 = right mouse button

    unsigned int button = 0;

    switch( _event->button() )
    {
    case Qt::LeftButton:
    {

        if( m_ctrl_pressed == true)
        {
            button = 2;
            m_fake_middle_click_activated = true;
        }else{
            button = 1;
        }
    }
        break;

    case Qt::MiddleButton:
    {
        button = 2;
    }
        break;

    case Qt::RightButton:
    {
        button = 3;
    }
        break;

    default:
        break;
    }

    this->getEventQueue()->mouseButtonPress( static_cast<float>( _event->x() ),
                                             static_cast<float>( _event->y() ),
                                             button );
}

void OSGWidget::getIntersectionPoint(int _x, int _y, osg::Vec3d &_inter_point, bool &_inter_exists)
{

    osgUtil::LineSegmentIntersector::Intersections intersections;

    osgViewer::View *view = m_viewer->getView(0);

    // if we click on the object
    if (view->computeIntersections(_x, this->size().height()-_y,intersections))
    {
        _inter_exists = true;

        osgUtil::LineSegmentIntersector::Intersections::iterator hitr = intersections.begin();

        //        if (!hitr->nodePath.empty() && !(hitr->nodePath.back()->getName().empty()))
        //        {
        //            // the geodes are identified by name.
        //            std::cout<<"Object \""<<hitr->nodePath.back()->getName()<<"\""<<std::endl;
        //        }
        //        else if (hitr->drawable.valid())
        //        {
        //            std::cout<<"Object \""<<hitr->drawable->className()<<"\""<<std::endl;
        //        }


        // we get the intersections in a osg::Vec3d
        _inter_point = hitr->getWorldIntersectPoint();

        _inter_point[2] /= m_zScale;

    }else{
        _inter_exists = false;
    }

    if(!_inter_exists)
    {
        osgUtil::PolytopeIntersector::Intersections intersections;
        osgUtil::PolytopeIntersector *polyintersector =
                new osgUtil::PolytopeIntersector(osgUtil::Intersector::CoordinateFrame::WINDOW,_x-3,this->size().height() -_y-3,_x+3,this->size().height() - _y+3);
        polyintersector->setPrimitiveMask(osgUtil::PolytopeIntersector::POINT_PRIMITIVES);
        osgUtil::IntersectionVisitor iv(polyintersector);

        polyintersector->setIntersectionLimit(osgUtil::PolytopeIntersector::LIMIT_NEAREST);
        osg::Camera *cam = view->getCamera();
        cam->accept(iv);
        intersections = polyintersector->getIntersections();

        if(!intersections.empty())
        {
            _inter_exists = true;

            osgUtil::PolytopeIntersector::Intersections::iterator hitr = intersections.begin();

            // we get the intersections in a osg::Vec3d
            _inter_point = hitr->localIntersectionPoint;

            _inter_point[2] /= m_zScale;
        }
        else
        {
            _inter_exists = false;
        }
    }
}

void OSGWidget::getIntersectionPoint(osg::Vec3d _world_point, osg::Vec3d &_inter_point, bool &_inter_exists)
{
    // project point
    osgViewer::View *view = m_viewer->getView(0);
    osg::Camera *cam = view->getCamera();

    const osg::Matrixd transmat
            = cam->getViewMatrix()
            * cam->getProjectionMatrix()
            * cam->getViewport()->computeWindowMatrix();

    osg::Vec4d vec(_world_point[0], _world_point[1], _world_point[2], 1.0);

    vec = vec * transmat;
    vec = vec / vec.w();

    float x = vec.x();
    float y = vec.y();

    osgUtil::LineSegmentIntersector::Intersections intersections;


    if (view->computeIntersections(x, y, intersections))
    {
        _inter_exists = true;

        osgUtil::LineSegmentIntersector::Intersections::iterator hitr = intersections.begin();

        // we get the intersections in a osg::Vec3d
        _inter_point = hitr->getWorldIntersectPoint();

        _inter_point[2] /= m_zScale;

    }else{
        _inter_exists = false;

        osgUtil::PolytopeIntersector::Intersections intersections;
        osgUtil::PolytopeIntersector *polyintersector =
                new osgUtil::PolytopeIntersector(osgUtil::Intersector::CoordinateFrame::WINDOW,x-3,y-3,x+3,y+3);
        polyintersector->setPrimitiveMask(osgUtil::PolytopeIntersector::POINT_PRIMITIVES);
        osgUtil::IntersectionVisitor iv(polyintersector);
        polyintersector->setIntersectionLimit(osgUtil::PolytopeIntersector::LIMIT_NEAREST);
        cam->accept(iv);
        intersections = polyintersector->getIntersections();

        if(!intersections.empty())
        {
            _inter_exists = true;

            osgUtil::PolytopeIntersector::Intersections::iterator hitr = intersections.begin();

            // we get the intersections in a osg::Vec3d
            _inter_point = hitr->localIntersectionPoint;

            _inter_point[2] /= m_zScale;
        }
    }
}

void OSGWidget::getIntersectionPointNode(int _x, int _y, osg::ref_ptr<osg::Node> &_inter_node,  bool &_inter_exists)
{
    osgUtil::PolytopeIntersector::Intersections intersections;
    osgUtil::PolytopeIntersector *polyintersector =
            new osgUtil::PolytopeIntersector(osgUtil::Intersector::CoordinateFrame::WINDOW,_x-3,this->size().height() -_y-3,_x+3,this->size().height() - _y+3);
    polyintersector->setPrimitiveMask(
                osgUtil::PolytopeIntersector::POINT_PRIMITIVES
                | osgUtil::PolytopeIntersector::LINE_PRIMITIVES);
    osgUtil::IntersectionVisitor iv(polyintersector);

    // do not work to restrict seauch
    //iv.apply(*m_geodesGroup);
    osgViewer::View *view = m_viewer->getView(0);

    polyintersector->setIntersectionLimit(osgUtil::PolytopeIntersector::LIMIT_NEAREST);
    osg::Camera *cam = view->getCamera();
    cam->accept(iv);
    intersections = polyintersector->getIntersections();

    m_geodesGroup->accept(iv);

    _inter_exists = false;

    if(!intersections.empty())
    {
        osgUtil::PolytopeIntersector::Intersections::iterator hitr = intersections.begin();

        while(hitr != intersections.end())
        {
            osg::ref_ptr<osg::Node> newnode = hitr->drawable->getParent(0);

            if(newnode != nullptr)
            {
                osg::Group *parent = newnode->getParent(0);
                if(parent != nullptr)
                {
                    std::string name = parent->getName();
                    if(name == MEASUREMENT_NAME)
                    {
                        _inter_exists = true;
                        _inter_node = newnode;
                        break;
                    }
                }

            }
            ++hitr;
        }
    }

}

void OSGWidget::mouseReleaseEvent(QMouseEvent* _event)
{

    // 1 = left mouse button
    // 2 = middle mouse button
    // 3 = right mouse button

    unsigned int button = 0;

    switch( _event->button() )
    {
    case Qt::LeftButton:
        if( m_fake_middle_click_activated == true)
        {
            button = 2;
            m_fake_middle_click_activated = false;
        }
        else
        {
            button = 1;
        }
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

    this->getEventQueue()->mouseButtonRelease( static_cast<float>( _event->x() ),
                                               static_cast<float>( _event->y() ),
                                               button );

}

void OSGWidget::wheelEvent( QWheelEvent* _event )
{

    _event->accept();
    int delta = _event->delta();

    // Inversion of wheel action : to be like in Google Maps
    // (just change test)
    osgGA::GUIEventAdapter::ScrollingMotion motion = delta < 0 ?   osgGA::GUIEventAdapter::SCROLL_UP
                                                                 : osgGA::GUIEventAdapter::SCROLL_DOWN;

    this->getEventQueue()->mouseScroll( motion );
}

bool OSGWidget::event( QEvent* _event )
{
    bool handled = QOpenGLWidget::event( _event );

    // This ensures that the OSG widget is always going to be repainted after the
    // user performed some interaction. Doing this in the event handler ensures
    // that we don't forget about some event and prevents duplicate code.
    switch( _event->type() )
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

void OSGWidget::onResize( int _width, int _height )
{
    std::vector<osg::Camera*> cameras;
    m_viewer->getCameras( cameras );

    cameras[0]->setViewport( 0, 0, _width, _height );
    //cameras[1]->setViewport( this->width() / 2, 0, this->width() / 2, this->height() );

    m_overlay->move(_width - m_overlay->width() - 10, _height - m_overlay->height()- 10);
}

osgGA::EventQueue* OSGWidget::getEventQueue() const
{
    osgGA::EventQueue* event_queue = m_graphicsWindow->getEventQueue();

    if( event_queue )
        return event_queue;
    else
        throw std::runtime_error( "Unable to obtain valid event queue");
}

void OSGWidget::getGeoOrigin(QPointF &_ref_lat_lon, double &_ref_alt)
{
    _ref_lat_lon = m_ref_lat_lon;
    _ref_alt = m_ref_alt;
}

// set initial values
void OSGWidget::setGeoOrigin(QPointF _latlon, double _alt)
{
    // Transform model
    osg::ref_ptr<osg::MatrixTransform> model_transform = new osg::MatrixTransform;
    m_ref_lat_lon = _latlon;
    m_ref_alt = _alt;
    m_ltp_proj.Reset(m_ref_lat_lon.x(), m_ref_lat_lon.y(),m_ref_alt);

    model_transform->setMatrix(osg::Matrix::identity()); //translate(0,0,0));
    osg::ref_ptr<osg::Geode> node = new osg::Geode();

    // hack : create an invisible point

    osg::Vec3d point;
    point[0] = m_ref_lat_lon.x();
    point[1] =  m_ref_lat_lon.y();
    point[2] = m_ref_alt;

    // create invisible point in geode
    // point
    osg::Geometry* shape_point_drawable = new osg::Geometry();
    osg::Vec3Array* vertices = new osg::Vec3Array;
    vertices->push_back(point);

    // pass the created vertex array to the points geometry object.
    shape_point_drawable->setVertexArray(vertices);

    osg::Vec4Array* colors = new osg::Vec4Array;
    // add a white color, colors take the form r,g,b,a with 0.0 off, 1.0 full on.
    osg::Vec4 color(0.0f,0.0f,0.0f,0.0f);
    colors->push_back(color);

    // pass the color array to points geometry, note the binding to tell the geometry
    // that only use one color for the whole object.
    shape_point_drawable->setColorArray(colors, osg::Array::BIND_OVERALL);

    // create and add a DrawArray Primitive (see include/osg/Primitive).  The first
    // parameter passed to the DrawArrays constructor is the Primitive::Mode which
    // in this case is POINTS (which has the same value GL_POINTS), the second
    // parameter is the index position into the vertex array of the first point
    // to draw, and the third parameter is the number of points to draw.
    shape_point_drawable->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS,0,vertices->size()));

    // fixed size points
    shape_point_drawable->getOrCreateStateSet()->setAttribute(new osg::Point(4.f), osg::StateAttribute::ON);

    node->removeDrawables(0);
    node->addDrawable(shape_point_drawable);

    // end add invisible point

    model_transform->addChild(node);

    // Add model without userdata
    m_models.push_back(node);
    m_modelsGroup->insertChild(0, node.get()); // put at the beginning to be drawn first
}

void OSGWidget::addGeode(osg::ref_ptr<osg::Geode> _geode)
{
    m_geodesGroup->addChild(_geode.get());
    m_geodes.push_back(_geode);
}

void OSGWidget::removeGeode(osg::ref_ptr<osg::Geode> _geode)
{
    // remove geode
    std::vector<osg::ref_ptr<osg::Geode>>::iterator position = std::find(m_geodes.begin(), m_geodes.end(), _geode);
    if (position != m_geodes.end()) // == myVector.end() means the element was not found
        m_geodes.erase(position);

    m_geodesGroup->removeChild(_geode);
}

void OSGWidget::addGroup(osg::ref_ptr<osg::Group> _group)
{
    //m_groups.push_back(_group);
    m_geodesGroup->addChild(_group.get());
}

void OSGWidget::removeGroup(osg::ref_ptr<osg::Group> _group)
{
    // remove group
    //    std::vector<osg::ref_ptr<osg::Group>>::iterator position = std::find(m_groups.begin(), m_groups.end(), _group);
    //    if (position != m_groups.end()) // == myVector.end() means the element was not found
    //        m_groups.erase(position);

    m_geodesGroup->removeChild(_group);
}

// reset view to home
void OSGWidget::home()
{
    if(m_viewer == nullptr)
        return;

    osgViewer::View *view = m_viewer->getView(0);
    if(view)
        view->home();
}

// tools : emit correspondant signal
void OSGWidget::startTool(QString &_message)
{
    emit signal_startTool(_message);
}

void OSGWidget::endTool(QString &_message)
{
    emit signal_endTool(_message);
}

void OSGWidget::cancelTool(QString &_message)
{
    emit signal_cancelTool(_message);
}

// convert x, y, z => lat, lon & alt
// if(m_ref_alt == INVALID_VALUE) do nothing
void OSGWidget::xyzToLatLonAlt(double _x, double _y, double _z, double &_lat, double &_lon, double &_alt)
{
    if(m_ref_alt == INVALID_VALUE)
        return;

    m_ltp_proj.Reverse(_x, _y, _z, _lat, _lon, _alt);
}


bool OSGWidget::generateGeoTiff(osg::ref_ptr<osg::Node> _node, QString _filename, double _pixel_size, OSGWidget::map_type _map_type)
{

    // get the translation in the  node
    osg::MatrixTransform *matrix_transform = dynamic_cast <osg::MatrixTransform*> (_node.get());
    osg::Vec3d translation = matrix_transform->getMatrix().getTrans();

    BoxVisitor boxVisitor;
    _node->accept(boxVisitor);

    osg::BoundingBox box = boxVisitor.getBoundingBox();

    // Create the edge of our picture
    // Set graphics contexts
    double x_max = box.xMax();
    double x_min = box.xMin();
    double y_max = box.yMax();
    double y_min = box.yMin();
    int width_pixel = ceil((x_max-x_min)/_pixel_size);
    int height_pixel = ceil((y_max-y_min)/_pixel_size);
    double width_meter = _pixel_size*width_pixel;
    double height_meter = _pixel_size*height_pixel;
    double cam_center_x = (x_max+x_min)/2 +  translation.x();
    double cam_center_y = (y_max+y_min)/2 +  translation.y();


    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->x = 0;
    traits->y = 0;
    traits->width = width_pixel;
    traits->height = height_pixel;
    traits->pbuffer = true;
    traits->alpha =  1;
    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());

    osg::ref_ptr< osg::Group > root( new osg::Group );
    root->addChild( _node );


    // setup MRT camera
    std::vector<osg::Texture2D*> attached_textures;
    osg::ref_ptr<osg::Camera> mrt_camera = new osg::Camera;
    mrt_camera->setGraphicsContext(gc);
    mrt_camera->setClearMask( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    mrt_camera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
    mrt_camera->setRenderOrder( osg::Camera::PRE_RENDER );
    mrt_camera->setViewport( 0, 0, width_pixel, height_pixel );
    mrt_camera->setClearColor(osg::Vec4(0., 0., 0., 0.));

    // Create our Texture
    osg::Texture2D* tex = new osg::Texture2D;
    tex->setTextureSize( width_pixel, height_pixel );
    tex->setSourceType( GL_UNSIGNED_BYTE );
    tex->setSourceFormat( GL_RGBA );
    tex->setInternalFormat( GL_RGBA32F_ARB );
    tex->setResizeNonPowerOfTwoHint( false );
    tex->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
    tex->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
    attached_textures.push_back( tex );
    mrt_camera->attach( osg::Camera::COLOR_BUFFER, tex );

    // set RTT textures to quad
    osg::Geode* geode( new osg::Geode );
    geode->addDrawable( osg::createTexturedQuadGeometry(
                            osg::Vec3(-1,-1,0), osg::Vec3(2.0,0.0,0.0), osg::Vec3(0.0,2.0,0.0)) );
    geode->getOrCreateStateSet()->setTextureAttributeAndModes( 0, attached_textures[0] );
    geode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    //geode->getOrCreateStateSet()->setMode( GL_DEPTH_TEST, osg::StateAttribute::ON );

    // configure postRenderCamera to draw fullscreen textured quad
    osg::Camera* post_render_camera( new osg::Camera );
    post_render_camera->setClearMask( 0 );
    post_render_camera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER, osg::Camera::FRAME_BUFFER );
    post_render_camera->setReferenceFrame( osg::Camera::ABSOLUTE_RF );
    post_render_camera->setRenderOrder( osg::Camera::POST_RENDER );
    post_render_camera->setViewMatrix( osg::Matrixd::identity() );
    post_render_camera->setProjectionMatrix( osg::Matrixd::identity() );

    if ( _map_type == map_type::OrthoMap ) post_render_camera->addChild( geode );

    root->addChild(post_render_camera);

    // Create the viewer
    osgViewer::Viewer viewer;
    viewer.setThreadingModel( osgViewer::Viewer::SingleThreaded );
    viewer.setCamera( mrt_camera.get() );
    viewer.getCamera()->setProjectionMatrixAsOrtho2D(-width_meter/2,width_meter/2,-height_meter/2,height_meter/2);

    // put our model in the center of our viewer
    viewer.setCameraManipulator(new osgGA::TrackballManipulator());
    double cam_center_z= (x_max-x_min)/2;

    osg::Vec3d eyes(cam_center_x,
                    cam_center_y,
                    box.zMin() + cam_center_z);
    osg::Vec3d center(cam_center_x,
                      cam_center_y,
                      box.zMin());
    osg::Vec3d normal(0,0,-1);
    viewer.getCameraManipulator()->setHomePosition(eyes,center,normal);

    viewer.setSceneData( root.get() );
    viewer.realize();

    // setup the callback
    osg::BoundingBox image_bounds;
    image_bounds.xMin() = cam_center_x-width_meter/2;
    image_bounds.xMax() = cam_center_x+width_meter/2;
    image_bounds.yMin() = cam_center_y-height_meter/2;
    image_bounds.yMax() = cam_center_y+height_meter/2;

    std::string screen_capture_filename = _filename.toStdString();

    if ( _map_type == map_type::OrthoMap )
    {
        SnapImage* final_draw_callback = new SnapImage(viewer.getCamera()->getGraphicsContext(),screen_capture_filename,m_ref_lat_lon, image_bounds,_pixel_size);
        mrt_camera->setFinalDrawCallback(final_draw_callback);
    }


    viewer.home();
    viewer.frame();
    if ( _map_type == map_type::AltMap )
    {
        GDALAllRegister();
        CPLPushErrorHandler(CPLQuietErrorHandler);

        GDALDataset *geotiff_dataset_alt;
        GDALDriver *driver_geotiff_alt;

        int no_data =  -9999;
        std::string file_prof = screen_capture_filename+".tif";

        driver_geotiff_alt = GetGDALDriverManager()->GetDriverByName("GTiff");
        geotiff_dataset_alt = driver_geotiff_alt->Create(file_prof.c_str(),width_pixel,height_pixel,1,GDT_Float32,NULL);

        float *buffer= new float[width_pixel];

        QProgressDialog progress_dialog("Write altitude map file...", "Abort altitude map", 0, height_pixel, this);
        progress_dialog.setWindowModality(Qt::WindowModal);

        for(int i=0; i<height_pixel; i++) {
            for(int j=0; j<width_pixel; j++) {
                QApplication::processEvents();
                osg::Vec3d _inter_point;
                osgUtil::LineSegmentIntersector::Intersections intersections;
                progress_dialog.setValue(i);
                if (progress_dialog.wasCanceled())
                    return false;
                if (viewer.computeIntersections(viewer.getCamera(),osgUtil::Intersector::WINDOW,j,height_pixel-i,intersections))
                {

                    osgUtil::LineSegmentIntersector::Intersections::iterator hitr = intersections.begin();

                    // we get the intersections in a osg::Vec3d
                    _inter_point = hitr->getWorldIntersectPoint();
                    float alt_point = _inter_point.z();
                    buffer[j] = alt_point;

                }else{
                    float alt_point = no_data;
                    buffer[j] = alt_point;
                }
            }
            // CPLErr GDALRasterBand::RasterIO( GDALRWFlag eRWFlag, int nXOff, int nYOff, int nXSize, int nYSize, void * pData, int nBufXSize, int nBufYSize, GDALDataType eBufType, int nPixelSpace, int nLineSpace )
            geotiff_dataset_alt->GetRasterBand(1)->RasterIO(GF_Write,0,i,width_pixel,1,buffer,width_pixel,1,GDT_Float32,0,0);
        }
		
		delete buffer;
		
        progress_dialog.setValue(height_pixel);
        geotiff_dataset_alt->GetRasterBand(1)->SetNoDataValue(no_data);

        // Setup output coordinate system
        double geo_transform[6] = { image_bounds.xMin(), _pixel_size, 0, image_bounds.yMax(), 0, -_pixel_size };
        geotiff_dataset_alt->SetGeoTransform(geo_transform);
        char *geo_reference_alt = NULL;
        OGRSpatialReference o_SRS_alt;
        o_SRS_alt.SetTM(m_ref_lat_lon.x(),m_ref_lat_lon.y(),0.9996,0,0);
        o_SRS_alt.SetWellKnownGeogCS( "WGS84" );
        o_SRS_alt.exportToWkt( &geo_reference_alt );

        geotiff_dataset_alt->SetProjection(geo_reference_alt);
        CPLFree( geo_reference_alt );
        GDALClose(geotiff_dataset_alt) ;

        GDALDestroyDriverManager();
    }

    return true;


}

void OSGWidget::enableLight(bool _state)
{
    bool lighton = true;
    if ( _state )
    {
        m_viewer->getView(0)->getCamera()->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);
        // disable shades on shader
        lighton = false;
    }
    else
    {
        m_viewer->getView(0)->getCamera()->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        // enable shades on shader
        lighton = true;
    }

    for(unsigned int i=0; i<m_models.size(); i++)
    {
        osg::StateSet* state_set = m_models[i]->getOrCreateStateSet();
        state_set->addUniform( new osg::Uniform( "lighton", lighton));
    }
}

void OSGWidget::enableStereo(bool _state)
{
    //osg::DisplaySettings::instance()->setStereoMode(osg::DisplaySettings::VERTICAL_INTERLACE);
    osg::DisplaySettings::instance()->setStereo(_state);
}

void OSGWidget::setNodeTransparency(osg::ref_ptr<osg::Node> _node, double _transparency_value)
{
    osg::StateSet* state_set = _node->getOrCreateStateSet();
    osg::StateAttribute* attr = state_set->getAttribute(osg::StateAttribute::MATERIAL);
    osg::Material* material = dynamic_cast<osg::Material*>(attr);

    state_set->setMode( GL_DEPTH_TEST, osg::StateAttribute::ON );

    if(_transparency_value == 0.0)
    {
        state_set->removeAttribute(osg::StateAttribute::MATERIAL);
        state_set->setMode( GL_BLEND, osg::StateAttribute::OFF);
    }
    else
    {
        state_set->setMode( GL_BLEND, osg::StateAttribute::ON);

        if(material == nullptr)
        {
            // Add the possibility of modifying the transparence
            material = new osg::Material;
            // Put the 3D model totally opaque
            material->setAlpha( osg::Material::FRONT, _transparency_value);
            state_set->setAttributeAndModes ( material, osg::StateAttribute::ON );

        }

        // Changes the transparency of the node
        material->setAlpha(osg::Material::FRONT, _transparency_value );

        // Turn on blending
        osg::ref_ptr<osg::BlendFunc> bf = new osg::BlendFunc(osg::BlendFunc::ONE_MINUS_SRC_ALPHA,osg::BlendFunc::SRC_ALPHA );
        state_set->setAttributeAndModes(bf);

        state_set->setAttributeAndModes( material, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    }

    // alpha on shader
    state_set->addUniform( new osg::Uniform( "alpha", float(_transparency_value) ));


    //    // test
    //    osg::StateSet* state_set = _node->getOrCreateStateSet();
    //    osg::StateAttribute* attr = state_set->getAttribute(osg::StateAttribute::MATERIAL);
    //    osg::Material* material = dynamic_cast<osg::Material*>(attr);
    //    material->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, _transparency_value) );
    //    state_set->setAttributeAndModes( material, osg::StateAttribute::OVERRIDE);
}

void OSGWidget::setNodeTranslationOffset(double _offset_x, double _offset_y, double _offset_z, osg::ref_ptr<osg::Node> _node, osg::Vec3d _trans)
{
    osg::ref_ptr<osg::MatrixTransform> model_transform =  dynamic_cast<osg::MatrixTransform*>(_node.get());

    osg::Matrix matrix = osg::Matrix::translate(_trans.x() + _offset_x, _trans.y() + _offset_y, _trans.z() + _offset_z);

    model_transform->setMatrix(matrix);

    // for shaders
    osg::ref_ptr<NodeUserData> data = (NodeUserData*)(_node->getUserData());
    if(data != nullptr)
    {
        data->zoffset = (float)_offset_z;
    }

    recomputeGlobalZMinMax();
}

void OSGWidget::setZScale(double _newValue)
{

    //    osgViewer::View *view = m_viewer->getView(0);
    //    osg::Vec3d eye1, center1, up1;
    //    osgGA::CameraManipulator *man = view->getCameraManipulator();
    //    man->getHomePosition(eye1,center1, up1);

    //    osg::Matrixd matrix = man->getMatrix();

    // change
    //double oldScale = m_zScale;
    m_zScale = _newValue;

    m_matrixTransform->setMatrix(osg::Matrix::scale(1.0, 1.0, m_zScale));
    if(m_models.size() > 0)
    {
        setCameraOnNode(m_models[0]);
    }

    //view->getCameraManipulator()->setHomePosition(eye,target,normal);
    home();

    //    matrix.ptr()[14] *= m_zScale / oldScale;
    //    view->getCameraManipulator()->setByMatrix(matrix);
}

void OSGWidget::configureShaders( osg::StateSet* stateSet )
{  
    const std::string vertexSourceBegin =
            "#version 130 \n"
            "uniform float zmin;"
            "uniform float deltaz;"
            "uniform float alpha;"
            "uniform float pointsize;"

            "out vec3 vertex_light_position;"
            "out vec3 vertex_light_half_vector;"
            "out vec3 vertex_normal;"
            "out vec4 fcolor;";



    const std::string vertexSourceEnd =
            "void main(void)"
            "{"
            // Calculate the normal value for this vertex, in world coordinates (multiply by gl_NormalMatrix)
            "    vertex_normal = normalize(gl_NormalMatrix * gl_Normal);"
            // Calculate the light position for this vertex
            "    vertex_light_position = normalize(gl_LightSource[0].position.xyz);"

            // Calculate the light's half vector
            "    vertex_light_half_vector = normalize(gl_LightSource[0].halfVector.xyz);"

            "    vec4 v = vec4(gl_Vertex);"
            "    float val = (v.z-zmin) / deltaz;"
            ""
            "    vec3 RGB = colorPalette(val);"
            "    fcolor = vec4( RGB.x, RGB.y, RGB.z, alpha);"
            "    gl_Position = gl_ModelViewProjectionMatrix*v;"
            "    gl_PointSize = 4.0 * pointsize / gl_Position.w;"
            "}";

    std::string vertexSource = vertexSourceBegin;
    vertexSource += ShaderColor::shaderSource(m_colorPalette);
    vertexSource += vertexSourceEnd;

    osg::Shader* vShader = new osg::Shader( osg::Shader::VERTEX, vertexSource );

    // without shading
    //    const std::string fragmentSourceOld =
    //            "#version 330 compatibility \n"
    //            "in vec4 fcolor;"
    //            "void main()"
    //            "{"
    //            "   gl_FragColor = fcolor;"
    //            "}";


    const std::string fragmentSource =
            "#version 130 \n"
            "uniform bool hasmesh;"
            "uniform bool lighton;"

            "in vec4 fcolor;"
            "in vec3 vertex_light_position;"
            "in vec3 vertex_light_half_vector;"
            "in vec3 vertex_normal;"

            "void main() {"
            "   vec4 color = fcolor;"
            "   if(!hasmesh || lighton)"
            "   {"
            "      color = fcolor;"
            "   }"
            "   else"
            "   {"
            // Calculate the ambient term
            "      vec4 ambient_color = gl_FrontMaterial.ambient * gl_LightSource[0].ambient + gl_LightModel.ambient * gl_FrontMaterial.ambient;"

            // Calculate the diffuse term
            "      vec4 diffuse_color = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;"

            // Calculate the specular value
            "      vec4 specular_color = gl_FrontMaterial.specular * gl_LightSource[0].specular * pow(max(dot(vertex_normal, vertex_light_half_vector), 0.0) , gl_FrontMaterial.shininess);"

            // Set the diffuse value (darkness). This is done with a dot product between the normal and the light
            // and the maths behind it is explained in the maths section of the site.
            "      float diffuse_value = max(dot(vertex_normal, vertex_light_position), 0.0);"

            // Set the output color of our current pixel
            "      vec4 material_color = ambient_color + diffuse_color * diffuse_value + specular_color;"

            "      color.r = material_color.r * fcolor.r;"
            "      color.g = material_color.g * fcolor.g;"
            "      color.b = material_color.b * fcolor.b;"
            "   }"
            "   gl_FragColor = color;"
            "}";

    osg::Shader* fShader = new osg::Shader( osg::Shader::FRAGMENT, fragmentSource );

    osg::Program* program = new osg::Program;
    program->setName("3dMetricsShader");
    program->addShader( fShader );
    program->addShader( vShader );
    stateSet->setAttribute( program, osg::StateAttribute::ON );

    stateSet->addUniform( new osg::Uniform( "alpha", 1.0f));
    stateSet->addUniform( new osg::Uniform( "pointsize", 32.0f));

    bool lighton = (m_viewer->getView(0)->getCamera()->getOrCreateStateSet()->getMode(GL_LIGHTING) == osg::StateAttribute::OFF);

    stateSet->addUniform( new osg::Uniform( "lighton", lighton));
    stateSet->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
}


// recompute global zmin and zmax for all models
void OSGWidget::recomputeGlobalZMinMax()
{
    m_modelsZMin = 0;
    m_modelsZMax = 0;

    if(m_models.size() == 0)
    {
        return;
    }

    bool first = true;

    for(unsigned int i=0; i<m_models.size(); i++)
    {
        osg::ref_ptr<NodeUserData> data = (NodeUserData*)m_models[i]->getUserData();
        if(data != nullptr)
        {
            if(first)
            {
                m_modelsZMin = data->zmin + data->zoffset + data->originalZoffset;
                m_modelsZMax = data->zmax + data->zoffset + data->originalZoffset;
                first = false;
                continue;
            }

            float zmin = data->zmin + data->zoffset + data->originalZoffset;
            float zmax = data->zmax + data->zoffset + data->originalZoffset;

            if(zmin < m_modelsZMin)
                m_modelsZMin = zmin;

            if(zmax > m_modelsZMax)
                m_modelsZMax = zmax;
        }
    }

    if(first)
    {
        // no 3D models loaded
        return;
    }
    float delta = m_modelsZMax - m_modelsZMin;
    float min = m_modelsZMin;
    if(m_useDisplayZMinMax)
    {
        delta = m_displayZMax - m_displayZMin;
        min = m_displayZMin;
    }

    for(unsigned int i=0; i<m_models.size(); i++)
    {
        osg::ref_ptr<NodeUserData> data = (NodeUserData*)m_models[i]->getUserData();

        if(data == nullptr)
            continue;

        osg::StateSet* state_set = m_models[i]->getOrCreateStateSet();
        state_set->addUniform( new osg::Uniform( "zmin", min - data->zoffset - data->originalZoffset));
        state_set->addUniform( new osg::Uniform( "deltaz", delta));
    }
}

bool OSGWidget::isEnabledShaderOnNode(osg::ref_ptr<osg::Node> _node)
{
    osg::ref_ptr<NodeUserData> data = (NodeUserData*)(_node->getUserData());
    if(data != nullptr)
    {
        return data->useShader;
    }
    return false;
}

void OSGWidget::enableShaderOnNode(osg::ref_ptr<osg::Node> _node, bool _enable)
{
    osg::ref_ptr<NodeUserData> data = (NodeUserData*)(_node->getUserData());
    if(data != nullptr)
    {
        osg::StateSet *stateSet= _node->getOrCreateStateSet();
        data->useShader = _enable;
        if(_enable)
        {
            configureShaders(stateSet);
        }
        else
        {
            stateSet->removeAttribute(osg::StateAttribute::PROGRAM);
        }
    }
}


void OSGWidget::setUseDisplayZMinMaxAndUpdate(bool _use)
{
    m_useDisplayZMinMax = _use;

    float delta = m_modelsZMax - m_modelsZMin;
    float min = m_modelsZMin;
    if(m_useDisplayZMinMax)
    {
        delta = m_displayZMax - m_displayZMin;
        min = m_displayZMin;
    }

    for(unsigned int i=0; i<m_models.size(); i++)
    {
        osg::ref_ptr<NodeUserData> data = (NodeUserData*)m_models[i]->getUserData();

        if(data == nullptr)
            continue;

        osg::StateSet* state_set = m_models[i]->getOrCreateStateSet();
        state_set->addUniform( new osg::Uniform( "zmin", min - data->zoffset - data->originalZoffset));
        state_set->addUniform( new osg::Uniform( "deltaz", delta));
    }
}

void OSGWidget::showZScale(bool _show)
{
    m_showZScale = _show;
    update();
}

void OSGWidget::setColorPalette(ShaderColor::Palette _palette)
{
    m_colorPalette = _palette;

    // process all models
    for(unsigned int i=0; i<m_models.size(); i++)
    {
        osg::ref_ptr<NodeUserData> data = (NodeUserData*)m_models[i]->getUserData();
        if(data != nullptr)
        {
            if(data->useShader)
            {
                osg::StateSet *stateSet= m_models[i]->getOrCreateStateSet();
                configureShaders(stateSet);
            }
        }
    }
}
