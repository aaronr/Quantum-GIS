/***************************************************************************
                         qgscomposer.cpp  -  description
                             -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgscomposer.h"

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgscomposerview.h"
#include "qgscomposition.h"
#include "qgscompositionwidget.h"
#include "qgscomposerlabel.h"
#include "qgscomposerlabelwidget.h"
#include "qgscomposerlegend.h"
#include "qgscomposerlegendwidget.h"
#include "qgscomposermap.h"
#include "qgscomposermapwidget.h"
#include "qgscomposerscalebar.h"
#include "qgscomposerscalebarwidget.h"
#include "qgsexception.h"
#include "qgsproject.h"
#include "qgsmessageviewer.h"
#include "qgscontexthelp.h"
#include "qgscursors.h"

#include <QDesktopWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QMatrix>
#include <QMessageBox>
#include <QPainter>

#include <QPrinter>
#include <QPrintDialog>
#include <QSettings>
#include <QIcon>
#include <QPixmap>
#if QT_VERSION < 0x040300
#include <Q3Picture>
#else
#include <QSvgGenerator>
#endif
#include <QToolBar>
#include <QImageWriter>
#include <QCheckBox>
#include <QSizeGrip>
#include <iostream>

QgsComposer::QgsComposer( QgisApp *qgis): QMainWindow()
{
  setupUi(this);
  setupTheme();

  QString myIconPath = QgsApplication::themePath();

  QAction* moveItemContentAction = new QAction(QIcon(QPixmap(myIconPath+"mActionMoveItemContent.png")), tr("Move Item content"), 0);
  moveItemContentAction->setCheckable(true);
  connect(moveItemContentAction, SIGNAL(triggered()), this, SLOT(moveItemContent()));
  toolBar->addAction(moveItemContentAction);
  //toolBar->addAction(QIcon(QPixmap(myIconPath+"mActionMoveItemContent.png")), tr("Move Item content"), this, SLOT(moveItemContent()));

  toolBar->addAction(QIcon(QPixmap(myIconPath+"mActionGroupItems.png")), tr("&Group Items"), this, SLOT(groupItems()));
  toolBar->addAction(QIcon(QPixmap(myIconPath+"mActionUngroupItems.png")), tr("&Ungroup Items"), this, SLOT(ungroupItems()));

  QActionGroup* toggleActionGroup = new QActionGroup(this);
  toggleActionGroup->addAction(moveItemContentAction);
  toggleActionGroup->addAction(mActionAddNewMap);
  toggleActionGroup->addAction(mActionAddNewLabel);
  toggleActionGroup->addAction(mActionAddNewLegend);
  toggleActionGroup->addAction(mActionAddNewScalebar);
  toggleActionGroup->addAction(mActionAddImage);
  toggleActionGroup->addAction(mActionSelectMoveItem);
  toggleActionGroup->setExclusive(true);
  

  setWindowTitle(tr("QGIS - print composer"));

  // Template save and load is not yet implemented, so disable those actions
  mActionOpenTemplate->setEnabled(false);
  mActionSaveTemplateAs->setEnabled(false);

  mActionAddNewMap->setCheckable(true);
  mActionAddNewLabel->setCheckable(true);
  mActionAddNewLegend->setCheckable(true);
  mActionSelectMoveItem->setCheckable(true);
  mActionAddNewScalebar->setCheckable(true);
  mActionAddImage->setCheckable(true);

  mQgis = qgis;
  mFirstTime = true;

#ifdef QGISDEBUG
  std::cout << "QgsComposer::QgsComposer" << std::endl;
#endif
 
  setMouseTracking(true);
  mSplitter->setMouseTracking(true);
  mViewFrame->setMouseTracking(true);

  //create composer view
  mView = new QgsComposerView (mViewFrame);
  connect(mView, SIGNAL(selectedItemChanged(const QgsComposerItem*)), this, SLOT(showItemOptions(const QgsComposerItem*)));
  connect(mView, SIGNAL(composerLabelAdded(QgsComposerLabel*)), this, SLOT(addComposerLabel(QgsComposerLabel*)));
  connect(mView, SIGNAL(composerMapAdded(QgsComposerMap*)), this, SLOT(addComposerMap(QgsComposerMap*)));
  connect(mView, SIGNAL(itemRemoved(QgsComposerItem*)), this, SLOT(deleteItem(QgsComposerItem*)));
  connect(mView, SIGNAL(composerScaleBarAdded(QgsComposerScaleBar*)), this, SLOT(addComposerScaleBar(QgsComposerScaleBar*)));
  connect(mView, SIGNAL(composerLegendAdded(QgsComposerLegend*)), this, SLOT(addComposerLegend(QgsComposerLegend*)));

  mComposition  = new QgsComposition(mQgis->getMapCanvas());
  mView->setComposition(mComposition);

  QgsCompositionWidget* compositionWidget = new QgsCompositionWidget(mCompositionOptionsFrame, mComposition);
  compositionWidget->show();

  mCompositionOptionsLayout = new QGridLayout( mCompositionOptionsFrame );
  mCompositionOptionsLayout->setMargin(0);
  mCompositionOptionsLayout->addWidget(compositionWidget);
  
  mPrinter = 0;

  QGridLayout *l = new QGridLayout(mViewFrame );
  l->setMargin(0);
  l->addWidget( mView, 0, 0 );

  mCompositionNameComboBox->insertItem( tr("Map 1") );

  //mComposition  = new QgsComposition( this, 1 );
  //mComposition->setActive ( true );

  // Create size grip (needed by Mac OS X for QMainWindow if QStatusBar is not visible)
  mSizeGrip = new QSizeGrip(this);
  mSizeGrip->resize(mSizeGrip->sizeHint());
  mSizeGrip->move(rect().bottomRight() - mSizeGrip->rect().bottomRight());

  restoreWindowState();
  selectItem(); // Set selection tool

  mView->setFocus();

  //connect with signals from QgsProject to read/write project files
  if(QgsProject::instance())
    {
      connect(QgsProject::instance(), SIGNAL(readProject(const QDomDocument&)), this, SLOT(readXML(const QDomDocument&)));
      connect(QgsProject::instance(), SIGNAL(writeProject(QDomDocument&)), this, SLOT(writeXML(QDomDocument&)));
    }
}

QgsComposer::~QgsComposer()
{
}

void QgsComposer::setupTheme()
{
  //calculate the active theme path
  QString myThemePath= QgsApplication::themePath();
  

  //now set all the icons
  mActionOpenTemplate->setIconSet(QIcon(QPixmap(myThemePath + "/mActionFileOpen.png")));
  mActionSaveTemplateAs->setIconSet(QIcon(QPixmap(myThemePath + "/mActionFileSaveAs.png")));
  mActionExportAsImage->setIconSet(QIcon(QPixmap(myThemePath + "/mActionExportMapServer.png")));
  mActionExportAsSVG->setIconSet(QIcon(QPixmap(myThemePath + "/mActionSaveAsSVG.png")));
  mActionPrint->setIconSet(QIcon(QPixmap(myThemePath + "/mActionFilePrint.png")));
  mActionZoomAll->setIconSet(QIcon(QPixmap(myThemePath + "/mActionZoomFullExtent.png")));
  mActionZoomIn->setIconSet(QIcon(QPixmap(myThemePath + "/mActionZoomIn.png")));
  mActionZoomOut->setIconSet(QIcon(QPixmap(myThemePath + "/mActionZoomOut.png")));
  mActionRefreshView->setIconSet(QIcon(QPixmap(myThemePath + "/mActionDraw.png")));
  mActionAddImage->setIconSet(QIcon(QPixmap(myThemePath + "/mActionSaveMapAsImage.png")));
  mActionAddNewMap->setIconSet(QIcon(QPixmap(myThemePath + "/mActionAddRasterLayer.png")));
  mActionAddNewLabel->setIconSet(QIcon(QPixmap(myThemePath + "/mActionLabel.png")));
  mActionAddNewLegend->setIconSet(QIcon(QPixmap(myThemePath + "/mActionAddLegend.png")));
  mActionAddNewScalebar->setIconSet(QIcon(QPixmap(myThemePath + "/mActionScaleBar.png")));
  mActionSelectMoveItem->setIconSet(QIcon(QPixmap(myThemePath + "/mActionPan.png")));
}

void QgsComposer::open ( void )
{
  if ( mFirstTime ) {
    //mComposition->createDefault();
    mFirstTime = false;
    show();
    zoomFull(); // zoomFull() does not work properly until we have called show()
  }

  else{
    show(); //make sure the window is displayed - with a saved project, it's possible to not have already called show()
            //is that a bug?
    raise(); //bring the composer window to the front
  }
}

void QgsComposer::showCompositionOptions ( QWidget *w ) 
{
  QWidget* currentWidget = mItemStackedWidget->currentWidget();
  mItemStackedWidget->removeWidget(currentWidget);
  mItemStackedWidget->addWidget(w);
}

void QgsComposer::showItemOptions(const QgsComposerItem* item)
{
  QWidget* currentWidget = mItemStackedWidget->currentWidget();

  if(!item)
    {
      mItemStackedWidget->removeWidget(currentWidget);
      mItemStackedWidget->setCurrentWidget(0);
      return;
    }

  QMap<QgsComposerItem*, QWidget*>::iterator it = mItemWidgetMap.find(const_cast<QgsComposerItem*>(item));
  if(it == mItemWidgetMap.constEnd())
    {
      return;
    }

  QWidget* newWidget = it.value();

  if(!newWidget || newWidget == currentWidget) //bail out if new widget does not exist or is already there
    {
      return;
    }

  mItemStackedWidget->removeWidget(currentWidget);
  mItemStackedWidget->addWidget(newWidget);
  mItemStackedWidget->setCurrentWidget(newWidget);
  //newWidget->show();
}

QgsMapCanvas *QgsComposer::mapCanvas(void)
{
  return mQgis->getMapCanvas();
}

QgsComposerView *QgsComposer::view(void)
{
  return mView;
}

/*QgsComposition *QgsComposer::composition(void)
{
  return mComposition;
  }*/

