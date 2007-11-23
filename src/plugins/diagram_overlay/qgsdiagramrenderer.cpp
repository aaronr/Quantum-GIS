#include "qgsdiagramrenderer.h"
#include "qgsdiagramfactory.h"
#include "qgsfeature.h"
#include <cmath>
#include <QDomElement>

QgsDiagramRenderer::QgsDiagramRenderer(const QList<int>& classificationAttributes): mClassificationAttributes(classificationAttributes)
{
}

QgsDiagramRenderer::~QgsDiagramRenderer()
{
  delete mFactory;
}

QgsDiagramRenderer::QgsDiagramRenderer()
{
}

QImage* QgsDiagramRenderer::renderDiagram(const QgsFeature& f) const
{
  if(!mFactory)
    {
      return 0;
    }

  //only scaling according to attributes does not need any items
  if(mItemInterpretation != QgsDiagramRenderer::ATTRIBUTE && mItems.size() < 1)
    {
      return 0;
    }

  int size;
  if(calculateDiagramSize(f, size) != 0)
    {
      return 0;
    }
 
  return mFactory->createDiagram(size, f);
}

int QgsDiagramRenderer::getDiagramDimensions(int& width, int& height, const QgsFeature& f) const
{
  //first find out classification value
  if(!mFactory || mItems.size() < 1)
    {
      return 1;
    }

  int size;
  if(calculateDiagramSize(f, size) != 0)
    {
      return 2;
    }
 
  if(mFactory->getDiagramDimensions(size, f, width, height) != 0)
    {
      return 3;
    }
  return 0;
}

int QgsDiagramRenderer::classificationValue(const QgsFeature& f, QVariant& value) const
{
  //find out attribute value of the feature
  QgsAttributeMap featureAttributes = f.attributeMap();
  
  QgsAttributeMap::const_iterator iter;
  
  if(value.type() == QVariant::String) //string type
    {
      //we can only handle one classification field for strings
      if(mClassificationAttributes.size() > 1)
	{
	  return 1;
	}
      
      iter = featureAttributes.find(mClassificationAttributes.first());
      if(iter == featureAttributes.constEnd())
	{
	  return 2;
	}
      value = iter.value();
    }
  else //numeric type
    {
      double currentValue;
      double totalValue = 0;

      QList<int>::const_iterator list_it = mClassificationAttributes.constBegin();
      for(; list_it != mClassificationAttributes.constEnd(); ++list_it)
	{
	  QgsAttributeMap::const_iterator iter = featureAttributes.find(*list_it);
	  if(iter == featureAttributes.constEnd())
	    {
	      continue;
	    }
	  currentValue = iter.value().toDouble();
	  totalValue += currentValue;
	}
      value = QVariant(totalValue);
    }
  return 0;
}

void QgsDiagramRenderer::addClassificationAttribute(int attrNr)
{
  mClassificationAttributes.push_back(attrNr);
}

bool QgsDiagramRenderer::readXML(const QDomNode& rendererNode)
{
  QDomElement rendererElem = rendererNode.toElement();

  //items
  QList<QgsDiagramItem> itemList;
  bool conversionOk;

  QString interpretationName = rendererNode.toElement().attribute("item_interpretation");
  if(interpretationName == "discrete")
    {
      mItemInterpretation = QgsDiagramRenderer::DISCRETE;
    }
  else if(interpretationName == "linear")
    {
      mItemInterpretation = QgsDiagramRenderer::LINEAR;
    }
  else if(interpretationName == "attribute")
    {
      mItemInterpretation = QgsDiagramRenderer::ATTRIBUTE;
    }
  else if(interpretationName == "constant")
    {
      mItemInterpretation = QgsDiagramRenderer::CONSTANT;
    }

  QDomNodeList itemNodeList = rendererElem.elementsByTagName("diagramitem"); 
  for(int i = 0; i < itemNodeList.size(); ++i)
    {
      QgsDiagramItem currentItem;
      QVariant currentValue;
      currentItem.size = itemNodeList.at(i).toElement().attribute("size").toInt();
      currentValue = QVariant(itemNodeList.at(i).toElement().attribute("value").toDouble(&conversionOk));
      if(!conversionOk) //string data?
	{
	  currentValue = QVariant(itemNodeList.at(i).toElement().attribute("value"));
	}
      currentItem.value = currentValue;
      itemList.push_back(currentItem);
    }
  setDiagramItems(itemList);
  return true;
}

