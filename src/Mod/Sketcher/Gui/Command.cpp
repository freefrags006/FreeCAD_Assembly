/***************************************************************************
 *   Copyright (c) 2008 J�rgen Riegel (juergen.riegel@web.de)              *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/


#include "PreCompiled.h"
#ifndef _PreComp_
# include <TopoDS_Shape.hxx>
# include <TopoDS_Face.hxx>
# include <TopoDS.hxx>
# include <BRepAdaptor_Surface.hxx>
# include <QApplication>
# include <QInputDialog>
# include <QMessageBox>
#endif

#include <App/DocumentObjectGroup.h>
#include <App/Plane.h>
#include <Gui/Application.h>
#include <Gui/Document.h>
#include <Gui/Command.h>
#include <Gui/Control.h>
#include <Gui/MainWindow.h>
#include <Gui/DlgEditFileIncludeProptertyExternal.h>
#include <Gui/SelectionFilter.h>

#include <Mod/Sketcher/App/SketchObjectSF.h>
#include <Mod/Sketcher/App/SketchObject.h>
#include <Mod/Part/App/Attacher.h>
#include <Mod/Part/App/Part2DObject.h>

#include "SketchOrientationDialog.h"
#include "ViewProviderSketch.h"
#include "TaskSketcherValidation.h"
#include "../App/Constraint.h"

using namespace std;
using namespace SketcherGui;
using namespace Part;


namespace SketcherGui {

    class ExceptionWrongInput : public Base::Exception {
    public:
        ExceptionWrongInput()
            : ErrMsg(QString())
        {

        }

        //Pass untranslated strings, enclosed in QT_TR_NOOP()
        ExceptionWrongInput(const char* ErrMsg){
            this->ErrMsg = QObject::tr( ErrMsg );
            this->setMessage(ErrMsg);
        }

        QString ErrMsg;
    };


    Attacher::eMapMode SuggestAutoMapMode(Attacher::eSuggestResult* pMsgId = 0,
                                      QString* message = 0,
                                      std::vector<Attacher::eMapMode>* allmodes = 0){
        //convert pointers into valid references, to avoid checking for null pointers everywhere
        Attacher::eSuggestResult buf;
        if (pMsgId == 0)
            pMsgId = &buf;
        Attacher::eSuggestResult &msg = *pMsgId;
        QString buf2;
        if (message == 0)
            message = &buf2;
        QString &msg_str = *message;

        App::PropertyLinkSubList tmpSupport;
        Gui::Selection().getAsPropertyLinkSubList(tmpSupport);

        AttachEngine3D eng;
        eng.setUp(tmpSupport);
        Attacher::eMapMode ret;
        ret = eng.listMapModes(msg, allmodes);
        switch(msg){
            case Attacher::srOK:
            break;
            case Attacher::srNoModesFit:
                msg_str = QObject::tr("There are no modes that accept the selected set of subelements");
            break;
            case Attacher::srLinkBroken:
                msg_str = QObject::tr("Broken link to support subelements");
            break;
            case Attacher::srUnexpectedError:
                msg_str = QObject::tr("Unexpected error");
            break;
            case Attacher::srIncompatibleGeometry:
                if(tmpSupport.getSubValues()[0].substr(0,4) == std::string("Face"))
                    msg_str = QObject::tr("Face is non-planar");
                else
                    msg_str = QObject::tr("Selected shapes are of wrong form (e.g., a curved edge where a straight one is needed)");
            break;
            default:
                msg_str = QObject::tr("Unexpected error");
                assert(0/*no message for eSuggestResult enum item*/);
        }

        return ret;
    }
} //namespace SketcherGui


/* Sketch commands =======================================================*/
DEF_STD_CMD_A(CmdSketcherNewSketch);

CmdSketcherNewSketch::CmdSketcherNewSketch()
    :Command("Sketcher_NewSketch")
{
    sAppModule      = "Sketcher";
    sGroup          = QT_TR_NOOP("Sketcher");
    sMenuText       = QT_TR_NOOP("Create sketch");
    sToolTipText    = QT_TR_NOOP("Create a new sketch");
    sWhatsThis      = "Sketcher_NewSketch";
    sStatusTip      = sToolTipText;
    sPixmap         = "Sketcher_NewSketch";
}

