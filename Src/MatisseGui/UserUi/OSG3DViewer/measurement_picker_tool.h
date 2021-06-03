#ifndef MATISSE_MEASUREMENT_PICKER_TOOL_H_
#define MATISSE_MEASUREMENT_PICKER_TOOL_H_

#include <QObject>
#include <osg/Node>

namespace matisse {

class MeasurementPickerTool : public QObject
{
    Q_OBJECT
public:
    explicit MeasurementPickerTool(QObject *parent = 0);

public slots:
    void start();
    void slot_toolEnded();
    void slot_toolClicked(int, int);

signals:
    void signal_nodeClicked(osg::Node *_node);
    void signal_noNodeClicked();
};

} // namespace matisse

#endif // MATISSE_MEASUREMENT_PICKER_TOOL_H_


