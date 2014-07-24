//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Andrew Manson <g.real.ate@gmail.com>
// Copyright 2013      Thibaut Gridel <tgridel@free.fr>
//

// Self
#include "PlacemarkTextAnnotation.h"

// Qt
#include <QImage>

// Marble
#include "AbstractProjection.h"
#include "GeoDataPlacemark.h"
#include "GeoDataStyle.h"
#include "GeoPainter.h"
#include "GeoWidgetBubble.h"
#include "ViewportParams.h"
#include "MarbleDirs.h"
#include "SceneGraphicsTypes.h"

#include <QDebug>

namespace Marble
{

PlacemarkTextAnnotation::PlacemarkTextAnnotation( GeoDataPlacemark *placemark ) :
    SceneGraphicsItem( placemark ),
    m_movingPlacemark( false ),
    m_iconFilename( MarbleDirs::path( "bitmaps/annotation.png" ) )
{
    GeoDataStyle *newStyle = new GeoDataStyle( *placemark->style() );
    newStyle->iconStyle().setIcon( QImage( m_iconFilename ) );

    placemark->setStyle( newStyle );
}

PlacemarkTextAnnotation::~PlacemarkTextAnnotation()
{
    // nothing to do
}

void PlacemarkTextAnnotation::paint( GeoPainter *painter, const ViewportParams *viewport )
{
    m_viewport = viewport;
    painter->drawImage( placemark()->coordinate(), placemark()->style()->iconStyle().icon() );

    qreal x, y;
    viewport->currentProjection()->screenCoordinates( placemark()->coordinate(), viewport, x, y );

    m_region = QRegion( x - 10 , y - 10 , 20 , 20 );
}

bool PlacemarkTextAnnotation::containsPoint( const QPoint &eventPos ) const
{
    return m_region.contains( eventPos );
}

void PlacemarkTextAnnotation::dealWithItemChange( const SceneGraphicsItem *other )
{
    Q_UNUSED( other );
}

const char *PlacemarkTextAnnotation::graphicType() const
{
    return SceneGraphicsTypes::SceneGraphicPlacemark;
}

bool PlacemarkTextAnnotation::mousePressEvent( QMouseEvent* event )
{
    Q_UNUSED( event );

    m_movingPlacemark = true;
    return true;
}

bool PlacemarkTextAnnotation::mouseMoveEvent( QMouseEvent *event )
{
    qreal lon, lat;
    m_viewport->geoCoordinates( event->pos().x(),
                                event->pos().y(),
                                lon, lat,
                                GeoDataCoordinates::Radian );

    if ( m_movingPlacemark ) {
        placemark()->setCoordinate( lon, lat );
    }

    return true;
}

bool PlacemarkTextAnnotation::mouseReleaseEvent( QMouseEvent *event )
{
    Q_UNUSED( event );

    m_movingPlacemark = false;
    return true;
}

void PlacemarkTextAnnotation::dealWithStateChange( SceneGraphicsItem::ActionState previousState )
{
    Q_UNUSED( previousState );
}

}