bool QgsDiagramRenderer::writeXML(QDomNode& overlay_node, QDomDocument& doc) const
{
  QDomElement rendererElement = doc.createElement("renderer");

  //write interpolation to xml file
  QString interpretationName;
  if(mItemInterpretation == QgsDiagramRenderer::DISCRETE)
    {
      interpretationName = "discrete";
    }
  else if(mItemInterpretation == QgsDiagramRenderer::LINEAR)
    {
      interpretationName = "linear";
    }
  else if(mItemInterpretation == QgsDiagramRenderer::ATTRIBUTE)
    {
      interpretationName = "attribute";
    }
  else if(mItemInterpretation == QgsDiagramRenderer::CONSTANT)
    {
      interpretationName = "constant";
    }
  rendererElement.setAttribute("item_interpretation", interpretationName);

  QList<QgsDiagramItem>::const_iterator item_it = mItems.constBegin();
  for(; item_it != mItems.constEnd(); ++item_it)
    {
      QDomElement itemElement = doc.createElement("diagramitem");
      itemElement.setAttribute("size", item_it->size);
      itemElement.setAttribute("value", item_it->value.toString());
      rendererElement.appendChild(itemElement);
    }

  overlay_node.appendChild(rendererElement);
  return true;
}

int QgsDiagramRenderer::createLegendContent(QMap<QString, QImage*> items) const
{
  if(!mFactory || mItems.size() < 1)
    {
      return 1;
    }

  //determine a size and value for the legend, use the middle item for this
  int element = (int)(mItems.size()/2);
  QString value = mItems.at(element).value.toString();
  int size = mItems.at(element).size;

  if(mFactory->createLegendContent(size, value, items) != 0)
    {
      return 2;
    }
  return 0;
}

int QgsDiagramRenderer::calculateDiagramSize(const QgsFeature& f, int& size) const
{
  //find out value for classificationAttribute
  QVariant value;
  if(classificationValue(f, value) != 0)
    {
      return 1;
    }

  if(mItemInterpretation == ATTRIBUTE)
    {
      size = value.toInt();
      return 0;
    }

  if(mItems.size() < 1)
    {
      return 2;
    }

  if(mItemInterpretation == CONSTANT)
    {
      size = mItems.constBegin()->size;
      return 0; 
    }

  //find out size
  bool sizeAssigned = false;

  QList<QgsDiagramItem>::const_iterator current_it = mItems.constBegin();
  QList<QgsDiagramItem>::const_iterator last_it = mItems.constEnd();
  
  if(value.type() == QVariant::String) //string types are handled differently
    {
      for(; current_it != mItems.constEnd(); ++current_it)
	{
	  if(current_it->value.toString() == value.toString())
	    {
	      size = current_it->size;
	      sizeAssigned = true;
	    }
	}
      if(!sizeAssigned)
	{
	  return 3;
	}
    }
  else //numerical types
    {
      for(; current_it != mItems.constEnd(); ++current_it)
	{
	  if(value.toDouble() < current_it->value.toDouble())
	    {
	      if(last_it == mItems.constEnd()) //values below classifications receive first items size
	    {
	      size = current_it->size;
	    }
	      else
		{
		  size = interpolateSize(value.toDouble(), last_it->value.toDouble(), \
					 current_it->value.toDouble(),	\
					 last_it->size, current_it->size);
		}
	      sizeAssigned = true;
	      break;
	    }
	  last_it = current_it;
	}

      if(!sizeAssigned)//values above classification receive last items size
	{
	  size = last_it->size;
	}
    }

  return 0;
}

int QgsDiagramRenderer::interpolateSize(double value, double lowerValue, double upperValue, \
						       int lowerSize, int upperSize) const
{
  switch(mItemInterpretation)
    {
    case DISCRETE:
      return lowerSize;
      break;

    case LINEAR:
      {
	if(value <= lowerValue)
	  {
	    return lowerSize;
	  }
	else if(value >= upperValue)
	  {
	    return upperSize;
	  }
	
	QgsDiagramFactory::SizeType t;
	if(mFactory)
	  {
	    t = mFactory->sizeType();
	  }
	
	if(!mFactory || t == QgsDiagramFactory::HEIGHT)
	  {
	    //do one dimensional linear interpolation
	    return (int)(((value - lowerValue) * upperSize + (upperValue - value) * lowerSize) / (upperValue - lowerValue));
	  }
	else if(t == QgsDiagramFactory::DIAMETER)
	  {
	    double lowerArea = (lowerSize/2)*(lowerSize/2)*M_PI;
	    double upperArea = (upperSize/2)*(upperSize/2)*M_PI;
	    double valueArea = ((value - lowerValue) * upperArea + (upperValue - value) * lowerArea)/(upperValue - lowerValue);
	    return (int)(2*sqrt(valueArea/M_PI));
	  }
      }
    default:
      return 1;
    }
  return 0; //something went wrong if we arrive here
}
