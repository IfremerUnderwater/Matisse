#ifndef GRAPHICALCHARTER_H
#define GRAPHICALCHARTER_H



/* COLORS */
#define MATISSE_BLACK           "#252a31"

/* FONTS */
#define MATISSE_FONT_TYPE               "Montserrat"
#define MATISSE_FONT_DEFAULT_SIZE_PT    12

/* dpi value used as reference for scaling */
#define REF_DPI                         98

/* WIDGET SIZES AND MARGINS FOR A REFERENCE DPI (then adapted to other dpi) */

// Main window
#define MAIN_WINDOW_MIN_WIDTH          1280
#define MAIN_WINDOW_MIN_HEIGHT          800

// ControlBar
#define CONTROLLBAR_HEIGHT              60
#define CB_HOME_BUTTON_WIDTH            60
#define CB_HOME_BUTTON_ICON             36
#define CB_VISU_SWAP_ICON               36
#define CB_VERSION_WIDTH                240
#define CB_VISU_INFO_WIDTH              160
#define CB_ON_PROCESS_WIDTH             420
#define CB_STOP_BUTTON_ICON             36

// Parameters
#define PARAM_LABEL_WIDTH_NOWRAP        180
#define PARAM_LABEL_WIDTH_WRAP          360
#define PARAM_SPINBOX_WIDTH             90
#define PARAM_TABLE_COL_WIDTH_MAX       65
#define PARAM_TABLE_ROW_HEIGHT          20
#define PARAM_TABLE_WIDTH_NOWRAP_MAX    175
#define PARAM_TABLE_CELL_PADDING        5
#define PARAM_WIDGET_ALIGN_HEIGHT_THRE  40
#define PARAM_WIDGET_FIELD_HSPACING     3
#define PARAM_GROUP_MARGIN_TOP          10
#define PARAM_GROUP_MARGIN_BOTTOM       10

// Job dialog
#define JD_HEIGHT                       400
#define JD_WIDTH                        600

// Preferences dialog
#define PD_HEIGHT                       500
#define PD_WIDTH                        800

#define ASSEMBLY_PROPS_LABEL_WIDTH      140

namespace MatisseCommon {

class GraphicalCharter
{
public:
    static GraphicalCharter& instance();

    GraphicalCharter(GraphicalCharter const&) = delete;        // Don't forget to disable copy
    void operator=(GraphicalCharter const&) = delete;   // Don't forget to disable copy

    double ptToPx(double _pt) {
        return _pt/72*m_dpi;
    }

    double pxToPt(double _px)
    {
        return _px*72/m_dpi;
    }

    double dpi() const;

    int dpiScaled(int _ref_pixel_size);

private:
    GraphicalCharter();       // forbid create instance outside
    ~GraphicalCharter();      // forbid to delete instance outside
    double m_dpi;
};

}

#endif // GRAPHICALCHARTER_H
