/***************************************************************************
                              qgswfsprovider.cpp    
                              -------------------
  begin                : July 25, 2006
  copyright            : (C) 2006 by Marco Hugentobler
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

#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgshttptransaction.h"
#include "qgswfsprovider.h"
#include "qgslogger.h"
#include <QDomDocument>
#include <QDomNodeList>
#include <cfloat>

#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif

static const QString TEXT_PROVIDER_KEY = "WFS";
static const QString TEXT_PROVIDER_DESCRIPTION = "WFS data provider";

static const QString WFS_NAMESPACE = "http://www.opengis.net/wfs";
static const QString GML_NAMESPACE = "http://www.opengis.net/gml";

QgsWFSProvider::QgsWFSProvider(const QString& uri): QgsVectorDataProvider(uri), mFilter(0), mUseIntersect(false), mSourceSRS(0)
{
  mFeatureIterator = mFeatures.begin();
  if(getFeature(uri) == 0)
    {
      //provider valid
    }
  else
    {
      //provider invalid
    }
}

QgsWFSProvider::~QgsWFSProvider()
{
  for(std::vector<QgsFeature*>::iterator it = mFeatures.begin(); it != mFeatures.end(); ++it)
    {
      delete (*it);
    }
  mFeatures.clear();
  delete mFilter;
}

QgsFeature* QgsWFSProvider::getFirstFeature(bool fetchAttributes)
{
  reset();
  getNextFeature(fetchAttributes);
}

QgsFeature* QgsWFSProvider::getNextFeature(bool fetchAttributes)
{
  std::list<int> attlist;
  //todo: add the indexes of all attributes
  for(int i = 0; i < mFields.size(); ++i)
    {
      attlist.push_back(i);
    }
  return getNextFeature(attlist, 1);
}

QgsFeature* QgsWFSProvider::getNextFeature(std::list<int> const & attlist, int featureQueueSize)
{
  while(true) //go through the loop until we find a feature in the filter
    {
      if(mFeatureIterator == mFeatures.end())
	{
	  return 0;
	}

      QgsFeature* f = new QgsFeature();
      unsigned char* geom = (*mFeatureIterator)->getGeometry();
      int geomSize = (*mFeatureIterator)->getGeometrySize();
      
      unsigned char* copiedGeom = new unsigned char[geomSize];
      memcpy(copiedGeom, geom, geomSize);
      f->setGeometryAndOwnership(copiedGeom, geomSize);
      f->setFeatureId((*mFeatureIterator)->featureId());
      
      const std::vector<QgsFeatureAttribute> attributes = (*mFeatureIterator)->attributeMap();
      for(std::list<int>::const_iterator it = attlist.begin(); it != attlist.end(); ++it)
	{
	  f->addAttribute(attributes[*it].fieldName(), attributes[*it].fieldValue(), attributes[*it].isNumeric());
	}
      ++mFeatureIterator;
      if(mFilter && mUseIntersect)
	{
	  if(f->geometry()->fast_intersects(mFilter))
	    {
	      return f;
	    }
	  else
	    {
	      delete f;
	    }
	}
      else
	{
	  return f;
	}
    }
}

bool QgsWFSProvider::getNextFeature(QgsFeature &feature, bool fetchAttributes) /*legacy*/
{
  return false;
}

int QgsWFSProvider::geometryType() const
{
  return mWKBType;
}

long QgsWFSProvider::featureCount() const
{
  return mFeatures.size();
}

int QgsWFSProvider::fieldCount() const
{
  return mFields.size();
}

std::vector<QgsField> const & QgsWFSProvider::fields() const
{
  return mFields;
}

void QgsWFSProvider::reset()
{
  mFeatureIterator = mFeatures.begin();
  delete mFilter;
  mFilter = 0;
}

QString QgsWFSProvider::minValue(int position)
{
  if(mMinMaxCash.size() == 0)
    {
      fillMinMaxCash();
    }
  return mMinMaxCash[position].first;
}

QString QgsWFSProvider::maxValue(int position)
{
  if(mMinMaxCash.size() == 0)
    {
      fillMinMaxCash();
    }
  return mMinMaxCash[position].second;
}

void QgsWFSProvider::fillMinMaxCash()
{
  QgsFeature* theFeature = 0;
  int fieldCount = fields().size();
  int i;
  double currentValue;
  
  std::vector<std::pair<double, double> > tempMinMax;
  tempMinMax.resize(fieldCount);
  for(i = 0; i < fieldCount; ++i)
    {
      tempMinMax[i] = std::make_pair(DBL_MAX, -DBL_MAX);
    }

  reset();
  while(theFeature = getNextFeature(true))
    {
      for(i = 0; i < fieldCount; ++i)
	{
	  currentValue = (theFeature->attributeMap())[i].fieldValue().toDouble();
	  if(currentValue < tempMinMax[i].first)
	    {
	      tempMinMax[i].first = currentValue;
	    }
	  if(currentValue > tempMinMax[i].second)
	    {
	      tempMinMax[i].second = currentValue;
	    }
	}
    }

  mMinMaxCash.clear();
  mMinMaxCash.resize(fieldCount);
  for(i = 0; i < fieldCount; ++i) 
    {
      mMinMaxCash[i] = std::make_pair(QString::number(tempMinMax[i].first), QString::number(tempMinMax[i].second));
    }
}

std::vector<QgsFeature>& QgsWFSProvider::identify(QgsRect *rect) /*legacy*/
{
  std::vector<QgsFeature> bla;
  return bla; //that's of course rubbish but the whole method isn't used in qgis
}

QString QgsWFSProvider::getProjectionWKT()
{
  return "";
}

QgsRect* QgsWFSProvider::extent()
{
  QgsRect* ext = new QgsRect(mExtent);
  return ext;
}

bool QgsWFSProvider::isValid()
{
  return true;
}

void QgsWFSProvider::select(QgsRect *mbr, bool useIntersect)
{
  reset();
  mFilter = new QgsRect(*mbr);
  mUseIntersect = useIntersect;
}

int QgsWFSProvider::getCapabilities(const QString& uri, QgsWFSProvider::REQUEST_ENCODING e, std::list<QString>& typenames, std::list< std::list<QString> >& crs)
{
  switch(e)
    {
    case QgsWFSProvider::GET:
      return getCapabilitiesGET(uri, typenames, crs);
    case QgsWFSProvider::POST:
      return getCapabilitiesPOST(uri, typenames, crs);
    case QgsWFSProvider::SOAP:
      return getCapabilitiesSOAP(uri, typenames, crs);
    }
  return 1;
}

int QgsWFSProvider::getFeature(const QString& uri)
{
  //GET or SOAP?
  if(uri.startsWith("SOAP://"))
    {
      mEncoding = QgsWFSProvider::SOAP;
    }
  else
    {
      mEncoding = QgsWFSProvider::GET;
    }

  QString describeFeatureUri = uri;
  describeFeatureUri.replace(QString("GetFeature"), QString("DescribeFeatureType"));
  if(describeFeatureType(describeFeatureUri, mFields) != 0)
  {
    return 1;
  }

  //find out the name of the attribute containing the geometry
  QString geometryAttribute;
  QString currentAttribute;
  for(std::vector<QgsField>::iterator iter =  mFields.begin(); iter != mFields.end(); ++iter)
    {
      currentAttribute = iter->type();
      if(currentAttribute.startsWith("gml:") && currentAttribute.endsWith("PropertyType"))
      {
  	geometryAttribute = iter->name();
	//erase the geometry attribute from mFields (QGIS distinguishes between geometry and thematic attributes)
	mFields.erase(iter);
	break;
      }
    }

  if(mEncoding == QgsWFSProvider::SOAP)
    {
      return getFeatureSOAP(uri, geometryAttribute);
    }
  else
    {
      return getFeatureGET(uri, geometryAttribute);
    }
  return 2;
}

int QgsWFSProvider::describeFeatureType(const QString& uri, std::vector<QgsField>& fields)
{
  switch(mEncoding)
    {
    case QgsWFSProvider::GET:
      return describeFeatureTypeGET(uri, fields);
    case QgsWFSProvider::POST:
      return describeFeatureTypePOST(uri, fields);
    case QgsWFSProvider::SOAP:
      return describeFeatureTypeSOAP(uri, fields);
    }
  return 1;
}