void CmdSketcherNewSketch::activated(int iMsg)
{
    Attacher::eMapMode mapmode = Attacher::mmDeactivated;
    bool bAttach = false;
    if (Gui::Selection().hasSelection()){
        Attacher::eSuggestResult msgid = Attacher::srOK;
        QString msg_str;
        std::vector<Attacher::eMapMode> validModes;
        mapmode = SuggestAutoMapMode(&msgid, &msg_str, &validModes);
        if (msgid == Attacher::srOK)
            bAttach = true;
        if (msgid != Attacher::srOK && msgid != Attacher::srNoModesFit){
            QMessageBox::warning(Gui::getMainWindow(),
                QObject::tr("Sketch mapping"),
                QObject::tr("Can't map the skecth to selected object. %1.").arg(msg_str));
            return;
        }
        if (validModes.size() > 1){
            validModes.insert(validModes.begin(), Attacher::mmDeactivated);
            bool ok;
            QStringList items;
            items.push_back(QObject::tr("Don't attach"));
            int iSugg = 0;//index of the auto-suggested mode in the list of valid modes
            for (int i = 0  ;  i < validModes.size()  ;  ++i){
                items.push_back(QString::fromLatin1(AttachEngine::eMapModeStrings[validModes[i]]));
                if (validModes[i] == mapmode)
                    iSugg = items.size()-1;
            }
            QString text = QInputDialog::getItem(Gui::getMainWindow(),
                qApp->translate(className(), "Sketch attachment"),
                qApp->translate(className(), "Select the method to attach this sketch to selected object"),
                items, iSugg, false, &ok);
            if (!ok) return;
            int index = items.indexOf(text);
            if (index == 0){
                bAttach = false;
                mapmode = Attacher::mmDeactivated;
            } else {
                bAttach = true;
                mapmode = validModes[index-1];
            }
        }
    }

    if (bAttach) {

        std::vector<Gui::SelectionObject> objects = Gui::Selection().getSelectionEx();
        //assert (objects.size() == 1); //should have been filtered out by SuggestAutoMapMode
        //Gui::SelectionObject &sel_support = objects[0];
        App::PropertyLinkSubList support;
        Gui::Selection().getAsPropertyLinkSubList(support);
        std::string supportString = support.getPyReprString();

        // create Sketch on Face
        std::string FeatName = getUniqueObjectName("Sketch");

        openCommand("Create a Sketch on Face");
        doCommand(Doc,"App.activeDocument().addObject('Sketcher::SketchObject','%s')",FeatName.c_str());
        if (mapmode >= 0 && mapmode < Attacher::mmDummy_NumberOfModes)
            doCommand(Gui,"App.activeDocument().%s.MapMode = \"%s\"",FeatName.c_str(),AttachEngine::eMapModeStrings[mapmode]);
        else
            assert(0 /* mapmode index out of range */);
        doCommand(Gui,"App.activeDocument().%s.Support = %s",FeatName.c_str(),supportString.c_str());
        doCommand(Gui,"App.activeDocument().recompute()");  // recompute the sketch placement based on its support
        doCommand(Gui,"Gui.activeDocument().setEdit('%s')",FeatName.c_str());

        Part::Feature *part = static_cast<Part::Feature*>(support.getValue());//if multi-part support, this will return 0
        if (part){
            App::DocumentObjectGroup* grp = part->getGroup();
            if (grp) {
                doCommand(Doc,"App.activeDocument().%s.addObject(App.activeDocument().%s)"
                             ,grp->getNameInDocument(),FeatName.c_str());
            }
        }
    }
    else {
        // ask user for orientation
        SketchOrientationDialog Dlg;

        if (Dlg.exec() != QDialog::Accepted)
            return; // canceled
        Base::Vector3d p = Dlg.Pos.getPosition();
        Base::Rotation r = Dlg.Pos.getRotation();

        // do the right view direction
        std::string camstring;
        switch(Dlg.DirType){
            case 0:
                camstring = "#Inventor V2.1 ascii \\n OrthographicCamera {\\n viewportMapping ADJUST_CAMERA \\n position 0 0 87 \\n orientation 0 0 1  0 \\n nearDistance -112.88701 \\n farDistance 287.28702 \\n aspectRatio 1 \\n focalDistance 87 \\n height 143.52005 }";
                break;
            case 1:
                camstring = "#Inventor V2.1 ascii \\n OrthographicCamera {\\n viewportMapping ADJUST_CAMERA \\n position 0 0 -87 \\n orientation -1 0 0  3.1415927 \\n nearDistance -112.88701 \\n farDistance 287.28702 \\n aspectRatio 1 \\n focalDistance 87 \\n height 143.52005 }";
                break;
            case 2:
                camstring = "#Inventor V2.1 ascii \\n OrthographicCamera {\\n viewportMapping ADJUST_CAMERA\\n  position 0 -87 0 \\n  orientation -1 0 0  4.712389\\n  nearDistance -112.88701\\n  farDistance 287.28702\\n  aspectRatio 1\\n  focalDistance 87\\n  height 143.52005\\n\\n}";
                break;
            case 3:
                camstring = "#Inventor V2.1 ascii \\n OrthographicCamera {\\n viewportMapping ADJUST_CAMERA\\n  position 0 87 0 \\n  orientation 0 0.70710683 0.70710683  3.1415927\\n  nearDistance -112.88701\\n  farDistance 287.28702\\n  aspectRatio 1\\n  focalDistance 87\\n  height 143.52005\\n\\n}";
                break;
            case 4:
                camstring = "#Inventor V2.1 ascii \\n OrthographicCamera {\\n viewportMapping ADJUST_CAMERA\\n  position 87 0 0 \\n  orientation 0.57735026 0.57735026 0.57735026  2.0943952 \\n  nearDistance -112.887\\n  farDistance 287.28699\\n  aspectRatio 1\\n  focalDistance 87\\n  height 143.52005\\n\\n}";
                break;
            case 5:
                camstring = "#Inventor V2.1 ascii \\n OrthographicCamera {\\n viewportMapping ADJUST_CAMERA\\n  position -87 0 0 \\n  orientation -0.57735026 0.57735026 0.57735026  4.1887903 \\n  nearDistance -112.887\\n  farDistance 287.28699\\n  aspectRatio 1\\n  focalDistance 87\\n  height 143.52005\\n\\n}";
                break;
        }
        std::string FeatName = getUniqueObjectName("Sketch");

        openCommand("Create a new Sketch");
        doCommand(Doc,"App.activeDocument().addObject('Sketcher::SketchObject','%s')",FeatName.c_str());
        doCommand(Doc,"App.activeDocument().%s.Placement = App.Placement(App.Vector(%f,%f,%f),App.Rotation(%f,%f,%f,%f))",FeatName.c_str(),p.x,p.y,p.z,r[0],r[1],r[2],r[3]);
        doCommand(Doc,"App.activeDocument().%s.MapMode = \"%s\"",FeatName.c_str(),AttachEngine::eMapModeStrings[int(Attacher::mmDeactivated)]);
        doCommand(Gui,"Gui.activeDocument().activeView().setCamera('%s')",camstring.c_str());
        doCommand(Gui,"Gui.activeDocument().setEdit('%s')",FeatName.c_str());
    }

}

