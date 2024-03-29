#ifndef MATISSE_OSG_WIDGET_H_
#define MATISSE_OSG_WIDGET_H_

#include <QGLWidget>
#include "shader_color.h"
#include "overlay_widget.h"

#ifdef __APPLE__
/* FIX COMPILE BUG:
    platform: OSX v10.11.6 (15G1611) with Qt v5.9.1 from Homebrew
    this solves the following compilation error:
    /usr/local/Cellar/qt/5.9.1/lib/QtGui.framework/Headers/qopenglversionfunctions.h:1089:23: error: unknown type name 'GLDEBUGPROC'
    somehow the GL_KHR_debug macro is set to 1 in qopengl.h, so
    #ifndef GL_KHR_debug
    typedef void (APIENTRY *GLDEBUGPROC)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const GLvoid *userParam);
    #endif
    are removed, causing the error "unknown type name" GLDEBUGPROC in qopenglversionfunctions.h
    possible causes:
        - some change in Qt v5.9.1 (older versions have worked, at least ~5.8 worked)
*/
typedef void (APIENTRY *GLDEBUGPROC)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const GLvoid *userParam);
#endif

#include <QPoint>
#include <QTimer>
#include <QtOpenGL>
#include <QFileDialog>
#include <QPointF>

#include <osg/ref_ptr>
#include <osg/Referenced>
#include <osgViewer/GraphicsWindow>
#include <osgViewer/CompositeViewer>
#include <GeographicLib/LocalCartesian.hpp>
#include <osg/Material>

#include "kml_handler.h"
#include "loading_mode.h"

#ifdef _WIN32
#include "gdal_priv.h"
#include "cpl_conv.h"
#include "ogr_spatialref.h"
#else
#include "gdal/gdal_priv.h"
#include "gdal/cpl_conv.h"
#include "gdal/ogr_spatialref.h"
#endif

#define INVALID_VALUE 100000

namespace matisse {

class OSGWidget : public QOpenGLWidget
{
    Q_OBJECT

private:

    // models' user data
    class NodeUserData : public osg::Referenced
    {
    public:
        NodeUserData() : Referenced() {}
        virtual ~NodeUserData() {}

        // values zmin and zmax from model (without offset)
        float m_zmin;
        float m_zmax;

        float m_zoffset;
        float m_original_z_offset;

        // use or not shader
        bool m_use_shader;
        bool m_has_mesh;
    };

public:
    OSGWidget( QWidget* _parent = 0);

    virtual ~OSGWidget();

    ///
    /// \brief setSceneFromFile load a scene from a 3D file
    /// \param _scene_file path to any 3D file supported by osg
    /// \return true if loading succeded
    ///
    bool setSceneFromFile(std::string _scene_file);

    ///
    /// \brief createNodeFromFile load a scene from a 3D file
    /// \param _scene_file path to any 3D file supported by osg
    /// \return node if loading succeded
    ///
    osg::ref_ptr<osg::Node> createNodeFromFile(std::string _scene_file);

    ///
    /// \brief createNodeFromFile load a scene from a 3D file
    /// \param _scene_file path to any 3D file supported by osg
    /// \return node if loading succeded
    ///
    osg::ref_ptr<osg::Node> createNodeFromFileWithGDAL(std::string _scene_file, LoadingMode _mode);

    ///
    /// \brief addNodeToScene add a node to the scene
    /// \param _node node to be added
    /// \return true if loading succeded
    ///
    bool addNodeToScene(osg::ref_ptr<osg::Node> _node, double _transparency=0.0, bool _home_model=true);

    ///
    /// \brief removeNodeFromScene remove a node from the scene
    /// \param _node node to be removed
    /// \return true if remove succeded
    ///
    bool removeNodeFromScene(osg::ref_ptr<osg::Node> _node);

    ///
    /// \brief setSceneData load a scene from a osg::ref_ptr<osg::Node>
    /// \param _sceneData pointer to scene data
    /// \return true if loading succeded
    ///
    //bool setSceneData(osg::ref_ptr<osg::Node> _sceneData);

    ///
    /// \brief setClearColor set the clear color for all cameras
    /// \param _r red [0..1]
    /// \param _g green [0..1]
    /// \param _b blue [0..1]
    /// \param _alpha transparency [0..1]
    ///
    void setClearColor(double _r, double _g, double _b, double _alpha=1.0);

    ///
    /// \brief clearSceneData removes scene data
    ///
    void clearSceneData();

    ///
    /// \brief getIntersectionPoint ray trace (x,y) point on display to 3D point
    /// \param _x x coord on display
    /// \param _y y coord on display
    /// \param _inter_point 3D intersection point on scene
    /// \param _inter_exists true if intersection exists
    ///
    void getIntersectionPoint(int _x, int _y, osg::Vec3d &_inter_point, bool &_inter_exists);

    void getIntersectionPoint(osg::Vec3d _world_point, osg::Vec3d &_inter_point, bool &_inter_exists);

    void getIntersectionPointNode(int _x, int _y, osg::ref_ptr<osg::Node> &_inter_node, bool &_inter_exists);


    void addGeode(osg::ref_ptr<osg::Geode> _geode);
    void removeGeode(osg::ref_ptr<osg::Geode> _geode);

    // for measurements
    void addGroup(osg::ref_ptr<osg::Group> _group);
    void removeGroup(osg::ref_ptr<osg::Group> _group);

