
/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   Gyps - Species Distribution Modelling Toolkit                         *
 *   This toolkit provides data transformation and visualisation           *
 *   tools for use in species distribution modelling tools such as GARP,   *
 *   CSM, Bioclim etc.                                                     *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "layerselector.h"

//qt includes
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qlistview.h>
#include <qsettings.h>
#include <qlabel.h>

//for the is valid gdal file functions
#include "openmodellergui.h"

//standard includes
#include <iostream>



LayerSelector::LayerSelector( QWidget* parent , const char* name , bool modal , WFlags fl  )
  : LayerSelectorBase( parent, name, modal, fl )
{
  QSettings mySettings;
  baseDirString = mySettings.readEntry("/openmodeller/projectionLayersDirectory","/tmp"), //initial dir
  lblBaseDir->setText(tr("Base Dir: ") + baseDirString);
  listFileTree->clear();
  listFileTree->setRootIsDecorated(true);
  listFileTree->setColumnWidthMode(0,QListView::Maximum);
  listFileTree->setColumnWidth(0,10);
  listFileTree->setColumnWidthMode(1,QListView::Maximum);
  listFileTree->setColumnWidth(1,10);
  listFileTree->setColumnWidthMode(2,QListView::Maximum);
  listFileTree->setColumnWidth(2,10);
  listParent = new QListViewItem(listFileTree,baseDirString);
  traverseDirectories(baseDirString,listParent);
  listParent->setOpen(true);
  listFileTree->triggerUpdate();
}

void LayerSelector::pbnDirectorySelector_clicked()
{

  QSettings mySettings;

  baseDirString = QFileDialog::getExistingDirectory(
          baseDirString, //initial dir
          this,
          "get existing directory",
          "Choose a directory",
          TRUE );
  mySettings.writeEntry("/openmodeller/projectionLayersDirectory",baseDirString);
  lblBaseDir->setText(tr("Base Dir: ") + baseDirString);
  listFileTree->clear();
  listParent = new QListViewItem(listFileTree,baseDirString);
  traverseDirectories(baseDirString,listParent);
  listParent->setOpen(true);
  listFileTree->triggerUpdate();
  
}




void LayerSelector::pbnOK_clicked()
{

  selectedLayersList.clear();
  QListViewItemIterator myIterator (listParent);
  while (myIterator.current())
  {
    if (myIterator.current()->isSelected())
    {
      if (myIterator.current()->text(1)=="AIG")
      {
        selectedLayersList+=myIterator.current()->text(0);
      }
      else
      {        
        selectedLayersList+=(myIterator.current()->parent()->text(0) + myIterator.current()->text(0));
      }
    }
    ++myIterator;
  }
  accept();

}


void LayerSelector::pbnCancel_clicked()
{
  selectedLayersList.clear();
  accept();
}



void LayerSelector::traverseDirectories(const QString& theDirName, QListViewItem* theParentListViewItem)
{
  QDir myDirectory(theDirName);
  myDirectory.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks );
  std::cout << "Current directory is: " << theDirName.ascii() << std::endl;

  const QFileInfoList* myFileInfoList = myDirectory.entryInfoList();
  QFileInfoListIterator myIterator(*myFileInfoList);
  QFileInfo*myFileInfo;

  bool myInvalidFileFlag = false;
  bool myInvalidFileProjFlag = false;
  QString myInvalidFileList;
  QString myInvalidFileProjList;

  while( (myFileInfo = myIterator.current() ) )
  {
    //Ignore directories
    if(myFileInfo->fileName() == "." ||myFileInfo->fileName() == ".." ) 
    {
      ++myIterator;
      continue;
    }

    //check to see if entry is a directory - if so iterate through it (recursive function)
    //a new tree node will be created each time
    if(myFileInfo->isDir() && myFileInfo->isReadable() )
    {
      QListViewItem * myItem = new QListViewItem(theParentListViewItem,myFileInfo->absFilePath());
      myItem->setText(1,"DIR");
      traverseDirectories(myFileInfo->absFilePath(),myItem );
      //if it turned out the dir contained an arc info coverage, the 'DIR' label above will have been changed
      //to AIG in the above travers call. Now we can test if this item has no child nodes and is not named AIG
      //we can prune it out of the tree
      if (myItem->childCount()<1)
      {
        delete myItem;
      }
      else
      {
        myItem->setOpen(true);
      }
    }

    //check to see if its an adf file type
    //only add the hdr.adf files to ensure multiple adf files from one directory aren't added
    else if (myFileInfo->extension(false)=="adf")
    {
      if (myFileInfo->fileName()=="hdr.adf")
      {
        std::cout << "Current filename is: " <<myFileInfo->filePath().ascii() << std::endl;
        //set the parent item as the layer because this is a coverage
        theParentListViewItem->setText(1,"AIG");

        if (OpenModellerGui::isValidGdalProj(myFileInfo->absFilePath()))
        {
          theParentListViewItem->setText(2,"Valid");
        }
        else
        {
          theParentListViewItem->setText(2,"Invalid");
        }
        //dont scan this dir any further (we assume there are not coverages nested inside coverages)!
        return;
      }
    }

    //check to see if entry is of the other required file types
    else if ((myFileInfo->extension(false)=="tif") ||
            (myFileInfo->extension(false)=="asc") ||
            (myFileInfo->extension(false)=="bil") ||
            (myFileInfo->extension(false)=="jpg")   )
    {      

      //test whether the file is GDAL compatible
      if (OpenModellerGui::isValidGdalFile(myFileInfo->absFilePath()) && OpenModellerGui::isValidGdalProj(myFileInfo->absFilePath()))
      {
        //GOOD FILE AND GOOD PROJ
        std::cout <<myFileInfo->absFilePath().ascii() << " is a valid GDAL file and contains projection info" << std::endl;
        QListViewItem * myItem = new QListViewItem(theParentListViewItem,myFileInfo->fileName());
        myItem->setText(1,myFileInfo->extension(false));
        myItem->setText(2,"Valid");
      }
      else if (OpenModellerGui::isValidGdalFile(myFileInfo->absFilePath()) && !OpenModellerGui::isValidGdalProj(myFileInfo->absFilePath()))
      {
        //GOOD FILE AND BAD PROJ
        std::cout <<myFileInfo->absFilePath().ascii() << " is a valid GDAL file but contains no projection info" << std::endl;
        QListViewItem * myItem = new QListViewItem(theParentListViewItem,myFileInfo->fileName());
        myItem->setText(1,myFileInfo->extension(false));
        myItem->setText(2,"Invalid");
        myInvalidFileProjFlag = true;	  
        myInvalidFileProjList +=myFileInfo->absFilePath()+"\n"; 	
      }
      else 
      {
        //BAD FILE AND/OR BAD PROJ
        myInvalidFileFlag = true;
        myInvalidFileList +=myFileInfo->absFilePath()+"\n";         
      } 
    }  
    ++myIterator;
  }


}   