bool CmdSketcherNewSketch::isActive(void)
{
    if (getActiveGuiDocument())
        return true;
    else
        return false;
}

DEF_STD_CMD_A(CmdSketcherEditSketch);

CmdSketcherEditSketch::CmdSketcherEditSketch()
    :Command("Sketcher_EditSketch")
{
    sAppModule      = "Sketcher";
    sGroup          = QT_TR_NOOP("Sketcher");
    sMenuText       = QT_TR_NOOP("Edit sketch");
    sToolTipText    = QT_TR_NOOP("Edit the selected sketch");
    sWhatsThis      = "Sketcher_EditSketch";
    sStatusTip      = sToolTipText;
    sPixmap         = "Sketcher_EditSketch";
}

void CmdSketcherEditSketch::activated(int iMsg)
{
    Gui::SelectionFilter SketchFilter("SELECT Sketcher::SketchObject COUNT 1");

    if (SketchFilter.match()) {
        Sketcher::SketchObject *Sketch = static_cast<Sketcher::SketchObject*>(SketchFilter.Result[0][0].getObject());
        openCommand("Edit Sketch");
        doCommand(Gui,"Gui.activeDocument().setEdit('%s')",Sketch->getNameInDocument());
    }
}

bool CmdSketcherEditSketch::isActive(void)
{
    return Gui::Selection().countObjectsOfType(Sketcher::SketchObject::getClassTypeId()) == 1;
}

