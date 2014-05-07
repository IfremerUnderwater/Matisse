from PyQt4.QtGui import QApplication
from MatisseDialog import MatisseDialog
from PyQt4.Qt import pyqtSlot

class MainApplication(QApplication) :
    def __init__(self, args):
        
        # IHM
        QApplication.__init__(self, args)
        self._helloPythonWidget = MatisseDialog()
        self._helloPythonWidget.signal_create_layer.connect(self.slot_create_layer)
        self._helloPythonWidget.show()

    @pyqtSlot(str, str)
    def slot_create_layer(self, job, fileName):
        print "MAIN: " + job + " Show image " + fileName
        
if __name__ == '__main__':
    import sys
 
    app = MainApplication(sys.argv)
 
 
    sys.exit(app.exec_())