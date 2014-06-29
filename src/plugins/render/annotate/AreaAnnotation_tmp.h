//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Andrew Manson            <g.real.ate@gmail.com>
// Copyright 2013      Thibaut Gridel           <tgridel@free.fr>
// Copyright 2014      Calin-Cristian Cruceru   <crucerucalincristian@gmail.com
//

#ifndef AREAANNOTATION_H
#define AREAANNOTATION_H

#include "SceneGraphicsItem.h"
#include "GeoDataCoordinates.h"
#include "GeoDataStyle.h"
#include "GeoDataLinearRing.h"


namespace Marble
{

class PolygonNode;

class AreaAnnotation : public SceneGraphicsItem
{
public:
    explicit AreaAnnotation( GeoDataPlacemark *placemark );

    ~AreaAnnotation();

    virtual void paint( GeoPainter *painter, const ViewportParams *viewport );

    virtual bool containsPoint( const QPoint &point ) const;

    /**
     * @brief Provides information for downcasting a SceneGraphicsItem.
     */
    virtual const char *graphicType() const;

private:
    void setupRegionsLists( QPainter *painter );
    void drawNodes( QPainter *painter );

    const ViewportParams *m_viewport;
    bool                  m_regionsInitialized;

    QList<PolygonNode>          m_outerNodesList;
    QList< QList<PolygonNode> > m_innerNodesList;
    QList<PolygonNode>          m_virtualNodesList;
    QList<QRegion>              m_boundariesList;

protected:
    virtual bool mousePressEvent( QMouseEvent *event );
    virtual bool mouseMoveEvent( QMouseEvent *event );
    virtual bool mouseReleaseEvent( QMouseEvent *event );
}

}