DEF_STD_CMD_A(CmdSketcherLeaveSketch);

CmdSketcherLeaveSketch::CmdSketcherLeaveSketch()
  : Command("Sketcher_LeaveSketch")
{
    sAppModule      = "Sketcher";
    sGroup          = QT_TR_NOOP("Sketcher");
    sMenuText       = QT_TR_NOOP("Leave sketch");
    sToolTipText    = QT_TR_NOOP("Close the editing of the sketch");
    sWhatsThis      = "Sketcher_LeaveSketch";
    sStatusTip      = sToolTipText;
    sPixmap         = "Sketcher_LeaveSketch";
    eType           = 0;
}

void CmdSketcherLeaveSketch::activated(int iMsg)
{
    openCommand("Sketch changed");
    doCommand(Gui,"Gui.activeDocument().resetEdit()");
    doCommand(Doc,"App.ActiveDocument.recompute()");
    commitCommand();

}

bool CmdSketcherLeaveSketch::isActive(void)
{
    Gui::Document *doc = getActiveGuiDocument();
    if (doc) {
        // checks if a Sketch Viewprovider is in Edit and is in no special mode
        SketcherGui::ViewProviderSketch* vp = dynamic_cast<SketcherGui::ViewProviderSketch*>(doc->getInEdit());
        if (vp && vp->getSketchMode() == ViewProviderSketch::STATUS_NONE)
            return true;
    }
    return false;
}

DEF_STD_CMD_A(CmdSketcherReorientSketch);

CmdSketcherReorientSketch::CmdSketcherReorientSketch()
    :Command("Sketcher_ReorientSketch")
{
    sAppModule      = "Sketcher";
    sGroup          = QT_TR_NOOP("Sketcher");
    sMenuText       = QT_TR_NOOP("Reorient sketch...");
    sToolTipText    = QT_TR_NOOP("Reorient the selected sketch");
    sWhatsThis      = "Sketcher_ReorientSketch";
    sStatusTip      = sToolTipText;
}