int QgsWFSProvider::getCapabilitiesGET(const QString& uri, std::list<QString>& typenames, std::list< std::list<QString> >& crs)
{
  QString request = uri + "SERVICE=WFS&REQUEST=GetCapabilities&VERSION=1.1.1";
  QByteArray result;
  QgsHttpTransaction http(request);
  http.getSynchronously(result);
  
  QDomDocument capabilitiesDocument;
  if(!capabilitiesDocument.setContent(result, true))
    {
      return 1; //error
    }
  
  

  //get the <FeatureType> elements
  QDomNodeList featureTypeList = capabilitiesDocument.elementsByTagNameNS(WFS_NAMESPACE, "FeatureType");
  for(unsigned int i = 0; i < featureTypeList.length(); ++i)
    {
      QDomElement featureTypeElem = featureTypeList.at(i).toElement();
      std::list<QString> featureSRSList; //SRS list for this feature

      //Name
      QDomNodeList nameList = featureTypeElem.elementsByTagNameNS(WFS_NAMESPACE, "Name");
      if(nameList.length() > 0)
	{
	  typenames.push_back(nameList.at(0).toElement().text());
	}
      
      //DefaultSRS is always the first entry in the feature crs list
      QDomNodeList defaultSRSList = featureTypeElem.elementsByTagNameNS(WFS_NAMESPACE, "DefaultSRS");
      if(defaultSRSList.length() > 0)
	{
	  featureSRSList.push_back(defaultSRSList.at(0).toElement().text());
	}

      //OtherSRS
      QDomNodeList otherSRSList = featureTypeElem.elementsByTagNameNS(WFS_NAMESPACE, "OtherSRS");
      for(unsigned int i = 0; i < otherSRSList.length(); ++i)
	{
	  featureSRSList.push_back(otherSRSList.at(i).toElement().text());
	}

      //Support <SRS> for compatibility with older versions
      QDomNodeList srsList = featureTypeElem.elementsByTagNameNS(WFS_NAMESPACE, "SRS");
      for(unsigned int i = 0; i < srsList.length(); ++i)
	{
	  featureSRSList.push_back(srsList.at(i).toElement().text());
	}

      crs.push_back(featureSRSList);
    }


  //print out result for a test
  QString resultString(result);
  qWarning(resultString);

  return 0;
}

int QgsWFSProvider::getCapabilitiesPOST(const QString& uri, std::list<QString>& typenames, std::list< std::list<QString> >& crs)
{
  return 1; //soon...
}

int QgsWFSProvider::getCapabilitiesSOAP(const QString& uri, std::list<QString>& typenames, std::list< std::list<QString> >& crs)
{
  return 1; //soon...
}

int QgsWFSProvider::getFeatureGET(const QString& uri, const QString& geometryAttribute)
{
  //assemble request string
  QString request = uri /*+ "&OUTPUTFORMAT=gml3"*/; //use gml2 as it is supported by most wfs servers
  QByteArray result;
  QgsHttpTransaction http(request);
  http.getSynchronously(result);
  
  QDomDocument getFeatureDocument;
  if(!getFeatureDocument.setContent(result, true))
    {
      return 1; //error
    }

  QDomElement featureCollectionElement = getFeatureDocument.documentElement();
  
  //get and set Extent
  if(getExtentFromGML2(&mExtent, featureCollectionElement) != 0)
    {
      return 3;
    }

  setSRSFromGML2(featureCollectionElement);  

  if(getFeaturesFromGML2(featureCollectionElement, geometryAttribute, mFeatures) != 0)
  {
    return 4;
  }

  return 0;
}

int QgsWFSProvider::getFeaturePOST(const QString& uri, const QString& geometryAttribute)
{
  return 1; //soon...
}

int QgsWFSProvider::getFeatureSOAP(const QString& uri, const QString& geometryAttribute)
{
  return 1; //soon...
}

int QgsWFSProvider::describeFeatureTypeGET(const QString& uri, std::vector<QgsField>& fields)
{
  QByteArray result;
  QgsHttpTransaction http(uri);
  http.getSynchronously(result);

  //find out the typename
  QString tname;
  QStringList tnamelist = uri.split("&");
  for(int i = 0; i < tnamelist.size(); ++i)
    {
      if(tnamelist.at(i).startsWith("typename", Qt::CaseInsensitive))
	{
	  QStringList tlist = tnamelist.at(i).split("=");
	  tname = tlist.at(1);
	}
    }

  //remove the namespace from tname
  if(tname.contains(":"))
    {
      tname = tname.section(":", 1, 1);
    }

  QDomDocument describeFeatureDocument;
  if(!describeFeatureDocument.setContent(result, true))
    {
      return 1; //error
    }

  qWarning(describeFeatureDocument.toString());
  
  //get the <schema> root element
  QDomNodeList schemaNodeList = describeFeatureDocument.elementsByTagNameNS("http://www.w3.org/2001/XMLSchema", "schema");
  if(schemaNodeList.length() < 1)
    {
      return 2;
    }
  QDomElement schemaElement = schemaNodeList.at(0).toElement();
  
  //find <element name="tname" type = ...>
  QString complexTypeType;
  QDomNodeList typeElementNodeList = schemaElement.elementsByTagNameNS("http://www.w3.org/2001/XMLSchema", "element");
  for(int i = 0; i < typeElementNodeList.length(); ++i)
    {
      QDomElement typeElement = typeElementNodeList.at(i).toElement();
      if(typeElement.attribute("name") == tname)
	{
	  complexTypeType = typeElement.attribute("type");
	}
    }

  if(complexTypeType.isEmpty())
    {
      return 3;
    }

  //remove the namespace on complexTypeType
  if(complexTypeType.contains(":"))
    {
      complexTypeType = complexTypeType.section(":", 1, 1);
    }

  //find <complexType name=complexTypeType
  QDomElement complexTypeElement;
  QDomNodeList complexTypeNodeList = schemaElement.elementsByTagNameNS("http://www.w3.org/2001/XMLSchema", "complexType");
  for(int i = 0; i < complexTypeNodeList.length(); ++i)
    {
      if(complexTypeNodeList.at(i).toElement().attribute("name") == complexTypeType)
	{
	  complexTypeElement = complexTypeNodeList.at(i).toElement();
	}
    }

  if(complexTypeElement.isNull())
    {
      return 4;
    }
  
  //now create the attributes
  QDomNodeList attributeNodeList = complexTypeElement.elementsByTagNameNS("http://www.w3.org/2001/XMLSchema", "element");
  for(int i = 0; i < attributeNodeList.length(); ++i)
    {
      QDomElement attributeElement = attributeNodeList.at(i).toElement();
      //attribute name
      QString name = attributeElement.attribute("name");
      //attribute type
      QString type = attributeElement.attribute("type");
      if(type.isEmpty())
	{
	  //todo: is the type name inside a <simpleType> element?
	}
      //todo: distinguish between numerical and non-numerical types
      fields.push_back(QgsField(name, type));
    }
  return 0;
}

int QgsWFSProvider::describeFeatureTypePOST(const QString& uri, std::vector<QgsField>& fields)
{
  return 1; //soon...
}

int QgsWFSProvider::describeFeatureTypeSOAP(const QString& uri, std::vector<QgsField>& fields)
{
  return 1; //soon...
}

int QgsWFSProvider::getExtentFromGML2(QgsRect* extent, const QDomElement& wfsCollectionElement) const
{
  QDomNodeList boundedByList = wfsCollectionElement.elementsByTagNameNS(GML_NAMESPACE, "boundedBy");
  if(boundedByList.length() < 1)
    {
      return 1;
    }
  QDomElement boundedByElement = boundedByList.at(0).toElement();
  QDomNode childNode = boundedByElement.firstChild();
  if(childNode.isNull())
    {
      return 2;
    }

  //support <gml:Box>, <gml:coordinates> and <gml:Envelope>,<gml::lowerCorner>,<gml::upperCorner>. What
  //about <gml:Envelope>, <gml:pos>?
  QString bboxName = childNode.localName();
  if(bboxName != "Box")
    {
      return 3;
    }

  QDomNode coordinatesNode = childNode.firstChild();
  if(coordinatesNode.localName() != "coordinates")
    {
      return 4;
    }

  std::list<QgsPoint> boundingPoints;
  if(readGML2Coordinates(boundingPoints, coordinatesNode.toElement()) != 0)
    {
      return 5;
    }
  
  if(boundingPoints.size() != 2)
    {
      return 6;
    }
 
  std::list<QgsPoint>::const_iterator it = boundingPoints.begin();
  extent->setXmin(it->x());
  extent->setYmin(it->y());
  ++it;
  extent->setXmax(it->x());
  extent->setYmax(it->y());
  return 0;
}

