/***************************************************************************
 *   Copyright (C) 2006 by Tim Sutton   *
 *   tim@linfiniti.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "mainwindow.h"
#include <QList>
#include <QFileDialog>
//
// QGIS Includes
//
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgssinglesymbolrendererv2.h>
#include <qgsmaplayerregistry.h>
#include <qgsvectorlayer.h>
#include <qgsrasterlayer.h>
#include <qgsmapcanvas.h>
//
// QGIS Map tools
//
#include "qgsmaptoolpan.h"
#include "qgsmaptoolzoom.h"

MainWindow::MainWindow(QWidget* parent, Qt::WFlags fl)
    : QMainWindow(parent,fl)
{
  //required by Qt4 to initialise the ui
  setupUi(this);

  // Create the Map Canvas
  mpMapCanvas= new QgsMapCanvas(0, 0);
  mpMapCanvas->enableAntiAliasing(true);
  mpMapCanvas->useImageToRender(false);
  mpMapCanvas->setCanvasColor(QColor(255, 255, 255));
  mpMapCanvas->freeze(false);
  mpMapCanvas->setVisible(true);
  mpMapCanvas->refresh();
  mpMapCanvas->show();

  // Lay our widgets out in the main window
  mpLayout = new QVBoxLayout(frameMap);
  mpLayout->addWidget(mpMapCanvas);

  //create the action behaviours
  connect(mpActionPan, SIGNAL(triggered()), this, SLOT(panMode()));
  connect(mpActionZoomIn, SIGNAL(triggered()), this, SLOT(zoomInMode()));
  connect(mpActionZoomOut, SIGNAL(triggered()), this, SLOT(zoomOutMode()));
  connect(mpActionAddLayer, SIGNAL(triggered()), this, SLOT(addVectorLayer()));
  connect(mpActionAddRasterLayer, SIGNAL(triggered()), this, SLOT(addRasterLayer()));

  //create a little toolbar
  mpMapToolBar = addToolBar(tr("File"));
  mpMapToolBar->addAction(mpActionAddLayer);
  mpMapToolBar->addAction(mpActionAddRasterLayer);
  mpMapToolBar->addAction(mpActionZoomIn);
  mpMapToolBar->addAction(mpActionZoomOut);
  mpMapToolBar->addAction(mpActionPan);

  //create the maptools
  mpPanTool = new QgsMapToolPan(mpMapCanvas);
  mpPanTool->setAction(mpActionPan);
  mpZoomInTool = new QgsMapToolZoom(mpMapCanvas, FALSE); // false = in
  mpZoomInTool->setAction(mpActionZoomIn);
  mpZoomOutTool = new QgsMapToolZoom(mpMapCanvas, TRUE ); //true = out
  mpZoomOutTool->setAction(mpActionZoomOut);
}

MainWindow::~MainWindow()
{
  delete mpZoomInTool;
  delete mpZoomOutTool;
  delete mpPanTool;
  delete mpMapToolBar;
  delete mpMapCanvas;
  delete mpLayout;
}

void MainWindow::panMode()
{
  mpMapCanvas->setMapTool(mpPanTool);

}
void MainWindow::zoomInMode()
{
  mpMapCanvas->setMapTool(mpZoomInTool);
}
void MainWindow::zoomOutMode()
{
  mpMapCanvas->setMapTool(mpZoomOutTool);
}
void MainWindow::addVectorLayer()
{
  QString myFileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                       QCoreApplication::applicationDirPath () + ".",
                       tr("Shape (*.shp)"));
  QFileInfo fileInfo(myFileName);
  QgsVectorLayer * mypLayer = new QgsVectorLayer(myFileName, fileInfo.fileName(), "ogr");
  if (mypLayer->isValid())
  {
    qDebug("Layer is valid");
  }
  else
  {
    qDebug("Layer is NOT valid");
    return;
  }

  // Add the Vector Layer to the Layer Registry
  QgsMapLayerRegistry::instance()->addMapLayer(mypLayer, TRUE, TRUE);


  // Add the layers to the Layer Set
  myList.append(QgsMapCanvasLayer(mypLayer, TRUE));//bool visibility
  // set the canvas to the extent of our layer
  mpMapCanvas->setExtent(mypLayer->extent());
  // Set the Map Canvas Layer Set
  mpMapCanvas->setLayerSet(myList);
   mpMapCanvas->refresh();
}

void MainWindow::addRasterLayer()
{
  QString myFileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                       QCoreApplication::applicationDirPath () + ".",
                       tr("GeoTIFF (*.tif)"));
  QFileInfo fileInfo(myFileName);
  //QString csrString = GEOGCS[\"WGS 84\",DATUM[\"unknown\",SPHEROID[\"WGS84\",6378137,298.257223563]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433]]
  QString csrString = "PROJCS[\"UTM Zone 31, Northern Hemisphere\", GEOGCS[\"WGS 84\",DATUM[\"unknown\",SPHEROID[\"WGS84\",6378137,298.257223563]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433]], PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0], PARAMETER[\"central_meridian\",3],  PARAMETER[\"scale_factor\",0.9996], PARAMETER[\"false_easting\",500000], PARAMETER[\"false_northing\",0],UNIT[\"metre\",1, AUTHORITY[\"EPSG\",\"9001\"]]]";




  QgsRasterLayer * mypLayer = new QgsRasterLayer(myFileName, fileInfo.fileName());

  QgsCoordinateReferenceSystem csr;
  csr.createFromWkt(csrString);

 // csr.createFromProj4("+proj=utm +zone=31 +units=m +datum=WGS84 +no_defs");
  if (csr.isValid())
  {
    qDebug("Proj is valid");
  }
  else
  {
    qDebug("Proj is NOT valid");
    return;
  }

  mypLayer->setCrs(csr);

  if (mypLayer->isValid())
  {
    qDebug("Layer is valid");
  }
  else
  {
    qDebug("Layer is NOT valid");
    return;
  }

  // Add the Vector Layer to the Layer Registry
  QgsMapLayerRegistry::instance()->addMapLayer(mypLayer, TRUE);

  // Add the layers to the Layer Set
  myList.append(QgsMapCanvasLayer(mypLayer, TRUE, TRUE));//bool visibility
  // set the canvas to the extent of our layer
  QgsRectangle rectangle = mypLayer->extent();
  qDebug() << "Xmin: " << rectangle.xMinimum();
  qDebug() << "Ymin: " << rectangle.yMinimum();
  qDebug() << "Xmax: " << rectangle.xMaximum();
  qDebug() << "Ymax: " << rectangle.yMaximum();

//  double pad=200;
//  rectangle.setXMinimum(rectangle.xMinimum()-pad);
//  rectangle.setYMinimum(rectangle.yMinimum()-pad);
//  rectangle.setXMaximum(rectangle.xMaximum()+pad);
//  rectangle.setYMaximum(rectangle.yMaximum()+pad);






  // Set the Map Canvas Layer Set
  mpMapCanvas->setLayerSet(myList);


      QListIterator<QgsMapCanvasLayer> i(myList);
         while (i.hasNext()) {
             QgsMapCanvasLayer cl = i.next();
             QgsMapLayer* l = cl.layer();
             qDebug() << "Layer name: " << l->name();
             qDebug() << "Layer visible: " << cl.isVisible();
         }


//         QgsMapRenderer* pRender = mpMapCanvas->mapRenderer();
//         QgsRectangle rect = pRender->fullExtent();
//           mpMapCanvas->setExtent(rect);
//         render.setExtent(rect)
         mpMapCanvas->setExtent(rectangle);

  mpMapCanvas->refresh();






}



