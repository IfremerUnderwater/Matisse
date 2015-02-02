# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui_MatisseDialog.ui'
#
# Created: Mon Jun 23 15:03:21 2014
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
        MatisseDialog.resize(605, 430)
        self.verticalLayout = QtGui.QVBoxLayout(MatisseDialog)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.splitter = QtGui.QSplitter(MatisseDialog)
        self.splitter.setOrientation(QtCore.Qt.Horizontal)
        self.splitter.setChildrenCollapsible(False)
        self.splitter.setObjectName(_fromUtf8("splitter"))
        self.treeJobs = QtGui.QTreeWidget(self.splitter)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.MinimumExpanding, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.treeJobs.sizePolicy().hasHeightForWidth())
        self.treeJobs.setSizePolicy(sizePolicy)
        self.treeJobs.setMinimumSize(QtCore.QSize(100, 0))
        self.treeJobs.setMaximumSize(QtCore.QSize(16777215, 16777215))
        self.treeJobs.setIconSize(QtCore.QSize(16, 16))
        self.treeJobs.setUniformRowHeights(True)
        self.treeJobs.setColumnCount(1)
        self.treeJobs.setObjectName(_fromUtf8("treeJobs"))
        self.treeJobs.headerItem().setText(0, _fromUtf8("1"))
        self.treeJobs.header().setVisible(False)
        self.treeJobs.header().setMinimumSectionSize(100)
        self.groupBox = QtGui.QGroupBox(self.splitter)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(1)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.groupBox.sizePolicy().hasHeightForWidth())
        self.groupBox.setSizePolicy(sizePolicy)
        self.groupBox.setObjectName(_fromUtf8("groupBox"))
        self.gridLayout = QtGui.QGridLayout(self.groupBox)
        self.gridLayout.setObjectName(_fromUtf8("gridLayout"))
        self.lblDate = QtGui.QLabel(self.groupBox)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.lblDate.sizePolicy().hasHeightForWidth())
        self.lblDate.setSizePolicy(sizePolicy)
        self.lblDate.setObjectName(_fromUtf8("lblDate"))
        self.gridLayout.addWidget(self.lblDate, 0, 0, 1, 1)
        self.lblComment = QtGui.QLabel(self.groupBox)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.lblComment.sizePolicy().hasHeightForWidth())
        self.lblComment.setSizePolicy(sizePolicy)
        self.lblComment.setObjectName(_fromUtf8("lblComment"))
        self.gridLayout.addWidget(self.lblComment, 1, 0, 1, 1)
        self.txtDescription = QtGui.QTextEdit(self.groupBox)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(1)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.txtDescription.sizePolicy().hasHeightForWidth())
        self.txtDescription.setSizePolicy(sizePolicy)
        self.txtDescription.setMaximumSize(QtCore.QSize(16777215, 50))
        self.txtDescription.setObjectName(_fromUtf8("txtDescription"))
        self.gridLayout.addWidget(self.txtDescription, 1, 1, 1, 2)
        self.libImage = QtGui.QLabel(self.groupBox)
        self.libImage.setObjectName(_fromUtf8("libImage"))
        self.gridLayout.addWidget(self.libImage, 2, 0, 1, 1)
        self.tableImages = QtGui.QTableWidget(self.groupBox)
        self.tableImages.setColumnCount(2)
        self.tableImages.setObjectName(_fromUtf8("tableImages"))
        self.tableImages.setRowCount(0)
        item = QtGui.QTableWidgetItem()
        self.tableImages.setHorizontalHeaderItem(0, item)
        item = QtGui.QTableWidgetItem()
        self.tableImages.setHorizontalHeaderItem(1, item)
        self.tableImages.horizontalHeader().setStretchLastSection(True)
        self.gridLayout.addWidget(self.tableImages, 3, 0, 1, 3)
        self.btnCreateLayer = QtGui.QPushButton(self.groupBox)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.btnCreateLayer.sizePolicy().hasHeightForWidth())
        self.btnCreateLayer.setSizePolicy(sizePolicy)
        self.btnCreateLayer.setMaximumSize(QtCore.QSize(16777215, 16777215))
        self.btnCreateLayer.setToolTip(_fromUtf8(""))
        self.btnCreateLayer.setObjectName(_fromUtf8("btnCreateLayer"))
        self.gridLayout.addWidget(self.btnCreateLayer, 4, 2, 1, 1)
        self.txtDate = QtGui.QLineEdit(self.groupBox)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(1)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.txtDate.sizePolicy().hasHeightForWidth())
        self.txtDate.setSizePolicy(sizePolicy)
        self.txtDate.setObjectName(_fromUtf8("txtDate"))
        self.gridLayout.addWidget(self.txtDate, 0, 1, 1, 2)
        self.verticalLayout.addWidget(self.splitter)
        self.horizontalLayout = QtGui.QHBoxLayout()
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.lblIconConnect = QtGui.QLabel(MatisseDialog)
        self.lblIconConnect.setMinimumSize(QtCore.QSize(20, 20))
        self.lblIconConnect.setMaximumSize(QtCore.QSize(20, 20))
        self.lblIconConnect.setText(_fromUtf8(""))
        self.lblIconConnect.setPixmap(QtGui.QPixmap(_fromUtf8("images/led-grey.png")))
        self.lblIconConnect.setScaledContents(True)
        self.lblIconConnect.setObjectName(_fromUtf8("lblIconConnect"))
        self.horizontalLayout.addWidget(self.lblIconConnect)
        self.btnListJobs = QtGui.QPushButton(MatisseDialog)
        self.btnListJobs.setText(_fromUtf8(""))
        icon = QtGui.QIcon()
        icon.addPixmap(QtGui.QPixmap(_fromUtf8("images/view-refresh-3.png")), QtGui.QIcon.Normal, QtGui.QIcon.Off)
        self.btnListJobs.setIcon(icon)
        self.btnListJobs.setObjectName(_fromUtf8("btnListJobs"))
        self.horizontalLayout.addWidget(self.btnListJobs)
        self.txtMessage = QtGui.QTextEdit(MatisseDialog)
        self.txtMessage.setMaximumSize(QtCore.QSize(16777215, 50))
        self.txtMessage.setObjectName(_fromUtf8("txtMessage"))
        self.horizontalLayout.addWidget(self.txtMessage)
        self.verticalLayout.addLayout(self.horizontalLayout)

        self.retranslateUi(MatisseDialog)
        QtCore.QMetaObject.connectSlotsByName(MatisseDialog)

    def retranslateUi(self, MatisseDialog):
        MatisseDialog.setWindowTitle(_translate("MatisseDialog", "MatissePlugin", None))
        self.groupBox.setTitle(_translate("MatisseDialog", "Job", None))
        self.lblDate.setText(_translate("MatisseDialog", "Date", None))
        self.lblComment.setText(_translate("MatisseDialog", "Comment", None))
        self.libImage.setText(_translate("MatisseDialog", "Images", None))
        item = self.tableImages.horizontalHeaderItem(1)
        item.setText(_translate("MatisseDialog", "Image", None))
        self.btnCreateLayer.setText(_translate("MatisseDialog", "Create Layer", None))
        self.btnListJobs.setToolTip(_translate("MatisseDialog", "Refresh", None))


if __name__ == "__main__":
    import sys
    app = QtGui.QApplication(sys.argv)
    MatisseDialog = QtGui.QDialog()
    ui = Ui_MatisseDialog()
    ui.setupUi(MatisseDialog)
    MatisseDialog.show()
    sys.exit(app.exec_())

