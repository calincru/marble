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
#include "AreaAnnotation.h"

#include <qmath.h>

#include "GeoDataPlacemark.h"
#include "GeoDataTypes.h"
#include "GeoPainter.h"
#include "ViewportParams.h"
#include "SceneGraphicTypes.h"

#include <QDebug>

namespace Marble
{

AreaAnnotation::AreaAnnotation( GeoDataPlacemark *placemark )
    : SceneGraphicsItem( placemark ),
      m_movedNode( -1 ),
      m_movingPolygon( false ),
      m_viewport( 0 )
{

}

void AreaAnnotation::paint(GeoPainter *painter, const ViewportParams *viewport )
{
    m_viewport = viewport;
    QList<QRegion> regionList;

    painter->save();
    if( placemark()->geometry()->nodeType() == GeoDataTypes::GeoDataPolygonType ) {
        const GeoDataPolygon *polygon = static_cast<const GeoDataPolygon*>( placemark()->geometry() );
        const GeoDataLinearRing &ring = polygon->outerBoundary();
        for( int i = 0; i < ring.size(); ++i ) {
            QRegion newRegion = painter->regionFromEllipse( ring.at(i), 15, 15 );

            if ( !m_selectedNodes.contains( i ) ) {
                painter->setBrush( Oxygen::aluminumGray3);
            } else {
                painter->setBrush( Oxygen::aluminumGray6 );
            }

            painter->drawEllipse( ring.at(i) , 15, 15 );
            regionList.append( newRegion );
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


    m_movedNodeCoords.set( lon, lat );
    for ( int i = 0; i < regionList.size() - 1; ++i ) {
        if( regionList.at(i).contains( event->pos()) ) {
            if ( event->button() == Qt::LeftButton ) {
                m_movedNode = i;
                return true;
            } else if ( event->button() == Qt::RightButton ) {
                return false;
            }
        }
    }

    if ( placemark()->geometry()->nodeType() == GeoDataTypes::GeoDataPolygonType ) {
        const GeoDataPolygon *polygon = static_cast<const GeoDataPolygon*>( placemark()->geometry() );
        if ( polygon->latLonAltBox().contains( GeoDataCoordinates(lon, lat) ) ) {
            if ( event->button() == Qt::LeftButton ) {
                m_movingPolygon = true;
                return true;
            } else if ( event->button() == Qt::RightButton ) {
                return false;
            }
        }
    }

    return false;
}

bool AreaAnnotation::mouseMoveEvent( QMouseEvent *event )
{
    if ( !m_viewport ) {
        return false;
    }

    if ( m_movedNode < 0 && !m_movingPolygon ) {
        return false;
    }

    qreal lon, lat;
    m_viewport->geoCoordinates( event->pos().x(),
                                event->pos().y(),
                                lon, lat,
                                GeoDataCoordinates::Radian );

    if ( m_movedNode >= 0 && placemark()->geometry()->nodeType() == GeoDataTypes::GeoDataPolygonType ) {
        GeoDataPolygon *polygon = static_cast<GeoDataPolygon*>( placemark()->geometry() );
        GeoDataLinearRing &ring = polygon->outerBoundary();
        ring[m_movedNode] = GeoDataCoordinates( lon, lat );
        return true;
    }

    Q_ASSERT( m_movingPolygon );
    qreal centerLonDiff = lon - m_movedNodeCoords.longitude();
    qreal centerLatDiff = lat - m_movedNodeCoords.latitude();

    if ( placemark()->geometry()->nodeType() == GeoDataTypes::GeoDataPolygonType ) {
        GeoDataPolygon *poly = static_cast<GeoDataPolygon*>( placemark()->geometry() );
        GeoDataLinearRing ring( poly->outerBoundary() );
        poly->outerBoundary().clear();

        for ( int i = 0; i < ring.size(); ++i ) {
            qreal ringLon = ring.at(i).longitude();
            qreal ringLat = ring.at(i).latitude();
            poly->outerBoundary().append( GeoDataCoordinates(ringLon + centerLonDiff, ringLat + centerLatDiff) );
        }

        m_movedNodeCoords.set( lon, lat );
        return true;
    }

    return false;
}

bool AreaAnnotation::mouseReleaseEvent( QMouseEvent *event )
{
    QList<QRegion> regionList = regions();

    m_movedNode = -1;
    m_movingPolygon = false;

    qreal lon, lat;
    m_viewport->geoCoordinates( event->pos().x(),
                                event->pos().y(),
                                lon, lat,
                                GeoDataCoordinates::Radian );

    // the node gets selected only if it is clicked and not moved. We return true
    // so the loop within AnnotatePlugin stops when checking SceneGraphicItems events.
    if ( qFabs(lon - m_movedNodeCoords.longitude()) > 0.001 ||
         qFabs(lat - m_movedNodeCoords.latitude()) > 0.001 ) {
        return true;
    }

    for( int i = 0; i < regionList.size()-1; ++i ) {
        if( regionList.at(i).contains( event->pos()) ) {

            if ( event->button() == Qt::LeftButton ) {
                if ( !m_selectedNodes.contains( i ) ) {
                    m_selectedNodes.append( i );
                } else {
                    m_selectedNodes.removeAll( i );
                }

                return true;
            }
        }
    }

    return false;
}

QList<int> AreaAnnotation::selectedNodes() const
{
    return m_selectedNodes;
}

const char *AreaAnnotation::graphicType() const
{
    return SceneGraphicTypes::SceneGraphicAreaAnnotation;
}

}
