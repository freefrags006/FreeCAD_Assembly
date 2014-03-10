#***************************************************************************
#*                                                                         *
#*   Copyright (c) 2013 - Juergen Riegel <FreeCAD@juergen-riegel.net>      *  
#*                                                                         *
#*   This program is free software; you can redistribute it and/or modify  *
#*   it under the terms of the GNU Lesser General Public License (LGPL)    *
#*   as published by the Free Software Foundation; either version 2 of     *
#*   the License, or (at your option) any later version.                   *
#*   for detail see the LICENCE text file.                                 *
#*                                                                         *
#*   This program is distributed in the hope that it will be useful,       *
#*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
#*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
#*   GNU Library General Public License for more details.                  *
#*                                                                         *
#*   You should have received a copy of the GNU Library General Public     *
#*   License along with this program; if not, write to the Free Software   *
#*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
#*   USA                                                                   *
#*                                                                         *
#***************************************************************************

import FreeCAD, Fem
try:
    from numpy import * 
    import matplotlib.pyplot as plt
    import Plot
except:
    FreeCAD.Console.PrintWarning("Error: matplotlib or numpy not installed, MaterialFit will not be working\n")
else:
    import csv
    import os

    if FreeCAD.GuiUp:
        import FreeCADGui,FemGui
        from FreeCAD import Vector
        from PyQt4 import QtCore, QtGui
        from pivy import coin
        import PyQt4.uic as uic

    __title__="Machine-Distortion Material fit and plot managment"
    __author__ = "Juergen Riegel"
    __url__ = "http://free-cad.sourceforge.net"


    StartMat = {'FEM_youngsmodulus'         :'7000.00',
                'PartDist_poissonratio'     :'0.30',
                'PartDist_platethickness'   :'0.0',
                'PartDist_lc1'              :'0.0',
                'PartDist_lc2'              :'0.0',
                'PartDist_lc3'              :'0.0',
                'PartDist_lc4'              :'0.0',
                'PartDist_lc5'              :'0.0',
                'PartDist_lc6'              :'0.0',
                'PartDist_lc7'              :'0.0',
                'PartDist_ltc1'             :'0.0',
                'PartDist_ltc2'             :'0.0',
                'PartDist_ltc3'             :'0.0',
                'PartDist_ltc4'             :'0.0',
                'PartDist_ltc5'             :'0.0',
                'PartDist_ltc6'             :'0.0',
                'PartDist_ltc7'             :'0.0'
                }
                
    def is_float_try(str):
        try:
            float(str)
            return True
        except ValueError:
            return False                    

    def makeMaterialFit(name):
        '''makeMaterial(name): makes an Material
        name there fore is a material name or an file name for a FCMat file'''
        obj = FreeCAD.ActiveDocument.addObject("App::MaterialObjectPython",name)
        _MaterialFit(obj)
        _ViewProviderMaterialFit(obj.ViewObject)
        #FreeCAD.ActiveDocument.recompute()
        return obj
        
        
    class _CommandMaterialFit:
        "the MachDist Material command definition"
        def GetResources(self):
            return {'Pixmap'  : 'MachDist_Material',
                    'MenuText': QtCore.QT_TRANSLATE_NOOP("MachDist_Material","Material fit"),
                    'Accel': "A, X",
                    'ToolTip': QtCore.QT_TRANSLATE_NOOP("MachDist_Material","Creates or edit the material definition.")}
            
        def Activated(self):
            
            if FemGui.getActiveAnalysis().PartMaxX < 0.01:
                QtGui.QMessageBox.critical(None, "Missing prerequisit","Do first a Part-Aligment!")
                return 
                
            MatObj = None
            for i in FemGui.getActiveAnalysis().Member:
                if i.isDerivedFrom("App::MaterialObject"):
                        MatObj = i
            
            if (not MatObj):
                FreeCAD.ActiveDocument.openTransaction("Create MaterialFit")
                FreeCADGui.addModule("MachDistMaterialFit")
                FreeCADGui.doCommand("mat = MachDistMaterialFit.makeMaterialFit('MaterialFit')")
                FreeCADGui.doCommand("App.activeDocument()."+FemGui.getActiveAnalysis().Name+".Member = App.activeDocument()."+FemGui.getActiveAnalysis().Name+".Member + [mat]")
                FreeCADGui.doCommand("Gui.activeDocument().setEdit(mat.Name,0)")
                #FreeCADGui.doCommand("MachDist.makeMaterial()")
            else:
                FreeCADGui.doCommand("Gui.activeDocument().setEdit('"+MatObj.Name+"',0)")
            
        def IsActive(self):
            if FemGui.getActiveAnalysis():
                return True
            else:
                return False

           
    class _MaterialFit:
        "The Material object"
        def __init__(self,obj):
            self.Type = "MachDistMaterialFit"
            obj.Proxy = self
            obj.Material = StartMat
            #obj.addProperty("App::PropertyString","MaterialName","Base",
            #                "The name of the distorion material")

            
        def execute(self,obj):
            return
            
        def onChanged(self,obj,prop):
            if prop in ["MaterialName"]:
                return

        def __getstate__(self):
            return self.Type

        def __setstate__(self,state):
            if state:
                self.Type = state
            
    class _ViewProviderMaterialFit:
        "A View Provider for the MaterialFit object"

        def __init__(self,vobj):
            vobj.Proxy = self
           
        def getIcon(self):
            import machdist_rc
            return ":/icons/MachDist_Material.svg"

        def claimChildren(self):
            return []

        def attach(self, vobj):
            self.ViewObject = vobj
            self.Object = vobj.Object


        def updateData(self, obj, prop):
            return

        def onChanged(self, vobj, prop):
            return
      
        def setEdit(self,vobj,mode):
            taskd = _MaterialFitTaskPanel(self.Object)
            taskd.obj = vobj.Object
            #taskd.update()
            FreeCADGui.Control.showDialog(taskd)
            return True
        
        def unsetEdit(self,vobj,mode):
            FreeCADGui.Control.closeDialog()
            return

        def __getstate__(self):
            return None

        def __setstate__(self,state):
            return None


    class _MaterialFitTaskPanel:
        '''The editmode TaskPanel for Material objects'''
        def __init__(self,obj):
            # the panel has a tree widget that contains categories
            # for the subcomponents, such as additions, subtractions.
            # the categories are shown only if they are not empty.
            form_class, base_class = uic.loadUiType(FreeCAD.getHomePath() + "Mod/Machining_Distortion/MaterialFit.ui")

            self.obj = obj
            self.formUi = form_class()
            self.form = QtGui.QWidget()
            self.formUi.setupUi(self.form)
            self.params = FreeCAD.ParamGet("User parameter:BaseApp/Preferences/Mod/Machining_Distortion")


            QtCore.QObject.connect(self.formUi.select_file_fit_LC, QtCore.SIGNAL("clicked()"), self.add_fit_dataLC)
            QtCore.QObject.connect(self.formUi.select_file_fit_LTC, QtCore.SIGNAL("clicked()"), self.add_fit_dataLTC)

            QtCore.QObject.connect(self.formUi.select_file_LC, QtCore.SIGNAL("clicked()"), self.add_L_data)
            QtCore.QObject.connect(self.formUi.select_file_LTC, QtCore.SIGNAL("clicked()"), self.add_LT_data)

            QtCore.QObject.connect(self.formUi.pushButton_SaveMat, QtCore.SIGNAL("clicked()"), self.saveMat)
            QtCore.QObject.connect(self.formUi.toolButton_chooseDir, QtCore.SIGNAL("clicked()"), self.chooseDir)
            QtCore.QObject.connect(self.formUi.comboBox_MaterialsInDir, QtCore.SIGNAL("currentIndexChanged(int)"), self.chooseMat)

            QtCore.QObject.connect(self.formUi.spinBox_Plate_Thickness, QtCore.SIGNAL("valueChanged(double)"), self.calcBuyToFly)
            
            self.polval1 = None
            self.polval2 = None
            self.xmax1   = None
            self.xmin1   = None

            self.PlotWin = None
            
            self.FitDoneL = False
            self.FitDoneLT = False

            self.update()
            
        def transferTo(self):
            "Transfer from the dialog to the object" 
            
            matmap = self.obj.Material

            matmap['FEM_youngsmodulus']       = str(self.formUi.spinBox_young_modulus.value())
            matmap['PartDist_poissonratio']   = str(self.formUi.spinBox_poisson_ratio.value())
            matmap['PartDist_platethickness'] = str(self.formUi.spinBox_Plate_Thickness.value())
            FemGui.getActiveAnalysis().PlateThikness = self.formUi.spinBox_Plate_Thickness.value()

            matmap['PartDist_lc1'] = str(self.formUi.lc1.value())
            matmap['PartDist_lc2'] = str(self.formUi.lc2.value())
            matmap['PartDist_lc3'] = str(self.formUi.lc3.value())
            matmap['PartDist_lc4'] = str(self.formUi.lc4.value())
            matmap['PartDist_lc5'] = str(self.formUi.lc5.value())
            matmap['PartDist_lc6'] = str(self.formUi.lc6.value())
            matmap['PartDist_lc7'] = str(self.formUi.lc7.value())

            matmap['PartDist_ltc1'] = str(self.formUi.ltc1.value())
            matmap['PartDist_ltc2'] = str(self.formUi.ltc2.value())
            matmap['PartDist_ltc3'] = str(self.formUi.ltc3.value())
            matmap['PartDist_ltc4'] = str(self.formUi.ltc4.value())
            matmap['PartDist_ltc5'] = str(self.formUi.ltc5.value())
            matmap['PartDist_ltc6'] = str(self.formUi.ltc6.value())
            matmap['PartDist_ltc7'] = str(self.formUi.ltc7.value())
            #print matmap
            self.obj.Material = matmap 

        def update(self):
            'fills the widgets'
            self.transferFrom()
            self.fillMaterialCombo()
            self.calcBuyToFly(FemGui.getActiveAnalysis().PlateThikness)

        def calcBuyToFly(self,value):
            volume = FemGui.getActiveAnalysis().MeshVolume
            MaxX = FemGui.getActiveAnalysis().PartMaxX
            MaxY = FemGui.getActiveAnalysis().PartMaxY
            BuyToFly = (volume/(MaxX*MaxY*value))*100
            self.formUi.lineEdit_BuyToFly.setText("%f"%BuyToFly + ' %')
            #print "calcBuyToFly ",MaxX,MaxY,value,volume,BuyToFly

        def transferFrom(self):
            "Transfer from the object to the dialog"
            matmap = self.obj.Material
            #print matmap
            self.formUi.spinBox_young_modulus.setValue(float(matmap['FEM_youngsmodulus']))
            self.formUi.spinBox_poisson_ratio.setValue(float(matmap['PartDist_poissonratio']))
            #thickness = float(matmap['PartDist_platethickness'])
            #if thickness < 0.1:
            thickness = FemGui.getActiveAnalysis().PlateThikness
            self.formUi.spinBox_Plate_Thickness.setValue(thickness)
           
            self.formUi.lineEdit_PartThickness.setText("%f mm"%FemGui.getActiveAnalysis().PartThikness)
            self.calcBuyToFly(thickness)
            self.formUi.lc1.setValue(float(matmap['PartDist_lc1']))
            self.formUi.lc2.setValue(float(matmap['PartDist_lc2']))
            self.formUi.lc3.setValue(float(matmap['PartDist_lc3']))
            self.formUi.lc4.setValue(float(matmap['PartDist_lc4']))
            self.formUi.lc5.setValue(float(matmap['PartDist_lc5']))
            self.formUi.lc6.setValue(float(matmap['PartDist_lc6']))
            self.formUi.lc7.setValue(float(matmap['PartDist_lc7']))

            self.formUi.ltc1.setValue(float(matmap['PartDist_ltc1']))
            self.formUi.ltc2.setValue(float(matmap['PartDist_ltc2']))
            self.formUi.ltc3.setValue(float(matmap['PartDist_ltc3']))
            self.formUi.ltc4.setValue(float(matmap['PartDist_ltc4']))
            self.formUi.ltc5.setValue(float(matmap['PartDist_ltc5']))
            self.formUi.ltc6.setValue(float(matmap['PartDist_ltc6']))
            self.formUi.ltc7.setValue(float(matmap['PartDist_ltc7']))

        def add_L_data(self):
            l_filename = QtGui.QFileDialog.getOpenFileName(None, 'Open file','','R-Script File for L Coefficients (*.txt)')
            values = self.parse_R_output(l_filename)
            self.formUi.lc1.setValue(values[0])
            self.formUi.lc2.setValue(values[1])
            self.formUi.lc3.setValue(values[2])
            self.formUi.lc4.setValue(values[3])
            self.formUi.lc5.setValue(values[4])
            self.formUi.lc6.setValue(values[5])
            self.formUi.lc7.setEnabled(False) 
            self.formUi.tabWidget.setCurrentIndex(0)
            
            
        def add_LT_data(self):
            lt_filename = QtGui.QFileDialog.getOpenFileName(None, 'Open file','','R-Script File for LT Coefficients (*.txt)')
            values = self.parse_R_output(lt_filename)
            self.formUi.ltc1.setValue(values[0])
            self.formUi.ltc2.setValue(values[1])
            self.formUi.ltc3.setValue(values[2])
            self.formUi.ltc4.setValue(values[3])
            self.formUi.ltc5.setValue(values[4])
            self.formUi.ltc6.setValue(values[5])
            self.formUi.ltc7.setEnabled(False) 
            self.formUi.tabWidget.setCurrentIndex(0)

        def parse_R_output(self,filename):
            file = open(str(filename))
            lines = file.readlines()
            found = False
            coeff = []
            for line in lines:
                if line[0:9] == "c0 to c5:":
                    found = True
                    coeff.append(float(line[15:]))
                    continue
                if found and line[0:4] == "MSE:":
                    found = False
                if found:
                    coeff.append(float(line[15:]))
            
            file.close()
            return coeff[0],coeff[1],coeff[2],coeff[3],coeff[4],coeff[5]

        def isAllowedAlterSelection(self):
            return False

        def isAllowedAlterView(self):
            return True

        def getStandardButtons(self):
            return int(QtGui.QDialogButtonBox.Ok) | int(QtGui.QDialogButtonBox.Cancel)
        
                    
        def accept(self):
            self.transferTo()
            Plot.closePlot()
            FreeCADGui.ActiveDocument.resetEdit()
                        
        def reject(self):
            Plot.closePlot()
            FreeCADGui.ActiveDocument.resetEdit()

        def saveMat(self):
            self.transferTo()
            filename = QtGui.QFileDialog.getSaveFileName(None, 'Save Material file file',self.params.GetString("MaterialDir",'/'),'FreeCAD material file (*.FCMat)')
            if(filename):
                import Material
                Material.exportFCMat(filename,self.obj.Material)
                
        def chooseDir(self):
            dirname = QtGui.QFileDialog.getExistingDirectory(None, 'Choose material directory',self.params.GetString("MaterialDir",'/'))
            if(dirname):
                self.params.SetString("MaterialDir",str(dirname))
                self.fillMaterialCombo()
        
        def chooseMat(self,index):
            if index <= 0:return 
            if len(self.pathList) == 0:return 
            
            import Material
            name = self.pathList[index-1]
            #print 'Import ', str(name)
            
            self.obj.Material = Material.importFCMat(str(name))
            #print self.obj.Material
            #print "choose Mat", index, name
            self.transferFrom()
            
        def fillMaterialCombo(self):
            import glob,os
            dirname = self.params.GetString("MaterialDir",'/')
            self.pathList = glob.glob(dirname + '/*.FCMat')
            self.formUi.comboBox_MaterialsInDir.clear()
            self.formUi.comboBox_MaterialsInDir.addItem('-> choose Material')
            for i in self.pathList:
                self.formUi.comboBox_MaterialsInDir.addItem(os.path.basename(i) )
        
        def checkChangedL(self,index):
            #print "change"
            self.updateFitL()
            self.updatePlot()
            
        def checkChangedLT(self,index):
            #print "change"
            self.updateFitLT()
            self.updatePlot()
        
        def updateFitL(self):
            #print "update fit L" 
            x1 = []
            y1 = []
            x2 = []
            y2 = []

            for i in range(self.formUi.tableWidget_LC.rowCount () ):
                if self.formUi.tableWidget_LC.cellWidget(i,3).checkState() == 2:
                    y1.append(float (self.formUi.tableWidget_LC.item(i,1).text()) )
                    x1.append(float (self.formUi.tableWidget_LC.item(i,2).text()) )
                            
            if len(y1):
                self.polval1 = polyfit(x1, y1, 6)
                self.xmin1 = min(x1)
                self.xmax1 = max(x1)
                #self.xmin2 = min(x2)
                #self.xmax2 = max(x2)
                self.formUi.lc1.setValue(self.polval1[0])
                self.formUi.lc2.setValue(self.polval1[1])
                self.formUi.lc3.setValue(self.polval1[2])
                self.formUi.lc4.setValue(self.polval1[3])
                self.formUi.lc5.setValue(self.polval1[4])
                self.formUi.lc6.setValue(self.polval1[5])
                self.formUi.lc7.setEnabled(True)
                self.formUi.lc7.setValue(self.polval1[6])
            self.FitDoneL = True
            
        def updateFitLT(self):
            #print "update fit LT" 
            x1 = []
            y1 = []
            x2 = []
            y2 = []
                
            for i in range(self.formUi.tableWidget_LTC.rowCount () ):
                if self.formUi.tableWidget_LTC.cellWidget(i,3).checkState() == 2:
                    y2.append(float (self.formUi.tableWidget_LTC.item(i,1).text()) )
                    x2.append(float (self.formUi.tableWidget_LTC.item(i,2).text()) )
            

            if len(y2):
                self.polval2 = polyfit(x2, y2, 6)
                self.formUi.ltc1.setValue(self.polval2[0])
                self.formUi.ltc2.setValue(self.polval2[1])
                self.formUi.ltc3.setValue(self.polval2[2])
                self.formUi.ltc4.setValue(self.polval2[3])
                self.formUi.ltc5.setValue(self.polval2[4])
                self.formUi.ltc6.setValue(self.polval2[5])
                self.formUi.ltc7.setEnabled(True)
                self.formUi.ltc7.setValue(self.polval2[6])
            self.FitDoneLT = True

        def updatePlot(self):
            if not self.PlotWin:
                self.PlotWin = Plot.figure("Fit diagram")
                self.PlotWin.legend = True
                
            newx = linspace(self.xmin1,self.xmax1, 100)
            newy1 = []
            newy2 = []
            
            
            for i in newx:
                if self.FitDoneL:
                    newy1.append(i**6*self.polval1[0]+i**5*self.polval1[1]+i**4*self.polval1[2]+i**3*self.polval1[3]+i**2*self.polval1[4]+i*self.polval1[5]+self.polval1[6])
                else:
                    newy1.append(0.0)
                if self.FitDoneLT:
                    newy2.append(i**6*self.polval2[0]+i**5*self.polval2[1]+i**4*self.polval2[2]+i**3*self.polval2[3]+i**2*self.polval2[4]+i*self.polval2[5]+self.polval2[6])
                else:
                    newy2.append(0.0)

            
            Plot.removeSerie(1)
            Plot.removeSerie(0)
            Plot.plot(newy1,newx,name='LC')
            Plot.plot(newy2,newx,name='LTC')
            
        def readCSV(self, fileName):
            inputcsvfile = open(fileName, 'r')
            r = csv.reader(inputcsvfile)
            
            res = {}
            
            for row in r:
                #print row
                if is_float_try(row[1]) and is_float_try(row[2]):
                    if res.has_key(float(row[2])):
                        res[float(row[2])] = (res[float(row[2])] + float(row[1]))/2.0
                    else:
                        res[float(row[2])] = float(row[1])
                    
            return res
            
        def add_fit_dataLC(self):
            l_filename = QtGui.QFileDialog.getOpenFileName(None, 'Open file','','L file (*.csv)')
            
            if l_filename == "": return 
            
            res = self.readCSV(l_filename)
            
            self.formUi.tableWidget_LC.setRowCount(0)
            
            for t in sorted(res.keys()):
                #print row[0],row[1],row[2]
                self.formUi.tableWidget_LC.insertRow(0)
                item = QtGui.QTableWidgetItem('LC')
                self.formUi.tableWidget_LC.setItem(0,0,item)
                item = QtGui.QTableWidgetItem(str(res[t]))
                self.formUi.tableWidget_LC.setItem(0,1,item)
                item = QtGui.QTableWidgetItem(str(t))
                self.formUi.tableWidget_LC.setItem(0,2,item)
                item = QtGui.QCheckBox()
                item.setChecked(2)
                QtCore.QObject.connect(item, QtCore.SIGNAL("stateChanged(int)"), self.checkChangedL)
                self.formUi.tableWidget_LC.setCellWidget(0,3,item)
                
            self.formUi.tableWidget_LC.resizeColumnsToContents()

            self.updateFitL()
            self.updatePlot()
            
        def add_fit_dataLTC(self):
            l_filename = QtGui.QFileDialog.getOpenFileName(None, 'Open file','','LT file (*.csv)')
            
            if l_filename == "": return 
            
            res = self.readCSV(l_filename)
            
            self.formUi.tableWidget_LTC.setRowCount(0)
            
            for t in sorted(res.keys()):
                #print row[0],row[1],row[2]
                self.formUi.tableWidget_LTC.insertRow(0)
                item = QtGui.QTableWidgetItem('LTC')
                self.formUi.tableWidget_LTC.setItem(0,0,item)
                item = QtGui.QTableWidgetItem(str(res[t]))
                self.formUi.tableWidget_LTC.setItem(0,1,item)
                item = QtGui.QTableWidgetItem(str(t))
                self.formUi.tableWidget_LTC.setItem(0,2,item)
                item = QtGui.QCheckBox()
                item.setChecked(2)
                QtCore.QObject.connect(item, QtCore.SIGNAL("stateChanged(int)"), self.checkChangedLT)
                self.formUi.tableWidget_LTC.setCellWidget(0,3,item)
            self.formUi.tableWidget_LTC.resizeColumnsToContents()    
            self.updateFitLT()
            self.updatePlot()
            
    FreeCADGui.addCommand('MachDist_MaterialFit',_CommandMaterialFit())