int QgsWFSProvider::setSRSFromGML2(const QDomElement& wfsCollectionElement)
{
  QgsDebugMsg("entering QgsWFSProvider::setSRSFromGML");
  //search <gml:boundedBy>
  QDomNodeList boundedByList = wfsCollectionElement.elementsByTagNameNS(GML_NAMESPACE, "boundedBy");
  if(boundedByList.size() < 1)
    {
      QgsDebugMsg("Error, could not find boundedBy element");
      return 1;
    }
  //search <gml:Envelope>
  QDomElement boundedByElem = boundedByList.at(0).toElement();
  QDomNodeList boxList = boundedByElem.elementsByTagNameNS(GML_NAMESPACE, "Box");
  if(boxList.size() < 1)
    {
      QgsDebugMsg("Error, could not find Envelope element");
      return 2;
    }
  QDomElement boxElem = boxList.at(0).toElement();
  //getAttribute 'srsName'
  QString srsName = boxElem.attribute("srsName");
  if(srsName.isEmpty())
    {
      QgsDebugMsg("Error, srsName is empty");
      return 3;
    }
  QgsDebugMsg("srsName is: " +srsName);


  //extract the EPSG id
  int epsgId;
  bool conversionSuccess;
  if(srsName.contains("#"))//geoserver has "http://www.opengis.net/gml/srs/epsg.xml#4326"
    {
      epsgId = srsName.section("#", 1, 1).toInt(&conversionSuccess);
      if(!conversionSuccess)
	{
	  return 4;
	}
    }
  else if(srsName.contains(":"))//mapserver has "EPSG:4326"
    {
      epsgId = srsName.section(":", 1, 1).toInt(&conversionSuccess);
      if(!conversionSuccess)
	{
	  return 5;
	}
    }

  mSourceSRS = new QgsSpatialRefSys();
  if(!mSourceSRS->createFromEpsg(epsgId))
    {
      QgsDebugMsg("Error, creation of QgsSpatialRefSys failed");
      delete mSourceSRS;
      mSourceSRS = 0;
      return 6;
    }
  return 0;
}
  
int QgsWFSProvider::getFeaturesFromGML2(const QDomElement& wfsCollectionElement, const QString& geometryAttribute, std::vector<QgsFeature*>& features) const
{
  QDomNodeList featureTypeNodeList = wfsCollectionElement.elementsByTagNameNS(GML_NAMESPACE, "featureMember");
  QDomElement currentFeatureMemberElem;
  QDomElement layerNameElem;
  QDomNode currentAttributeChild;
  QDomElement currentAttributeElement;
  int counter = 0;
  QgsFeature* f = 0;
  unsigned char* wkb = 0;
  int wkbSize = 0;
  QGis::WKBTYPE currentType;

  for(int i = 0; i < featureTypeNodeList.size(); ++i)
    {
      f = new QgsFeature(counter);
      currentFeatureMemberElem = featureTypeNodeList.at(i).toElement();
      //the first child element is always <namespace:layer>
      layerNameElem = currentFeatureMemberElem.firstChild().toElement();
      //the children are the attributes
      currentAttributeChild = layerNameElem.firstChild();
      while(!currentAttributeChild.isNull())
	{
	  currentAttributeElement = currentAttributeChild.toElement();
	  if(currentAttributeElement.localName() != "boundedBy")
	    {
	      if((currentAttributeElement.localName()) != geometryAttribute) //a normal attribute
		{
		  f->addAttribute(currentAttributeElement.localName(), currentAttributeElement.text(), false);
		}
	      else //a geometry attribute
		{
		  getWkbFromGML2(currentAttributeElement, &wkb, &wkbSize, &currentType);
		  mWKBType = currentType; //a more sophisticated method is necessary
		  f->setGeometryAndOwnership(wkb, wkbSize);
		}
	    }
	  currentAttributeChild = currentAttributeChild.nextSibling();
	}
      if(wkb && wkbSize > 0)
	{
	  features.push_back(f);
	}
      ++counter;
    }
  return 0;
}