void CmdSketcherReorientSketch::activated(int iMsg)
{
    Sketcher::SketchObject* sketch = Gui::Selection().getObjectsOfType<Sketcher::SketchObject>().front();
    if (sketch->Support.getValue()) {
        int ret = QMessageBox::question(Gui::getMainWindow(),
            qApp->translate("Sketcher_ReorientSketch","Sketch has support"),
            qApp->translate("Sketcher_ReorientSketch","Sketch with a support face cannot be reoriented.\n"
                                                      "Do you want to detach it from the support?"),
            QMessageBox::Yes|QMessageBox::No);
        if (ret == QMessageBox::No)
            return;
        sketch->Support.setValue(0);
    }

    // ask user for orientation
    SketchOrientationDialog Dlg;

    if (Dlg.exec() != QDialog::Accepted)
        return; // canceled
    Base::Vector3d p = Dlg.Pos.getPosition();
    Base::Rotation r = Dlg.Pos.getRotation();

    // do the right view direction
    std::string camstring;
    switch(Dlg.DirType){
        case 0:
            camstring = "#Inventor V2.1 ascii \\n OrthographicCamera {\\n viewportMapping ADJUST_CAMERA \\n "
                "position 0 0 87 \\n orientation 0 0 1  0 \\n nearDistance -112.88701 \\n farDistance 287.28702 \\n "
                "aspectRatio 1 \\n focalDistance 87 \\n height 143.52005 }";
            break;
        case 1:
            camstring = "#Inventor V2.1 ascii \\n OrthographicCamera {\\n viewportMapping ADJUST_CAMERA \\n "
                "position 0 0 -87 \\n orientation -1 0 0  3.1415927 \\n nearDistance -112.88701 \\n farDistance 287.28702 \\n "
                "aspectRatio 1 \\n focalDistance 87 \\n height 143.52005 }";
            break;
        case 2:
            camstring = "#Inventor V2.1 ascii \\n OrthographicCamera {\\n viewportMapping ADJUST_CAMERA\\n "
                "position 0 -87 0 \\n  orientation -1 0 0  4.712389\\n  nearDistance -112.88701\\n  farDistance 287.28702\\n "
                "aspectRatio 1\\n  focalDistance 87\\n  height 143.52005\\n\\n}";
            break;
        case 3:
            camstring = "#Inventor V2.1 ascii \\n OrthographicCamera {\\n viewportMapping ADJUST_CAMERA\\n "
                "position 0 87 0 \\n  orientation 0 0.70710683 0.70710683  3.1415927\\n  nearDistance -112.88701\\n  farDistance 287.28702\\n "
                "aspectRatio 1\\n  focalDistance 87\\n  height 143.52005\\n\\n}";
            break;
        case 4:
            camstring = "#Inventor V2.1 ascii \\n OrthographicCamera {\\n viewportMapping ADJUST_CAMERA\\n "
                "position 87 0 0 \\n  orientation 0.57735026 0.57735026 0.57735026  2.0943952 \\n  nearDistance -112.887\\n  farDistance 287.28699\\n "
                "aspectRatio 1\\n  focalDistance 87\\n  height 143.52005\\n\\n}";
            break;
        case 5:
            camstring = "#Inventor V2.1 ascii \\n OrthographicCamera {\\n viewportMapping ADJUST_CAMERA\\n "
                "position -87 0 0 \\n  orientation -0.57735026 0.57735026 0.57735026  4.1887903 \\n  nearDistance -112.887\\n  farDistance 287.28699\\n "
                "aspectRatio 1\\n  focalDistance 87\\n  height 143.52005\\n\\n}";
            break;
    }

    openCommand("Reorient Sketch");
    doCommand(Doc,"App.ActiveDocument.%s.Placement = App.Placement(App.Vector(%f,%f,%f),App.Rotation(%f,%f,%f,%f))"
                 ,sketch->getNameInDocument(),p.x,p.y,p.z,r[0],r[1],r[2],r[3]);
    doCommand(Gui,"Gui.ActiveDocument.setEdit('%s')",sketch->getNameInDocument());
}

bool CmdSketcherReorientSketch::isActive(void)
{
    return Gui::Selection().countObjectsOfType
        (Sketcher::SketchObject::getClassTypeId()) == 1;
}

DEF_STD_CMD_A(CmdSketcherMapSketch);

CmdSketcherMapSketch::CmdSketcherMapSketch()
  : Command("Sketcher_MapSketch")
{
    sAppModule      = "Sketcher";
    sGroup          = QT_TR_NOOP("Sketcher");
    sMenuText       = QT_TR_NOOP("Map sketch to face...");
    sToolTipText    = QT_TR_NOOP("Map a sketch to a face");
    sWhatsThis      = "Sketcher_MapSketch";
    sStatusTip      = sToolTipText;
    sPixmap         = "Sketcher_MapSketch";
}