void QgsComposer::zoomFull(void)
{
  mView->fitInView(0, 0, mComposition->paperWidth(), mComposition->paperHeight(), Qt::KeepAspectRatio);
}

void QgsComposer::on_mActionZoomAll_activated(void)
{
  zoomFull();
}

void QgsComposer::on_mActionZoomIn_activated(void)
{
  mView->scale(2, 2);
  //mView->update();
}

void QgsComposer::on_mActionZoomOut_activated(void)
{
  mView->scale(.5, .5);
  //mView->update();
}

void QgsComposer::on_mActionRefreshView_activated(void)
{
  /*
  mComposition->refresh();
  mView->update();
  */
}

void QgsComposer::on_mActionPrint_activated(void)
{
  if(!mComposition)
    {
      return;
    }

  QPrinter printer;

  //try to set most of the print dialog settings based on composer properties
  if(mComposition->paperHeight() > mComposition->paperWidth())
    {
      printer.setOrientation(QPrinter::Portrait);
    }
  else
    {
      printer.setOrientation(QPrinter::Landscape);
    }

  //set resolution based on composer setting
  

  printer.setFullPage(true);
  printer.setColorMode(QPrinter::Color);
  
  QPrintDialog printDialog(&printer);
  
  if(printDialog.exec() == QDialog::Accepted)
    {
      QPainter p(&printer);

      QgsComposition::PlotStyle savedPlotStyle = mComposition->plotStyle();
      mComposition->setPlotStyle(QgsComposition::Print);

#if QT_VERSION < 0x040400
      QRectF paperRect(0, 0, mComposition->paperWidth(), mComposition->paperHeight());
      QRect pageRect = printer.pageRect();
      mComposition->render(&p, pageRect, paperRect);
#else
      //better in case of custom page size, but only possible with Qt>4.4
      QRectF paperRectMM = printer.pageRect(QPrinter::Millimeter);
      QRectF paperRectPixel = printer.pageRect(QPrinter::DevicePixel);
      mComposition->render(&p, paperRectPixel, paperRectMM);
#endif   
      mComposition->setPlotStyle(savedPlotStyle);
    }
#if 0
  /* Uff!!! It is impossible to set a custom page size for QPrinter.
   * Only the sizes hardcoded in Qt library can be used.
   * 'Fortunately', it seems that everything is written to postscript output file,
   * regardless the pages size ->
   *
   * 1) outputToFile == false: If the output page size doesn't match the user defined size 
   *                           (in QgsComposer) the output is rescaled and centered so that 
   *                           it fit to the select output page size. 
   *                           a warning is displayed, that the map was rescaled.
   *                           
   * 2) outputToFile == true:  the output postscript file is written (page size is not
   *                           important but bigger is better because it lefts enough space
   *                           in BoundingBox definition), then the file is reopened, 
   *                           and the BoundingBox is redefined.
   */

  // NOTE: QT 3.2 has QPrinter::setOptionEnabled but only for three options 
  if (!mPrinter)
  {

    mPrinter = new QPrinter(QPrinter::PrinterResolution);
    //mPrinter = new QPrinter ( QPrinter::HighResolution );
    //mPrinter = new QPrinter ( QPrinter::ScreenResolution );
    mPrinter->setFullPage(true);
#ifndef Q_OS_MACX
    // For Qt/Mac 3, don't set outputToFile to true before calling setup 
    // because it wiil suppress the Print dialog and output to file without
    // giving the user a chance to select a printer instead.
    // The Mac Print dialog provides an option to create a pdf which is
    // intended to be invisible to the application. If an eps is desired,
    // a custom Mac Print dialog is needed.

    // There is a bug in Qt<=4.2.2 (dialog is not correct) if output is set to file
    // => disable until they fix it
    //mPrinter->setOutputFileName ( QDir::convertSeparators ( QDir::home().path() + "/" + "qgis.eps") );
#endif
    mPrinter->setColorMode(QPrinter::Color);
    mPrinter->setPageSize(QPrinter::A4); //would be nice set this based on the composition paper size
  } else
  {
    // Because of bug in Qt<=4.2.2 (dialog is not correct) we have to reset always
    // to printer otherwise print to file is checked but printer combobox is in dialog
    mPrinter->setOutputFileName(NULL);
  }

  //set the resolution and paper orientation each time we call up the dialog, not just the first time we run it
  mPrinter->setResolution(mComposition->resolution());

  if (mComposition->paperOrientation() == QgsComposition::Portrait)
  {
    mPrinter->setOrientation(QPrinter::Portrait);
  } else
  {
    mPrinter->setOrientation(QPrinter::Landscape);
  }


  //if ( mPrinter->setup(this) ) {
  QPrintDialog printDialog(mPrinter, this);
  if (printDialog.exec() == QDialog::Accepted)
    {
      // TODO: mPrinter->setup() moves the composer under Qgisapp, get it to foreground somehow
      //       raise() for now, is it something better?
      raise();

      // TODO: Qt does not add pagesize to output file, it can cause problems if ps2pdf is used 
      // or if default page on printer is different.
      // We should add somewhere in output file:
      // << /PageSize [ %d %d ] >> setpagedevice
      // %d %d is width and height in points

      // WARNING: If QCanvasView recieves repaint signal during the printing
      // (e.g. covered by QPrinter::setup dialog) it breaks somehow drawing of QCanvas items 
      // (for example not all features in the map are drawn.
      // I don't know how to stop temporarily updating, (I don't want to reimplement 
      // repaint in QCanvasView, so I unset the view, print and reset.
      mView->setScene(0);

      int resolution = mPrinter->resolution();

      std::cout << "Resolution = " << resolution << std::endl;

      //double scale = resolution / 25.4 / mComposition->scale();

      mComposition->setPlotStyle(QgsComposition::Postscript);

      if (!mPrinter->outputFileName().isNull())
        {
          try
          {
            std::cout << "Print to file" << std::endl;

            QPrinter::PageSize psize=QPrinter::A4; //sensible default

            // WARNING mPrinter->outputFormat() returns always 0 in Qt 4.2.2
            // => we have to check extension
            bool isPs = false;
            if (mPrinter->outputFileName().right(3).toLower() == ".ps" || mPrinter->outputFileName().right(4).toLower() == ".eps")
              {
                isPs = true;
              }
            //if ( mPrinter->outputFormat() == QPrinter::PostScriptFormat )
            if (isPs)
              {
                // NOTE: setPageSize after setup() works, but setOrientation does not
                //   -> the BoundingBox must follow the orientation 

                psize = mPrinter->pageSize();
                // B0 ( 1000x1414mm = 2835x4008pt ) is the biggest defined in Qt, a map can be bigger 
                // but probably not bigger than 9999x9999pt = 3527x3527mm 
                mPrinter->setPageSize(QPrinter::B0);
              }

            QPainter p(mPrinter);
            //p.scale(scale, scale);

            //QRectF renderArea(0, 0, (mComposition->paperWidth() * mComposition->scale()),
	    //(mComposition->paperHeight() * mComposition->scale()));

            mComposition->canvas()->render(&p/*, renderArea*/);

            p.end();

            std::cout << "mPrinter->outputFormat() = " << mPrinter->outputFormat() << std::endl;


            //if ( mPrinter->outputFormat() == QPrinter::PostScriptFormat )
            if (isPs)
              {
                // reset the page
                mPrinter->setPageSize(psize);

                QFile f(mPrinter->outputFileName());

                // Overwrite the bounding box
                std::cout << "Overwrite the bounding box" << std::endl;
                if (!f.open(QIODevice::ReadWrite))
                  {
                    throw QgsIOException(tr("Couldn't open ") + f.name() + tr(" for read/write"));
                  }
                Q_LONG offset = 0;
                Q_LONG size;
                bool found = false;
                QString s;
                char buf[101];
                while (!f.atEnd())
                  {
                    size = f.readLine(buf, 100);
                    s = QString(buf);
                    if (s.find("%%BoundingBox:") == 0)
                      {
                        found = true;
                        break;
                      }
                    offset += size;
                  }

                if (found)
                  {
                    int w, h;

                    w = (int) (72 * mComposition->paperWidth() / 25.4);
                    h = (int) (72 * mComposition->paperHeight() / 25.4);
                    if (mPrinter->orientation() == QPrinter::Landscape)
                      {
                        int tmp = w;
                        w = h;
                        h = tmp;
                      }
                    s.sprintf("%%%%BoundingBox: 0 0 %d %d", w, h);

                    if (s.length() > size)
                      {
                        int shift = s.length() - size;
                        shiftFileContent(&f, offset + size + 1, shift);
                    } else
                      {
                        if (!f.at(offset))
                          {
                            QMessageBox::warning(this, tr("Error in Print"), tr("Cannot seek"));
                        } else
                          {
                            // Write spaces (for case the size > s.length() )
                            QString es;
                            es.fill(' ', size - 1);
                            f.flush();
                            if (f.writeBlock(es.toLocal8Bit().data(), size - 1) < size - 1)
                              {
                                QMessageBox::warning(this, tr("Error in Print"), tr("Cannot overwrite BoundingBox"));
                              }
                            f.flush();
                            f.at(offset);
                            f.flush();
                            if (f.writeBlock(s.toLocal8Bit().data(), s.length()) < s.length() - 1)
                              {
                                QMessageBox::warning(this, tr("Error in Print"), tr("Cannot overwrite BoundingBox"));
                              }
                            f.flush();
                          }     //END else (!f.at(offset))
                      }         //END else (s.length() > size)
                  }             //END if(found)
                else
                  {
                    QMessageBox::warning(this, tr("Error in Print"), tr("Cannot find BoundingBox"));
                  }
                f.close();

                // Overwrite translate
                if (mPrinter->orientation() == QPrinter::Portrait)
                  {
                    std::cout << "Orientation portraint -> overwrite translate" << std::endl;
                    if (!f.open(QIODevice::ReadWrite))
                      {
                        throw QgsIOException(tr("Couldn't open ") + f.name() + tr(" for read/write"));
                      }
                    offset = 0;
                    found = false;

                    //Example Qt3:
                    //0 4008 translate 1 -1 scale/defM ...
                    //QRegExp rx ( "^0 [^ ]+ translate ([^ ]+ [^ ]+) scale/defM matrix CM d \\} d" );
                    //Example Qt4:
                    //0 0 translate 0.239999 -0.239999 scale } def
                    QRegExp rx("^0 [^ ]+ translate ([^ ]+ [^ ]+) scale \\} def");

                    while (!f.atEnd())
                      {
                        size = f.readLine(buf, 100);
                        s = QString(buf);
                        if (rx.search(s) != -1)
                          {
                            found = true;
                            break;
                          }
                        offset += size;
                      }         //END while( !f.atEnd() )

                    if (found)
                      {
                        int trans;

                        trans = (int) (72 * mComposition->paperHeight() / 25.4);
                        std::cout << "trans = " << trans << std::endl;
                        //Qt3:
                        //s.sprintf( "0 %d translate %s scale/defM matrix CM d } d", trans, (const char *)rx.cap(1).toLocal8Bit().data() );
                        //Qt4:
                        s.sprintf("0 %d translate %s scale } def\n", trans, (const char *) rx.cap(1).toLocal8Bit().data());

                        std::cout << "s.length() = " << s.length() << " size = " << size << std::endl;
                        if (s.length() > size)
                          {
                            //QMessageBox::warning(this, tr("Error in Print"), tr("Cannot format translate"));
                            // Move the content up
                            int shift = s.length() - size;
                            /*
                               int last = f.size() + shift -1;
                               for ( int i = last; i > offset + size; i-- )
                               {
                               f.at(i-shift);
                               QByteArray ba = f.read(1);
                               f.at(i);
                               f.write(ba);
                               }
                             */
                            shiftFileContent(&f, offset + size + 1, shift);
                          }     //END if( s.length() > size)

                        // Overwrite the row
                        if (!f.at(offset))
                          {
                            QMessageBox::warning(this, tr("Error in Print"), tr("Cannot seek"));
                        } else
                          {
                            /* Write spaces (for case the size > s.length() ) */
                            QString es;
                            es.fill(' ', size - 1);
                            f.flush();
                            if (f.writeBlock(es.toLocal8Bit().data(), size - 1) < size - 1)
                              {
                                QMessageBox::warning(this, tr("Error in Print"), tr("Cannot overwrite translate"));
                              }
                            f.flush();
                            f.at(offset);
                            f.flush();
                            if (f.writeBlock(s.toLocal8Bit().data(), s.length()) < s.length() - 1)
                              {
                                QMessageBox::warning(this, tr("Error in Print"), tr("Cannot overwrite translate"));
                              }
                            f.flush();
                          }     //END else
                    } else
                      {
                        QMessageBox::warning(this, tr("Error in Print"), tr("Cannot find translate"));
                      }
                    f.close();
                  }
              }
          }
          catch(QgsIOException e)
          {
            QMessageBox::warning(this, tr("File IO Error"), e.what());
          }
      } else
        {                       // print to printer
          bool print = true;

          // Check size 
          std::cout << "Paper: " << mPrinter->widthMM() << " x " << mPrinter->heightMM() << std::endl;
          if (mComposition->paperWidth() != mPrinter->widthMM() || mComposition->paperHeight() != mPrinter->heightMM())
            {
              int answer = QMessageBox::warning(0, tr("Paper does not match"),
                                                tr("The selected paper size does not match the composition size"),
                                                QMessageBox::Ok, QMessageBox::Abort);

              if (answer == QMessageBox::Abort)
                {
                  print = false;
                }
            }                   //END if(compositionSize != paperSize)

          if (print)
            {
              std::cout << "Printing ... " << std::endl;
              QPainter p(mPrinter);
              //p.scale(scale, scale);

	      //MH: is this necessary?
              //QRectF renderArea(0, 0, (mComposition->paperWidth() * mComposition->scale()),
	      //(mComposition->paperHeight() * mComposition->scale()));

              mComposition->canvas()->render(&p/*, renderArea*/);

              p.end();
              std::cout << "... printing finished" << std::endl;
            }                   //END if ( print )
        }

      mComposition->setPlotStyle(QgsComposition::Preview);
      mView->setScene(mComposition->canvas());
  } else
    {
      raise();
    }
#endif //0
}


