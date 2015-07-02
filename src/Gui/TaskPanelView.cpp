/***************************************************************************
 *   Copyright (c) 2009 Jürgen Riegel <juergen.riegel@web.de>              *
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
# include <QRadioButton>
#endif

/// Here the FreeCAD includes sorted by Base,App,Gui......

#include "TaskPanelView.h"
#include "BitmapFactory.h"
#include "QSint/include/QSint"
#include <Base/Console.h>

using namespace Gui;
using namespace Gui::DockWnd;


#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QWidget>

namespace Gui {
namespace DockWnd {
class Ui_MainWindow2
{
public:
    QAction *actionNew;
    QAction *actionLoad;
    QAction *actionSave;
    QAction *actionPrint;
    QGridLayout *gridLayout;
    QSint::ActionPanel *ActionPanel;
    QSint::ActionGroup *ActionGroup1;
    QVBoxLayout *verticalLayout;
    QRadioButton *rbDefaultScheme;
    QRadioButton *rbXPBlueScheme;
    QRadioButton *rbXPBlue2Scheme;
    QRadioButton *rbVistaScheme;
    QRadioButton *rbMacScheme;
    QRadioButton *rbAndroidScheme;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *MainWindow2)
    {
        if (MainWindow2->objectName().isEmpty())
            MainWindow2->setObjectName(QString::fromUtf8("MainWindow2"));
        MainWindow2->resize(529, 407);
        MainWindow2->setStyleSheet(QString::fromUtf8("\n"
            "QWidget2 {\n"
"    background-color: green;\n"
"}\n"
""));
        actionNew = new QAction(MainWindow2);
        actionNew->setObjectName(QString::fromUtf8("actionNew"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/document-new.svg"), QSize(), QIcon::Normal, QIcon::Off);
        actionNew->setIcon(icon);
        actionLoad = new QAction(MainWindow2);
        actionLoad->setObjectName(QString::fromUtf8("actionLoad"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/document-open.svg"), QSize(), QIcon::Normal, QIcon::Off);
        actionLoad->setIcon(icon1);
        actionSave = new QAction(MainWindow2);
        actionSave->setObjectName(QString::fromUtf8("actionSave"));
        actionSave->setEnabled(false);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/document-save.svg"), QSize(), QIcon::Normal, QIcon::Off);
        actionSave->setIcon(icon2);
        actionPrint = new QAction(MainWindow2);
        actionPrint->setObjectName(QString::fromUtf8("actionPrint"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/document-print.svg"), QSize(), QIcon::Normal, QIcon::Off);
        actionPrint->setIcon(icon3);
        gridLayout = new QGridLayout(MainWindow2);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        ActionPanel = new QSint::ActionPanel(MainWindow2);
        ActionPanel->setObjectName(QString::fromUtf8("ActionPanel"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ActionPanel->sizePolicy().hasHeightForWidth());
        ActionPanel->setSizePolicy(sizePolicy);

        gridLayout->addWidget(ActionPanel, 0, 0, 2, 1);

        ActionGroup1 = new QSint::ActionGroup(MainWindow2);
        ActionGroup1->setObjectName(QString::fromUtf8("ActionGroup1"));
        ActionGroup1->setProperty("expandable", QVariant(true));
        ActionGroup1->setProperty("header", QVariant(true));
        verticalLayout = new QVBoxLayout(ActionGroup1);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        rbDefaultScheme = new QRadioButton(ActionGroup1);
        rbDefaultScheme->setObjectName(QString::fromUtf8("rbDefaultScheme"));
        rbDefaultScheme->setChecked(true);

        verticalLayout->addWidget(rbDefaultScheme);

        rbXPBlueScheme = new QRadioButton(ActionGroup1);
        rbXPBlueScheme->setObjectName(QString::fromUtf8("rbXPBlueScheme"));

        verticalLayout->addWidget(rbXPBlueScheme);

        rbXPBlue2Scheme = new QRadioButton(ActionGroup1);
        rbXPBlue2Scheme->setObjectName(QString::fromUtf8("rbXPBlue2Scheme"));

        verticalLayout->addWidget(rbXPBlue2Scheme);

        rbVistaScheme = new QRadioButton(ActionGroup1);
        rbVistaScheme->setObjectName(QString::fromUtf8("rbVistaScheme"));

        verticalLayout->addWidget(rbVistaScheme);

        rbMacScheme = new QRadioButton(ActionGroup1);
        rbMacScheme->setObjectName(QString::fromUtf8("rbMacScheme"));

        verticalLayout->addWidget(rbMacScheme);

        rbAndroidScheme = new QRadioButton(ActionGroup1);
        rbAndroidScheme->setObjectName(QString::fromUtf8("rbAndroidScheme"));

        verticalLayout->addWidget(rbAndroidScheme);


        gridLayout->addWidget(ActionGroup1, 0, 1, 1, 1);

        verticalSpacer = new QSpacerItem(20, 57, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 1, 1, 1, 1);


        retranslateUi(MainWindow2);

        QMetaObject::connectSlotsByName(MainWindow2);
    } // setupUi

    void retranslateUi(QWidget *MainWindow2)
    {
        MainWindow2->setWindowTitle(QApplication::translate("MainWindow2", "ActionBox Example", 0, QApplication::UnicodeUTF8));
        actionNew->setText(QApplication::translate("MainWindow2", "Create new file", 0, QApplication::UnicodeUTF8));
        actionLoad->setText(QApplication::translate("MainWindow2", "Load a file", 0, QApplication::UnicodeUTF8));
        actionSave->setText(QApplication::translate("MainWindow2", "Save current file", 0, QApplication::UnicodeUTF8));
        actionPrint->setText(QApplication::translate("MainWindow2", "Print file contents", 0, QApplication::UnicodeUTF8));
        ActionGroup1->setProperty("headerText", QVariant(QApplication::translate("MainWindow2", "Choose Scheme", 0, QApplication::UnicodeUTF8)));
        rbDefaultScheme->setText(QApplication::translate("MainWindow2", "Default", 0, QApplication::UnicodeUTF8));
        rbXPBlueScheme->setText(QApplication::translate("MainWindow2", "XP Blue", 0, QApplication::UnicodeUTF8));
        rbXPBlue2Scheme->setText(QApplication::translate("MainWindow2", "XP Blue 2", 0, QApplication::UnicodeUTF8));
        rbVistaScheme->setText(QApplication::translate("MainWindow2", "Vista", 0, QApplication::UnicodeUTF8));
        rbMacScheme->setText(QApplication::translate("MainWindow2", "MacOS", 0, QApplication::UnicodeUTF8));
        rbAndroidScheme->setText(QApplication::translate("MainWindow2", "Android", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(MainWindow2);
    } // retranslateUi

};

namespace Ui {
    class MainWindow2: public Ui_MainWindow2 {};
} // namespace Ui

class Ui_MainWindow
{
public:
    QVBoxLayout *verticalLayout;
    QVBoxLayout *verticalLayout_3;
    QLabel *label;
    QFrame *line_2;
    QGridLayout *gridLayout_2;
    QSint::ActionBox *ActionBox1;
    QSint::ActionBox *ActionBox2;
    QSpacerItem *verticalSpacer;
    QSint::ActionBox *ActionBox3;
    QSint::ActionBox *ActionBox4;
    QVBoxLayout *verticalLayout_4;
    QLabel *label_2;
    QFrame *line;
    QVBoxLayout *verticalLayout_2;
    QSint::ActionLabel *ActionLabel1;
    QSint::ActionLabel *ActionLabel2;
    QSint::ActionLabel *ActionLabel3;

    void setupUi(QWidget *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(642, 509);
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        MainWindow->setStyleSheet(QString::fromUtf8("\n"
"QWidget2 {\n"
"	background-color: green;\n"
"}\n"
""));
        verticalLayout = new QVBoxLayout(MainWindow);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        label = new QLabel(MainWindow);
        label->setObjectName(QString::fromUtf8("label"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Maximum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy1);

        verticalLayout_3->addWidget(label);

        line_2 = new QFrame(MainWindow);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        verticalLayout_3->addWidget(line_2);


        verticalLayout->addLayout(verticalLayout_3);

        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        ActionBox1 = new QSint::ActionBox(MainWindow);
        ActionBox1->setObjectName(QString::fromUtf8("ActionBox1"));
        ActionBox1->setFrameShape(QFrame::StyledPanel);
        ActionBox1->setFrameShadow(QFrame::Raised);

        gridLayout_2->addWidget(ActionBox1, 0, 0, 1, 1);

        ActionBox2 = new QSint::ActionBox(MainWindow);
        ActionBox2->setObjectName(QString::fromUtf8("ActionBox2"));
        ActionBox2->setFrameShape(QFrame::StyledPanel);
        ActionBox2->setFrameShadow(QFrame::Raised);

        gridLayout_2->addWidget(ActionBox2, 1, 0, 1, 1);

        verticalSpacer = new QSpacerItem(94, 28, QSizePolicy::Minimum, QSizePolicy::Minimum);

        gridLayout_2->addItem(verticalSpacer, 3, 0, 1, 1);

        ActionBox3 = new QSint::ActionBox(MainWindow);
        ActionBox3->setObjectName(QString::fromUtf8("ActionBox3"));
        ActionBox3->setFrameShape(QFrame::StyledPanel);
        ActionBox3->setFrameShadow(QFrame::Raised);

        gridLayout_2->addWidget(ActionBox3, 0, 1, 1, 1);

        ActionBox4 = new QSint::ActionBox(MainWindow);
        ActionBox4->setObjectName(QString::fromUtf8("ActionBox4"));
        ActionBox4->setFrameShape(QFrame::StyledPanel);
        ActionBox4->setFrameShadow(QFrame::Raised);

        gridLayout_2->addWidget(ActionBox4, 1, 1, 1, 1);


        verticalLayout->addLayout(gridLayout_2);

        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        label_2 = new QLabel(MainWindow);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        sizePolicy1.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy1);

        verticalLayout_4->addWidget(label_2);

        line = new QFrame(MainWindow);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout_4->addWidget(line);


        verticalLayout->addLayout(verticalLayout_4);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        ActionLabel1 = new QSint::ActionLabel(MainWindow);
        ActionLabel1->setObjectName(QString::fromUtf8("ActionLabel1"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(ActionLabel1->sizePolicy().hasHeightForWidth());
        ActionLabel1->setSizePolicy(sizePolicy2);

        verticalLayout_2->addWidget(ActionLabel1);

        ActionLabel2 = new QSint::ActionLabel(MainWindow);
        ActionLabel2->setObjectName(QString::fromUtf8("ActionLabel2"));
        sizePolicy2.setHeightForWidth(ActionLabel2->sizePolicy().hasHeightForWidth());
        ActionLabel2->setSizePolicy(sizePolicy2);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/document-open.svg"), QSize(), QIcon::Normal, QIcon::Off);
        ActionLabel2->setIcon(icon);

        verticalLayout_2->addWidget(ActionLabel2);

        ActionLabel3 = new QSint::ActionLabel(MainWindow);
        ActionLabel3->setObjectName(QString::fromUtf8("ActionLabel3"));
        sizePolicy2.setHeightForWidth(ActionLabel3->sizePolicy().hasHeightForWidth());
        ActionLabel3->setSizePolicy(sizePolicy2);
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/document-print.svg"), QSize(), QIcon::Normal, QIcon::Off);
        ActionLabel3->setIcon(icon1);

        verticalLayout_2->addWidget(ActionLabel3);


        verticalLayout->addLayout(verticalLayout_2);


        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QWidget *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "ActionBox Example", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("MainWindow", "ActionBoxes", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("MainWindow", "ActionLabels", 0, QApplication::UnicodeUTF8));
        ActionLabel1->setText(QApplication::translate("MainWindow", "Simple clickable action", 0, QApplication::UnicodeUTF8));
        ActionLabel2->setText(QApplication::translate("MainWindow", "Simple clickable action with icon", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        ActionLabel3->setToolTip(QApplication::translate("MainWindow", "Tooltip of the ActionLabel", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        ActionLabel3->setText(QApplication::translate("MainWindow", "Simple clickable action with icon and tooltip", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(MainWindow);
    } // retranslateUi

};
namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui
}

}

#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QMessageBox>


MainWindow1::MainWindow1(QWidget *parent) :
    QWidget(parent),
    ui(new Gui::DockWnd::Ui::MainWindow)
{
    ui->setupUi(this);

    // setup ActionBox 1
    QIcon open = QIcon::fromTheme(QString::fromLatin1("document-open"));
    ui->ActionBox1->setIcon(open.pixmap(24,24));
    ui->ActionBox1->header()->setText(QString::fromAscii("Header of the group"));
    connect(ui->ActionBox1->header(), SIGNAL(clicked()), this, SLOT(executeAction()));

    QSint::ActionLabel *a1 = ui->ActionBox1->createItem(QString::fromAscii("This action has no icon"));
    connect(a1, SIGNAL(clicked()), this, SLOT(executeAction()));
    QIcon print = QIcon::fromTheme(QString::fromLatin1("document-print"));
    QSint::ActionLabel *a2 = ui->ActionBox1->createItem(print.pixmap(24,24),
                                                QString::fromAscii("This action has icon"));
    connect(a2, SIGNAL(clicked()), this, SLOT(executeAction()));

    QLayout *hbl1 = ui->ActionBox1->createHBoxLayout();
    QSint::ActionLabel *a3 = ui->ActionBox1->createItem(QString::fromAscii("1st action in row"), hbl1);
    connect(a3, SIGNAL(clicked()), this, SLOT(executeAction()));
    QSint::ActionLabel *a4 = ui->ActionBox1->createItem(QString::fromAscii("2nd action in row"), hbl1);
    connect(a4, SIGNAL(clicked()), this, SLOT(executeAction()));

    // setup ActionBox 2
    QIcon save = QIcon::fromTheme(QString::fromLatin1("document-save"));
    ui->ActionBox2->setIcon(save.pixmap(24,24));
    ui->ActionBox2->header()->setText(QString::fromAscii("Checkable actions allowed"));
    connect(ui->ActionBox2->header(), SIGNAL(clicked()), this, SLOT(executeAction()));

    QSint::ActionLabel *b1 = ui->ActionBox2->createItem(QString::fromAscii("Action 1 (Exclusive)"));
    b1->setCheckable(true);
    b1->setAutoExclusive(true);
    b1->setChecked(true);
    QSint::ActionLabel *b2 = ui->ActionBox2->createItem(QString::fromAscii("Action 2 (Exclusive)"));
    b2->setCheckable(true);
    b2->setAutoExclusive(true);
    QSint::ActionLabel *b3 = ui->ActionBox2->createItem(QString::fromAscii("Action 3 (Exclusive)"));
    b3->setCheckable(true);
    b3->setAutoExclusive(true);

    QSint::ActionLabel *b4 = ui->ActionBox2->createItem(QString::fromAscii("Non-exclusive but still checkable"));
    b4->setCheckable(true);

    // setup ActionBox 3
    ui->ActionBox3->setIcon(print.pixmap(24,24));
    ui->ActionBox3->header()->setText(QString::fromAscii("Also, widgets allowed as well"));

    ui->ActionBox3->addWidget(new QPushButton(QString::fromAscii("PushButton"), this));
    ui->ActionBox3->addWidget(new QCheckBox(QString::fromAscii("CheckBox"), this));
    QLayout *hbl3 = ui->ActionBox3->createHBoxLayout();
    ui->ActionBox3->addWidget(new QRadioButton(QString::fromAscii("RadioButton 1"), this), hbl3);
    ui->ActionBox3->addWidget(new QRadioButton(QString::fromAscii("RadioButton 2"), this), hbl3);

    // setup ActionBox 4
    ui->ActionBox4->setIcon(open.pixmap(24,24));
    ui->ActionBox4->header()->setText(QString::fromAscii("ActionBox with different scheme"));

    ui->ActionBox4->createItem(QString::fromAscii("This action has no icon"));
    ui->ActionBox4->createItem(print.pixmap(24,24),
                                                QString::fromAscii("This action has icon"));
    QLayout *hbl4 = ui->ActionBox4->createHBoxLayout();
    ui->ActionBox4->createItem(QString::fromAscii("1st action in row"), hbl4);
    ui->ActionBox4->createItem(QString::fromAscii("2nd action in row"), hbl4);
    ui->ActionBox4->createItem(QString::fromAscii("3rd action in row"), hbl4);

    const char* ActionBoxNewStyle =
        "QSint--ActionBox {"
            "background-color: #333333;"
            "border: 1px solid #000000;"
            "text-align: left;"
        "}"

        "QSint--ActionBox:hover {"
            "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #666666, stop: 1 #333333);"
            "border: 1px solid #222222;"
        "}"

        "QSint--ActionBox QSint--ActionLabel[class='header'] {"
            "text-align: center;"
            "font: 14px bold;"
            "color: #999999;"
            "background-color: transparent;"
            "border: 1px solid transparent;"
        "}"

        "QSint--ActionBox QSint--ActionLabel[class='header']:hover {"
            "color: #aaaaaa;"
            "text-decoration: underline;"
            "border: 1px dotted #aaaaaa;"
        "}"

        "QSint--ActionBox QSint--ActionLabel[class='action'] {"
            "background-color: transparent;"
            "border: none;"
            "color: #777777;"
            "text-align: left;"
            "font: 11px;"
        "}"

        "QSint--ActionBox QSint--ActionLabel[class='action']:hover {"
            "color: #888888;"
            "text-decoration: underline;"
        "}"

        "QSint--ActionBox QSint--ActionLabel[class='action']:on {"
            "background-color: #ddeeff;"
            "color: #006600;"
        "}"
    ;

    ui->ActionBox4->setStyleSheet(QString::fromAscii(ActionBoxNewStyle));
}

MainWindow1::~MainWindow1()
{
    delete ui;
}

void MainWindow1::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow1::executeAction()
{
    QMessageBox::about(0, QString::fromAscii("Action clicked"), QString::fromAscii("Do something here :)"));
}

MainWindow2::MainWindow2(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow2)
{
    ui->setupUi(this);

    // create ActionGroups on ActionPanel

    QIcon save = QIcon::fromTheme(QString::fromLatin1("document-save"));
    QSint::ActionGroup *group1 = ui->ActionPanel->createGroup(save.pixmap(24,24), QString::fromAscii("Expandable Group"));
    group1->addAction(ui->actionNew);
    group1->addAction(ui->actionLoad);
    group1->addWidget(new QPushButton(QString::fromAscii("Just a button"), this));
    group1->addAction(ui->actionSave);
    group1->addAction(ui->actionPrint);
    group1->addWidget(new QPushButton(QString::fromAscii("Just another button"), this));

    QIcon redo = QIcon::fromTheme(QString::fromLatin1("edit-redo"));
    QSint::ActionGroup *group2 = ui->ActionPanel->createGroup(redo.pixmap(24,24), QString::fromAscii("Non-Expandable Group"), false);
    group2->addAction(ui->actionNew);
    group2->addAction(ui->actionLoad);
    group2->addAction(ui->actionSave);
    group2->addAction(ui->actionPrint);

    ui->ActionPanel->addWidget(new QLabel(QString::fromAscii("Action Group without header"), this));

    QSint::ActionGroup *group3 = ui->ActionPanel->createGroup();
    group3->addAction(ui->actionNew);

    QHBoxLayout *hbl = new QHBoxLayout();
    group3->groupLayout()->addLayout(hbl);
    hbl->addWidget(group3->addAction(ui->actionLoad, false));
    hbl->addWidget(group3->addAction(ui->actionSave, false));

    group3->addAction(ui->actionPrint);

    ui->ActionPanel->addStretch();


    // setup standalone ActionGroup

    ui->ActionGroup1->setScheme(QSint::WinXPPanelScheme::defaultScheme());

    ui->ActionGroup1->addWidget(ui->rbDefaultScheme);
    ui->ActionGroup1->addWidget(ui->rbXPBlueScheme);
    ui->ActionGroup1->addWidget(ui->rbXPBlue2Scheme);
    ui->ActionGroup1->addWidget(ui->rbVistaScheme);
    ui->ActionGroup1->addWidget(ui->rbMacScheme);
    ui->ActionGroup1->addWidget(ui->rbAndroidScheme);

    adjustSize();
}

MainWindow2::~MainWindow2()
{
    delete ui;
}

void MainWindow2::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow2::executeAction()
{
    QMessageBox::about(0, QString::fromAscii("Action clicked"), QString::fromAscii("Do something here :)"));
}

void MainWindow2::on_rbDefaultScheme_toggled(bool b)
{
    if (b)
        ui->ActionPanel->setScheme(QSint::ActionPanelScheme::defaultScheme());
}

void MainWindow2::on_rbXPBlueScheme_toggled(bool b)
{
    if (b)
        ui->ActionPanel->setScheme(QSint::WinXPPanelScheme::defaultScheme());

}

void MainWindow2::on_rbXPBlue2Scheme_toggled(bool b)
{
    if (b)
        ui->ActionPanel->setScheme(QSint::WinXPPanelScheme2::defaultScheme());
}

void MainWindow2::on_rbVistaScheme_toggled(bool b)
{
    if (b)
        ui->ActionPanel->setScheme(QSint::WinVistaPanelScheme::defaultScheme());
}

void MainWindow2::on_rbMacScheme_toggled(bool b)
{
    if (b)
        ui->ActionPanel->setScheme(QSint::MacPanelScheme::defaultScheme());
}

void MainWindow2::on_rbAndroidScheme_toggled(bool b)
{
    if (b)
        ui->ActionPanel->setScheme(QSint::AndroidPanelScheme::defaultScheme());
}


/* TRANSLATOR Gui::DockWnd::TaskPanelView */

