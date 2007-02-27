/***************************************************************************
                         qgsdiagramoverlay.h  -  description
                         -------------------
    begin                : January 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDIAGRAMOVERLAY_H
#define QGSDIAGRAMOVERLAY_H

#include "qgsvectoroverlay.h"

class QgsDiagramRenderer;

/**An overlay class that places diagrams on a vectorlayer*/
class QgsDiagramOverlay: public QgsVectorOverlay
{
 public:
  QgsDiagramOverlay(QgsVectorLayer* vl);
  ~QgsDiagramOverlay();
  void createOverlayObjects(const QgsRect& viewExtent);
  void drawOverlayObjects(QPainter * p, const QgsRect& viewExtent, QgsMapToPixel * cXf, QgsCoordinateTransform* ct) const;

  //setters and getters
  QString name() const{return "diagram";}
  void setDiagramRenderer(QgsDiagramRenderer* r);
  const QgsDiagramRenderer* diagramRenderer() const {return mDiagramRenderer;}

  bool readXML(const QDomNode& overlayNode);
  bool writeXML(QDomNode& layer_node, QDomDocument& doc) const;
  int createLegendContent(std::list<std::pair<QString, QImage*> >& content) const;

   /**Helper function that returns the attribute index from an attribute name*/
  static int indexFromAttributeName(const QString& name, const QgsVectorLayer* vl);
  /**Helper function that returns the attribute name from an attribute index*/
  static QString attributeNameFromIndex(int index, const QgsVectorLayer* vl);

protected:
  int getOverlayObjectSize(int& width, int& height, double value, const QgsFeature& f) const;
  
 private:
  /**Does the classification and manages the diagram generation*/
  QgsDiagramRenderer* mDiagramRenderer;
};

#endif