bool QgsComposer::shiftFileContent ( QFile *file, Q_LONG start, int shift )
{
	int last = file->size() + shift -1;
	for ( int i = last; i >= start + shift; i-- )
	{
            if ( !file->at(i-shift) ) return false;
	    QByteArray ba = file->read(1);
	    if ( ba.isEmpty() ) return false;
	    if ( !file->at(i) ) return false;
	    if ( file->write(ba) != 1 ) return false;
	}
	return true;
}

void QgsComposer::on_mActionExportAsImage_activated(void)
{

#if 0
  // Image size 
  int width = (int) (mComposition->resolution() * mComposition->paperWidth() / 25.4); 
  int height = (int) (mComposition->resolution() * mComposition->paperHeight() / 25.4); 

  int memuse = width * height * 3 / 1000000;  // pixmap + image
#ifdef QGISDEBUG
  std::cout << "Image " << width << " x " << height << std::endl;
  std::cout << "memuse = " << memuse << std::endl;
#endif

  if ( memuse > 200 ) { // cca 4500 x 4500
    int answer = QMessageBox::warning ( 0, tr("Big image"), 
        tr("To create image ") + QString::number(width) + " x " 
        + QString::number(height) 
        + tr(" requires circa ") 
        + QString::number(memuse) + tr(" MB of memory"), 
        QMessageBox::Ok,  QMessageBox::Abort );
  
    raise ();
    if ( answer == QMessageBox::Abort ) return;
  }

  // Get file and format (stolen from qgisapp.cpp but modified significantely)

  //create a map to hold the QImageIO names and the filter names
  //the QImageIO name must be passed to the mapcanvas saveas image function
  typedef QMap<QString, QString> FilterMap;
  FilterMap myFilterMap;

  //find out the last used filter
  QSettings myQSettings;  // where we keep last used filter in persistant state
  QString myLastUsedFormat = myQSettings.readEntry("/UI/lastSaveAsImageFormat", "png" );
  QString myLastUsedFile = myQSettings.readEntry("/UI/lastSaveAsImageFile","qgis.png");
  QFileInfo file(myLastUsedFile);

  // get a list of supported output image types
  int myCounterInt=0;
  QString myFilters;
  QString myLastUsedFilter;
  for ( ; myCounterInt < QImageWriter::supportedImageFormats().count(); myCounterInt++ )
  {
    QString myFormat=QString(QImageWriter::supportedImageFormats().at( myCounterInt ));
    QString myFilter = myFormat + " " + tr("format") + " (*." + myFormat.lower() + " *." + myFormat.upper() + ")";

    if ( myCounterInt > 0 ) myFilters += ";;";
    myFilters += myFilter;
    myFilterMap[myFilter] = myFormat;
    if ( myFormat == myLastUsedFormat ) 
    { 
      myLastUsedFilter = myFilter;
    }
  }
#ifdef QGISDEBUG
  std::cout << "Available Filters Map: " << std::endl;
  FilterMap::Iterator myIterator;
  for ( myIterator = myFilterMap.begin(); myIterator != myFilterMap.end(); ++myIterator )
  {
    std::cout << myIterator.key().toLocal8Bit().data() << "  :  " << myIterator.data().toLocal8Bit().data() << std::endl;
  }
#endif

  //create a file dialog using the the filter list generated above
  std::auto_ptr < QFileDialog > myQFileDialog(
      new QFileDialog(
        this,
        tr("Choose a filename to save the map image as"),
        file.path(),
        myFilters
        )
      );
  myQFileDialog->selectFile( file.fileName() );

  // allow for selection of more than one file
  myQFileDialog->setMode(QFileDialog::AnyFile);

  // set the filter to the last one used
  myQFileDialog->selectFilter(myLastUsedFilter);

  // set the 'Open' button to something that makes more sense
  myQFileDialog->setAcceptMode(QFileDialog::AcceptSave);

  //prompt the user for a filename
  QString myOutputFileNameQString; // = myQFileDialog->getSaveFileName(); //delete this

  int result = myQFileDialog->exec();
  raise ();
  
  if ( result != QDialog::Accepted) return;

  myOutputFileNameQString = myQFileDialog->selectedFiles().first();
  QString myFilterString = myQFileDialog->selectedFilter();
#ifdef QGISDEBUG
  std::cout << "Selected filter: " << myFilterString.toLocal8Bit().data() << std::endl;
  std::cout << "Image type: " << myFilterMap[myFilterString].toLocal8Bit().data() << std::endl;
#endif

  myQSettings.writeEntry("/UI/lastSaveAsImageFormat" , myFilterMap[myFilterString] );
  myQSettings.writeEntry("/UI/lastSaveAsImageFile", myOutputFileNameQString);

  if ( myOutputFileNameQString == "" ) return;

  double scale = (double) (mComposition->resolution() / 25.4 / mComposition->scale());

  mView->setScene(0);
  mComposition->setPlotStyle ( QgsComposition::Print );

  QPixmap pixmap ( width, height );
  pixmap.fill ( QColor(255,255,255) ) ;
  QPainter p(&pixmap);
  p.scale ( scale, scale);

QRectF renderArea(0,0,(mComposition->paperWidth() * mComposition->scale()),(mComposition->paperHeight() * mComposition->scale()));

  mComposition->canvas()->render(&p, renderArea);
  p.end();

  mComposition->setPlotStyle ( QgsComposition::Preview );
  mView->setScene(mComposition->canvas());

  pixmap.save ( myOutputFileNameQString, myFilterMap[myFilterString].toLocal8Bit().data() );
#endif //0
}


