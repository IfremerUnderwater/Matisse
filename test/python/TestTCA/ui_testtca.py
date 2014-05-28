# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui_testtca.ui'
#
# Created: Tue Jan 14 11:44:01 2014
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

class Ui_TestTCA(object):
    def setupUi(self, TestTCA):
        TestTCA.setObjectName(_fromUtf8("TestTCA"))
        TestTCA.resize(718, 634)
        self.verticalLayout_2 = QtGui.QVBoxLayout(TestTCA)
        self.verticalLayout_2.setObjectName(_fromUtf8("verticalLayout_2"))
        self.horizontalLayout_2 = QtGui.QHBoxLayout()
        self.horizontalLayout_2.setObjectName(_fromUtf8("horizontalLayout_2"))
        self.label = QtGui.QLabel(TestTCA)
        self.label.setObjectName(_fromUtf8("label"))
        self.horizontalLayout_2.addWidget(self.label)
        self._LE_server = QtGui.QLineEdit(TestTCA)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self._LE_server.sizePolicy().hasHeightForWidth())
        self._LE_server.setSizePolicy(sizePolicy)
        self._LE_server.setMinimumSize(QtCore.QSize(120, 0))
        self._LE_server.setReadOnly(True)
        self._LE_server.setObjectName(_fromUtf8("_LE_server"))
        self.horizontalLayout_2.addWidget(self._LE_server)
        self._LA_serverStatus = QtGui.QLabel(TestTCA)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self._LA_serverStatus.sizePolicy().hasHeightForWidth())
        self._LA_serverStatus.setSizePolicy(sizePolicy)
        self._LA_serverStatus.setMinimumSize(QtCore.QSize(32, 32))
        self._LA_serverStatus.setText(_fromUtf8(""))
        self._LA_serverStatus.setObjectName(_fromUtf8("_LA_serverStatus"))
        self.horizontalLayout_2.addWidget(self._LA_serverStatus)
        spacerItem = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_2.addItem(spacerItem)
        self.verticalLayout_2.addLayout(self.horizontalLayout_2)
        self.groupBox = QtGui.QGroupBox(TestTCA)
        self.groupBox.setObjectName(_fromUtf8("groupBox"))
        self.horizontalLayout_6 = QtGui.QHBoxLayout(self.groupBox)
        self.horizontalLayout_6.setObjectName(_fromUtf8("horizontalLayout_6"))
        self._TRW_processings = QtGui.QTreeWidget(self.groupBox)
        self._TRW_processings.setObjectName(_fromUtf8("_TRW_processings"))
        self._TRW_processings.header().setVisible(False)
        self.horizontalLayout_6.addWidget(self._TRW_processings)
        self.verticalLayout = QtGui.QVBoxLayout()
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.horizontalLayout_3 = QtGui.QHBoxLayout()
        self.horizontalLayout_3.setObjectName(_fromUtf8("horizontalLayout_3"))
        self.label_2 = QtGui.QLabel(self.groupBox)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_2.sizePolicy().hasHeightForWidth())
        self.label_2.setSizePolicy(sizePolicy)
        self.label_2.setMinimumSize(QtCore.QSize(85, 0))
        self.label_2.setObjectName(_fromUtf8("label_2"))
        self.horizontalLayout_3.addWidget(self.label_2)
        self._LE_creationDate = QtGui.QLineEdit(self.groupBox)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self._LE_creationDate.sizePolicy().hasHeightForWidth())
        self._LE_creationDate.setSizePolicy(sizePolicy)
        self._LE_creationDate.setMinimumSize(QtCore.QSize(80, 0))
        self._LE_creationDate.setMaximumSize(QtCore.QSize(80, 16777215))
        self._LE_creationDate.setReadOnly(True)
        self._LE_creationDate.setObjectName(_fromUtf8("_LE_creationDate"))
        self.horizontalLayout_3.addWidget(self._LE_creationDate)
        spacerItem1 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_3.addItem(spacerItem1)
        self.verticalLayout.addLayout(self.horizontalLayout_3)
        self.horizontalLayout_4 = QtGui.QHBoxLayout()
        self.horizontalLayout_4.setObjectName(_fromUtf8("horizontalLayout_4"))
        self.label_3 = QtGui.QLabel(self.groupBox)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_3.sizePolicy().hasHeightForWidth())
        self.label_3.setSizePolicy(sizePolicy)
        self.label_3.setMinimumSize(QtCore.QSize(85, 0))
        self.label_3.setObjectName(_fromUtf8("label_3"))
        self.horizontalLayout_4.addWidget(self.label_3)
        self._LE_author = QtGui.QLineEdit(self.groupBox)
        self._LE_author.setMinimumSize(QtCore.QSize(160, 0))
        self._LE_author.setMaximumSize(QtCore.QSize(160, 16777215))
        self._LE_author.setReadOnly(True)
        self._LE_author.setObjectName(_fromUtf8("_LE_author"))
        self.horizontalLayout_4.addWidget(self._LE_author)
        spacerItem2 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_4.addItem(spacerItem2)
        self.verticalLayout.addLayout(self.horizontalLayout_4)
        self.horizontalLayout_5 = QtGui.QHBoxLayout()
        self.horizontalLayout_5.setObjectName(_fromUtf8("horizontalLayout_5"))
        self.label_4 = QtGui.QLabel(self.groupBox)
        self.label_4.setObjectName(_fromUtf8("label_4"))
        self.horizontalLayout_5.addWidget(self.label_4)
        spacerItem3 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_5.addItem(spacerItem3)
        self.verticalLayout.addLayout(self.horizontalLayout_5)
        self._TXT_comments = QtGui.QTextEdit(self.groupBox)
        self._TXT_comments.setReadOnly(True)
        self._TXT_comments.setObjectName(_fromUtf8("_TXT_comments"))
        self.verticalLayout.addWidget(self._TXT_comments)
        spacerItem4 = QtGui.QSpacerItem(20, 40, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.verticalLayout.addItem(spacerItem4)
        self.horizontalLayout_6.addLayout(self.verticalLayout)
        self.verticalLayout_2.addWidget(self.groupBox)
        self.horizontalLayout = QtGui.QHBoxLayout()
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        spacerItem5 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem5)
        self._PB_apply = QtGui.QPushButton(TestTCA)
        self._PB_apply.setObjectName(_fromUtf8("_PB_apply"))
        self.horizontalLayout.addWidget(self._PB_apply)
        self._PB_cancel = QtGui.QPushButton(TestTCA)
        self._PB_cancel.setObjectName(_fromUtf8("_PB_cancel"))
        self.horizontalLayout.addWidget(self._PB_cancel)
        self.verticalLayout_2.addLayout(self.horizontalLayout)

        self.retranslateUi(TestTCA)
        QtCore.QMetaObject.connectSlotsByName(TestTCA)

    def retranslateUi(self, TestTCA):
        TestTCA.setWindowTitle(_translate("TestTCA", "Traitements", None))
        self.label.setText(_translate("TestTCA", "Serveur:", None))
        self.groupBox.setTitle(_translate("TestTCA", "Traitements", None))
        self._TRW_processings.headerItem().setText(0, _translate("TestTCA", "Traitements", None))
        self.label_2.setText(_translate("TestTCA", "Date de création:", None))
        self.label_3.setText(_translate("TestTCA", "Auteur:", None))
        self.label_4.setText(_translate("TestTCA", "Informations:", None))
        self._PB_apply.setText(_translate("TestTCA", "Lancer", None))
        self._PB_cancel.setText(_translate("TestTCA", "Fermer", None))


if __name__ == "__main__":
    import sys
    app = QtGui.QApplication(sys.argv)
    TestTCA = QtGui.QDialog()
    ui = Ui_TestTCA()
    ui.setupUi(TestTCA)
    TestTCA.show()
    sys.exit(app.exec_())

