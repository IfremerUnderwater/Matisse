#ifndef MEASUREMENTPICKERTOOL_H
#define MEASUREMENTPICKERTOOL_H

#include <QObject>
#include <osg/Node>

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

#endif // MEASUREMENTPICKERTOOL_H


