# -*- coding: utf-8 -*-
"""
/***************************************************************************
 MatissePlugin
 
 ***************************************************************************/
"""
# Import the PyQt and QGIS libraries
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.QtNetwork import *
from qgis.core import *

# Generated by "pyrcc4 -o resources.py resources.qrc"
import resources
# Import the code for the dialog
from MatisseDialog import MatisseDialog
import os.path

class MatissePlugin:

    def __init__(self, iface):
        # Save reference to the QGIS interface
        self.iface = iface
        # initialize plugin directory
        self.plugin_dir = os.path.dirname(__file__)
        # initialize locale
        '''
        locale = QSettings().value("locale/userLocale")[0:2]
        localePath = os.path.join(self.plugin_dir, 'i18n', 'testtca_{}.qm'.format(locale))

        if os.path.exists(localePath):
            self.translator = QTranslator()
            self.translator.load(localePath)
            if qVersion() > '4.3.3':
                QCoreApplication.installTranslator(self.translator)
'''

        
        # Create the dialog (after translation) and keep reference
        self.dlg = MatisseDialog()

    @pyqtSlot(str, str)
    def slot_show_image(self, job, fileName):
        print "MAIN: Show image " + fileName
        self.iface.addRasterLayer(fileName, job)
        

    def initGui(self):
        # Create action that will start plugin configuration
        self.action = QAction(
            QIcon(":/images/images/ifremer-grand.jpg"),
            u"Matisse", self.iface.mainWindow())
        # connect the action to the run method
        self.action.triggered.connect(self.run)
        

        # Add toolbar button and menu item
        self.iface.addToolBarIcon(self.action)
        self.iface.addPluginToMenu(u"&Matisse", self.action)
        
        
    def unload(self):
        # Remove the plugin menu item and icon
        self.iface.removePluginMenu(u"&Matisse", self.action)
        self.iface.removeToolBarIcon(self.action)

    # run method that performs all the real work
    def run(self):
        # show the dialog
        self.dlg.show()
        self.dlg.signal_create_layer.connect(self.slot_show_image)
        # Run the dialog event loop
        result = self.dlg.exec_()
        # See if OK was pressed
        if result == 1:
            # do something useful (delete the line containing pass and
            # substitute with your code)
            pass