void CmdSketcherMapSketch::activated(int iMsg)
{
    QString msg_str;
    try{
        Attacher::eMapMode suggMapMode;
        std::vector<Attacher::eMapMode> validModes;

        //check that selection is valid for at least some mapping mode.
        Attacher::eSuggestResult msgid = Attacher::srOK;
        suggMapMode = SuggestAutoMapMode(&msgid, &msg_str, &validModes);

        App::Document* doc = App::GetApplication().getActiveDocument();
        std::vector<App::DocumentObject*> sketches = doc->getObjectsOfType(Part::Part2DObject::getClassTypeId());
        if (sketches.empty()) {
            QMessageBox::warning(Gui::getMainWindow(),
                qApp->translate(className(), "No sketch found"),
                qApp->translate(className(), "The document doesn't have a sketch"));
            return;
        }

        bool ok;
        QStringList items;
        for (std::vector<App::DocumentObject*>::iterator it = sketches.begin(); it != sketches.end(); ++it)
            items.push_back(QString::fromUtf8((*it)->Label.getValue()));
        QString text = QInputDialog::getItem(Gui::getMainWindow(),
            qApp->translate(className(), "Select sketch"),
            qApp->translate(className(), "Select a sketch from the list"),
            items, 0, false, &ok);
        if (!ok) return;
        int index = items.indexOf(text);
        Part2DObject &sketch = *(static_cast<Part2DObject*>(sketches[index]));


        // check circular dependency
        std::vector<Gui::SelectionObject> selobjs = Gui::Selection().getSelectionEx();
        for(  int i = 0  ;  i < selobjs.size()  ;  i++){
            App::DocumentObject* part = static_cast<Part::Feature*>(selobjs[i].getObject());
            if (!part) {
                assert(0);
                throw Base::Exception("Unexpected null pointer in CmdSketcherMapSketch::activated");
            }
            std::vector<App::DocumentObject*> input = part->getOutList();
            if (std::find(input.begin(), input.end(), &sketch) != input.end()) {
                throw ExceptionWrongInput(QT_TR_NOOP("Some of the selected objects depend on the sketch to be mapped. Circular dependencies are not allowed!"));
            }
        }

        //Ask for a new mode.
        //outline:
        // * find out the modes that are compatible with selection.
        // * Test if current mode is OK.
        // * fill in the dialog
        // * execute the dialog
        // * collect dialog result
        // * action

        bool bAttach = true;
        bool bCurIncompatible = false;
        // * find out the modes that are compatible with selection.
        eMapMode curMapMode = eMapMode(sketch.MapMode.getValue());
        // * Test if current mode is OK.
        if (std::find(validModes.begin(), validModes.end(), curMapMode) == validModes.end())
            bCurIncompatible = true;

        // * fill in the dialog
        validModes.insert(validModes.begin(), Attacher::mmDeactivated);
        if(bCurIncompatible)
            validModes.push_back(curMapMode);
        //bool ok; //already defined
        //QStringList items; //already defined
        items.clear();
        items.push_back(QObject::tr("Don't attach"));
        int iSugg = 0;//index of the auto-suggested mode in the list of valid modes
        int iCurr = 0;//index of current mode in the list of valid modes
        for (int i = 0  ;  i < validModes.size()  ;  ++i){
            items.push_back(QString::fromLatin1(AttachEngine::eMapModeStrings[validModes[i]]));
            if (validModes[i] == curMapMode) {
                iCurr = items.size() - 1;
                items.back().append(bCurIncompatible?
                                        qApp->translate(className()," (incompatible with selection)")
                                      :
                                        qApp->translate(className()," (current)")  );
            }
            if (validModes[i] == suggMapMode){
                iSugg = items.size() - 1;
                if(iSugg == 1){
                    iSugg = 0;//redirect deactivate to detach
                } else {
                    items.back().append(qApp->translate(className()," (suggested)"));
                }
            }
        }
        // * execute the dialog
        text = QInputDialog::getItem(Gui::getMainWindow()
            ,qApp->translate(className(), "Sketch attachment")
            ,bCurIncompatible?
              qApp->translate(className(), "Current attachment mode is incompatible with the new selection. Select the method to attach this sketch to selected objects.")
              :
              qApp->translate(className(), "Select the method to attach this sketch to selected objects.")
            ,items
            , bCurIncompatible ? iSugg : iCurr
            , false
            , &ok);
        // * collect dialog result
        if (!ok) return;
        index = items.indexOf(text);
        if (index == 0){
            bAttach = false;
            suggMapMode = Attacher::mmDeactivated;
        } else {
            bAttach = true;
            suggMapMode = validModes[index-1];
        }

        // * action
        std::string featName = sketch.getNameInDocument();
        if (bAttach) {
            App::PropertyLinkSubList support;
            Gui::Selection().getAsPropertyLinkSubList(support);
            std::string supportString = support.getPyReprString();

            openCommand("Attach Sketch");
            doCommand(Gui,"App.activeDocument().%s.MapMode = \"%s\"",featName.c_str(),AttachEngine::eMapModeStrings[suggMapMode]);
            doCommand(Gui,"App.activeDocument().%s.Support = %s",featName.c_str(),supportString.c_str());
            commitCommand();
        } else {
            openCommand("Detach Sketch");
            doCommand(Gui,"App.activeDocument().%s.MapMode = \"%s\"",featName.c_str(),AttachEngine::eMapModeStrings[suggMapMode]);
            doCommand(Gui,"App.activeDocument().%s.Support = None",featName.c_str());
            commitCommand();
        }
    } catch (ExceptionWrongInput &e) {
        QMessageBox::warning(Gui::getMainWindow(),
                             qApp->translate(className(), "Map sketch"),
                             qApp->translate(className(), "Can't map a sketch to support:\n"
                                         "%1").arg(e.ErrMsg.length() ? e.ErrMsg : msg_str));
    }
}

