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
#include "AreaAnnotation.h"

#include "GeoDataPlacemark.h"
#include "GeoDataTypes.h"
#include "GeoPainter.h"
#include "ViewportParams.h"


namespace Marble
{

AreaAnnotation::AreaAnnotation( GeoDataPlacemark *placemark )
    : SceneGraphicsItem( placemark ),
      m_movedPoint( -1 ),
      m_movingPolygon( false ),
      m_viewport( 0 )
{

}

void AreaAnnotation::paint(GeoPainter *painter, const ViewportParams *viewport )
{
    m_viewport = viewport;
    QList<QRegion> regionList;

    painter->save();
    painter->setBrush( Oxygen::aluminumGray1 );
    if( placemark()->geometry()->nodeType() == GeoDataTypes::GeoDataPolygonType ) {
        const GeoDataPolygon *polygon = static_cast<const GeoDataPolygon*>( placemark()->geometry() );
        const GeoDataLinearRing &ring = polygon->outerBoundary();
        for( int i = 0; i<  ring.size(); ++i ) {
            painter->drawEllipse( ring.at(i) , 10, 10 );
            regionList.append( painter->regionFromEllipse( ring.at(i), 10, 10 ));
        }
        regionList.append( painter->regionFromPolygon( ring, Qt::OddEvenFill ) );
    }
    painter->restore();
    setRegions( regionList );
}

bool AreaAnnotation::mousePressEvent( QMouseEvent *event )
{
    QList<QRegion> regionList = regions();
    qreal lat, lon;
    m_viewport->geoCoordinates( event->pos().x(), event->pos().y(),
                                lon, lat,
                                GeoDataCoordinates::Radian );

    // react to all ellipse point markers and skip the polygon
    for( int i = 0; i < regionList.size()-1; ++i ) {
        if( regionList.at(i).contains( event->pos()) ) {
            m_movedPoint = i;
            return true;
        }
    }

    if ( placemark()->geometry()->nodeType() == GeoDataTypes::GeoDataPolygonType ) {
        const GeoDataPolygon *polygon = static_cast<const GeoDataPolygon*>( placemark()->geometry() );
        if ( polygon->latLonAltBox().contains( GeoDataCoordinates(lon, lat) ) ) {
            m_movingPolygon = true;
            m_movedPointCoords.set( lon, lat );
            return true;
        }
    }

    return false;
}

bool AreaAnnotation::mouseMoveEvent( QMouseEvent *event )
{
    if ( !m_viewport ) {
        return false;
    }

    if ( m_movedPoint < 0 && !m_movingPolygon ) {
        return false;
    }

    qreal lon, lat;
    m_viewport->geoCoordinates( event->pos().x(),
                                event->pos().y(),
                                lon, lat,
                                GeoDataCoordinates::Radian );

    if( m_movedPoint >= 0 && placemark()->geometry()->nodeType() == GeoDataTypes::GeoDataPolygonType ) {
        GeoDataPolygon *polygon = static_cast<GeoDataPolygon*>( placemark()->geometry() );
        GeoDataLinearRing &ring = polygon->outerBoundary();
        ring[m_movedPoint] = GeoDataCoordinates( lon, lat );
        return true;
    }

    Q_ASSERT( m_movingPolygon );
    qreal centerLonDiff = lon - m_movedPointCoords.longitude();
    qreal centerLatDiff = lat - m_movedPointCoords.latitude();

    if ( placemark()->geometry()->nodeType() == GeoDataTypes::GeoDataPolygonType ) {
        GeoDataPolygon *poly = static_cast<GeoDataPolygon*>( placemark()->geometry() );
        GeoDataLinearRing ring( poly->outerBoundary() );
        poly->outerBoundary().clear();

        for ( int i = 0; i < ring.size(); ++i ) {
            qreal ringLon = ring.at(i).longitude();
            qreal ringLat = ring.at(i).latitude();
            poly->outerBoundary().append( GeoDataCoordinates(ringLon + centerLonDiff, ringLat + centerLatDiff) );
        }

        m_movedPointCoords.set( lon, lat );
        return true;
    }

    return false;
}

bool AreaAnnotation::mouseReleaseEvent( QMouseEvent *event )
{
    Q_UNUSED(event);
    m_movedPoint = -1;
    m_movingPolygon = false;
    return true;
}

}
