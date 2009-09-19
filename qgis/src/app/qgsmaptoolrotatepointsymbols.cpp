/***************************************************************************
    qgsmaptoolrotatepointsymbols.cpp
    --------------------------------
    begin                : September 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolrotatepointsymbols.h"
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgspointrotationitem.h"
#include "qgsrenderer.h"
#include "qgssymbol.h"
#include "qgsvectorlayer.h"
#include <QGraphicsPixmapItem>
#include <QMessageBox>
#include <QMouseEvent>

QgsMapToolRotatePointSymbols::QgsMapToolRotatePointSymbols( QgsMapCanvas* canvas ): QgsMapToolEdit( canvas ), \
    mActiveLayer( 0 ), mFeatureNumber( 0 ), mCurrentMouseAzimut( 0.0 ), mCurrentRotationFeature( 0.0 ), mRotating( false ), mRotationItem( 0 )
{

}

QgsMapToolRotatePointSymbols::~QgsMapToolRotatePointSymbols()
{
  delete mRotationItem;
}

bool QgsMapToolRotatePointSymbols::layerIsRotatable( QgsMapLayer* ml )
{
  if ( !ml )
  {
    return false;
  }

  //a vector layer
  QgsVectorLayer* vLayer = dynamic_cast<QgsVectorLayer*>( ml );
  if ( !vLayer )
  {
    return false;
  }

  //does it have point or multipoint type?
  if ( vLayer->geometryType() != QGis::Point )
  {
    return false;
  }

  //does it have a least one rotation attribute?
  QList<int> rotationAttributes;
  layerRotationAttributes( vLayer, rotationAttributes );
  if ( rotationAttributes.size() < 1 )
  {
    return false;
  }
  return true;
}

void QgsMapToolRotatePointSymbols::canvasPressEvent( QMouseEvent * e )
{
  if ( !mCanvas )
  {
    return;
  }

  mActiveLayer = currentVectorLayer();
  if ( !mActiveLayer )
  {
    return;
  }

  if ( mActiveLayer->geometryType() != QGis::Point || !mActiveLayer->isEditable() )
  {
    return;
  }

  //find the closest feature to the pressed position
  QgsMapCanvasSnapper canvasSnapper( mCanvas );
  QList<QgsSnappingResult> snapResults;
  if ( canvasSnapper.snapToCurrentLayer( e->pos(), snapResults, QgsSnapper::SnapToVertex, -1 ) != 0 || snapResults.size() < 1 )
  {
    QMessageBox::critical( 0, tr( "No point feature" ), tr( "No point feature was detected at the clicked position. Please click closer to the feature or enhance the search tolerance under Settings->Options->Digitizing->Serch radius for vertex edits" ) );
    return; //error during snapping
  }

  mFeatureNumber = snapResults.at( 0 ).snappedAtGeometry;

  //get list with renderer rotation attributes
  if ( layerRotationAttributes( mActiveLayer, mCurrentRotationAttributes ) != 0 )
  {
    return;
  }

  if ( mCurrentRotationAttributes.size() < 1 )
  {
    QMessageBox::critical( 0, tr( "No rotation Attributes" ), tr( "The active point layer does not have a rotation attribute" ) );
    return;
  }

  mSnappedPoint = toCanvasCoordinates( snapResults.at( 0 ).snappedVertex );

  //find out initial arrow direction
  QgsFeature pointFeature;
  if ( !mActiveLayer->featureAtId( mFeatureNumber, pointFeature, false, true ) )
  {
    return;
  }
  const QgsAttributeMap pointFeatureAttributes = pointFeature.attributeMap();
  const QgsAttributeMap::const_iterator attIt = pointFeatureAttributes.find( mCurrentRotationAttributes.at( 0 ) );
  if ( attIt == pointFeatureAttributes.constEnd() )
  {
    return;
  }

  mCurrentRotationFeature = attIt.value().toDouble();
  createPixmapItem();
  if ( mRotationItem )
  {
    mRotationItem->setPointLocation( snapResults.at( 0 ).snappedVertex );
  }
  mCurrentMouseAzimut = calculateAzimut( e->pos() );
  setPixmapItemRotation( (int)(mCurrentMouseAzimut) );
  mRotating = true;
}

void QgsMapToolRotatePointSymbols::canvasMoveEvent( QMouseEvent * e )
{
  if ( !mRotating )
  {
    return;
  }

  double azimut = calculateAzimut( e->pos() );
  double azimutDiff = azimut - mCurrentMouseAzimut;

  //assign new feature rotation, making sure to respect the 0 - 360 degree range
  mCurrentRotationFeature += azimutDiff;
  if ( mCurrentRotationFeature < 0 )
  {
    mCurrentRotationFeature = 360 - mCurrentRotationFeature;
  }
  else if ( mCurrentRotationFeature >= 360 )
  {
    mCurrentRotationFeature -= 360;
  }
  mCurrentMouseAzimut = azimut;
  if ( mCurrentMouseAzimut < 0 )
  {
    mCurrentMouseAzimut = 360 - mCurrentMouseAzimut;
  }
  else if ( mCurrentMouseAzimut >= 360 )
  {
    mCurrentMouseAzimut -= 360;
  }
  setPixmapItemRotation( (int)(mCurrentRotationFeature) );
}

void QgsMapToolRotatePointSymbols::canvasReleaseEvent( QMouseEvent * e )
{
  if ( mRotating && mActiveLayer )
  {
    mActiveLayer->beginEditCommand( tr( "Rotate symbol" ) );
    bool rotateSuccess = true;

    //write mCurrentRotationFeature to all rotation attributes of feature (mFeatureNumber)
    QList<int>::const_iterator it = mCurrentRotationAttributes.constBegin();
    for ( ; it != mCurrentRotationAttributes.constEnd(); ++it )
    {
      if ( !mActiveLayer->changeAttributeValue( mFeatureNumber, *it, (int)(mCurrentRotationFeature), true ) )
      {
        rotateSuccess = false;
      }
    }

    if ( rotateSuccess )
    {
      mActiveLayer->endEditCommand();
    }
    else
    {
      mActiveLayer->destroyEditCommand();
    }
  }
  mRotating = false;
  delete mRotationItem;
  mRotationItem = 0;
  mCanvas->refresh();
}

int QgsMapToolRotatePointSymbols::layerRotationAttributes( const QgsVectorLayer* vl, QList<int>& attList )
{
  attList.clear();
  if ( !vl )
  {
    return 1;
  }

  //get renderer
  const QgsRenderer* layerRenderer = vl->renderer();
  if ( !layerRenderer )
  {
    return 2;
  }

  //get renderer symbols
  const QList<QgsSymbol*> rendererSymbols = layerRenderer->symbols();
  int currentRotationAttribute;

  QList<QgsSymbol*>::const_iterator symbolIt = rendererSymbols.constBegin();
  for ( ; symbolIt != rendererSymbols.constEnd(); ++symbolIt )
  {
    currentRotationAttribute = ( *symbolIt )->rotationClassificationField();
    if ( currentRotationAttribute >= 0 )
    {
      attList.push_back( currentRotationAttribute );
    }
  }
  return 0;
}

double QgsMapToolRotatePointSymbols::calculateAzimut( const QPoint& mousePos )
{
  int dx = mousePos.x() - mSnappedPoint.x();
  int dy = mousePos.y() - mSnappedPoint.y();
  return 180 - atan2( (double) dx, (double) dy ) * 180.0 / M_PI;
}

void QgsMapToolRotatePointSymbols::createPixmapItem()
{
  delete mRotationItem;
  mRotationItem = new QgsPointRotationItem( mCanvas );
  mRotationItem->setSymbol( QgsApplication::defaultThemePath() + "mActionArrowUp.png" );
  mCanvas->scene()->addItem( mRotationItem );
}

void QgsMapToolRotatePointSymbols::setPixmapItemRotation( double rotation )
{
  mRotationItem->setSymbolRotation( rotation );
  mRotationItem->update();
}

