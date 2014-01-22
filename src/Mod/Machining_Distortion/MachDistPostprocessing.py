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

import FreeCAD, Fem, CalculixLib ,MachDistMoveTools, MachDistPlot
import os,sys,string,math,shutil,glob,subprocess,tempfile,re

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
        return {'Pixmap'  : 'Fem_Result',
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

        # regex to extract the three rotations and on Z translation out of the file name
        pars = re.compile('Case-x_rot([-+]?\d*\.?\d+)_y_rot([-+]?\d*\.?\d+)_z_rot([-+]?\d*\.?\d+)_z_l([-+]?\d*\.?\d+).*',re.IGNORECASE)
        
        CmaxL = 0.0
        CminL = 100000.0
        CmaxX = 0.0
        CminX = 100000.0
        CmaxY = 0.0
        CminY = 100000.0
        CmaxZ = 0.0
        CminZ = 100000.0
       
        RotXList = []
        RotYList = []
        RotZList = []
        TransZList = []

        MaxList = []
        MaxXList = []
        MaxYList = []
        MaxZList = []
        
        for filename in ResFileList:
            print filename
            m = CalculixLib.readResult(filename);
            MeshObject = None
            if m.has_key('Displacement'): 
                disp =  m['Displacement']
                ResultName = os.path.splitext(os.path.basename(filename))[0]
                
                RotX = 0.0
                RotY = 0.0
                RotZ = 0.0
                TransZ = 0.0
                m = pars.match(ResultName)
                
                if m:
                    g = m.groups()
                    RotX = float(g[0])
                    RotY = float(g[1])
                    RotZ = float(g[2])
                    TransZ = float(g[3])
                else:
                    print "Could not match: ",ResultName
                RotXList.append(RotX)
                RotYList.append(RotY)
                RotZList.append(RotZ)
                TransZList.append(TransZ)
    
                print "Case values: ", RotX,RotY,RotZ,TransZ
                
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
                
                MaxList.append(maxL)
                MaxXList.append(maxX)
                MaxYList.append(maxY)
                MaxZList.append(maxZ)

                
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
        
        StatObject.MaxList = MaxList
        StatObject.MaxXList = MaxXList
        StatObject.MaxYList = MaxYList
        StatObject.MaxZList = MaxZList

        StatObject.RotXList = RotXList
        StatObject.RotYList = RotYList
        StatObject.RotZList = RotZList
        StatObject.TransZList = TransZList
        
        FreeCAD.activeDocument().recompute()
       
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
        return "Fem_Result"


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
            
        taskd = _ResultControlTaskPanel(self.Object)
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
        
        obj.addProperty("App::PropertyFloatList","RotXList","Statistics","List of the X-Rotations")
        obj.addProperty("App::PropertyFloatList","RotYList","Statistics","List of the Y-Rotations")
        obj.addProperty("App::PropertyFloatList","RotZList","Statistics","List of the Z-Rotations")
        obj.addProperty("App::PropertyFloatList","TransZList","Statistics","List of the Z-Translation")
        
        obj.addProperty("App::PropertyFloatList","MaxList","Statistics","List of the maximum discplacment value")
        obj.addProperty("App::PropertyFloatList","MaxXList","Statistics","List of the maximum discplacment value in X")
        obj.addProperty("App::PropertyFloatList","MaxYList","Statistics","List of the maximum discplacment value in Y")
        obj.addProperty("App::PropertyFloatList","MaxZList","Statistics","List of the maximum discplacment value in Z")
       
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
            
        taskd = _StatisticTaskPanel(self.Object)
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
        form_class, base_class = uic.loadUiType(FreeCAD.getHomePath() + "Mod/Machining_Distortion/ShowDisplacement.ui")

        self.obj = object
        self.formUi = form_class()
        self.form = QtGui.QWidget()
        self.formUi.setupUi(self.form)

        #Connect Signals and Slots
        QtCore.QObject.connect(self.formUi.radioButton_Displacement, QtCore.SIGNAL("clicked(bool)"), self.displacementClicked)
        QtCore.QObject.connect(self.formUi.radioButton_NoColor, QtCore.SIGNAL("clicked(bool)"), self.noColorClicked)
        QtCore.QObject.connect(self.formUi.checkBox_ShowDisplacement, QtCore.SIGNAL("clicked(bool)"), self.showDisplacementClicked)

        QtCore.QObject.connect(self.formUi.verticalScrollBar_Factor, QtCore.SIGNAL("valueChanged(int)"), self.sliderValue)

        QtCore.QObject.connect(self.formUi.spinBox_SliderFactor, QtCore.SIGNAL("valueChanged(double)"), self.sliderMaxValue)
        QtCore.QObject.connect(self.formUi.spinBox_DisplacementFactor, QtCore.SIGNAL("valueChanged(double)"), self.displacementFactorValue)

        self.update()

    def getStandardButtons(self):
        return int(QtGui.QDialogButtonBox.Close)
        
    def displacementClicked(self,bool):
        QtGui.qApp.setOverrideCursor(QtCore.Qt.WaitCursor)
        self.setColorDisplacement()
        QtGui.qApp.restoreOverrideCursor()
                
    def noColorClicked(self,bool):
        self.MeshObject.ViewObject.NodeColor = {}
        
    def showDisplacementClicked(self,bool):
        QtGui.qApp.setOverrideCursor(QtCore.Qt.WaitCursor)
        print bool
        self.setDisplacement()
        QtGui.qApp.restoreOverrideCursor()
    
    def sliderValue(self,value):
        if(self.formUi.checkBox_ShowDisplacement.isChecked()):
            self.MeshObject.ViewObject.animate(value)
        
        self.formUi.spinBox_DisplacementFactor.setValue(value)

    def sliderMaxValue(self,value):
        print 'sliderMaxValue()',value
        self.formUi.verticalScrollBar_Factor.setMaxValue(int(value))
        
    def displacementFactorValue(self,value):
        print 'displacementFactorValue()'
        self.formUi.verticalScrollBar_Factor.setValue(value)
        
    def setColorDisplacement(self):
        if self.obj:
            values = self.obj.Values
            maxL = self.obj.max
            minL = self.obj.min
            
            self.formUi.lineEdit_Max.setText(str(maxL))
            self.formUi.lineEdit_Min.setText(str(minL))
            self.formUi.doubleSpinBox_MinValueColor.setValue(maxL)
            
            self.MeshObject.ViewObject.setNodeColorByResult(self.obj)
            
    def setDisplacement(self):
        print 'setDisplacement()', self.formUi.checkBox_ShowDisplacement.isChecked()
        if self.formUi.checkBox_ShowDisplacement.isChecked():
            self.MeshObject.ViewObject.setNodeDisplacementByResult(self.obj)
            self.MeshObject.ViewObject.animate(self.formUi.verticalScrollBar_Factor.value())
        else:
            self.MeshObject.ViewObject.animate(0)

    def update(self):
        'fills the widgets'

        self.MeshObject = None
        if FemGui.getActiveAnalysis():
            for i in FemGui.getActiveAnalysis().Member:
                if i.isDerivedFrom("Fem::FemMeshObject"):
                    self.MeshObject = i

        self.setColorDisplacement()
        self.setDisplacement()
        
    def accept(self):
        self.noColorClicked(False)
        FreeCADGui.Control.closeDialog()
        
                    
    def reject(self):
        self.noColorClicked(False)
        self.MeshObject.ViewObject.animate(0)
        FreeCADGui.Control.closeDialog()
    
class _StatisticTaskPanel:
    '''The control for the displacement post-processing'''
    def __init__(self,object):
        # the panel has a tree widget that contains categories
        # for the subcomponents, such as additions, subtractions.
        # the categories are shown only if they are not empty.
        form_class, base_class = uic.loadUiType(FreeCAD.getHomePath() + "Mod/Machining_Distortion/Statistic.ui")

        self.obj = object
        self.formUi = form_class()
        self.form = QtGui.QWidget()
        self.formUi.setupUi(self.form)

        #Connect Signals and Slots
        #QtCore.QObject.connect(self.formUi.radioButton_Displacement, QtCore.SIGNAL("clicked(bool)"), self.displacementClicked)
        #QtCore.QObject.connect(self.formUi.radioButton_NoColor, QtCore.SIGNAL("clicked(bool)"), self.noColorClicked)
        #QtCore.QObject.connect(self.formUi.checkBox_ShowDisplacement, QtCore.SIGNAL("clicked(bool)"), self.showDisplacementClicked)

        #QtCore.QObject.connect(self.formUi.verticalScrollBar_Factor, QtCore.SIGNAL("valueChanged(int)"), self.sliderValue)

        #QtCore.QObject.connect(self.formUi.spinBox_SliderFactor, QtCore.SIGNAL("valueChanged(double)"), self.sliderMaxValue)
        #QtCore.QObject.connect(self.formUi.spinBox_DisplacementFactor, QtCore.SIGNAL("valueChanged(double)"), self.displacementFactorValue)

        self.update()

    def getStandardButtons(self):
        return int(QtGui.QDialogButtonBox.Close)
        

    def update(self):
        'fills the widgets'
        print "update(self) ToDo" 
        
    def accept(self):
        FreeCADGui.Control.closeDialog()
        
                    
    def reject(self):
        FreeCADGui.Control.closeDialog()
        
    
FreeCADGui.addCommand('MachDist_ReadResult',_CommandReadResults())