bool CmdSketcherMapSketch::isActive(void)
{
    return getActiveGuiDocument() != 0;
}

DEF_STD_CMD_A(CmdSketcherViewSketch);

CmdSketcherViewSketch::CmdSketcherViewSketch()
  : Command("Sketcher_ViewSketch")
{
    sAppModule      = "Sketcher";
    sGroup          = QT_TR_NOOP("Sketcher");
    sMenuText       = QT_TR_NOOP("View sketch");
    sToolTipText    = QT_TR_NOOP("View sketch perpendicular to sketch plane");
    sWhatsThis      = "Sketcher_ViewSketch";
    sStatusTip      = sToolTipText;
    sPixmap         = "Sketcher_ViewSketch";
    eType           = 0;
}

void CmdSketcherViewSketch::activated(int iMsg)
{
    Gui::Document *doc = getActiveGuiDocument();
    SketcherGui::ViewProviderSketch* vp = dynamic_cast<SketcherGui::ViewProviderSketch*>(doc->getInEdit());
    doCommand(Gui,"Gui.ActiveDocument.ActiveView.setCameraOrientation(App.ActiveDocument.%s.Placement.Rotation.Q)"
                 ,vp->getObject()->getNameInDocument());
}

bool CmdSketcherViewSketch::isActive(void)
{
    Gui::Document *doc = getActiveGuiDocument();
    if (doc) {
        // checks if a Sketch Viewprovider is in Edit and is in no special mode
        SketcherGui::ViewProviderSketch* vp = dynamic_cast<SketcherGui::ViewProviderSketch*>(doc->getInEdit());
        if (vp && vp->getSketchMode() == ViewProviderSketch::STATUS_NONE)
            return true;
    }
    return false;
}

DEF_STD_CMD_A(CmdSketcherValidateSketch);

CmdSketcherValidateSketch::CmdSketcherValidateSketch()
  : Command("Sketcher_ValidateSketch")
{
    sAppModule      = "Sketcher";
    sGroup          = QT_TR_NOOP("Sketcher");
    sMenuText       = QT_TR_NOOP("Validate sketch...");
    sToolTipText    = QT_TR_NOOP("Validate sketch");
    sWhatsThis      = "Sketcher_ValidateSketch";
    sStatusTip      = sToolTipText;
    eType           = 0;
}

void CmdSketcherValidateSketch::activated(int iMsg)
{
    std::vector<Gui::SelectionObject> selection = getSelection().getSelectionEx(0, Sketcher::SketchObject::getClassTypeId());
    if (selection.size() != 1) {
        QMessageBox::warning(Gui::getMainWindow(),
            qApp->translate("CmdSketcherValidateSketch", "Wrong selection"),
            qApp->translate("CmdSketcherValidateSketch", "Select one sketch, please."));
        return;
    }

    Sketcher::SketchObject* Obj = static_cast<Sketcher::SketchObject*>(selection[0].getObject());
    Gui::Control().showDialog(new TaskSketcherValidation(Obj));
}

bool CmdSketcherValidateSketch::isActive(void)
{
    return (hasActiveDocument() && !Gui::Control().activeDialog());
}

