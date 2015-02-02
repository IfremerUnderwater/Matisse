from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *

# Simple application de test de Qt et qgis 
 
class HelloPython(QWidget):
    def __init__(self, parent=None):
        super(HelloPython, self).__init__(parent)
        helloLabel = QLabel("Say Hello To PyQT!")       
        helloLineEdit =QLineEdit()
 
        mainLayout = QGridLayout()
        mainLayout.addWidget(helloLabel, 0, 0)
        mainLayout.addWidget(helloLineEdit, 0, 1)
 
        self.setLayout(mainLayout)
        self.setWindowTitle("My Python App")
 
if __name__ == '__main__':
    import sys
    
    app = QApplication(sys.argv)
 
    helloPythonWidget = HelloPython()
    helloPythonWidget.show()
 
    sys.exit(app.exec_())
    