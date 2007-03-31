/***************************************************************************
                          qgsvectorfilewriter.cpp  
                          generic vector file writer 
                             -------------------
    begin                : Sat Jun 16 2004
    copyright            : (C) 2004 by Tim Sutton
    email                : tim at linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsfield.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsspatialrefsys.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectordataprovider.h"

#include <QFile>
#include <QTextCodec>

#include <cassert>

#include <ogrsf_frmts.h>


QgsVectorFileWriter::QgsVectorFileWriter(const QString& shapefileName,
                                         const QString& fileEncoding,
                                         const QgsFieldMap& fields,
                                         QGis::WKBTYPE geometryType,
                                         const QgsSpatialRefSys* srs)
  : mDS(NULL), mLayer(NULL), mGeom(NULL), mError(NoError)
{

  // save the layer as a shapefile
  QString driverName = "ESRI Shapefile";
  
  // find driver in OGR
  OGRSFDriver *poDriver;
  OGRRegisterAll();
  poDriver =
      OGRSFDriverRegistrar::GetRegistrar()->
      GetDriverByName(driverName.toLocal8Bit().data());
  
  if (poDriver == NULL)
  {
    mError = ErrDriverNotFound;
    return;
  }

  // create the data source
  mDS = poDriver->CreateDataSource(shapefileName, NULL);
  if (mDS == NULL)
  {
    mError = ErrCreateDataSource;
    return;
  }
  
  QgsDebugMsg("Created data source");
  
  // use appropriate codec
  mCodec = QTextCodec::codecForName(fileEncoding.toLocal8Bit().data());
  if (!mCodec)
  {
    QgsDebugMsg("error finding QTextCodec for " + fileEncoding);
    mCodec = QTextCodec::codecForLocale();
  }
  
  // consider spatial reference system of the layer
  OGRSpatialReference* ogrRef = NULL;
  if (srs)
  {
    QString srsWkt = srs->toWkt();
    ogrRef = new OGRSpatialReference(srsWkt.toLocal8Bit().data());
  }
 
  // datasource created, now create the output layer
  QString layerName = shapefileName.left(shapefileName.find(".shp"));
  OGRwkbGeometryType wkbType = static_cast<OGRwkbGeometryType>(geometryType);
  mLayer = mDS->CreateLayer(QFile::encodeName(layerName).data(), ogrRef, wkbType, NULL);
  
  if (srs)
    delete ogrRef;
  
  if (mLayer == NULL)
  {
    mError = ErrCreateLayer;
    return;
  }
  
  QgsDebugMsg("created layer");

  // create the fields
  QgsDebugMsg("creating " + QString::number(fields.size()) + " fields");
  
  mFields = fields;

  QgsFieldMap::const_iterator fldIt;
  for (fldIt = fields.begin(); fldIt != fields.end(); ++fldIt)
  {
    const QgsField& attrField = fldIt.value();
    
    OGRFieldType ogrType;
    switch (attrField.type())
    {
      case QVariant::String:
        ogrType = OFTString;
        break;
      case QVariant::Int:
        ogrType = OFTInteger;
        break;
      case QVariant::Double:
        ogrType = OFTReal;
        break;
      default:
        assert(0 && "invalid variant type!");
    }

    // create field definition
    OGRFieldDefn fld(mCodec->fromUnicode(attrField.name()), ogrType);
    fld.SetWidth(attrField.length());
    fld.SetPrecision(attrField.precision());

    // create the field
    QgsDebugMsg("creating field " + attrField.name() + " width length " + QString::number(attrField.length()));
    if (mLayer->CreateField(&fld) != OGRERR_NONE)
    {
      QgsDebugMsg("error creating field " + attrField.name());
    }
  }

  QgsDebugMsg("Done creating fields");

  // create instance of OGR geometry (will be used to import from WKB)
  switch (geometryType)
  {
    case QGis::WKBPoint:
    case QGis::WKBPoint25D:
      mGeom = new OGRPoint;
      break;
    case QGis::WKBLineString:
    case QGis::WKBLineString25D:
      mGeom = new OGRLineString;
      break;
    case QGis::WKBPolygon:
    case QGis::WKBPolygon25D:
      mGeom = new OGRPolygon;
      break;
    case QGis::WKBMultiPoint:
    case QGis::WKBMultiPoint25D:
      mGeom = new OGRMultiPoint;
      break;
    case QGis::WKBMultiLineString:
    case QGis::WKBMultiLineString25D:
      mGeom = new OGRMultiLineString;
      break;
    case QGis::WKBMultiPolygon:
    case QGis::WKBMultiPolygon25D:
      mGeom = new OGRMultiPolygon;
      break;
    default:
      assert(0 && "invalid WKB type");
  }
}

QgsVectorFileWriter::WriterError QgsVectorFileWriter::hasError()
{
  return mError;
}
    
bool QgsVectorFileWriter::addFeature(QgsFeature& feature)
{
  if (hasError() != NoError)
    return false;
  
  QgsAttributeMap::const_iterator it;

    // create the feature
  OGRFeature *poFeature = new OGRFeature(mLayer->GetLayerDefn());

    // attribute handling
  const QgsAttributeMap& attributes = feature.attributeMap();
  for (it = attributes.begin(); it != attributes.end(); it++)
  {
    QString attrName = mFields[it.key()].name();
    QByteArray encAttrName = mCodec->fromUnicode(attrName);
    const QVariant& attrValue = it.value();

    switch (attrValue.type())
    {
      case QVariant::Int:
        poFeature->SetField(encAttrName.data(), attrValue.toInt());
        break;
      case QVariant::Double:
        poFeature->SetField(encAttrName.data(), attrValue.toDouble());
        break;
      case QVariant::String:
        poFeature->SetField(encAttrName.data(), mCodec->fromUnicode(attrValue.toString()).data());
        break;
      default:
        assert(0 && "invalid variant type");
    }
  }
    
  // build geometry from WKB
  QgsGeometry* geom = feature.geometry();
  if (mGeom->importFromWkb(geom->wkbBuffer(), geom->wkbSize()) != OGRERR_NONE)
  {
    QgsDebugMsg("Failed to import geometry from WKB");
  }
    
  // set geometry (ownership is not passed to OGR)
  poFeature->SetGeometry(mGeom);
    
  // put the created feature to layer
  if (mLayer->CreateFeature(poFeature) != OGRERR_NONE)
  {
    QgsDebugMsg("Failed to create feature in shapefile");
    OGRFeature::DestroyFeature(poFeature);
    return false;
  }

  OGRFeature::DestroyFeature(poFeature);
  
  return true;
}
    
QgsVectorFileWriter::~QgsVectorFileWriter()
{
  if (mGeom)
    delete mGeom;
  
  if (mDS)
    OGRDataSource::DestroyDataSource(mDS);
}




QgsVectorFileWriter::WriterError
    QgsVectorFileWriter::writeAsShapefile(QgsVectorLayer* layer,
                                          const QString& shapefileName,
                                          const QString& fileEncoding)
{
  
  QgsVectorDataProvider* provider = layer->getDataProvider();
  
  QgsVectorFileWriter* writer = new QgsVectorFileWriter(shapefileName,
      fileEncoding, provider->fields(), provider->geometryType(), &layer->srs());
  
  // check whether file creation was successfull
  WriterError err = writer->hasError();
  if (err != NoError)
  {
    delete writer;
    return err;
  }

  QgsAttributeList allAttr = provider->allAttributesList();
  QgsFeature fet;
  
  provider->reset();

  // write all features
  while (provider->getNextFeature(fet, true, allAttr))
  {
    writer->addFeature(fet);
  }
  
  delete writer;
  
  return NoError;
}
