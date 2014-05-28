# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'C:\QGIS Dufour\apps\qgis\python\plugins\TestTCA\ui_info.ui'
#
# Created: Wed Jan 08 17:49:12 2014
#      by: PyQt4 UI code generator 4.10.3
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

class Ui_InfoDialog(object):
    def setupUi(self, InfoDialog):
        InfoDialog.setObjectName(_fromUtf8("InfoDialog"))
        InfoDialog.resize(400, 300)
        self.verticalLayout = QtGui.QVBoxLayout(InfoDialog)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.horizontalLayout = QtGui.QHBoxLayout()
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.label = QtGui.QLabel(InfoDialog)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label.sizePolicy().hasHeightForWidth())
        self.label.setSizePolicy(sizePolicy)
        self.label.setMinimumSize(QtCore.QSize(85, 0))
        self.label.setObjectName(_fromUtf8("label"))
        self.horizontalLayout.addWidget(self.label)
        self._LE_creationDate = QtGui.QLineEdit(InfoDialog)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self._LE_creationDate.sizePolicy().hasHeightForWidth())
        self._LE_creationDate.setSizePolicy(sizePolicy)
        self._LE_creationDate.setMinimumSize(QtCore.QSize(80, 0))
        self._LE_creationDate.setMaximumSize(QtCore.QSize(80, 16777215))
        self._LE_creationDate.setReadOnly(True)
        self._LE_creationDate.setObjectName(_fromUtf8("_LE_creationDate"))
        self.horizontalLayout.addWidget(self._LE_creationDate)
        spacerItem = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem)
        self.verticalLayout.addLayout(self.horizontalLayout)
        self.horizontalLayout_2 = QtGui.QHBoxLayout()
        self.horizontalLayout_2.setObjectName(_fromUtf8("horizontalLayout_2"))
        self.label_2 = QtGui.QLabel(InfoDialog)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_2.sizePolicy().hasHeightForWidth())
        self.label_2.setSizePolicy(sizePolicy)
        self.label_2.setMinimumSize(QtCore.QSize(85, 0))
        self.label_2.setObjectName(_fromUtf8("label_2"))
        self.horizontalLayout_2.addWidget(self.label_2)
        self._LE_author = QtGui.QLineEdit(InfoDialog)
        self._LE_author.setMinimumSize(QtCore.QSize(160, 0))
        self._LE_author.setMaximumSize(QtCore.QSize(160, 16777215))
        self._LE_author.setReadOnly(True)
        self._LE_author.setObjectName(_fromUtf8("_LE_author"))
        self.horizontalLayout_2.addWidget(self._LE_author)
        spacerItem1 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_2.addItem(spacerItem1)
        self.verticalLayout.addLayout(self.horizontalLayout_2)
        self.horizontalLayout_3 = QtGui.QHBoxLayout()
        self.horizontalLayout_3.setObjectName(_fromUtf8("horizontalLayout_3"))
        self.label_3 = QtGui.QLabel(InfoDialog)
        self.label_3.setObjectName(_fromUtf8("label_3"))
        self.horizontalLayout_3.addWidget(self.label_3)
        spacerItem2 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_3.addItem(spacerItem2)
        self.verticalLayout.addLayout(self.horizontalLayout_3)
        self._TXT_comments = QtGui.QTextEdit(InfoDialog)
        self._TXT_comments.setReadOnly(True)
        self._TXT_comments.setObjectName(_fromUtf8("_TXT_comments"))
        self.verticalLayout.addWidget(self._TXT_comments)

        self.retranslateUi(InfoDialog)
        QtCore.QMetaObject.connectSlotsByName(InfoDialog)

    def retranslateUi(self, InfoDialog):
        InfoDialog.setWindowTitle(_translate("InfoDialog", "Dialog", None))
        self.label.setText(_translate("InfoDialog", "Date de création:", None))
        self.label_2.setText(_translate("InfoDialog", "Auteur:", None))
        self.label_3.setText(_translate("InfoDialog", "Informations:", None))


if __name__ == "__main__":
    import sys
    app = QtGui.QApplication(sys.argv)
    InfoDialog = QtGui.QDialog()
    ui = Ui_InfoDialog()
    ui.setupUi(InfoDialog)
    InfoDialog.show()
    sys.exit(app.exec_())

