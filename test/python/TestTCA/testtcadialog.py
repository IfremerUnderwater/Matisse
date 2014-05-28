# -*- coding: utf-8 -*-
"""
/***************************************************************************
 TestTCADialog
																 A QGIS plugin
 Un premier test de plugin by TCA
														 -------------------
				begin								: 2013-10-30
				copyright						: (C) 2013 by Chrisar/TCA
				email								: thierry.cabassud@chrisar.fr
 ***************************************************************************/

/***************************************************************************
 *																																				 *
 *	 This program is free software; you can redistribute it and/or modify	*
 *	 it under the terms of the GNU General Public License as published by	*
 *	 the Free Software Foundation; either version 2 of the License, or		 *
 *	 (at your option) any later version.																	 *
 *																																				 *
 ***************************************************************************/
"""
# pyuic4 -o ui_testtca.py -x ui_testtca.ui
# pyrcc4 -o resources.py resources.qrc
from PyQt4 import QtCore, QtGui, QtNetwork
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.QtNetwork import *
from qgis.core import *
from qgis.gui import *

from ui_testtca import Ui_TestTCA
from infoDialog import InfoDialog

# create the dialog for zoom to point

class TestTCADialog(QtGui.QDialog):
	def __init__(self):
		QtGui.QDialog.__init__(self)
		self._hostAddress = "192.168.0.191"
		# Set up the user interface from Designer.
		self._ui = Ui_TestTCA()
		self._ui.setupUi(self)
		# connection au serveur et recuperation des processeurs
		self._socket = QtNetwork.QTcpSocket(self)
		# Ajout des connects...
		# self.socket.connect(QObject, SIGNAL()
		self._socket.readyRead.connect(self.slot_readyRead)
		self._socket.disconnected.connect(self.slot_disconnected)
# 				self.ui._PB_apply.clicked.connect(self.slot_readyRead)
		# QObject.connect(self.socket, SIGNAL("readyRead()"), self, SLOT("slot_readyRead()"))
		# QObject.connect(self.ui._PB_cancel, SIGNAL("clicked()"), self, SLOT("reject()"))
		self._ui._PB_cancel.clicked.connect(self.reject)
		self._ui._PB_apply.clicked.connect(self.slot_loadParameters)
# 				self._ui._PB_information.clicked.connect(self.slot_getInformation)
		self._ui._TRW_processings.itemClicked.connect(self.slot_itemSelected)
		self._usableProcessors = {}
		# self.ui._PB_apply.clicked.connect(self.slotReadyRead)
		# remplissage du dictionaire des fonctions
		self._messagesDict = {'$PROC:': 'processProc', '$INFO:' : 'processInfo', '$PARM:' : 'processParams', '$MODL:' : 'processParamsModel'}
		self._ui._PB_apply.setDisabled(True)
		
		self._parametersModel = QByteArray()

		return

	def processProc(self, command=""):
		processor = command[6:]
		if (processor[0] == "+"):
			processor = processor[1:]
			print "add	processor", processor
			# self._ui._LW_processings.addItem(processor)
			configurations = processor.split(",")
			assemblyName = configurations[0]
			assembly = QTreeWidgetItem()
			assembly.setText(0, assemblyName)
			for configuration in configurations[1:]:
				user = QTreeWidgetItem()
				user.setText(0, configuration)
				assembly.addChild(user)
			self._ui._TRW_processings.addTopLevelItem(assembly)
		elif (processor[0] == "-"):
			print "remove	processor"
			
		return
		
	def processInfo(self, command=""):
		values = command[6:].split(",")
		self._ui._LE_creationDate.setText(values[0])
		self._ui._LE_author.setText(values[1])
		self._ui._TXT_comments.setText(values[2])
		
		return

	def processParams(self, command=""):
		values = command[6:].split(",")
		
		return

	def processParamsModel(self, command=""):
		self._parametersModel.clear()
		self._parametersModel.append(command)
		
		return

	@pyqtSlot()
	def slot_disconnected(self):
		self._ui._LE_server.setText("")
		self._ui._LE_creationDate.setText("")
		self._ui._LE_author.setText("")
		self._ui._TXT_comments.setText("")
		self._ui._TRW_processings.clear()
		self._ui._PB_apply.setDisabled(True)

		return

	@pyqtSlot()
	def slot_readyRead(self):
		# QMessageBox.information(self, "Datas", "Datas")
		array = QByteArray()
		while self._socket.bytesAvailable() > 0:
			array.append(self._socket.readAll())
		datas = array.__str__()
		print "DATAS=" + datas
		commands = datas.split(";")
		print "taille cmd" + str(len(commands))
		for command in commands:
			print "COMMANDE= " + command
			command = command.strip()
			commandFunc = command[0:6]
			if (self._messagesDict.has_key(commandFunc)):
				func = getattr(self, self._messagesDict[commandFunc])
				func(command)
			else:
					print "Ailleurs..."
					print "Reception: " + datas
					
		return

	def showEvent(self, e):
		print "show..."
		self._ui._TRW_processings.clear()
		self._socket.connectToHost(QtNetwork.QHostAddress(self._hostAddress), 54637)
		self._ui._LE_server.setText(self._hostAddress)
		
		return
	
	def hideEvent(self, e):
		print "hide..."
		self._socket.close()
		self._ui._LE_server.setText("")
		
		return

	@pyqtSlot()
	def slot_loadParameters(self):
		# teste chargement carte...
		# layer =	QgsVectorLayer("C:/qgis_sample_data/shapefiles/alaska.shp", "Test TCA", "ogr")
		# QgsMapLayerRegistry.instance().addMapLayer(layer)
		sendData = QByteArray()
		sendData.append("$STAR:")
# 				selectedItem = self._ui._LW_processings.selectedItems()[0]
#			sendData.append(selectedItem.text())
		sendData.append(";")
		self._socket.write(sendData)
		self._socket.flush()
		
		return

	@pyqtSlot(QTreeWidgetItem)
	def slot_itemSelected(self, item):
		# charge infos
		print(item.text(0))
		sendData = QByteArray()
		sendData.append(item.text(0))
		if (item.parent() != None):
			self._ui._PB_apply.setDisabled(False)
			sendData.prepend(item.parent().text(0) + ",")
		else:
							self._ui._PB_apply.setDisabled(True)
		sendData.prepend("$INF?:")
		sendData.append(";")
# 				selectedItem = self._ui._LW_processings.selectedItems()[0]
		self._socket.write(sendData)
		self._socket.flush()
		
		return