void QgsComposer::on_mActionExportAsSVG_activated(void)
{
  QString myQSettingsLabel = "/UI/displaySVGWarning";
  QSettings myQSettings;

  bool displaySVGWarning = myQSettings.value(myQSettingsLabel, true).toBool();

  if (displaySVGWarning)
  {
    QgsMessageViewer* m = new QgsMessageViewer(this);
    m->setWindowTitle(tr("SVG warning"));
    m->setCheckBoxText(tr("Don't show this message again"));
    m->setCheckBoxState(Qt::Unchecked);
    m->setCheckBoxVisible(true);
    m->setCheckBoxQSettingsLabel(myQSettingsLabel);
    m->setMessageAsHtml(tr("<p>The SVG export function in Qgis has several "
                           "problems due to bugs and deficiencies in the "
#if QT_VERSION < 0x040300
                           "Qt4 svg code. Of note, text does not "
                           "appear in the SVG file and there are problems "
                           "with the map bounding box clipping other items "
                           "such as the legend or scale bar.</p>"
#else
                           "Qt4 svg code. In particular, there are problems "
                           "with layers not being clipped to the map "
                           "bounding box.</p>"
#endif
                           "If you require a vector-based output file from "
                           "Qgis it is suggested that you try printing "
                           "to PostScript if the SVG output is not "
                           "satisfactory."
                           "</p>"));
    m->exec();
  }
  QString myLastUsedFile = myQSettings.readEntry("/UI/lastSaveAsSvgFile","qgis.svg");
  QFileInfo file(myLastUsedFile);
  QFileDialog *myQFileDialog = new QFileDialog( this, tr("Choose a filename to save the map as"),
                                                file.path(), tr("SVG Format") + " (*.svg *SVG)" );
  myQFileDialog->selectFile( file.fileName() );
  myQFileDialog->setMode(QFileDialog::AnyFile);
  myQFileDialog->setAcceptMode(QFileDialog::AcceptSave);

  int result = myQFileDialog->exec();
  raise ();
  if ( result != QDialog::Accepted) return;

  QString myOutputFileNameQString = myQFileDialog->selectedFiles().first();
  if ( myOutputFileNameQString == "" ) return;

  myQSettings.writeEntry("/UI/lastSaveAsSvgFile", myOutputFileNameQString);

  //mView->setScene(0);//don't redraw the scene on the display while we render
  mComposition->setPlotStyle ( QgsComposition::Print );

#if QT_VERSION < 0x040300
  Q3Picture pic;
  QPainter p(&pic);
  QRectF renderArea(0,0, (mComposition->paperWidth() * mComposition->scale()), (mComposition->paperHeight() * mComposition->scale()) );
#else
  QSvgGenerator generator;
  generator.setFileName(myOutputFileNameQString);
  generator.setSize(QSize( (int)mComposition->paperWidth(), (int)mComposition->paperHeight() ));
  generator.setResolution(25.4); //because the rendering is done in mm, convert the dpi

  QPainter p(&generator);
  QRectF renderArea(0,0, mComposition->paperWidth(), mComposition->paperHeight());
#endif
  mComposition->render(&p, renderArea, renderArea);
  p.end();

  mComposition->setPlotStyle ( QgsComposition::Preview );
  //mView->setScene(mComposition->canvas()); //now that we're done, set the view to show the scene again

#if QT_VERSION < 0x040300
  QRect br = pic.boundingRect();

  pic.save ( myOutputFileNameQString, "svg" );
#endif
}