    ///
    /// \brief getGeoOrigin get geographic origin
    /// \param _ref_lat_lon lat lon origin
    /// \param _ref_alt alt origin
    ///
    void getGeoOrigin(QPointF &_ref_lat_lon, double &_ref_alt);

    // set initial values
    void setGeoOrigin(QPointF _latlon, double _alt);

    // reset to home position
    void home();

    // convert x, y, z => lat, lon & alt
    // if(m_ref_alt == INVALID_VALUE) do nothing
    void xyzToLatLonAlt(double _x, double _y, double _z, double &_lat, double &_lon, double &_alt);

    enum eMapType {
        ORTHO_MAP = 0,
        ALT_MAP = 1
    };

    void setNodeTransparency(osg::ref_ptr<osg::Node> _node, double _transparency_value=0.0);

    void setNodeTranslationOffset(double _x, double _y, double _z, osg::ref_ptr<osg::Node> _node, osg::Vec3d _trans);

signals:
    void si_showMeasurementSavingPopup(double _norm, QString _measurement_type, int _measurement_index);
    void si_onMousePress(Qt::MouseButton _button, int _x, int _y);
    void si_onMouseMove(int _x, int _y);

    // tools
    void si_startTool(QString &_message);
    void si_endTool(QString &_message);
    void si_cancelTool(QString &_message);

    void si_activedLight(bool &_activated);

public:
    // tools : emit correspondant signal
    void startTool(QString &_message);
    void endTool(QString &_message);
    void cancelTool(QString &_message);

    // screen2D
    bool generateGeoTiff(osg::ref_ptr<osg::Node> _node, QString _filename, double _pixel_size, OSGWidget::eMapType _map_type);

    //
    void enableLight(bool _state);
    void enableStereo(bool _state);

    osgViewer::View* getView() { return  m_viewer->getView(0); }
    void frame() { m_viewer->frame(); }

    osg::Camera* getCamera() { return  m_viewer->getView(0)->getCamera(); }

    double getZScale() const { return m_z_scale; }
    void setZScale(double _newValue);

    static const char *const MEASUREMENT_NAME;

    bool isEnabledShaderOnNode(osg::ref_ptr<osg::Node> _node);
    void enableShaderOnNode(osg::ref_ptr<osg::Node> _node, bool _enable);

    double getModelsZMin() const { return m_models_z_min; }
    double getModelsZMax() const { return m_models_z_max; }

    double getDisplayZMin() const { return m_display_z_min; }
    void setDisplayZMin(double _zmin) { m_display_z_min = _zmin; }

    double getDisplayZMax() const { return m_display_z_max; }
    void setDisplayZMax(double _zmax) { m_display_z_max = _zmax; }

    bool isUseDisplayZMinMax() const { return m_use_display_z_min_max; }
    void setUseDisplayZMinMaxAndUpdate(bool _use);

    bool isZScaleShowing() const { return m_show_z_scale; }
    void showZScale(bool _show);

    ShaderColor::ePalette getColorPalette() const { return m_color_palette; }
    void setColorPalette(ShaderColor::ePalette _palette);

    double getRefAlt() const { return m_ref_alt == INVALID_VALUE ? 0 : m_ref_alt; }

protected:
    virtual void paintGL();
    virtual void resizeGL( int _width, int _height );
    virtual void paintOverlayGL();
    //virtual void paintEvent(QPaintEvent*event);

    //void drawOverlay();

    virtual void keyPressEvent( QKeyEvent* _event );
    virtual void keyReleaseEvent( QKeyEvent* _event );

    virtual void mouseMoveEvent( QMouseEvent* _event );
    virtual void mousePressEvent( QMouseEvent* _event );
    virtual void mouseReleaseEvent( QMouseEvent* _event );
    virtual void wheelEvent( QWheelEvent* _event );

    virtual bool event( QEvent* _event );

    virtual void initializeGL();
    QTimer m_timer;

private:
    virtual void onResize( int _width, int _height );

    osgGA::EventQueue* getEventQueue() const;

    osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> m_graphics_window;
    osg::ref_ptr<osgViewer::CompositeViewer> m_viewer;

    osg::ref_ptr<osg::Group> m_global_group;
    osg::ref_ptr<osg::Group> m_models_group;
    osg::ref_ptr<osg::Group> m_geodes_group;

    std::vector<osg::ref_ptr<osg::Geode>> m_geodes;
    std::vector<osg::ref_ptr<osg::Node>> m_models;

    // Georef objects
    KMLHandler m_kml_handler;
    QPointF m_ref_lat_lon;
    double m_ref_alt;
    GeographicLib::LocalCartesian m_ltp_proj;
    bool m_ctrl_pressed;
    bool m_fake_middle_click_activated;

    // z scale
    double m_z_scale;

    // global matrix transform (Z scale only)
    osg::ref_ptr<osg::MatrixTransform> m_matrix_transform;

    void setCameraOnNode(osg::ref_ptr<osg::Node> _node);

    // for shaders
    void configureShaders( osg::StateSet* _state_set );

    // recompute global zmin and zmax for all models
    void recomputeGlobalZMinMax();
    float m_models_z_min;
    float m_models_z_max;

    // for using custom values
    bool m_use_display_z_min_max;

    float m_display_z_min;
    float m_display_z_max;

    bool m_show_z_scale;
    ShaderColor::ePalette m_color_palette;

    OverlayWidget *m_overlay;
};

} // namespace matisse

#endif // MATISSE_OSG_WIDGET_H_
