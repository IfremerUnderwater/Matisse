#ifndef MATISSE_MEASUREMENT_PICKER_TOOL_H_
#define MATISSE_MEASUREMENT_PICKER_TOOL_H_

#include <QObject>
#include <osg/Node>

namespace matisse {

class MeasurementPickerTool : public QObject
{
    Q_OBJECT
public:
    explicit MeasurementPickerTool(QObject *_parent = 0);

public slots:
    void sl_start();
    void sl_toolEnded();
    void sl_toolClicked(int, int);

signals:
    void si_nodeClicked(osg::Node *_node);
    void si_noNodeClicked();
};

} // namespace matisse

#endif // MATISSE_MEASUREMENT_PICKER_TOOL_H_


