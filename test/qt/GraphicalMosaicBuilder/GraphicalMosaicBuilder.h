#ifndef GRAPHICALMOSAICBUILDER_H
#define GRAPHICALMOSAICBUILDER_H

#include <QMainWindow>

namespace Ui {
class GraphicalMosaicBuilder;
}

class GraphicalMosaicBuilder : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit GraphicalMosaicBuilder(QWidget *parent = 0);
    ~GraphicalMosaicBuilder();
    
private:
    Ui::GraphicalMosaicBuilder *ui;
};

#endif // GRAPHICALMOSAICBUILDER_H
