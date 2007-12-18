/***************************************************************************
                         qgsuniquevaluerenderer.cpp  -  description
                             -------------------
    begin                : July 2004
    copyright            : (C) 2004 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id: qgsuniquevaluerenderer.cpp 5371 2006-04-25 01:52:13Z wonder $ */

#include "qgsuniquevaluerenderer.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgssymbol.h"
#include "qgssymbologyutils.h"

#include <QDomNode>
#include <QPainter>
#include <QImage>
#include <vector>

QgsUniqueValueRenderer::QgsUniqueValueRenderer(QGis::VectorType type): mClassificationField(0)
{
    mVectorType = type;
}

QgsUniqueValueRenderer::QgsUniqueValueRenderer(const QgsUniqueValueRenderer& other)
{
    mVectorType = other.mVectorType;
    mClassificationField = other.mClassificationField;
    QMap<QString, QgsSymbol*> s = other.mSymbols;
    for(QMap<QString, QgsSymbol*>::iterator it=s.begin(); it!=s.end(); ++it)
    {
	QgsSymbol* s = new QgsSymbol(* it.value());
	insertValue(it.key(), s);
    }
}

QgsUniqueValueRenderer& QgsUniqueValueRenderer::operator=(const QgsUniqueValueRenderer& other)
{
    if(this != &other)
    {
        mVectorType = other.mVectorType;
        mClassificationField = other.mClassificationField;
        clearValues();
        for(QMap<QString, QgsSymbol*>::iterator it=mSymbols.begin(); it!=mSymbols.end(); ++it)
        {
            QgsSymbol* s = new QgsSymbol(*it.value());
            insertValue(it.key(), s);
        }
    }
    return *this;
}

QgsUniqueValueRenderer::~QgsUniqueValueRenderer()
{
    for(QMap<QString,QgsSymbol*>::iterator it=mSymbols.begin();it!=mSymbols.end();++it)
    {
	delete it.value();
    }
}

const QList<QgsSymbol*> QgsUniqueValueRenderer::symbols() const
{
    QList <QgsSymbol*> symbollist;
    for(QMap<QString, QgsSymbol*>::const_iterator it = mSymbols.begin(); it!=mSymbols.end(); ++it)
    {
	symbollist.append(it.value());
    }
    return symbollist;
}

void QgsUniqueValueRenderer::insertValue(QString name, QgsSymbol* symbol)
{
    mSymbols.insert(name, symbol);
}

void QgsUniqueValueRenderer::setClassificationField(int field)
{
    mClassificationField=field;
}

int QgsUniqueValueRenderer::classificationField()
{
    return mClassificationField;
}

bool QgsUniqueValueRenderer::willRenderFeature(QgsFeature *f)
{
  return (symbolForFeature(f) != 0);
}
    
void QgsUniqueValueRenderer::renderFeature(QPainter* p, QgsFeature& f,QImage* img, 
	double* scalefactor, bool selected, double widthScale)
{
  QgsSymbol* symbol = symbolForFeature(&f);
  if(!symbol) //no matching symbol
    {
      if ( img && mVectorType == QGis::Point )
	{
	  img->fill(0);
	}
      else if ( mVectorType != QGis::Point )
	{
	  p->setPen(Qt::NoPen);
	  p->setBrush(Qt::NoBrush);
	}
      return;
    }
  
  // Point 
  if ( img && mVectorType == QGis::Point ) 
    {
      *img = symbol->getPointSymbolAsImage(  widthScale, selected, mSelectionColor );
      if ( scalefactor ) 
	{
	  *scalefactor = 1;
	}
    } 
  
  // Line, polygon
  else if ( mVectorType != QGis::Point )
    {
      if( !selected ) 
	{
	  QPen pen=symbol->pen();
	  pen.setWidthF ( widthScale * pen.width() );
	  p->setPen(pen);
	  p->setBrush(symbol->brush());
	}
      else
	{
	  QPen pen=symbol->pen();
	  pen.setWidthF ( widthScale * pen.width() );
	  pen.setColor(mSelectionColor);
	  QBrush brush=symbol->brush();
		brush.setColor(mSelectionColor);
		p->setPen(pen);
		p->setBrush(brush);
	}
    }
}

QgsSymbol* QgsUniqueValueRenderer::symbolForFeature(const QgsFeature* f)
{
  //first find out the value
  const QgsAttributeMap& attrs = f->attributeMap();
  QString value = attrs[mClassificationField].toString();
  
  QMap<QString,QgsSymbol*>::iterator it=mSymbols.find(value);
  if(it == mSymbols.end())
    {
      return 0;
    }
  else
    {
      return it.value();
    }
}

void QgsUniqueValueRenderer::readXML(const QDomNode& rnode, QgsVectorLayer& vl)
{
    mVectorType = vl.vectorType();
    QDomNode classnode = rnode.namedItem("classificationfield");
    int classificationfield = classnode.toElement().text().toInt();
    this->setClassificationField(classificationfield);

    QDomNode symbolnode = rnode.namedItem("symbol");
    while (!symbolnode.isNull())
    {
	QgsSymbol* msy = new QgsSymbol(mVectorType);
	msy->readXML ( symbolnode );
	this->insertValue(msy->lowerValue(),msy);
	symbolnode = symbolnode.nextSibling();
    vl.setRenderer(this);
    }
}

void QgsUniqueValueRenderer::clearValues()
{
    for(QMap<QString,QgsSymbol*>::iterator it=mSymbols.begin();it!=mSymbols.end();++it)
    {
	delete it.value();
    }
    mSymbols.clear();
}

QString QgsUniqueValueRenderer::name() const
{
    return "Unique Value";
}

QgsAttributeList QgsUniqueValueRenderer::classificationAttributes() const
{
    QgsAttributeList list;
    list.append(mClassificationField);
    return list;
}

bool QgsUniqueValueRenderer::writeXML( QDomNode & layer_node, QDomDocument & document ) const
{
    bool returnval=true;
    QDomElement uniquevalue=document.createElement("uniquevalue");
    layer_node.appendChild(uniquevalue);
    QDomElement classificationfield=document.createElement("classificationfield");
    QDomText classificationfieldtxt=document.createTextNode(QString::number(mClassificationField));
    classificationfield.appendChild(classificationfieldtxt);
    uniquevalue.appendChild(classificationfield);
    for(QMap<QString,QgsSymbol*>::const_iterator it=mSymbols.begin();it!=mSymbols.end();++it)
    {
	if(!(it.value()->writeXML(uniquevalue,document)))
	{
	    returnval=false;  
	}
    }
    return returnval;
}

QgsRenderer* QgsUniqueValueRenderer::clone() const
{
    QgsUniqueValueRenderer* r = new QgsUniqueValueRenderer(*this);
    return r;
}