void QgsComposer::setToolActionsOff(void)
{
#if 0
  mActionOpenTemplate->setOn ( false );
  mActionSaveTemplateAs->setOn ( false );
  mActionExportAsImage->setOn ( false );
  mActionExportAsSVG->setOn ( false );
  mActionPrint->setOn ( false );
  mActionZoomAll->setOn ( false );
  mActionZoomIn->setOn ( false );
  mActionZoomOut->setOn ( false );
  mActionRefreshView->setOn ( false );
  mActionAddNewMap->setOn ( false );
  mActionAddImage->setOn ( false );
  mActionAddNewLabel->setOn ( false );
  mActionAddNewVectLegend->setOn ( false );
  mActionAddNewScalebar->setOn ( false );
  mActionSelectMoveItem->setOn ( false );
#endif //0
}

void QgsComposer::selectItem(void)
{
#if 0
  mComposition->setTool ( QgsComposition::Select );
  setToolActionsOff();
  mActionSelectMoveItem->setOn ( true );
#endif //0
}

void QgsComposer::on_mActionSelectMoveItem_activated(void)
{
  if(mView)
    {
      mView->setCurrentTool(QgsComposerView::Select);
    }
}

void QgsComposer::on_mActionAddNewMap_activated(void)
{
  if(mView)
    {
      mView->setCurrentTool(QgsComposerView::AddMap);
    }
}

