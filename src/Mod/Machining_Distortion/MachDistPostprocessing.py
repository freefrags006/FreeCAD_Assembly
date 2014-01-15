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

import FreeCAD, Fem, CalculixLib ,MachDistMoveTools
import os,sys,string,math,shutil,glob,subprocess,tempfile

from ApplyingBC_IC  import ApplyingBC_IC

if FreeCAD.GuiUp:
    import FreeCADGui,FemGui
    from FreeCAD import Vector
    from PyQt4 import QtCore, QtGui
    from pivy import coin
    import PyQt4.uic as uic

__title__="Machine-Distortion Postprocessing"
__author__ = "Juergen Riegel"
__url__ = "http://free-cad.sourceforge.net"


def makeMachDistDisplacement(name):
    '''makeMachDistAnalysis(name): makes a MachDist Displacement object'''
    obj = FreeCAD.ActiveDocument.addObject("Fem::FemResultVectorPython",name)
    _MachDistDisplacement(obj)
    _ViewProviderMachDistDisplacement(obj.ViewObject)
    return obj
    
def makeMachDistResultStat(name):
    '''makeMachDistResultStat(name): makes a MachDist ResultStat object'''
    obj = FreeCAD.ActiveDocument.addObject("App::FeaturePython",name)
    _MachDistResultStat(obj)
    _ViewProviderMachDistResultStat(obj.ViewObject)
    return obj
    


class _CommandReadResults:
    "the MachDist JobControl command definition"
    def GetResources(self):
        return {'Pixmap'  : 'MachDist_Download',
                'MenuText': QtCore.QT_TRANSLATE_NOOP("MachDist_ReadResults","Read results"),
                'Accel': "A",
                'ToolTip': QtCore.QT_TRANSLATE_NOOP("MachDist_ReadResults","Dialog to generate the jobs")}
        
    def Activated(self):
        JobDir = FemGui.getActiveAnalysis().OutputDir
        print JobDir
        
        ResFileList = glob.glob(JobDir + '/*.frd')
        
        if not len(ResFileList) == FemGui.getActiveAnalysis().OutputCount:
            QtGui.QMessageBox.critical(None, "Result error","Count of input and output files differs - Calculation not finished?")
            return
        
        StatObject = makeMachDistResultStat('ResultStatistic')
        StatObject.Label = 'ResultStatistic'
        FemGui.getActiveAnalysis().Member = FemGui.getActiveAnalysis().Member + [StatObject]

        CmaxL = 0.0
        CminL = 100000.0
        CmaxX = 0.0
        CminX = 100000.0
        CmaxY = 0.0
        CminY = 100000.0
        CmaxZ = 0.0
        CminZ = 100000.0
       
        for filename in ResFileList:
            print filename
            m = CalculixLib.readResult(filename);
            MeshObject = None
            if m.has_key('Displacement'): 
                disp =  m['Displacement']
                ResultName = os.path.splitext(os.path.basename(filename))[0]
                ResultObject = makeMachDistDisplacement('Displacement')
                ResultObject.Label = ResultName
                ResultObject.Values = disp.values()
                ResultObject.DataType = 'Displacement'
                ResultObject.ElementNumbers = disp.keys()
                FemGui.getActiveAnalysis().Member = FemGui.getActiveAnalysis().Member + [ResultObject]
                maxL = 0.0
                minL = 100000.0
                maxX = 0.0
                minX = 100000.0
                maxY = 0.0
                minY = 100000.0
                maxZ = 0.0
                minZ = 100000.0
                
                for i in disp.values():
                    if i.Length > maxL:
                        maxL = i.Length
                    if i.Length < minL:
                        minL = i.Length
                    if i.x > maxX:
                        maxX = i.x
                    if i.x < minX:
                        minX = i.x
                    if i.y > maxY:
                        maxY = i.y
                    if i.y < minY:
                        minY = i.y
                    if i.z > maxZ:
                        maxX = i.z
                    if i.z < minZ:
                        minZ = i.z
                
                ResultObject.max  = maxL
                ResultObject.min  = minL
                ResultObject.maxX = maxX
                ResultObject.minX = minX
                ResultObject.maxY = maxY
                ResultObject.minY = minY
                ResultObject.maxZ = maxZ
                ResultObject.minZ = minZ
                
                if maxL > CmaxL:
                    CmaxL = maxL
                if minL < CminL:
                    CminL = minL
                if maxX > CmaxX:
                    CmaxX = maxX
                if minX < CminX:
                    CminX = minX
                if maxY > CmaxY:
                    CmaxY = maxY
                if minY < CminY:
                    CminY = minY
                if maxZ > CmaxZ:
                    CmaxX = maxZ
                if minZ < CminZ:
                    CminZ = minZ
            else:
                QtGui.QMessageBox.critical(None, "Result error","Output file holds no Displacement values!")

        StatObject.max  = CmaxL
        StatObject.min  = CminL
        StatObject.maxX = CmaxX
        StatObject.minX = CminX
        StatObject.maxY = CmaxY
        StatObject.minY = CminY
        StatObject.maxZ = CmaxZ
        StatObject.minZ = CminZ
        
       
    def IsActive(self):
        import FemGui
        return FreeCADGui.ActiveDocument != None and FemGui.getActiveAnalysis() != None

