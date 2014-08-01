//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2014      Calin Cruceru  <crucerucalincristian@gmail.com>
//

// Self
#include "PolylineAnnotation.h"

// Qt

// Marble
#include "SceneGraphicsTypes.h"


namespace Marble
{

PolylineAnnotation::PolylineAnnotation( GeoDataPlacemark *placemark ) :
    SceneGraphicsItem( placemark )
{
    // nothing to do
}

PolylineAnnotation::~PolylineAnnotation()
{
    // nothing to do
}

void PolylineAnnotation::paint( GeoPainter *painter, const ViewportParams *viewport )
{

}

bool PolylineAnnotation::containsPoint( const QPoint &eventPos ) const
{
    return false;
}

void PolylineAnnotation::dealWithItemChange( const SceneGraphicsItem *other )
{
    Q_UNUSED( other );
}

void PolylineAnnotation::move( const GeoDataCoordinates &source, const GeoDataCoordinates &destination )
{
    Q_UNUSED( source );
    Q_UNUSED( destination );
}

bool PolylineAnnotation::mousePressEvent( QMouseEvent *event )
{
    return false;
}

bool PolylineAnnotation::mouseMoveEvent( QMouseEvent *event )
{
    return false;
}

bool PolylineAnnotation::mouseReleaseEvent( QMouseEvent *event )
{
    return false;
}

void PolylineAnnotation::dealWithStateChange( SceneGraphicsItem::ActionState previousState )
{
    Q_UNUSED( previousState );
}

const char *PolylineAnnotation::graphicType() const
{
    return SceneGraphicsTypes::SceneGraphicPolyline;
}

}