DEF_STD_CMD_A(CmdSketcherMergeSketches);

CmdSketcherMergeSketches::CmdSketcherMergeSketches()
  : Command("Sketcher_MergeSketches")
{
    sAppModule      = "Sketcher";
    sGroup          = QT_TR_NOOP("Sketcher");
    sMenuText       = QT_TR_NOOP("Merge sketches");
    sToolTipText    = QT_TR_NOOP("Merge sketches");
    sWhatsThis      = "Sketcher_MergeSketches";
    sStatusTip      = sToolTipText;
    eType           = 0;
    sPixmap         = "Sketcher_MergeSketch";
}

void CmdSketcherMergeSketches::activated(int iMsg)
{
    std::vector<Gui::SelectionObject> selection = getSelection().getSelectionEx(0, Sketcher::SketchObject::getClassTypeId());
    if (selection.size() < 2) {
        QMessageBox::warning(Gui::getMainWindow(),
            qApp->translate("CmdSketcherMergeSketches", "Wrong selection"),
            qApp->translate("CmdSketcherMergeSketches", "Select at least two sketches, please."));
        return;
    }

    Sketcher::SketchObject* Obj1 = static_cast<Sketcher::SketchObject*>(selection[0].getObject());
    
    App::Document* doc = App::GetApplication().getActiveDocument();
       
    // create Sketch 
    std::string FeatName = getUniqueObjectName("Sketch");

    openCommand("Create a merge Sketch");
    doCommand(Doc,"App.activeDocument().addObject('Sketcher::SketchObject','%s')",FeatName.c_str());
    
    Sketcher::SketchObject* mergesketch = static_cast<Sketcher::SketchObject*>(doc->getObject(FeatName.c_str()));
    
    int baseGeometry=0;
    int baseConstraints=0;
    
    for (std::vector<Gui::SelectionObject>::const_iterator it=selection.begin(); it != selection.end(); ++it) {
        const Sketcher::SketchObject* Obj = static_cast<const Sketcher::SketchObject*>((*it).getObject());
        int addedGeometries=mergesketch->addGeometry(Obj->getInternalGeometry());
        
        int addedConstraints=mergesketch->addConstraints(Obj->Constraints.getValues());
        
        for(int i=0; i<=(addedConstraints-baseConstraints); i++){
                Sketcher::Constraint * constraint= mergesketch->Constraints.getValues()[i+baseConstraints];
                
                if(constraint->First!=Sketcher::Constraint::GeoUndef || constraint->First==-1 || constraint->First==-2) // not x, y axes or origin
                    constraint->First+=baseGeometry;
                if(constraint->Second!=Sketcher::Constraint::GeoUndef || constraint->Second==-1 || constraint->Second==-2) // not x, y axes or origin
                    constraint->Second+=baseGeometry;
                if(constraint->Third!=Sketcher::Constraint::GeoUndef || constraint->Third==-1 || constraint->Third==-2) // not x, y axes or origin
                    constraint->Third+=baseGeometry;
        }
            
        baseGeometry=addedGeometries+1;
        baseConstraints=addedConstraints+1;
    }
    
    doCommand(Gui,"App.activeDocument().recompute()");
    
}

bool CmdSketcherMergeSketches::isActive(void)
{
    return (hasActiveDocument() && !Gui::Control().activeDialog());
}



void CreateSketcherCommands(void)
{
    Gui::CommandManager &rcCmdMgr = Gui::Application::Instance->commandManager();

    rcCmdMgr.addCommand(new CmdSketcherNewSketch());
    rcCmdMgr.addCommand(new CmdSketcherEditSketch());
    rcCmdMgr.addCommand(new CmdSketcherLeaveSketch());
    rcCmdMgr.addCommand(new CmdSketcherReorientSketch());
    rcCmdMgr.addCommand(new CmdSketcherMapSketch());
    rcCmdMgr.addCommand(new CmdSketcherViewSketch());
    rcCmdMgr.addCommand(new CmdSketcherValidateSketch());
    rcCmdMgr.addCommand(new CmdSketcherMergeSketches());
}