int QgsWFSProvider::getWkbFromGML2(const QDomNode& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const
{
  QDomNode geometryChild = geometryElement.firstChild();
  if(geometryChild.isNull())
    {
      return 1;
    }
  QDomElement geometryTypeElement = geometryChild.toElement();
  QString geomType = geometryTypeElement.localName();
  if(geomType == "Point")
    {
      return getWkbFromGML2Point(geometryTypeElement, wkb, wkbSize, type);
    }
  else if(geomType == "LineString")
    {
      return getWkbFromGML2LineString(geometryTypeElement, wkb, wkbSize, type);
    }
  else if(geomType == "Polygon")
    {
      return getWkbFromGML2Polygon(geometryTypeElement, wkb, wkbSize, type);
    }
  else if(geomType == "MultiPoint")
    {
      return getWkbFromGML2MultiPoint(geometryTypeElement, wkb, wkbSize, type);
    }
  else if(geomType == "MultiLineString")
    {
      return getWkbFromGML2MultiLineString(geometryTypeElement, wkb, wkbSize, type);
    }
  else if(geomType == "MultiPolygon")
    {
      return getWkbFromGML2MultiPolygon(geometryTypeElement, wkb, wkbSize, type);
    }
  else //unknown type
    {
      *wkb = 0;
      *wkbSize = 0;
    }
  return 0;
}

int QgsWFSProvider::getWkbFromGML2Point(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const
{
  QDomNodeList coordList = geometryElement.elementsByTagNameNS(GML_NAMESPACE, "coordinates");
  if(coordList.size() < 1)
    {
      return 1;
    }
  QDomElement coordElement = coordList.at(0).toElement();
  std::list<QgsPoint> pointCoordinate;
  if(readGML2Coordinates(pointCoordinate, coordElement) != 0)
    {
      return 2;
    }
  
  if(pointCoordinate.size() < 1)
    {
      return 3;
    }
  
  std::list<QgsPoint>::const_iterator point_it = pointCoordinate.begin();
  char e = endian();
  double x = point_it->x();
  double y = point_it->y();
  int size = 1 + sizeof(int) + 2 * sizeof(double);
  *wkb = new unsigned char[size];
  *wkbSize = size;
  *type = QGis::WKBPoint;
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  memcpy(&(*wkb)[wkbPosition], &e, 1);
  wkbPosition += 1;
  memcpy(&(*wkb)[wkbPosition], type, sizeof(int));
  wkbPosition += sizeof(int);
  memcpy(&(*wkb)[wkbPosition], &x, sizeof(double));
  wkbPosition += sizeof(double);
  memcpy(&(*wkb)[wkbPosition], &y, sizeof(double));
  return 0;
}

int QgsWFSProvider::getWkbFromGML2Polygon(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const
{
  //read all the coordinates (as QgsPoint) into memory. Each linear ring has an entry in the vector
  std::vector<std::list<QgsPoint> > ringCoordinates;
  
  //read coordinates for outer boundary
  QDomNodeList outerBoundaryList = geometryElement.elementsByTagNameNS(GML_NAMESPACE, "outerBoundaryIs");
  if(outerBoundaryList.size() < 1) //outer ring is necessary
    {
      return 1; 
    }
  QDomElement coordinatesElement = outerBoundaryList.at(0).firstChild().firstChild().toElement();
  if(coordinatesElement.isNull())
    {
      return 2;
    }
  std::list<QgsPoint> exteriorPointList;
  if(readGML2Coordinates(exteriorPointList, coordinatesElement) != 0)
    {
      return 3;
    }
  ringCoordinates.push_back(exteriorPointList);

  //read coordinates for inner boundary
  QDomNodeList innerBoundaryList = geometryElement.elementsByTagNameNS(GML_NAMESPACE, "innerBoundaryIs");
  for(int i = 0; i < innerBoundaryList.size(); ++i)
    {
      std::list<QgsPoint> interiorPointList;
      QDomElement coordinatesElement = innerBoundaryList.at(i).firstChild().firstChild().toElement();
      if(coordinatesElement.isNull())
	{
	  return 4;
	}
      if(readGML2Coordinates(interiorPointList, coordinatesElement) != 0)
	{
	  return 5;
	}
      ringCoordinates.push_back(interiorPointList); 
    }

  //calculate number of bytes to allocate
  int nrings = 1 + innerBoundaryList.size();
  int npoints = 0;//total number of points
  for(std::vector<std::list<QgsPoint> >::const_iterator it = ringCoordinates.begin(); it != ringCoordinates.end(); ++it)
    {
      npoints += it->size();
    } 
  int size = 1 + 2 * sizeof(int) + nrings * sizeof(int) + 2 * npoints * sizeof(double);
  *wkb = new unsigned char[size];
  *wkbSize = size;
  *type = QGis::WKBPolygon;
  char e = endian();
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  int nPointsInRing = 0;
  double x, y;
  
  //fill the contents into *wkb
  memcpy(&(*wkb)[wkbPosition], &e, 1);
  wkbPosition += 1;
  memcpy(&(*wkb)[wkbPosition], type, sizeof(int));
  wkbPosition += sizeof(int);
  memcpy(&(*wkb)[wkbPosition], &nrings, sizeof(int));
  wkbPosition += sizeof(int);
  for(std::vector<std::list<QgsPoint> >::const_iterator it = ringCoordinates.begin(); it != ringCoordinates.end(); ++it)
    {
      nPointsInRing = it->size(); 
      memcpy(&(*wkb)[wkbPosition], &nPointsInRing, sizeof(int));
      wkbPosition += sizeof(int);
      //iterate through the string list converting the strings to x-/y- doubles
      std::list<QgsPoint>::const_iterator iter;
      for(iter = it->begin(); iter != it->end(); ++iter)
	{
	  x = iter->x();
	  y = iter->y();
	  //qWarning("currentCoordinate: " + QString::number(x) + " // " + QString::number(y));
	  memcpy(&(*wkb)[wkbPosition], &x, sizeof(double));
	  wkbPosition += sizeof(double);
	  memcpy(&(*wkb)[wkbPosition], &y, sizeof(double));
	  wkbPosition += sizeof(double);
	}
    }
  return 0;
}

int QgsWFSProvider::getWkbFromGML2LineString(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const
{
  QDomNodeList coordinatesList = geometryElement.elementsByTagNameNS(GML_NAMESPACE, "coordinates");
  if(coordinatesList.size() < 1)
    {
      return 1;
    }
  QDomElement coordinatesElement = coordinatesList.at(0).toElement();
  std::list<QgsPoint> lineCoordinates;
  if(readGML2Coordinates(lineCoordinates, coordinatesElement) != 0)
    {
      return 2;
    }

  char e = endian();
  int size = 1 + 2 * sizeof(int) + lineCoordinates.size() * 2* sizeof(double);
  *wkb = new unsigned char[size];
  *wkbSize = size;
  *type = QGis::WKBLineString;
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  double x, y;
  int nPoints = lineCoordinates.size();
  
  //fill the contents into *wkb
  memcpy(&(*wkb)[wkbPosition], &e, 1);
  wkbPosition += 1;
  memcpy(&(*wkb)[wkbPosition], type, sizeof(int));
  wkbPosition += sizeof(int);
  memcpy(&(*wkb)[wkbPosition], &nPoints, sizeof(int));
  wkbPosition += sizeof(int);
  
  std::list<QgsPoint>::const_iterator iter;
  for(iter = lineCoordinates.begin(); iter != lineCoordinates.end(); ++iter)
    {
      x = iter->x();
      y = iter->y();
      memcpy(&(*wkb)[wkbPosition], &x, sizeof(double));
      wkbPosition += sizeof(double);
      memcpy(&(*wkb)[wkbPosition], &y, sizeof(double));
      wkbPosition += sizeof(double);
    }
  return 0;
}

int QgsWFSProvider::getWkbFromGML2MultiPoint(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const
{
  std::list<QgsPoint> pointList;
  std::list<QgsPoint> currentPoint;
  QDomNodeList pointMemberList = geometryElement.elementsByTagNameNS(GML_NAMESPACE, "pointMember");
  if(pointMemberList.size() < 1)
    {
      return 1;
    }
  QDomNodeList pointNodeList;
  QDomNodeList coordinatesList;
  for(int i = 0; i < pointMemberList.size(); ++i)
    {
      //<Point> element
      pointNodeList = pointMemberList.at(i).toElement().elementsByTagNameNS(GML_NAMESPACE, "Point");
      if(pointNodeList.size() < 1)
	{
	  continue;
	}
      //<coordinates> element
      coordinatesList = pointNodeList.at(0).toElement().elementsByTagNameNS(GML_NAMESPACE, "coordinates");
      if(coordinatesList.size() < 1)
	{
	  continue;
	}
      currentPoint.clear();
      if(readGML2Coordinates(currentPoint, coordinatesList.at(0).toElement()) != 0)
	{
	  continue;
	}
      if(currentPoint.size() < 1)
	{
	  continue;
	}
      pointList.push_back((*currentPoint.begin()));
    }

  //calculate the required wkb size
  int size = 1 + 2 * sizeof(int) + pointList.size() * (2 * sizeof(double) + 1 + sizeof(int));
  *wkb = new unsigned char[size];
  *wkbSize = size;
  *type = QGis::WKBMultiPoint;

  //fill the wkb content
  char e = endian();
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  int nPoints = pointList.size(); //number of points
  double x, y;
  memcpy(&(*wkb)[wkbPosition], &e, 1);
  wkbPosition += 1;
  memcpy(&(*wkb)[wkbPosition], type, sizeof(int));
  wkbPosition += sizeof(int);
  memcpy(&(*wkb)[wkbPosition], &nPoints, sizeof(int));
  wkbPosition += sizeof(int);
  for(std::list<QgsPoint>::const_iterator it = pointList.begin(); it != pointList.end(); ++it)
    {
      memcpy(&(*wkb)[wkbPosition], &e, 1);
      wkbPosition += 1;
      memcpy(&(*wkb)[wkbPosition], type, sizeof(int));
      wkbPosition += sizeof(int);
      x = it->x();
      memcpy(&(*wkb)[wkbPosition], &x, sizeof(double));
      wkbPosition += sizeof(double);
      y = it->y();
      memcpy(&(*wkb)[wkbPosition], &y, sizeof(double));
      wkbPosition += sizeof(double);
    }
  return 0;
}

int QgsWFSProvider::getWkbFromGML2MultiLineString(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const
{
  //geoserver has
  //<gml:MultiLineString>
  //<gml:lineStringMember>
  //<gml:LineString>
  
  //mapserver has directly
  //<gml:MultiLineString
  //<gml:LineString

  std::list<std::list<QgsPoint> > lineCoordinates; //first list: lines, second list: points of one line
  QDomElement currentLineStringElement;
  QDomNodeList currentCoordList;

  QDomNodeList lineStringMemberList = geometryElement.elementsByTagNameNS(GML_NAMESPACE, "lineStringMember");
  if(lineStringMemberList.size() > 0) //geoserver
    {
      for(int i = 0; i < lineStringMemberList.size(); ++i)
	{
	  QDomNodeList lineStringNodeList = geometryElement.elementsByTagNameNS(GML_NAMESPACE, "LineString");
	  if(lineStringNodeList.size() < 1)
	    {
	      return 1;
	    }
	  currentLineStringElement = lineStringNodeList.at(0).toElement();
	  currentCoordList = currentLineStringElement.elementsByTagNameNS(GML_NAMESPACE, "coordinates");
	  if(currentCoordList.size() < 1)
	    {
	      return 2;
	    }
	  std::list<QgsPoint> currentPointList;
	  if(readGML2Coordinates(currentPointList, currentCoordList.at(0).toElement()) != 0)
	    {
	      return 3;
	    }
	  lineCoordinates.push_back(currentPointList);
	}
    }
  else
    {
      QDomNodeList lineStringList = geometryElement.elementsByTagNameNS(GML_NAMESPACE, "LineString");
      if(lineStringList.size() > 0) //mapserver
	{
	  for(int i = 0; i < lineStringList.size(); ++i)
	    {
	      currentLineStringElement = lineStringList.at(i).toElement();
	      currentCoordList = currentLineStringElement.elementsByTagNameNS(GML_NAMESPACE, "coordinates");
	      if(currentCoordList.size() < 1)
		{
		  return 4;
		}
	      std::list<QgsPoint> currentPointList;
	      if(readGML2Coordinates(currentPointList, currentCoordList.at(0).toElement()) != 0)
		{
		  return 5;
		}
	      lineCoordinates.push_back(currentPointList);
	    }
	}
      else
	{
	  return 6;
	}
    }


  //calculate the required wkb size
  int size = (lineCoordinates.size() + 1) * (1 + 2 * sizeof(int));
  for(std::list<std::list<QgsPoint> >::const_iterator it = lineCoordinates.begin(); it != lineCoordinates.end(); ++it)
    {
      size += it->size() * 2 * sizeof(double);
    }
  *wkb = new unsigned char[size];
  *wkbSize = size;
  *type = QGis::WKBMultiLineString;
  
  //fill the wkb content
  char e = endian();
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  int nLines = lineCoordinates.size();
  int nPoints; //number of points in a line
  double x, y;
  memcpy(&(*wkb)[wkbPosition], &e, 1);
  wkbPosition += 1;
  memcpy(&(*wkb)[wkbPosition], type, sizeof(int));
  wkbPosition += sizeof(int);
  memcpy(&(*wkb)[wkbPosition], &nLines, sizeof(int));
  wkbPosition += sizeof(int);
  for(std::list<std::list<QgsPoint> >::const_iterator it = lineCoordinates.begin(); it != lineCoordinates.end(); ++it)
    {
      memcpy(&(*wkb)[wkbPosition], &e, 1);
      wkbPosition += 1;
      memcpy(&(*wkb)[wkbPosition], type, sizeof(int));
      wkbPosition += sizeof(int);
      nPoints = it->size();
      memcpy(&(*wkb)[wkbPosition], &nPoints, sizeof(int));
      wkbPosition += sizeof(int);
      for(std::list<QgsPoint>::const_iterator iter = it->begin(); iter != it->end(); ++iter)
	{
	  x = iter->x();
	  //qWarning("x is: " + QString::number(x));
	  y = iter->y();
	  //qWarning("y is: " + QString::number(y));
	  memcpy(&(*wkb)[wkbPosition], &x, sizeof(double));
	  wkbPosition += sizeof(double);
	  memcpy(&(*wkb)[wkbPosition], &y, sizeof(double));
	  wkbPosition += sizeof(double);
	}
    }
  return 0;  
}

int QgsWFSProvider::getWkbFromGML2MultiPolygon(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const
{
  //first list: different polygons, second list: different rings, third list: different points
  std::list<std::list<std::list<QgsPoint> > > multiPolygonPoints;
  QDomElement currentPolygonMemberElement;
  QDomNodeList polygonList;
  QDomElement currentPolygonElement;
  QDomNodeList outerBoundaryList;
  QDomElement currentOuterBoundaryElement;
  QDomElement currentInnerBoundaryElement;
  QDomNodeList innerBoundaryList;
  QDomNodeList linearRingNodeList;
  QDomElement currentLinearRingElement;
  QDomNodeList currentCoordinateList;

  QDomNodeList polygonMemberList = geometryElement.elementsByTagNameNS(GML_NAMESPACE, "polygonMember");
  for(int i = 0; i < polygonMemberList.size(); ++i)
    {
      std::list<std::list<QgsPoint> > currentPolygonList;
      currentPolygonMemberElement = polygonMemberList.at(i).toElement();
      polygonList = currentPolygonMemberElement.elementsByTagNameNS(GML_NAMESPACE, "Polygon");
      if(polygonList.size() < 1)
	{
	  continue;
	}
      currentPolygonElement = polygonList.at(0).toElement();
      
      //find exterior ring
      outerBoundaryList = currentPolygonElement.elementsByTagNameNS(GML_NAMESPACE, "outerBoundaryIs");
      if(outerBoundaryList.size() < 1)
	{
	  continue;
	}
      
      currentOuterBoundaryElement = outerBoundaryList.at(0).toElement();
      std::list<QgsPoint> ringCoordinates;
     
      linearRingNodeList = currentOuterBoundaryElement.elementsByTagNameNS(GML_NAMESPACE, "LinearRing");
      if(linearRingNodeList.size() < 1)
	{
	  continue;
	}
      currentLinearRingElement = linearRingNodeList.at(i).toElement();
      currentCoordinateList = currentLinearRingElement.elementsByTagNameNS(GML_NAMESPACE, "coordinates");
      if(currentCoordinateList.size() < 1)
	{
	  continue;
	}
      if(readGML2Coordinates(ringCoordinates, currentCoordinateList.at(0).toElement()) != 0)
	{
	  continue;
	}
      currentPolygonList.push_back(ringCoordinates);

      //find interior rings
      QDomNodeList innerBoundaryList = currentPolygonElement.elementsByTagNameNS(GML_NAMESPACE, "innerBoundaryIs");
      for(int i = 0; i < innerBoundaryList.size(); ++i)
	{
	  std::list<QgsPoint> ringCoordinates;
	  currentInnerBoundaryElement = innerBoundaryList.at(i).toElement();
	  linearRingNodeList = currentOuterBoundaryElement.elementsByTagNameNS(GML_NAMESPACE, "LinearRing");
	  if(linearRingNodeList.size() < 1)
	    {
	      continue;
	    }
	  currentLinearRingElement = linearRingNodeList.at(i).toElement(); 
	  currentCoordinateList = currentLinearRingElement.elementsByTagNameNS(GML_NAMESPACE, "coordinates");
	  if(currentCoordinateList.size() < 1)
	    {
	      continue;
	    }
	  if(readGML2Coordinates(ringCoordinates, currentCoordinateList.at(0).toElement()) != 0)
	    {
	      continue;
	    } 
	  currentPolygonList.push_back(ringCoordinates);
	}
      multiPolygonPoints.push_back(currentPolygonList);
    }
  
  int size = 1 + 2 * sizeof(int);
  //calculate the wkb size
  for(std::list<std::list<std::list<QgsPoint> > >::const_iterator it = multiPolygonPoints.begin(); it != multiPolygonPoints.end(); ++it)
    {
      size += 1 + 2 * sizeof(int);
      for(std::list<std::list<QgsPoint> >::const_iterator iter = it->begin(); iter != it->end(); ++iter)
	{
	  size += sizeof(int) + 2 * iter->size() * sizeof(double);
	}
    }
  *wkb = new unsigned char[size];
  *wkbSize = size;
  *type = QGis::WKBMultiPolygon;
  int polygonType = QGis::WKBPolygon;
  char e = endian();
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  double x, y;
  int nPolygons = multiPolygonPoints.size();
  int nRings;
  int nPointsInRing;
  
  //fill the contents into *wkb
  memcpy(&(*wkb)[wkbPosition], &e, 1);
  wkbPosition += 1;
  memcpy(&(*wkb)[wkbPosition], type, sizeof(int));
  wkbPosition += sizeof(int);
  memcpy(&(*wkb)[wkbPosition], &nPolygons, sizeof(int));
  wkbPosition += sizeof(int);
  
  for(std::list<std::list<std::list<QgsPoint> > >::const_iterator it = multiPolygonPoints.begin(); it != multiPolygonPoints.end(); ++it)
    {
      memcpy(&(*wkb)[wkbPosition], &e, 1);
      wkbPosition += 1;
      memcpy(&(*wkb)[wkbPosition], &polygonType, sizeof(int));
      wkbPosition += sizeof(int);
      nRings = it->size();
      memcpy(&(*wkb)[wkbPosition], &nRings, sizeof(int));
      wkbPosition += sizeof(int);
      for(std::list<std::list<QgsPoint> >::const_iterator iter = it->begin(); iter != it->end(); ++iter)
	{
	  nPointsInRing = iter->size();
	  memcpy(&(*wkb)[wkbPosition], &nPointsInRing, sizeof(int));
	  wkbPosition += sizeof(int);
	  for(std::list<QgsPoint>::const_iterator iterator = iter->begin(); iterator != iter->end(); ++iterator)
	    {
	      x = iterator->x();
	      y = iterator->y();
	      memcpy(&(*wkb)[wkbPosition], &x, sizeof(double));
	      wkbPosition += sizeof(double);
	      memcpy(&(*wkb)[wkbPosition], &y, sizeof(double));
	      wkbPosition += sizeof(double);
	    }
	}
    }
  return 0;
}

int QgsWFSProvider::readGML2Coordinates(std::list<QgsPoint>& coords, const QDomElement elem) const
{
  QString coordSeparator = ",";
  QString tupelSeparator = " ";
  //"decimal" has to be "."

  coords.clear();

  if(elem.hasAttribute("cs"))
    {
      coordSeparator = elem.attribute("cs");
    }
  if(elem.hasAttribute("ts"))
    {
      tupelSeparator = elem.attribute("ts");
    }

  QStringList tupels = elem.text().split(tupelSeparator, QString::SkipEmptyParts);
  QStringList tupel_coords;
  double x, y;
  bool conversionSuccess;

  QStringList::const_iterator it;
  for(it = tupels.constBegin(); it != tupels.constEnd(); ++it)
    {
      tupel_coords = (*it).split(coordSeparator, QString::SkipEmptyParts);
      if(tupel_coords.size() < 2)
	{
	  continue;
	}
      x = tupel_coords.at(0).toDouble(&conversionSuccess);
      if(!conversionSuccess)
	{
	  return 1;
	}
      y = tupel_coords.at(1).toDouble(&conversionSuccess);
      if(!conversionSuccess)
	{
	  return 1;
	}
      coords.push_back(QgsPoint(x, y));
    }
  return 0;
}

int QgsWFSProvider::getSrid()
{
  if(mSourceSRS)
    {
      return mSourceSRS->srid();
    }
  else
    {
      return 0;
    }
}

QString QgsWFSProvider::name() const
{
  return TEXT_PROVIDER_KEY;
}

QString QgsWFSProvider::description() const
{
  return TEXT_PROVIDER_DESCRIPTION;
}


QGISEXTERN QgsWFSProvider* classFactory(const QString *uri)
{
  return new QgsWFSProvider(*uri);
}

QGISEXTERN QString providerKey()
{
    return TEXT_PROVIDER_KEY;
}

QGISEXTERN QString description()
{
    return TEXT_PROVIDER_DESCRIPTION;
}

QGISEXTERN bool isProvider()
{
  return true;
}



//methods for reading GML3. Not needed at the moment as most servers support GML2

#if 0
int QgsWFSProvider::getExtentFromGML3(QgsRect* extent, const QDomElement& wfsCollectionElement) const
{
  QDomNodeList boundedByList = wfsCollectionElement.elementsByTagNameNS(GML_NAMESPACE, "boundedBy");
  if(boundedByList.length() < 1)
    {
      return 1;
    }
  QDomElement boundedByElement = boundedByList.at(0).toElement();
  QDomNode childNode = boundedByElement.firstChild();
  if(childNode.isNull())
    {
      return 2;
    }

  //support <gml:Box>, <gml:coordinates> and <gml:Envelope>,<gml::lowerCorner>,<gml::upperCorner>. What
  //about <gml:Envelope>, <gml:pos>?
  QString bboxName = childNode.localName();
  if(bboxName == "Envelope")
    {
      QDomElement envelopeElement = childNode.toElement();
      QDomNodeList posList = envelopeElement.elementsByTagNameNS(GML_NAMESPACE, "pos");
      if(posList.size() < 2)
	{
	  return 3;
	}
      QDomElement llPosElement = posList.at(0).toElement();
      QStringList llStringList = llPosElement.text().split(" ", QString::SkipEmptyParts);
      QDomElement urPosElement = posList.at(1).toElement();
      QStringList urStringList = urPosElement.text().split(" ", QString::SkipEmptyParts);
      
      if(llStringList.size() < 2 || urStringList.size() < 2)
	{
	  return 4;
	}

      bool conversionSuccess;
      double xLeft = llStringList.at(0).toDouble(&conversionSuccess);
      if(!conversionSuccess){return 5;}
      double yLow = llStringList.at(1).toDouble(&conversionSuccess);
      if(!conversionSuccess){return 5;}
      double xRight = urStringList.at(0).toDouble(&conversionSuccess);
      if(!conversionSuccess){return 5;}
      double yUp = urStringList.at(1).toDouble(&conversionSuccess);
      if(!conversionSuccess){return 5;}

      extent->setXmin(xLeft);
      extent->setYmin(yLow);
      extent->setXmax(xRight);
      extent->setYmax(yUp);
    }
  else if(bboxName == "Box")
    {
      QDomElement boxElement = childNode.toElement();
      QDomNodeList coordinatesList = boxElement.elementsByTagNameNS(GML_NAMESPACE, "coordinates");
      if(coordinatesList.length() < 1)
	{
	  return 11;
	}
      QStringList coordinatesStringList = coordinatesList.at(0).toElement().text().split(" ", QString::SkipEmptyParts);
      if(coordinatesStringList.size() < 2)
	{
	  return 12;
	}
      QStringList lowCoordinateList = coordinatesStringList.at(0).split(",", QString::SkipEmptyParts);
      if(lowCoordinateList.size() < 2)
	{
	  return 13;
	}
      
      bool conversionSuccess;
      double xLow = lowCoordinateList.at(0).toDouble(&conversionSuccess);
      if(conversionSuccess)
	{
	  extent->setXmin(xLow);
	}
      else
	{
	  return 14;
	}
      double yLow = lowCoordinateList.at(1).toDouble(&conversionSuccess);
      if(conversionSuccess)
	{
	  extent->setYmin(yLow);
	}
      else
	{
	  return 15;
	}
      QStringList upCoordinateList = coordinatesStringList.at(1).split(",", QString::SkipEmptyParts);
      if(upCoordinateList.size() < 2)
	{
	  return 16;
	}
      double xUp = upCoordinateList.at(0).toDouble(&conversionSuccess);
      if(conversionSuccess)
	{
	  extent->setXmax(xUp);
	}
      else
	{
	  return 17;
	}
      double yUp = upCoordinateList.at(1).toDouble(&conversionSuccess);
      if(conversionSuccess)
	{
	  extent->setYmax(yUp);
	}
      else
	{
	  return 18;
	}
    }
  return 0;
}

int QgsWFSProvider::getFeaturesFromGML3(const QDomElement& wfsCollectionElement, const QString& geometryAttribute, std::vector<QgsFeature*>& features) const
{
  QDomNodeList featureTypeNodeList = wfsCollectionElement.elementsByTagNameNS(GML_NAMESPACE, "featureMember");
  QDomElement currentFeatureMemberElem;
  QDomElement layerNameElem;
  QDomNode currentAttributeChild;
  QDomElement currentAttributeElement;
  int counter = 0;
  QgsFeature* f = 0;
  unsigned char* wkb = 0;
  int wkbSize = 0;
  QGis::WKBTYPE currentType;

  for(int i = 0; i < featureTypeNodeList.size(); ++i)
    {
      f = new QgsFeature(counter);
      currentFeatureMemberElem = featureTypeNodeList.at(i).toElement();
      //the first child element is always <namespace:layer>
      layerNameElem = currentFeatureMemberElem.firstChild().toElement();
      //the children are the attributes
      currentAttributeChild = layerNameElem.firstChild();
      while(!currentAttributeChild.isNull())
	{
	  currentAttributeElement = currentAttributeChild.toElement();
	  if(currentAttributeElement.localName() != "boundedBy")
	    {
	      if((currentAttributeElement.localName()) != geometryAttribute) //a normal attribute
		{
		  f->addAttribute(currentAttributeElement.localName(), currentAttributeElement.text(), false);
		}
	      else //a geometry attribute
		{
		  getWkbFromGML3(currentAttributeElement, &wkb, &wkbSize, &currentType);
		  mWKBType = currentType; //a more sophisticated method is necessary
		  f->setGeometryAndOwnership(wkb, wkbSize);
		}
	    }
	  currentAttributeChild = currentAttributeChild.nextSibling();
	}
      if(wkb && wkbSize > 0)
	{
	  features.push_back(f);
	}
      ++counter;
    }
  return 1;
}

int QgsWFSProvider::getWkbFromGML3(const QDomNode& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const
{
  QDomNode geometryChild = geometryElement.firstChild();
  if(geometryChild.isNull())
    {
      return 1;
    }
  QDomElement geometryTypeElement = geometryChild.toElement();
  QString geomType = geometryTypeElement.localName();
  if(geomType == "Point")
    {
      return getWkbFromGML3Point(geometryTypeElement, wkb, wkbSize, type);
    }
  else if(geomType == "Polygon")
    {
      return getWkbFromGML3Polygon(geometryTypeElement, wkb, wkbSize, type);
    }
  else if(geomType == "LineString")
    {
      return getWkbFromGML3LineString(geometryTypeElement, wkb, wkbSize, type);
    }
  else if(geomType == "MultiCurve")
    {
      return getWkbFromGML3MultiCurve(geometryTypeElement, wkb, wkbSize, type); 
    }
  else if(geomType == "MultiSurface")
    {
      return getWkbFromGML3MultiSurface(geometryTypeElement, wkb, wkbSize, type);
    }
  else //unknown type
    {
      *wkb = 0;
      *wkbSize = 0;
    }
  return 0;
}

int QgsWFSProvider::getWkbFromGML3Point(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const
{
  QDomNodeList posList = geometryElement.elementsByTagNameNS(GML_NAMESPACE, "pos");
  if(posList.size() < 1)
    {
      return 1;
    }
  QDomElement posElement = posList.at(0).toElement();
  std::list<QgsPoint> pointCoordinate;
  if(readCoordinatesFromPosList(pointCoordinate, posElement) != 0)
    {
      return 2;
    }
  
  if(pointCoordinate.size() < 1)
    {
      return 3;
    }
  
  std::list<QgsPoint>::const_iterator point_it = pointCoordinate.begin();
  char e = endian();
  double x = point_it->x();
  double y = point_it->y();
  int size = 1 + sizeof(int) + 2 * sizeof(double);
  *wkb = new unsigned char[size];
  *wkbSize = size;
  *type = QGis::WKBPoint;
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  memcpy(&(*wkb)[wkbPosition], &e, 1);
  wkbPosition += 1;
  memcpy(&(*wkb)[wkbPosition], type, sizeof(int));
  wkbPosition += sizeof(int);
  memcpy(&(*wkb)[wkbPosition], &x, sizeof(double));
  wkbPosition += sizeof(double);
  memcpy(&(*wkb)[wkbPosition], &y, sizeof(double));
  return 0;
}

int QgsWFSProvider::getWkbFromGML3LineString(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const
{
  QDomNodeList posList = geometryElement.elementsByTagNameNS(GML_NAMESPACE, "posList");
  if(posList.size() < 1)
    {
      return 1;
    }
  QDomElement posListElement = posList.at(0).toElement();
  std::list<QgsPoint> lineCoordinates;
  if(readCoordinatesFromPosList(lineCoordinates, posListElement) != 0)
    {
      return 2;
    }
  char e = endian();
  int size = 1 + 2 * sizeof(int) + lineCoordinates.size() * 2* sizeof(double);
  *wkb = new unsigned char[size];
  *wkbSize = size;
  *type = QGis::WKBLineString;
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  double x, y;
  int nPoints = lineCoordinates.size();
  
  //fill the contents into *wkb
  memcpy(&(*wkb)[wkbPosition], &e, 1);
  wkbPosition += 1;
  memcpy(&(*wkb)[wkbPosition], type, sizeof(int));
  wkbPosition += sizeof(int);
  memcpy(&(*wkb)[wkbPosition], &nPoints, sizeof(int));
  wkbPosition += sizeof(int);
  
  std::list<QgsPoint>::const_iterator iter;
  for(iter = lineCoordinates.begin(); iter != lineCoordinates.end(); ++iter)
    {
      x = iter->x();
      y = iter->y();
      memcpy(&(*wkb)[wkbPosition], &x, sizeof(double));
      wkbPosition += sizeof(double);
      memcpy(&(*wkb)[wkbPosition], &y, sizeof(double));
      wkbPosition += sizeof(double);
    }
  return 0;
}

int QgsWFSProvider::getWkbFromGML3Polygon(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const
{
  QDomNodeList exteriorList = geometryElement.elementsByTagNameNS(GML_NAMESPACE, "exterior");
  if(exteriorList.size() < 1) //exterior ring is necessary
    {
      return 1;
    }
  //then take the <gml:interior> elements into account
  QDomNodeList interiorList = geometryElement.elementsByTagNameNS(GML_NAMESPACE, "interior");
  
  //read all the coordinates (as QgsPoint) into memory. Each linear ring has an entry in the vector
  std::vector<std::list<QgsPoint> > ringCoordinates;
  
  //exterior ring
  QDomElement posListElement = exteriorList.at(0).firstChild().firstChild().toElement();
  std::list<QgsPoint> exteriorPointList;
  if(readCoordinatesFromPosList(exteriorPointList, posListElement) != 0)
    {
      return 2;
    }
  ringCoordinates.push_back(exteriorPointList);
  
  //interior rings
  for(int i = 0; i < interiorList.size(); ++i)
    {
      posListElement = interiorList.at(i).firstChild().firstChild().toElement();
      std::list<QgsPoint> interiorPointList;
      if(readCoordinatesFromPosList(interiorPointList, posListElement) != 0)
	{
	  return 3;
	}
      ringCoordinates.push_back(interiorPointList);
    }
  
  //calculate number of bytes to allocate
  int nrings = 1 + interiorList.size();
  int npoints = 0;//total number of points
  for(std::vector<std::list<QgsPoint> >::const_iterator it = ringCoordinates.begin(); it != ringCoordinates.end(); ++it)
    {
      npoints += it->size();
    } 
  int size = 1 + 2 * sizeof(int) + nrings * sizeof(int) + 2 * npoints * sizeof(double);
  *wkb = new unsigned char[size];
  *wkbSize = size;
  *type = QGis::WKBPolygon;
  char e = endian();
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  int nPointsInRing = 0;
  double x, y;
  
  //fill the contents into *wkb
  memcpy(&(*wkb)[wkbPosition], &e, 1);
  wkbPosition += 1;
  memcpy(&(*wkb)[wkbPosition], type, sizeof(int));
  wkbPosition += sizeof(int);
  memcpy(&(*wkb)[wkbPosition], &nrings, sizeof(int));
  wkbPosition += sizeof(int);
  for(std::vector<std::list<QgsPoint> >::const_iterator it = ringCoordinates.begin(); it != ringCoordinates.end(); ++it)
    {
      nPointsInRing = it->size(); 
      memcpy(&(*wkb)[wkbPosition], &nPointsInRing, sizeof(int));
      wkbPosition += sizeof(int);
      //iterate through the string list converting the strings to x-/y- doubles
      std::list<QgsPoint>::const_iterator iter;
      for(iter = it->begin(); iter != it->end(); ++iter)
	{
	  x = iter->x();
	  y = iter->y();
	  //qWarning("currentCoordinate: " + QString::number(x) + " // " + QString::number(y));
	  memcpy(&(*wkb)[wkbPosition], &x, sizeof(double));
	  wkbPosition += sizeof(double);
	  memcpy(&(*wkb)[wkbPosition], &y, sizeof(double));
	  wkbPosition += sizeof(double);
	}
    }
  return 0;
}

int QgsWFSProvider::getWkbFromGML3MultiSurface(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const
{
  //get <surfaceMembers> tag
  QDomNodeList surfaceMembersList = geometryElement.elementsByTagNameNS(GML_NAMESPACE, "surfaceMembers");
  if(surfaceMembersList.size() < 1)
    {
      return 1;
    }
  QDomElement surfaceMembersElement = surfaceMembersList.at(0).toElement();
  
  QDomNodeList polygonNodeList = surfaceMembersElement.elementsByTagNameNS(GML_NAMESPACE, "Polygon");
  
  //first list: different polygons, second list: different rings, third list: different points
  std::list<std::list<std::list<QgsPoint> > > multiPolygonPoints;
  QDomNodeList exteriorNodeList;
  QDomElement currentExteriorElement;
  QDomNodeList interiorNodeList;
  QDomElement currentInteriorElement;
  QDomNodeList linearRingNodeList;
  QDomElement linearRingElement;
  QDomNodeList posListNodeList;
  QDomElement posListElement;
  QDomElement currentPolygonElement;
  
  for(int i = 0; i < polygonNodeList.size(); ++i)
    {
      currentPolygonElement = polygonNodeList.at(i).toElement();
      std::list<std::list<QgsPoint> > currentPolygonList;
      
      //find exterior ring
      exteriorNodeList = currentPolygonElement.elementsByTagNameNS(GML_NAMESPACE, "exterior");
      for(int i = 0; i < exteriorNodeList.size(); ++i) //normally only one
	{
	  currentExteriorElement = exteriorNodeList.at(i).toElement();
	  linearRingNodeList = currentExteriorElement.elementsByTagNameNS(GML_NAMESPACE, "LinearRing");
	  if(linearRingNodeList.size() < 1)
	    {
	      continue;
	    }
	  linearRingElement = linearRingNodeList.at(i).toElement();
	  posListNodeList = linearRingElement.elementsByTagNameNS(GML_NAMESPACE, "posList");
	  if(posListNodeList.size() < 1)
	    {
	      continue;
	    }
	  posListElement = posListNodeList.at(i).toElement();
	  std::list<QgsPoint> ringCoordinates;
	  if(readCoordinatesFromPosList(ringCoordinates, posListElement) != 0)
	    {
	      continue;
	    }
	  currentPolygonList.push_back(ringCoordinates);
	}
      //find interior rings
      interiorNodeList = currentPolygonElement.elementsByTagNameNS(GML_NAMESPACE, "interior");
      for(int i = 0; i < interiorNodeList.size(); ++i)
	{
	  currentInteriorElement = interiorNodeList.at(i).toElement();
	  linearRingNodeList = currentInteriorElement.elementsByTagNameNS(GML_NAMESPACE, "LinearRing");
	  if(linearRingNodeList.size() < 1)
	    {
	      continue;
	    }
	  linearRingElement = linearRingNodeList.at(i).toElement();
	  posListNodeList = linearRingElement.elementsByTagNameNS(GML_NAMESPACE, "posList");
	  if(posListNodeList.size() < 1)
	    {
	      continue;
	    }
	  posListElement = posListNodeList.at(i).toElement();
	  std::list<QgsPoint> ringCoordinates;
	  if(readCoordinatesFromPosList(ringCoordinates, posListElement) != 0)
	    {
	      continue;
	    }
	  currentPolygonList.push_back(ringCoordinates);
	}
      multiPolygonPoints.push_back(currentPolygonList);
    }
  
  int size = 1 + 2 * sizeof(int);
  //calculate the wkb size
  for(std::list<std::list<std::list<QgsPoint> > >::const_iterator it = multiPolygonPoints.begin(); it != multiPolygonPoints.end(); ++it)
    {
      size += 1 + 2 * sizeof(int);
      for(std::list<std::list<QgsPoint> >::const_iterator iter = it->begin(); iter != it->end(); ++iter)
	{
	  size += sizeof(int) + 2 * iter->size() * sizeof(double);
	}
    }
  *wkb = new unsigned char[size];
  *wkbSize = size;
  *type = QGis::WKBMultiPolygon;
  int polygonType = QGis::WKBPolygon;
  char e = endian();
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  double x, y;
  int nPolygons = multiPolygonPoints.size();
  int nRings;
  int nPointsInRing;
  
  //fill the contents into *wkb
  memcpy(&(*wkb)[wkbPosition], &e, 1);
  wkbPosition += 1;
  memcpy(&(*wkb)[wkbPosition], type, sizeof(int));
  wkbPosition += sizeof(int);
  memcpy(&(*wkb)[wkbPosition], &nPolygons, sizeof(int));
  wkbPosition += sizeof(int);
  
  for(std::list<std::list<std::list<QgsPoint> > >::const_iterator it = multiPolygonPoints.begin(); it != multiPolygonPoints.end(); ++it)
    {
      memcpy(&(*wkb)[wkbPosition], &e, 1);
      wkbPosition += 1;
      memcpy(&(*wkb)[wkbPosition], &polygonType, sizeof(int));
      wkbPosition += sizeof(int);
      nRings = it->size();
      memcpy(&(*wkb)[wkbPosition], &nRings, sizeof(int));
      wkbPosition += sizeof(int);
      for(std::list<std::list<QgsPoint> >::const_iterator iter = it->begin(); iter != it->end(); ++iter)
	{
	  nPointsInRing = iter->size();
	  memcpy(&(*wkb)[wkbPosition], &nPointsInRing, sizeof(int));
	  wkbPosition += sizeof(int);
	  for(std::list<QgsPoint>::const_iterator iterator = iter->begin(); iterator != iter->end(); ++iterator)
	    {
	      x = iterator->x();
	      y = iterator->y();
	      memcpy(&(*wkb)[wkbPosition], &x, sizeof(double));
	      wkbPosition += sizeof(double);
	      memcpy(&(*wkb)[wkbPosition], &y, sizeof(double));
	      wkbPosition += sizeof(double);
	    }
	}
    }
  return 0;
}

int QgsWFSProvider::getWkbFromGML3MultiCurve(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const
{
  QDomNodeList curveMembersList = geometryElement.elementsByTagNameNS(GML_NAMESPACE, "curveMembers");
  if(curveMembersList.size() < 1)
    {
      return 1;
    }
  QDomElement curveMembersElement = curveMembersList.at(0).toElement();
  //analyze the <LineString> tags
  QDomNodeList LineStringNodeList = geometryElement.elementsByTagNameNS(GML_NAMESPACE, "LineString");
  //the first list contains the lines, the second one the coordinates for each line 
  std::list<std::list<QgsPoint> > lineCoordinates;
  QDomElement currentLineStringElement;
  QDomNodeList currentPosList;
  
  for(int i = 0; i < LineStringNodeList.size(); ++i)
    {
      currentLineStringElement = LineStringNodeList.at(i).toElement();
      //posList element
      currentPosList = currentLineStringElement.elementsByTagNameNS(GML_NAMESPACE, "posList");
      if(currentPosList.size() < 1)
	{
	  return 2;
	}
      std::list<QgsPoint> currentPointList;
      if(readCoordinatesFromPosList(currentPointList, currentPosList.at(0).toElement()) != 0)
	{
	  return 3;
	}
      lineCoordinates.push_back(currentPointList);
    }
  
  //calculate the required wkb size
  int size = (lineCoordinates.size() + 1) * (1 + 2 * sizeof(int));
  for(std::list<std::list<QgsPoint> >::const_iterator it = lineCoordinates.begin(); it != lineCoordinates.end(); ++it)
    {
      size += it->size() * 2 * sizeof(double);
    }
  *wkb = new unsigned char[size];
  *wkbSize = size;
  *type = QGis::WKBMultiLineString;
  
  //fill the wkb content
  char e = endian();
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  int nLines = lineCoordinates.size();
  int nPoints; //number of points in a line
  double x, y;
  memcpy(&(*wkb)[wkbPosition], &e, 1);
  wkbPosition += 1;
  memcpy(&(*wkb)[wkbPosition], type, sizeof(int));
  wkbPosition += sizeof(int);
  memcpy(&(*wkb)[wkbPosition], &nLines, sizeof(int));
  wkbPosition += sizeof(int);
  for(std::list<std::list<QgsPoint> >::const_iterator it = lineCoordinates.begin(); it != lineCoordinates.end(); ++it)
    {
      memcpy(&(*wkb)[wkbPosition], &e, 1);
      wkbPosition += 1;
      memcpy(&(*wkb)[wkbPosition], type, sizeof(int));
      wkbPosition += sizeof(int);
      nPoints = it->size();
      memcpy(&(*wkb)[wkbPosition], &nPoints, sizeof(int));
      wkbPosition += sizeof(int);
      for(std::list<QgsPoint>::const_iterator iter = it->begin(); iter != it->end(); ++iter)
	{
	  x = iter->x();
	  //qWarning("x is: " + QString::number(x));
	  y = iter->y();
	  //qWarning("y is: " + QString::number(y));
	  memcpy(&(*wkb)[wkbPosition], &x, sizeof(double));
	  wkbPosition += sizeof(double);
	  memcpy(&(*wkb)[wkbPosition], &y, sizeof(double));
	  wkbPosition += sizeof(double);
	}
    }
  return 0;
}

int QgsWFSProvider::readCoordinatesFromPosList(std::list<QgsPoint>& coords, const QDomElement elem) const
{
  //srsDimension
  int srsDimension;
  int srsDiff; //difference betwen srsDimension and 2
  QString srsDimensionString = elem.attributeNS(GML_NAMESPACE, "srsDimension");
  if(srsDimensionString.isEmpty())
    {
      srsDimension = 2;
    }
  else
    {
      bool conversionSuccess;
      srsDimension = srsDimensionString.toInt(&conversionSuccess);
      if(!conversionSuccess)
	{
	  srsDimension = 2;
	}
    }

  if(srsDimension < 2)
    {
      return 1;
    }
  srsDiff = srsDimension - 2;

  coords.clear();
  QStringList coordinateStringList = elem.text().split(" ", QString::SkipEmptyParts);
  QStringList::const_iterator iter;

  bool conversionSuccess;
  double currentX;
  double currentY;
  for(iter = coordinateStringList.constBegin(); iter != coordinateStringList.constEnd(); ++iter)
	{
	  currentX = iter->toDouble(&conversionSuccess);
	  if(!conversionSuccess)
	    {
	      coords.clear();
	      return 1;
	    }
	  ++iter;
	  if(iter == coordinateStringList.constEnd()) //odd number of doubles
	    {
	      coords.clear();
	      return 2;
	    }
	  currentY = iter->toDouble(&conversionSuccess);
	  if(!conversionSuccess)
	    {
	      coords.clear();
	      return 1;
	    }
	  for(int i = 0; i < srsDiff; ++i) //for higher dimensional entries
	    {
	      ++iter;
	    }
	  coords.push_back(QgsPoint(currentX, currentY));
	  //qWarning("adding coordinates " + QString::number(currentX) + " // " + QString::number(currentY) + " to coordinates list");
	}
  return 0;
}

int QgsWFSProvider::setSRSFromGML3(const QDomElement& wfsCollectionElement)
{
  QgsDebugMsg("entering QgsWFSProvider::setSRSFromGML");
  //search <gml:boundedBy>
  QDomNodeList boundedByList = wfsCollectionElement.elementsByTagNameNS(GML_NAMESPACE, "boundedBy");
  if(boundedByList.size() < 1)
    {
      QgsDebugMsg("Error, could not find boundedBy element");
      return 1;
    }
  //search <gml:Envelope>
  QDomElement boundedByElem = boundedByList.at(0).toElement();
  QDomNodeList envelopeList = boundedByElem.elementsByTagNameNS(GML_NAMESPACE, "Envelope");
  if(envelopeList.size() < 1)
    {
      QgsDebugMsg("Error, could not find Envelope element");
      return 2;
    }
  QDomElement envelopeElem = envelopeList.at(0).toElement();
  //getAttribute 'srsName'
  QString srsName = envelopeElem.attribute("srsName");
  if(srsName.isEmpty())
    {
      QgsDebugMsg("Error, srsName is empty");
      return 3;
    }
  QgsDebugMsg("srsName is: " +srsName);
  mSourceSRS = new QgsSpatialRefSys();
  if(!mSourceSRS->createFromOgcWmsCrs(srsName))
    {
      QgsDebugMsg("Error, creation of QgsSpatialRefSys failed");
      delete mSourceSRS;
      mSourceSRS = 0;
      return 4;
    }
  return 0;
}
#endif //0 //methods for reading GML3
