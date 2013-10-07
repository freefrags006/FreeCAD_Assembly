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

import FreeCAD, Fem, Plot
from numpy import * 
import matplotlib.pyplot as plt
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
            'PartDist_platethickness'   :'40.0',
            'PartDist_lc1'              :'0.0',
            'PartDist_lc2'              :'0.0',
            'PartDist_lc3'              :'0.0',
            'PartDist_lc4'              :'0.0',
            'PartDist_lc5'              :'0.0',
            'PartDist_lc6'              :'0.0',
            'PartDist_ltc1'             :'0.0',
            'PartDist_ltc2'             :'0.0',
            'PartDist_ltc3'             :'0.0',
            'PartDist_ltc4'             :'0.0',
            'PartDist_ltc5'             :'0.0',
            'PartDist_ltc6'             :'0.0'
            }
            

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
        return {'Pixmap'  : 'MachDist_MaterialFit',
                'MenuText': QtCore.QT_TRANSLATE_NOOP("MachDist_Material","Material"),
                'Accel': "A, X",
                'ToolTip': QtCore.QT_TRANSLATE_NOOP("MachDist_Material","Creates or edit the material definition.")}
        
    def Activated(self):
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
        taskd.update()
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


        QtCore.QObject.connect(self.formUi.select_file, QtCore.SIGNAL("clicked()"), self.add_fit_data)
        QtCore.QObject.connect(self.formUi.pushButton_SaveMat, QtCore.SIGNAL("clicked()"), self.saveMat)
        QtCore.QObject.connect(self.formUi.toolButton_chooseDir, QtCore.SIGNAL("clicked()"), self.chooseDir)
        QtCore.QObject.connect(self.formUi.comboBox_MaterialsInDir, QtCore.SIGNAL("currentIndexChanged(int)"), self.chooseMat)
        
        self.update()
        
    def transferTo(self):
        "Transfer from the dialog to the object" 
        
        matmap = self.obj.Material

        matmap['FEM_youngsmodulus']       = str(self.formUi.spinBox_young_modulus.value())
        matmap['PartDist_poissonratio']   = str(self.formUi.spinBox_poisson_ratio.value())
        matmap['PartDist_platethickness'] = str(self.formUi.spinBox_Plate_Thickness.value())


        #matmap['PartDist_lc1'] = str(self.formUi.lc1.value())
        self.obj.Material = matmap 

    
    def transferFrom(self):
        "Transfer from the object to the dialog"
        matmap = self.obj.Material

        self.formUi.spinBox_young_modulus.setValue(float(matmap['FEM_youngsmodulus']))
        self.formUi.spinBox_poisson_ratio.setValue(float(matmap['PartDist_poissonratio']))
        self.formUi.spinBox_Plate_Thickness.setValue(float(matmap['PartDist_platethickness']))


        #self.formUi.lc1.setValue(float(matmap['PartDist_lc1']))

    def isAllowedAlterSelection(self):
        return False

    def isAllowedAlterView(self):
        return True

    def getStandardButtons(self):
        return int(QtGui.QDialogButtonBox.Ok) | int(QtGui.QDialogButtonBox.Cancel)
    
    def update(self):
        'fills the widgets'
        self.transferFrom()
        self.fillMaterialCombo()


        return 
                
    def accept(self):
        self.transferTo()
        FreeCADGui.ActiveDocument.resetEdit()
                    
    def reject(self):
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
        if index == 0:return 
        import Material
        name = self.pathList[index-1]
        #print 'Import ', str(name)
        
        self.obj.Material = Material.importFCMat(str(name))
        #print self.obj.Material
        
        self.transferFrom()
        
    def fillMaterialCombo(self):
        import glob,os
        dirname = self.params.GetString("MaterialDir",'/')
        self.pathList = glob.glob(dirname + '/*.FCMat')
        self.formUi.comboBox_MaterialsInDir.clear()
        self.formUi.comboBox_MaterialsInDir.addItem('-> choose Material')
        for i in self.pathList:
            self.formUi.comboBox_MaterialsInDir.addItem(os.path.basename(i) )
        
    def add_fit_data(self):
        l_filename = QtGui.QFileDialog.getOpenFileName(None, 'Open file','','LT file (*.csv)')
        
        if l_filename == "": return 
        
        x = []
        y = []

        inputcsvfile = open(l_filename, 'r')
        r = csv.reader(inputcsvfile)
        
        for row in r:
            if row[0] == '1' or row[0] == '2':
                #print row[0],row[1],row[2]
                self.formUi.tableWidget.insertRow(0)
                item = QtGui.QTableWidgetItem(row[0])
                self.formUi.tableWidget.setItem(0,0,item)
                item = QtGui.QTableWidgetItem(row[1])
                self.formUi.tableWidget.setItem(0,1,item)
                item = QtGui.QTableWidgetItem(row[2])
                self.formUi.tableWidget.setItem(0,2,item)
            if row[0] == '1' :
                xval = float(row[1])
                yval = float(row[2])
                #print xval,yval
                x.append(xval)
                y.append(yval)

        print x,y
        
        polval = polyfit(x, y, 6)


        xmin = min(x)
        xmax = max(x)
        xdist = xmax - xmin
        ymin = min(y)
        ymax = max(y)
        ydist = ymax - ymin

        newx = linspace(xmin, xmax, 100)

        newy = []
        for i in newx:
            newy.append(i**6*polval[0]+i**5*polval[1]+i**4*polval[2]+i**3*polval[3]+i**2*polval[4]+i*polval[5]+polval[6])

        Plot.figure("Fit diagram")
        Plot.plot(newx,newy)
        
FreeCADGui.addCommand('MachDist_MaterialFit',_CommandMaterialFit())
