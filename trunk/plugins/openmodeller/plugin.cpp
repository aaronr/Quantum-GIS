/***************************************************************************
  plugin.cpp 
  Import tool for various worldmap analysis output files
Functions:

-------------------
begin                : Jan 21, 2004
copyright            : (C) 2004 by Tim Sutton
email                : tim@linfiniti.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id$ */

// includes

#include <qgisapp.h>
#include <qgsmaplayer.h>
#include <qgsrasterlayer.h>
#include <plugin.h>


#include <qtoolbar.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qlineedit.h>
#include <qaction.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qfileinfo.h>
//non qt includes
#include <iostream>
#include <openmodellergui.h>

// xpm for creating the toolbar icon
#include "icon_om.xpm"
// 
static const char * const ident_ = "$Id$";

static const char * const name_ = "openMOdeller Wizard";
static const char * const description_ = "Plugin to run openModeller in QGIS.";
static const char * const version_ = "Version 0.1";
static const QgisPlugin::PLUGINTYPE type_ = QgisPlugin::UI;


/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param qgis Pointer to the QGIS main window
 * @param _qI Pointer to the QGIS interface object
 */
Plugin::Plugin(QgisApp * theQGisApp, QgisIface * theQgisInterFace):
          qgisMainWindowPointer(theQGisApp), 
          qGisInterface(theQgisInterFace),
          QgisPlugin( name_, description_, version_, type_ )
{
}

Plugin::~Plugin()
{

}


/*
 * Initialize the GUI interface for the plugin 
 */
void Plugin::initGui()
{
  // add a menu with 2 items
  QPopupMenu *pluginMenu = new QPopupMenu(qgisMainWindowPointer);

  pluginMenu->insertItem(QIconSet(icon_om),"&WorldMap Results Importer", this, SLOT(run()));

  menuBarPointer = ((QMainWindow *) qgisMainWindowPointer)->menuBar();

  menuIdInt = qGisInterface->addMenu("&Biodiversity", pluginMenu);
  // Create the action for tool
  QAction *myQActionPointer = new QAction("WorldMap Importer", QIconSet(icon_om), "&Wmi",0, this, "run");
  // Connect the action to the run
  connect(myQActionPointer, SIGNAL(activated()), this, SLOT(run()));
  // Add the toolbar
  toolBarPointer = new QToolBar((QMainWindow *) qgisMainWindowPointer, "Biodiversity");
  toolBarPointer->setLabel("WorldMap Importer");
  // Add the zoom previous tool to the toolbar
  myQActionPointer->addTo(toolBarPointer);


}

// Defined by interface
void Plugin::help()
{
  //implement me
}

// Slot called when the buffer menu item is activated
void Plugin::run()
{
  OpenModellerGui *myOpenModellerGui=new OpenModellerGui(qgisMainWindowPointer,"openModeller Wizard",true,0);
  //listen for when the layer has been made so we can draw it
  connect(myOpenModellerGui, SIGNAL(drawRasterLayer(QString)), this, SLOT(drawRasterLayer(QString)));
  myOpenModellerGui->show();
}
//!draw a raster layer in the qui - intended to respond to signal sent by diolog when it as finished creating
//layer
void Plugin::drawRasterLayer(QString theQString)
{
  QFileInfo myFileInfo(theQString);
  QString myDirNameQString = myFileInfo.dirPath();
  QString myBaseNameQString = myFileInfo.baseName();
  QgsRasterLayer *layer = new QgsRasterLayer(theQString, myBaseNameQString);
  layer->setColorRampingType(QgsRasterLayer::BLUE_GREEN_RED);
  layer->setDrawingStyle(QgsRasterLayer::SINGLE_BAND_PSEUDO_COLOR);
  qGisInterface->addRasterLayer(layer);
}
// Unload the plugin by cleaning up the GUI
void Plugin::unload()
{
  // remove the GUI
  menuBarPointer->removeItem(menuIdInt);
  delete toolBarPointer;
}
/** 
 * Required extern functions needed  for every plugin 
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
extern "C" QgisPlugin * classFactory(QgisApp * theQGisAppPointer, QgisIface * theQgisInterfacePointer)
{
  return new Plugin(theQGisAppPointer, theQgisInterfacePointer);
}

// Return the name of the plugin - note that we do not user class members as
// the class may not yet be insantiated when this method is called.
extern "C" QString name()
{
    return name_;
}

// Return the description
extern "C" QString description()
{
    return description_;
}

// Return the type (either UI or MapLayer plugin)
extern "C" int type()
{
    return type_;
}

// Return the version number for the plugin
extern "C" QString version()
{
    return version_;
}

// Delete ourself
extern "C" void unload(QgisPlugin * thePluginPointer)
{
  delete thePluginPointer;
}