# ==== Displacement result object ===========================================

class _MachDistDisplacement:
    "The Material object"
    def __init__(self,obj):
        self.Type = "MachDistDisplacement"
        obj.addProperty("App::PropertyFloat","max","Statistics","max displacement")
        obj.addProperty("App::PropertyFloat","min","Statistics","min displacement")
        obj.addProperty("App::PropertyFloat","maxX","Statistics","max displacement in X")
        obj.addProperty("App::PropertyFloat","minX","Statistics","min displacement in X")
        obj.addProperty("App::PropertyFloat","maxY","Statistics","max displacement in Y")
        obj.addProperty("App::PropertyFloat","minY","Statistics","min displacement in Y")
        obj.addProperty("App::PropertyFloat","maxZ","Statistics","max displacement in Z")
        obj.addProperty("App::PropertyFloat","minZ","Statistics","min displacement in Z")

        obj.Proxy = self

        
    def execute(self,obj):
        return
        
        
class _ViewProviderMachDistDisplacement:
    "A View Provider for the Material object"

    def __init__(self,vobj):
        vobj.Proxy = self
       
    def getIcon(self):
        import machdist_rc
        return ":/icons/MachDist_NewAnalysis.svg"


    def attach(self, vobj):
        self.ViewObject = vobj
        self.Object = vobj.Object

  
    def doubleClicked(self,vobj):
        import FemGui
        if FemGui.getActiveAnalysis() == None:
            if FreeCADGui.activeWorkbench().name() != 'MachiningDistortionWorkbench':
                FreeCADGui.activateWorkbench("MachiningDistortionWorkbench")
            FemGui.setActiveAnalysis(self.Object)
            return True
            
        taskd = _ResultControlTaskPanel()
        taskd.obj = vobj.Object
        taskd.update()
        FreeCADGui.Control.showDialog(taskd)
        return True
        

    def __getstate__(self):
        return None

    def __setstate__(self,state):
        return None

# ==== Result statistics object ===========================================
        
class _MachDistResultStat:
    "The Material object"
    def __init__(self,obj):
        self.Type = "MachDistAnalysis"
        obj.Proxy = self
        #obj.Material = StartMat
        obj.addProperty("App::PropertyInteger","ResultCount","Statistics","Number of read results")
        obj.addProperty("App::PropertyFloat","max","Statistics","max displacement")
        obj.addProperty("App::PropertyFloat","min","Statistics","min displacement")
        obj.addProperty("App::PropertyFloat","maxX","Statistics","max displacement in X")
        obj.addProperty("App::PropertyFloat","minX","Statistics","min displacement in X")
        obj.addProperty("App::PropertyFloat","maxY","Statistics","max displacement in Y")
        obj.addProperty("App::PropertyFloat","minY","Statistics","min displacement in Y")
        obj.addProperty("App::PropertyFloat","maxZ","Statistics","max displacement in Z")
        obj.addProperty("App::PropertyFloat","minZ","Statistics","min displacement in Z")

        
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
        
class _ViewProviderMachDistResultStat:
    "A View Provider for the Material object"

    def __init__(self,vobj):
        #vobj.addProperty("App::PropertyLength","BubbleSize","Base", str(translate("MachDist","The size of the axis bubbles")))
        vobj.Proxy = self
       
    def getIcon(self):
        import machdist_rc
        return ":/icons/MachDist_NewAnalysis.svg"


    def attach(self, vobj):
        self.ViewObject = vobj
        self.Object = vobj.Object
        self.bubbles = None


    def updateData(self, obj, prop):
        return

    def onChanged(self, vobj, prop):
        return
  
    def doubleClicked(self,vobj):
        import FemGui
        if FemGui.getActiveAnalysis() == None:
            if FreeCADGui.activeWorkbench().name() != 'MachiningDistortionWorkbench':
                FreeCADGui.activateWorkbench("MachiningDistortionWorkbench")
            FemGui.setActiveAnalysis(self.Object)
            return True
            
        taskd = _JobControlTaskPanel()
        taskd.obj = vobj.Object
        taskd.update()
        FreeCADGui.Control.showDialog(taskd)
        return True
        

    def __getstate__(self):
        return None

    def __setstate__(self,state):
        return None

        
    
    