void QgsComposer::on_mActionAddNewLegend_activated(void)
{
  if(mView)
    {
      mView->setCurrentTool(QgsComposerView::AddLegend);
    }
}

void QgsComposer::on_mActionAddNewLabel_activated(void)
{
  if(mView)
    {
      mView->setCurrentTool(QgsComposerView::AddLabel);
    }
}

void QgsComposer::on_mActionAddNewScalebar_activated(void)
{
  if(mView)
    {
      mView->setCurrentTool(QgsComposerView::AddScalebar);
    }
}

void QgsComposer::on_mActionAddImage_activated(void)
{
#if 0
  mComposition->setTool ( QgsComposition::AddPicture );
  setToolActionsOff();
  mActionAddImage->setOn ( true );
  mView->setCursor(QCursor(cross_hair_cursor));
#endif //0
}

void QgsComposer::moveItemContent()
{
  if(mView)
    {
      mView->setCurrentTool(QgsComposerView::MoveItemContent);
    }
}
 
void QgsComposer::groupItems(void)
{
  if(mView)
    {
      mView->groupItems();
    }
}

void QgsComposer::ungroupItems(void)
{
  if(mView)
    {
      mView->ungroupItems();
    }
}

void QgsComposer::moveEvent ( QMoveEvent *e ) { saveWindowState(); }

