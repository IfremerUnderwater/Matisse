# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui_MatisseDialog.ui'
#
# Created: Thu Mar 27 17:39:27 2014
#      by: PyQt4 UI code generator 4.10.2
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    def _fromUtf8(s):
        return s

try:
    _encoding = QtGui.QApplication.UnicodeUTF8
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig, _encoding)
except AttributeError:
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig)

class Ui_MatisseDialog(object):
    def setupUi(self, MatisseDialog):
        MatisseDialog.setObjectName(_fromUtf8("MatisseDialog"))
        MatisseDialog.resize(392, 333)
        self.txtMessage = QtGui.QTextEdit(MatisseDialog)
        self.txtMessage.setGeometry(QtCore.QRect(70, 280, 301, 41))
        self.txtMessage.setObjectName(_fromUtf8("txtMessage"))
        self.groupBox = QtGui.QGroupBox(MatisseDialog)
        self.groupBox.setGeometry(QtCore.QRect(10, 10, 371, 261))
        self.groupBox.setObjectName(_fromUtf8("groupBox"))
        self.treeJobs = QtGui.QTreeWidget(self.groupBox)
        self.treeJobs.setGeometry(QtCore.QRect(10, 20, 131, 201))
        self.treeJobs.setIconSize(QtCore.QSize(16, 16))
        self.treeJobs.setUniformRowHeights(True)
        self.treeJobs.setColumnCount(1)
        self.treeJobs.setObjectName(_fromUtf8("treeJobs"))
        self.treeJobs.headerItem().setText(0, _fromUtf8("1"))
        self.treeJobs.header().setVisible(False)
        self.treeJobs.header().setMinimumSectionSize(100)
        self.txtDescription = QtGui.QTextEdit(self.groupBox)
        self.txtDescription.setGeometry(QtCore.QRect(160, 20, 201, 201))
        self.txtDescription.setObjectName(_fromUtf8("txtDescription"))
        self.btnListJobs = QtGui.QPushButton(self.groupBox)
        self.btnListJobs.setGeometry(QtCore.QRect(60, 230, 81, 23))
        self.btnListJobs.setObjectName(_fromUtf8("btnListJobs"))
        self.btnCreateLayer = QtGui.QPushButton(self.groupBox)
        self.btnCreateLayer.setGeometry(QtCore.QRect(280, 230, 75, 23))
        self.btnCreateLayer.setObjectName(_fromUtf8("btnCreateLayer"))
        self.lblIconConnect = QtGui.QLabel(MatisseDialog)
        self.lblIconConnect.setGeometry(QtCore.QRect(20, 290, 21, 20))
        self.lblIconConnect.setText(_fromUtf8(""))
        self.lblIconConnect.setPixmap(QtGui.QPixmap(_fromUtf8("images/led-grey.png")))
        self.lblIconConnect.setScaledContents(True)
        self.lblIconConnect.setObjectName(_fromUtf8("lblIconConnect"))

        self.retranslateUi(MatisseDialog)
        QtCore.QMetaObject.connectSlotsByName(MatisseDialog)

    def retranslateUi(self, MatisseDialog):
        MatisseDialog.setWindowTitle(_translate("MatisseDialog", "MatissePlugin", None))
        self.groupBox.setTitle(_translate("MatisseDialog", "Travaux", None))
        self.btnListJobs.setText(_translate("MatisseDialog", "Rafraichir", None))
        self.btnCreateLayer.setText(_translate("MatisseDialog", "Creer Layer", None))


if __name__ == "__main__":
    import sys
    app = QtGui.QApplication(sys.argv)
    MatisseDialog = QtGui.QDialog()
    ui = Ui_MatisseDialog()
    ui.setupUi(MatisseDialog)
    MatisseDialog.show()
    sys.exit(app.exec_())

