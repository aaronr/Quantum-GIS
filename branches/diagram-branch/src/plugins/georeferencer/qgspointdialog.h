/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef QGSPOINTDIALOG_H
#define QGSPOINTDIALOG_H

#include <vector>

#include <qcursor.h>

#include <qgsmapcanvas.h>

#include "qgsrasterlayer.h"

#include <ui_qgspointdialogbase.h>
#include <QDialog>

class QAction;
class QActionGroup;
class QgsGeorefDataPoint;
class QgisIface;

class QgsPointDialog : public QDialog, private Ui::QgsPointDialogBase
{
Q_OBJECT
public:
  QgsPointDialog(QString layerPath, QgisIface* theQgisInterface,
                 QWidget* parent = 0, Qt::WFlags fl = 0);
  ~QgsPointDialog();

  void showCoordDialog(QgsPoint& pixelCoords);
  void deleteDataPoint(QgsPoint& pixelCoords);

public slots:
  
  void addPoint(const QgsPoint& pixelCoords, const QgsPoint& mapCoords);
  void on_pbnCancel_clicked();
  void on_pbnGenerateWorldFile_clicked();
  void on_pbnGenerateAndLoad_clicked();
  void on_pbnSelectWorldFile_clicked();
  void on_pbnSelectModifiedRaster_clicked();
  void zoomIn();
  void zoomOut();
  void zoomToLayer();
  void pan();
  void addPoint();
  void deletePoint();
  void enableRelevantControls(void);
   
private:

  bool generateWorldFile();
  QString guessWorldFileName(const QString& raster);
  
  QActionGroup* mMapToolGroup;
  QAction* mActionZoomIn;
  QAction* mActionZoomOut;
  QAction* mActionZoomToLayer;
  QAction* mActionPan;
  QAction* mActionAddPoint;
  QAction* mActionDeletePoint;
  
  QgsMapCanvas* mCanvas;
  QgsRasterLayer* mLayer;
  
//  std::vector<QgsPoint> mPixelCoords, mMapCoords;
//  std::vector<QString> mAcetateIDs;
  std::vector<QgsGeorefDataPoint*> mPoints;
  QgisIface* mIface;
};

#endif
