//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2014      Calin Cruceru  <crucerucalincristian@gmail.com>
//

#ifndef POLYLINEANNOTATION_H
#define POLYLINEANNOTATION_H

#include "SceneGraphicsItem.h"


namespace Marble
{

class PolylineAnnotation : public SceneGraphicsItem
{
public:
    explicit PolylineAnnotation( GeoDataPlacemark *placemark );
    ~PolylineAnnotation();

    virtual void paint( GeoPainter *painter, const ViewportParams *viewport );

    virtual bool containsPoint( const QPoint &eventPos ) const;

    virtual void dealWithItemChange( const SceneGraphicsItem *other );

    virtual void move( const GeoDataCoordinates &source, const GeoDataCoordinates &destination );

    /**
     * @brief Provides information for downcasting a SceneGraphicsItem.
     */
    virtual const char *graphicType() const;

protected:
    virtual bool mousePressEvent( QMouseEvent *event );
    virtual bool mouseMoveEvent( QMouseEvent *event );
    virtual bool mouseReleaseEvent( QMouseEvent *event );

    virtual void dealWithStateChange( SceneGraphicsItem::ActionState previousState );

private:
    QList<QRegion> m_regions;
};

}

#endif