class _ResultControlTaskPanel:
    '''The control for the displacement post-processing'''
    def __init__(self,object):
        # the panel has a tree widget that contains categories
        # for the subcomponents, such as additions, subtractions.
        # the categories are shown only if they are not empty.
        form_class, base_class = uic.loadUiType(FreeCAD.getHomePath() + "Mod/Fem/ShowDisplacement.ui")

        self.obj = object
        self.formUi = form_class()
        self.form = QtGui.QWidget()
        self.formUi.setupUi(self.form)

        #Connect Signals and Slots
        QtCore.QObject.connect(self.formUi.radioButton_Displacement, QtCore.SIGNAL("clicked(bool)"), self.displacementClicked)
        QtCore.QObject.connect(self.formUi.radioButton_Stress, QtCore.SIGNAL("clicked(bool)"), self.stressClicked)
        QtCore.QObject.connect(self.formUi.radioButton_NoColor, QtCore.SIGNAL("clicked(bool)"), self.noColorClicked)
        QtCore.QObject.connect(self.formUi.checkBox_ShowDisplacement, QtCore.SIGNAL("clicked(bool)"), self.showDisplacementClicked)

        QtCore.QObject.connect(self.formUi.verticalScrollBar_Factor, QtCore.SIGNAL("valueChanged(int)"), self.sliderValue)

        QtCore.QObject.connect(self.formUi.spinBox_SliderFactor, QtCore.SIGNAL("valueChanged(double)"), self.sliderMaxValue)
        QtCore.QObject.connect(self.formUi.spinBox_DisplacementFactor, QtCore.SIGNAL("valueChanged(double)"), self.displacementFactorValue)

        self.DisplacementObject = None
        self.StressObject = None

        self.update()
        


    def getStandardButtons(self):
        return int(QtGui.QDialogButtonBox.Close)
        
    def displacementClicked(self,bool):
        QtGui.qApp.setOverrideCursor(QtCore.Qt.WaitCursor)
        self.setColorDisplacement()
        QtGui.qApp.restoreOverrideCursor()
        
    def stressClicked(self,bool):
        print 'stressClicked()'
        QtGui.qApp.setOverrideCursor(QtCore.Qt.WaitCursor)
        self.setColorStress()
        QtGui.qApp.restoreOverrideCursor()
        
    def noColorClicked(self,bool):
        self.MeshObject.ViewObject.NodeColor = {}
        self.MeshObject.ViewObject.ElementColor = {}
        
    def showDisplacementClicked(self,bool):
        QtGui.qApp.setOverrideCursor(QtCore.Qt.WaitCursor)
        self.setDisplacement()
        QtGui.qApp.restoreOverrideCursor()
    
    def sliderValue(self,value):
        if(self.formUi.checkBox_ShowDisplacement.isChecked()):
            self.MeshObject.ViewObject.animate(value)
        
        self.formUi.spinBox_DisplacementFactor.setValue(value)

    def sliderMaxValue(self,value):
        print 'sliderMaxValue()'
        self.formUi.verticalScrollBar_Factor.setMaximum(value)
        
    def displacementFactorValue(self,value):
        print 'displacementFactorValue()'
        self.formUi.verticalScrollBar_Factor.setValue(value)
        
    def setColorDisplacement(self):
        if self.DisplacementObject:
            values = self.DisplacementObject.Values
            maxL = 0.0
            for i in values:
                if i.Length > maxL:
                    maxL = i.Length
            
            self.formUi.lineEdit_Max.setText(str(maxL))
            self.formUi.doubleSpinBox_MinValueColor.setValue(maxL)
            
            self.MeshObject.ViewObject.setNodeColorByResult(self.DisplacementObject)
            
    def setDisplacement(self):
        if self.DisplacementObject:
            self.MeshObject.ViewObject.setNodeDisplacementByResult(self.DisplacementObject)   
    
    def setColorStress(self):
        if self.StressObject:
            values = self.StressObject.Values
            maxVal = max(values)
            self.formUi.doubleSpinBox_MinValueColor.setValue(maxVal)
            
            self.MeshObject.ViewObject.setNodeColorByResult(self.StressObject)

    def update(self):
        'fills the widgets'

        self.MeshObject = None
        if FemGui.getActiveAnalysis():
            for i in FemGui.getActiveAnalysis().Member:
                if i.isDerivedFrom("Fem::FemMeshObject"):
                    self.MeshObject = i

        for i in FemGui.getActiveAnalysis().Member:
            if i.isDerivedFrom("Fem::FemResultVector"):
                if i.DataType == 'Displacement':
                    self.DisplacementObject = i
        for i in FemGui.getActiveAnalysis().Member:
            if i.isDerivedFrom("Fem::FemResultValue"):
                if i.DataType == 'VanMisesStress':
                    self.StressObject = i
                
    def accept(self):
        FreeCADGui.Control.closeDialog()
        
                    
    def reject(self):
        FreeCADGui.Control.closeDialog()
    
    
    
FreeCADGui.addCommand('MachDist_ReadResult',_CommandReadResults())