void QgsComposer::resizeEvent ( QResizeEvent *e )
{
  // Move size grip when window is resized
  mSizeGrip->move(rect().bottomRight() - mSizeGrip->rect().bottomRight());

  saveWindowState();
}

void QgsComposer::saveWindowState()
{
  QSettings settings;
  settings.setValue("/Composer/geometry", saveGeometry());
  settings.setValue("/Composer/splitterState", mSplitter->saveState());
}

void QgsComposer::restoreWindowState()
{
  QSettings settings;
  restoreGeometry(settings.value("/Composer/geometry").toByteArray());
  QVariant splitterState = settings.value("/Composer/splitterState");
  if (splitterState != QVariant::QVariant())
    mSplitter->restoreState(settings.value("/Composer/splitterState").toByteArray());
  else
  {
    QList<int> defaultSize;
    defaultSize << 300 << 100; // page display 300 pixels, details pane 100 pixels
    mSplitter->setSizes(defaultSize);
  }
}

void QgsComposer::on_helpPButton_clicked()
{
  QgsContextHelp::run(context_id);
}

void QgsComposer::on_closePButton_clicked()
{
  close();
}

void QgsComposer::projectRead(void)
{
#ifdef QGISDEBUG
  std::cout << "QgsComposer::projectRead" << std::endl;
#endif
  //if ( mComposition ) delete mComposition;
  //mComposition  = new QgsComposition( this, 1 );

  // Read composition if it is defined in project
  QStringList l = QgsProject::instance()->subkeyList ( "Compositions", "" );

  bool found = false;
  for ( QStringList::iterator it = l.begin(); it != l.end(); ++it ) {
    std::cout << "key: " << (*it).toLocal8Bit().data() << std::endl;
    if ( (*it).compare ( "composition_1" ) == 0 ) {
      found = true;
      break;
    }
  }

  if ( found ) {
    //mComposition->readSettings ( );
    mFirstTime = false;
  } else { 
    if ( isVisible() ) {
      //mComposition->createDefault();
      mFirstTime = false;
    } else {
      mFirstTime = true;
    }
  }

  //mComposition->setActive ( true );
}

void QgsComposer::newProject(void)
{
#ifdef QGISDEBUG
  std::cout << "QgsComposer::newProject" << std::endl;
#endif
  //if ( mComposition ) delete mComposition;

  //mComposition  = new QgsComposition( this, 1 );
  //mComposition->setActive ( true );

  // If composer is visible, create default immediately, otherwise wait for the first open()
  if ( isVisible() ) {
    //mComposition->createDefault();
    mFirstTime = false;
  } else {
    mFirstTime = true;
  }
}

bool QgsComposer::writeSettings ( void )
{
#ifdef WIN32
  bool ok = true;
#else
  bool ok = false;
#endif
  return ok;
}

bool QgsComposer::readSettings ( void )
{
#ifdef WIN32
  bool ok = true;
#else
  bool ok = false;
#endif
  return ok;
}

void  QgsComposer::writeXML(QDomDocument& doc)
{
  QDomNodeList nl = doc.elementsByTagName("qgis");
  if (nl.count() < 1)
    {
      return;
    }
  QDomElement qgisElem = nl.at(0).toElement();
  if(qgisElem.isNull())
    {
      return;
    }
  
  QDomElement composerElem = doc.createElement("Composer");
  qgisElem.appendChild( composerElem );

  //store composer items:
  QMap<QgsComposerItem*, QWidget*>::const_iterator itemIt = mItemWidgetMap.constBegin();
  for(; itemIt != mItemWidgetMap.constEnd(); ++itemIt)
    {
      itemIt.key()->writeXML(composerElem, doc);
    }

  //store composer view

  //store composition
  if(mComposition)
    {
      mComposition->writeXML(composerElem, doc);
    }


  return;
}

