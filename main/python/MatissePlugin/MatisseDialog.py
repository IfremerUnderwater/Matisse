'''
Created on 25 mars 2014

@author: d.amar
'''
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.QtNetwork import *
from qgis.core import *
from PyQt4 import QtCore

import resources

# Generated by pyuic4 -o ui_MatissDialog.py -x ui_MatissDialog.ui
# TODO: Code a commenter
# TODO: Donner la possibilite de changer de port de connexion

from ui_MatisseDialog import Ui_MatisseDialog

class MatisseDialog(QDialog):
    
    signal_create_layer = pyqtSignal(str, str, str)
    
    def __init__(self, parent=None):
        
        # IHM
        QDialog.__init__(self)
        self._ui = Ui_MatisseDialog()
        self._ui.setupUi(self);
        # 
        self._ui.lblIconConnect.setPixmap(QPixmap(":/images/led-grey.png"))
        self._ui.btnListJobs.setIcon(QIcon(":/images/view-refresh-3.png"));

        self._connected = False
        self._messagesDict = {'JOB': 'processJob', 'CONFIG' : 'processConfig', 'JOBEXECUTION' : 'processExecution'}

        # Slot/signal connection
        self._ui.btnListJobs.clicked.connect(self.slot_listjobs)
        self._ui.btnCreateLayer.clicked.connect(self.slot_createLayer)
        self._ui.treeJobs.itemClicked.connect(self.slot_job_selected)
        
        # UI Tuning
        #   Table
        self._ui.tableImages.setColumnWidth(0, 20);
        
        #  Splitter
        self._ui.splitter.setSizes([150, 200])
        self._ui.splitter.setStretchFactor(0, 0);
        self._ui.splitter.setStretchFactor(1, 1);
        #self._ui.tableImages.horizontalHeader().setResizeMode(QHeaderView.Stretch)
        
        
        self._jobs={}
        self._assemblies={}
        
        # Client
        self._socket = QTcpSocket(self)
        self._socket.connected.connect(self.slot_socket_connected)

        self._socket.readyRead.connect(self.slot_socket_readyRead)
        self._socket.disconnected.connect(self.slot_socket_disconnected)
        self._socket.connectToHost(QHostAddress.LocalHost, 8888)
        self.clearJobs()

    @pyqtSlot(QTreeWidgetItem)
    def slot_job_selected(self, item ):
        if (item.childCount() == 0 ):
            self._selectedJob = str(item.text(0))
            job = self._jobs[str(item.text(0))]
 
            description= job["comment"] 
                     
            self._ui.txtDate.setText(job["date"])
            self._ui.txtDescription.setText(description)
            self._ui.btnCreateLayer.setEnabled(True)
            rowIndex = 0
            
            
            # Remove all
            while self._ui.tableImages.rowCount() > 0:
                self._ui.tableImages.removeRow(0);
            

            for result in job["results"]:
                self._ui.tableImages.insertRow(rowIndex);
                # Checkbox
                cbxItem = QCheckBox();
                cbxItem.setChecked(True)
                self._ui.tableImages.setCellWidget(rowIndex, 0, cbxItem);
                # Filename
                fileItem = QTableWidgetItem(result)

                self._ui.tableImages.setItem(rowIndex, 1, fileItem)
                fileItem.setToolTip(result)
                
                
                
    @pyqtSlot()
    def slot_createLayer(self):
        print "Create Layer"
        if (self._selectedJob is not None):
            #job = self._jobs[self._selectedJob]
            
            for rowIndex in range(self._ui.tableImages.rowCount()):
                
                cbxItem = self._ui.tableImages.cellWidget(rowIndex, 0)
                fileItem = self._ui.tableImages.item(rowIndex, 1)
                isChecked = cbxItem.isChecked()
                if (isChecked):
                    result = fileItem.text();

                    fileInfo = QFileInfo(result)
                    if (not fileInfo.isAbsolute()):
                        fileInfo = QFileInfo(self._config + "\\"+ result)
                    if (fileInfo.exists()):
                        self.signal_create_layer.emit(self._selectedJob, fileInfo.fileName(), fileInfo.absoluteFilePath())
                     
    @pyqtSlot()    
    def slot_socket_connected(self):
        print "Connected"
        self._connected = True
        self._ui.lblIconConnect.setPixmap(QPixmap(":/images/images/led-green.png"))
        self.slot_listjobs()

    @pyqtSlot()    
    def slot_socket_readyRead(self):
            print "ReadData";
            array = QByteArray()
            while (self._socket.bytesAvailable() > 0) :
                array.append(self._socket.readAll());
            data = array.__str__()
            
            frames = data.split("^");
            
            for frame in frames:
                if (frame!="") : 
                    print "DATAS=" + frame
                    strs = frame.split(':',1)
                    cmd = strs[0].lstrip('$')
                    print cmd
                    
                    values=strs[1].split(";")
                    if (self._messagesDict.has_key(cmd)):
                        func = getattr(self, self._messagesDict[cmd])
                        func(values)
                    else:
                        print "Unknown message: " + frame
                           
            
        
    @pyqtSlot()    
    def slot_socket_disconnected(self):
        print "Disconnected"
        self._connected = False
        self._ui.lblIconConnect.setPixmap(QPixmap(":/images/images/led-grey.png"))
        self.clearJobs()        
        
    @pyqtSlot()    
    def slot_listjobs(self):
        print "ListJobs"
        if self._connected:
            # Effacer la liste des jobs
            self.clearJobs()
            
            # Envoyer la demande de mise a jour 
            sendData = QByteArray()
            sendData.append("$LISTJOBS")
            self._socket.write(sendData)
            self._socket.flush()
            
 
    def processJob(self, values=""):
        print "Receive Job"
        if (len(values)>5):
            jobName=values[0]
            assemblyName=values[1]
            date=values[2]
            comment=values[3]
            resultsCount=int(values[4])
            #print "Job Name:"  + jobName
            #print "Assembly Name:"  + assemblyName
            #print "Date:" + date
            #print "Comment:" + comment
            #print "Results number:" + str(resultsCount)
            
            results=[]
            for i  in range(resultsCount):
                result = values[5+i]
                #print "Result:" + result
                results.append(result)
            
            
           
            self.addJob(jobName, assemblyName, date, comment, results)

    def addJob(self, jobName, assemblyName, date, comment, results):
        
        
        if (assemblyName not in self._assemblies):
            assemblyItem =  QTreeWidgetItem([assemblyName])
            self._assemblies[assemblyName]=assemblyItem
            self._ui.treeJobs.addTopLevelItem(assemblyItem)
        else:
            assemblyItem = self._assemblies[assemblyName]
            
        self._jobs[jobName] = {"date": date, "comment" : comment, "results" : results}
        QTreeWidgetItem(assemblyItem, [jobName])
        self._ui.treeJobs.expandAll()
        
        
    def processConfig(self, values=""):
        self._config=values[0]
        print "Config: " + self._config
        
    def processExecution(self, values=""):
        jobName=values[0]
        print "Execution de : " + jobName
        self._ui.txtMessage.append("Execution de " + jobName + "\n")
        self.slot_listjobs();       

    def clearJobs(self):
        self._jobs={}
        self._assemblies={}
        self._selectedJob = None
        self._ui.btnCreateLayer.setEnabled(False)
        self._ui.treeJobs.clear()
        self._ui.txtDescription.clear()
        
    
        