/***************************************************************************
    qgsvectordataprovider.cpp - DataProvider Interface for vector layers
     --------------------------------------
    Date                 : 26-Oct-2004
    Copyright            : (C) 2004 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include <QTextCodec>

#include "qgsvectordataprovider.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgslogger.h"

QgsVectorDataProvider::QgsVectorDataProvider(QString uri)
    : QgsDataProvider(uri),
      mEncoding(QTextCodec::codecForLocale()),
      mFetchFeaturesWithoutGeom(FALSE)
{
}


QgsVectorDataProvider::~QgsVectorDataProvider()
{
}

QString QgsVectorDataProvider::storageType() const
{
  return "Generic vector file";
}

long QgsVectorDataProvider::updateFeatureCount()
{
  return -1;
}

bool QgsVectorDataProvider::getFeatureAtId(int featureId,
                                      QgsFeature& feature,
                                      bool fetchGeometry,
                                      QgsAttributeList fetchAttributes)
{
  return 0;
}

void QgsVectorDataProvider::getFeatureAttributes(int key, int& row, QgsFeature *f)
{
}

void QgsVectorDataProvider::getFeatureGeometry(int key, QgsFeature *f)
{
}


bool QgsVectorDataProvider::addFeatures(QgsFeatureList & flist)
{
  return false;
}

bool QgsVectorDataProvider::deleteFeatures(const QgsFeatureIds & id)
{
  return false;
}

bool QgsVectorDataProvider::addAttributes(const QgsNewAttributesMap & attributes)
{
  return false;
}

bool QgsVectorDataProvider::deleteAttributes(const QgsAttributeIds & attributes)
{
  return false;
}

bool QgsVectorDataProvider::changeAttributeValues(const QgsChangedAttributesMap & attr_map)
{
  return false;
}

QString QgsVectorDataProvider::getDefaultValue(const QString& attr, QgsFeature* f)
{
  return "";
}

bool QgsVectorDataProvider::changeGeometryValues(QgsGeometryMap & geometry_map)
{
  return false;
}

bool QgsVectorDataProvider::createSpatialIndex()
{
    return false;
}

int QgsVectorDataProvider::capabilities() const
{
  return QgsVectorDataProvider::NoCapabilities;
}


void QgsVectorDataProvider::setEncoding(const QString& e)
{
  QTextCodec* ncodec=QTextCodec::codecForName(e.toLocal8Bit().data());
  if(ncodec)
  {
  	mEncoding=ncodec;
  }
  else
  {
    QgsDebugMsg("error finding QTextCodec for " + e);
  }
}

QString QgsVectorDataProvider::encoding() const
{
  if (mEncoding)
  {
    	return mEncoding->name();
  }
	
  return "";
}

QString QgsVectorDataProvider::capabilitiesString() const
{
  QStringList abilitiesList;

  int abilities = capabilities();

  if (abilities & QgsVectorDataProvider::AddFeatures)
  {
    abilitiesList += "Add Features";
    QgsDebugMsg("Capability: Add Features");
  }

  if (abilities & QgsVectorDataProvider::DeleteFeatures)
  {
    abilitiesList += "Delete Features";
    QgsDebugMsg("Capability: Delete Features");
  }

  if (abilities & QgsVectorDataProvider::ChangeAttributeValues)
  {
    abilitiesList += "Change Attribute Values";
    QgsDebugMsg("Capability: Change Attribute Values");
  }

  if (abilities & QgsVectorDataProvider::AddAttributes)
  {
    abilitiesList += "Add Attributes";
    QgsDebugMsg("Capability: Add Attributes");
  }

  if (abilities & QgsVectorDataProvider::DeleteAttributes)
  {
    abilitiesList += "Delete Attributes";
    QgsDebugMsg("Capability: Delete Attributes");
  }

  if (abilities & QgsVectorDataProvider::CreateSpatialIndex)
  {
    // TODO: Tighten up this test.  See QgsOgrProvider for details.
    abilitiesList += "Create Spatial Index";
    QgsDebugMsg("Capability: Create Spatial Index");
  }

  if (abilities & QgsVectorDataProvider::SelectAtId)
  {
    // Not really meaningful to the user.
    // abilitiesList += "Select at ID";
    QgsDebugMsg("Capability: Select at ID");
  }

  if (abilities & QgsVectorDataProvider::ChangeGeometries)
  {
    abilitiesList += "Change Geometries";
    QgsDebugMsg("Capability: Change Geometries");
  }

  if (abilities & QgsVectorDataProvider::SelectGeometryAtId)
  {

    if (abilities & QgsVectorDataProvider::RandomSelectGeometryAtId)
    {
      abilitiesList += "Select Geometries by ID (random access)";
      QgsDebugMsg("Capability: Select Geometries by ID (random access)");
    }
    else if (abilities & QgsVectorDataProvider::SequentialSelectGeometryAtId)
    {
      abilitiesList += "Select Geometries by ID (sequential access)";
      QgsDebugMsg("Capability: Select Geometries by ID (sequential access)");
    }
    else
    {
      abilitiesList += "Select Geometries by ID (unknown access method)";
      QgsDebugMsg("Capability: Select Geometries by ID (unknown access method)");
    }
  }

  return abilitiesList.join(", ");

}


int QgsVectorDataProvider::indexFromFieldName(const QString& fieldName) const
{
  const QgsFieldMap& theFields = fields();
  int counter = 0;

  for (QgsFieldMap::const_iterator it = theFields.begin(); it != theFields.end(); ++it)
  {
    if(*it == fieldName)
    {
      return counter;
    }
    ++counter;
  }
  return -1;
}

QgsAttributeList QgsVectorDataProvider::allAttributesList()
{
  uint count = fieldCount();
  QgsAttributeList list;

  for (uint i = 0; i < count; i++)
    list.append(i);

  return list;
}

void QgsVectorDataProvider::setFetchFeaturesWithoutGeom(bool fetch)
{
  mFetchFeaturesWithoutGeom = fetch;
}