void QgsComposer::readXML(const QDomDocument& doc)
{
  //delete composer view and composition
  delete mView;
  mView = 0;
  //delete every child of mViewFrame
  QObjectList viewFrameChildren = mViewFrame->children();
  QObjectList::iterator it = viewFrameChildren.begin();
  for(; it != viewFrameChildren.end(); ++it)
    {
      delete (*it);
    }

  //delete composition widget
  QgsCompositionWidget* oldCompositionWidget = dynamic_cast<QgsCompositionWidget*>(mCompositionOptionsFrame->children().at(0));
  delete oldCompositionWidget;
  delete mCompositionOptionsLayout;
  mCompositionOptionsLayout = 0;

  //look for Composer element

  QDomNodeList nl = doc.elementsByTagName("Composer");
  if(nl.size() < 1)
    {
      return; //nothing to do...
    }
  QDomElement composerElem = nl.at(0).toElement();

  //look for Composition element
  QDomNodeList cnl = composerElem.elementsByTagName("Composition");
  if(cnl.size() > 0)
    {
      QDomElement compositionElem = cnl.at(0).toElement();

      //todo: move in function because duplicated code with constructor
      mView = new QgsComposerView(mViewFrame);
      connect(mView, SIGNAL(selectedItemChanged(const QgsComposerItem*)), this, SLOT(showItemOptions(const QgsComposerItem*)));
      connect(mView, SIGNAL(composerLabelAdded(QgsComposerLabel*)), this, SLOT(addComposerLabel(QgsComposerLabel*)));
      connect(mView, SIGNAL(composerMapAdded(QgsComposerMap*)), this, SLOT(addComposerMap(QgsComposerMap*)));
      connect(mView, SIGNAL(itemRemoved(QgsComposerItem*)), this, SLOT(deleteItem(QgsComposerItem*)));
      connect(mView, SIGNAL(composerScaleBarAdded(QgsComposerScaleBar*)), this, SLOT(addComposerScaleBar(QgsComposerScaleBar*)));
      connect(mView, SIGNAL(composerLegendAdded(QgsComposerLegend*)), this, SLOT(addComposerLegend(QgsComposerLegend*)));

      mComposition = new QgsComposition(mQgis->getMapCanvas());
      mComposition->readXML(compositionElem, doc);

      QGridLayout *l = new QGridLayout(mViewFrame );
      l->setMargin(0);
      l->addWidget( mView, 0, 0 );

      //create compositionwidget
      QgsCompositionWidget* compositionWidget = new QgsCompositionWidget(mCompositionOptionsFrame, mComposition);
      compositionWidget->show();

      mCompositionOptionsLayout = new QGridLayout( mCompositionOptionsFrame );
      mCompositionOptionsLayout->setMargin(0);
      mCompositionOptionsLayout->addWidget(compositionWidget);

      //read and restore all the items
      
      //composer labels
      QDomNodeList composerLabelList = composerElem.elementsByTagName("ComposerLabel");
      for(int i = 0; i < composerLabelList.size(); ++i)
	{
	  QDomElement currentComposerLabelElem = composerLabelList.at(i).toElement();
	  QgsComposerLabel* newLabel = new QgsComposerLabel(mComposition);
	  newLabel->readXML(currentComposerLabelElem, doc);
	  addComposerLabel(newLabel);
	  mComposition->addItem(newLabel);
	  mComposition->update();
	}

      //composer maps
      QDomNodeList composerMapList = composerElem.elementsByTagName("ComposerMap");
      for(int i = 0; i < composerMapList.size(); ++i)
	{
	  QDomElement currentComposerMapElem = composerMapList.at(i).toElement();
	  QgsComposerMap* newMap = new QgsComposerMap(mComposition);
	  newMap->readXML(currentComposerMapElem, doc);
	  addComposerMap(newMap);
	  mComposition->addItem(newMap);
	  mComposition->update();
	}

      //composer scalebars
      QDomNodeList composerScaleBarList = composerElem.elementsByTagName("ComposerScaleBar");
      for(int i = 0; i < composerScaleBarList.size(); ++i)
	{
	  QDomElement currentScaleBarElem = composerScaleBarList.at(i).toElement();
	  QgsComposerScaleBar* newScaleBar = new QgsComposerScaleBar(mComposition);
	  newScaleBar->readXML(currentScaleBarElem, doc);
	  addComposerScaleBar(newScaleBar);
	  mComposition->addItem(newScaleBar);
	  mComposition->update();
	}

      //composer legends
      QDomNodeList composerLegendList = composerElem.elementsByTagName("ComposerLegend");
      for(int i = 0; i < composerLegendList.size(); ++i)
	{
	  QDomElement currentLegendElem = composerLegendList.at(i).toElement();
	  QgsComposerLegend* newLegend = new QgsComposerLegend(mComposition);
	  newLegend->readXML(currentLegendElem, doc);
	  addComposerLegend(newLegend);
	  mComposition->addItem(newLegend);
	  mComposition->update();
	}
    }

  mView->setComposition(mComposition);
}

void QgsComposer::addComposerMap(QgsComposerMap* map)
{
  if(!map)
    {
      return;
    }

  QgsComposerMapWidget* mapWidget= new QgsComposerMapWidget(map);
  mItemWidgetMap.insert(map, mapWidget);
}

void QgsComposer::addComposerLabel(QgsComposerLabel* label)
{
  if(!label)
    {
      return;
    }
  
  QgsComposerLabelWidget* labelWidget = new QgsComposerLabelWidget(label);
  mItemWidgetMap.insert(label, labelWidget);
}

void QgsComposer::addComposerScaleBar(QgsComposerScaleBar* scalebar)
{
  if(!scalebar)
    {
      return;
    }
  
  QgsComposerScaleBarWidget* sbWidget = new QgsComposerScaleBarWidget(scalebar);
  mItemWidgetMap.insert(scalebar, sbWidget);
}

void QgsComposer::addComposerLegend(QgsComposerLegend* legend)
{
  if(!legend)
    {
      return;
    }
  
  QgsComposerLegendWidget* lWidget = new QgsComposerLegendWidget(legend);
  mItemWidgetMap.insert(legend, lWidget);
}

void QgsComposer::deleteItem(QgsComposerItem* item)
{
  QMap<QgsComposerItem*, QWidget*>::iterator it = mItemWidgetMap.find(item);

  if(it == mItemWidgetMap.end())
    {
      return;
    }

  delete (it.value());
  mItemWidgetMap.remove(it.key());
}