TaskPanelView::TaskPanelView(Gui::Document* pcDocument, QWidget *parent)
  : DockWindow(pcDocument,parent)
{
#if QT_VERSION <= 0x040104
    // tmp. disable the file logging to suppress some bothering warnings related
    // to Qt 4.1 because it will really pollute the log file with useless stuff
    Base::Console().SetEnabledMsgType("File", ConsoleMsgType::MsgType_Wrn, false);
    Base::Console().SetEnabledMsgType("File", ConsoleMsgType::MsgType_Log, false);
#endif

    setWindowTitle(tr( "Task View"));

    QTabWidget* tab = new QTabWidget(this);
    QGridLayout* gridLayout = new QGridLayout(this);
    gridLayout->addWidget(tab, 0, 0, 0, 0);

    QWidget* m1 = new MainWindow1(this);
    tab->addTab(m1, QString::fromLatin1("MainWindow1"));
    QWidget* m2 = new MainWindow2(this);
    tab->addTab(m2, QString::fromLatin1("MainWindow2"));
#if 0
    iisTaskPanel *taskPanel = new iisTaskPanel(this);
    iisTaskBox *tb1 = new iisTaskBox(
        Gui::BitmapFactory().pixmap("document-new"),QLatin1String("Group of Tasks"),true, this);
    taskPanel->addWidget(tb1);
    gridLayout->addWidget(taskPanel, 0, 0, 2, 1);

    iisIconLabel *i1 = new iisIconLabel(
        Gui::BitmapFactory().iconFromTheme("zoom-in"), QLatin1String("Do Task 1"), tb1);
    tb1->addIconLabel(i1);
    //connect(i1, SIGNAL(activated()), this, SLOT(task1()));

    iisIconLabel *i2 = new iisIconLabel(
        Gui::BitmapFactory().iconFromTheme("zoom-out"), QLatin1String("Do Task 2"), tb1);
    tb1->addIconLabel(i2);

    QHBoxLayout *hbl = new QHBoxLayout();
    tb1->groupLayout()->addLayout(hbl);

    iisIconLabel *i3 = new iisIconLabel(
        Gui::BitmapFactory().iconFromTheme("edit-copy"), QLatin1String("Do Task 3"), tb1);
    tb1->addIconLabel(i3, false);
    hbl->addWidget(i3);

    iisIconLabel *i4 = new iisIconLabel(
        Gui::BitmapFactory().iconFromTheme("edit-cut"), QLatin1String("Do Task 4"), tb1);
    tb1->addIconLabel(i4, false);
    hbl->addWidget(i4);
    i4->setColors(Qt::red, Qt::green, Qt::gray);
    i4->setFocusPen(QPen());

    iisIconLabel *i5 = new iisIconLabel(
        Gui::BitmapFactory().iconFromTheme("edit-paste"), QLatin1String("Do Task 5"), tb1);
    tb1->addIconLabel(i5);

    iisTaskBox *tb2 = new iisTaskBox(
        Gui::BitmapFactory().pixmap("document-print"), QLatin1String("Non-expandable Group"), false, this);
    taskPanel->addWidget(tb2);

    iisIconLabel *i21 = new iisIconLabel(
        Gui::BitmapFactory().iconFromTheme("document-new"), QLatin1String("Do Task 2.1"), tb2);
    tb2->addIconLabel(i21);

    iisIconLabel *i22 = new iisIconLabel(
        Gui::BitmapFactory().iconFromTheme("document-open"), QLatin1String("Do Task 2.2"), tb2);
    tb2->addIconLabel(i22);
    i22->setEnabled(false);

    iisIconLabel *i23 = new iisIconLabel(
        Gui::BitmapFactory().iconFromTheme("document-save"), QLatin1String("Do Task 2.3"), tb2);
    tb2->addIconLabel(i23);

    iisTaskBox *tb3 = new iisTaskBox(QPixmap(), QLatin1String("Group without Icons"), true, this);
    taskPanel->addWidget(tb3);

    iisIconLabel *i31 = new iisIconLabel(QPixmap(), QLatin1String("Do Task 3.1"), tb3);
    tb3->addIconLabel(i31);

    iisIconLabel *i32 = new iisIconLabel(QPixmap(), QLatin1String("Do Task 3.2"), tb3);
    tb3->addIconLabel(i32);

    tb3->groupLayout()->addWidget(new QLabel(QLatin1String("Widgets also allowed:"), this));
    tb3->groupLayout()->addWidget(new QPushButton(QLatin1String("A Button"), this));

    // Other widgets can be also added to the panel
    QLabel *l1 = new QLabel(QLatin1String("A group without header"), this);
    taskPanel->addWidget(l1);


    iisTaskGroup *tb4 = new iisTaskGroup(this);
    taskPanel->addWidget(tb4);

    iisIconLabel *i41 = new iisIconLabel(
        Gui::BitmapFactory().iconFromTheme("application-exit"), QLatin1String("Do Task 4.1"), tb4);
    tb4->addIconLabel(i41);

    iisIconLabel *i42 = new iisIconLabel(QPixmap(), QLatin1String("Do Task 4.2"), tb4);
    tb4->addIconLabel(i42);

    taskPanel->addStretch();
    taskPanel->setScheme(iisWinXPTaskPanelScheme::defaultScheme());
    //tb1->setScheme(iisWinXPTaskPanelScheme::defaultScheme());
    tb2->setScheme(iisWinXPTaskPanelScheme2::defaultScheme());
    tb3->setScheme(iisWinXPTaskPanelScheme2::defaultScheme());
    //tb4->setScheme(iisWinXPTaskPanelScheme::defaultScheme());
#endif
    onUpdate();

    Gui::Selection().Attach(this);

#if QT_VERSION <= 0x040104
    Base::Console().SetEnabledMsgType("File", ConsoleMsgType::MsgType_Wrn, true);
    Base::Console().SetEnabledMsgType("File", ConsoleMsgType::MsgType_Log, true);
#endif
}

TaskPanelView::~TaskPanelView()
{
    Gui::Selection().Detach(this);
}

/// @cond DOXERR
void TaskPanelView::OnChange(Gui::SelectionSingleton::SubjectType &rCaller,
                            Gui::SelectionSingleton::MessageType Reason)
{
    /*
    std::string temp;

    if (Reason.Type == SelectionChanges::AddSelection) {
    }
    else if (Reason.Type == SelectionChanges::ClrSelection) {
    }
    else if (Reason.Type == SelectionChanges::RmvSelection) {
    }
    */
}

void TaskPanelView::onUpdate(void)
{
}

bool TaskPanelView::onMsg(const char* pMsg,const char** ppReturn)
{
    return false;
}
/// @endcond

#include "moc_TaskPanelView.cpp"
