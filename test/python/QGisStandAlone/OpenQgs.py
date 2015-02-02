#!/usr/bin/python
'''
Created on 11 juin 2014

Script de chargement  et modification d'un projet QGis.
Le script charge le projet ajoute un layer raster et sauve le projet

@author: d.amar
'''
from qgis.core import *
from PyQt4.QtGui import *
from PyQt4.QtCore import *
from pprint import pprint
import sys, os, platform


def addLayer(projectFile, layerName, rasterFile):
    
    layerType = { QgsMapLayer.VectorLayer: "Vector", QgsMapLayer.RasterLayer: "Raster"}
    
    if (platform.system() == 'Windows'):
        oswgeo4w = os.environ.get("OSGEO4W_ROOT");
        QgsApplication.setPrefixPath(oswgeo4w+"\\apps\\qgis", True);
    else:
        # Linux
        QgsApplication.setPrefixPath("/usr", True);
    
    QgsApplication.initQgis()
    print 'QGis initialized'
    a = QApplication( sys.argv )
    # load a project
    p = QgsProject.instance()
    qgsInfo = QFileInfo( projectFile )
    
    isLoaded = p.read( qgsInfo )
    print 'IsLoaded '+str(isLoaded)
    print 'Title: ' + p.title()
    
    # Open MapRegistry
    mlr = QgsMapLayerRegistry.instance()
    print
    print 'Layers count: ' + str(mlr.count())
    layers = mlr.mapLayers();
    
    # Display some layer information
    #pprint(layers)
    for layerId in layers:
        layer = layers[layerId]
        print 'Layer name: ' + layer.name()
        print 'Layer abstract: ' + layer.abstract()
        print 'Layer type: ' + layerType[layer.type()]
        
    # Add a layer
    print
    print "Add Layer"
    tifInfo = QFileInfo( rasterFile )
    rasterLayer =  QgsRasterLayer(tifInfo.absoluteFilePath(), layerName )
    
    crs = QgsCoordinateReferenceSystem();
    rasterLayer.setCrs(crs)
    
    mlr.addMapLayer(rasterLayer, True)
    
    print
    print 'Layers count: ' + str(mlr.count())
    layers = mlr.mapLayers();
    for layerId in layers:
        layer = layers[layerId]
        print 'Layer name: ' + layer.name()
        print 'Layer abstract: ' + layer.abstract()
        print 'Layer type: ' + layerType[layer.type()]     
    
    # Save project
    print
    print "Save project"
    p.write()
    
    QgsApplication.exit() 
    print "Add Layer Done."   


    