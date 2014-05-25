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
#include "MarbleMath.h"

#include <QDebug>


namespace Marble
{

AreaAnnotation::AreaAnnotation( GeoDataPlacemark *placemark )
    : SceneGraphicsItem( placemark ),
      m_movedNodeIndex( -1 ),
      m_rightClickedNode( -1 ),
      m_viewport( 0 )
{

}

void AreaAnnotation::paint(GeoPainter *painter, const ViewportParams *viewport )
{
    m_viewport = viewport;
    QList<QRegion> regionList;

    painter->save();
    if ( placemark()->geometry()->nodeType() == GeoDataTypes::GeoDataPolygonType ) {
        const GeoDataPolygon *polygon = static_cast<const GeoDataPolygon*>( placemark()->geometry() );
        const GeoDataLinearRing &ring = polygon->outerBoundary();
        for ( int i = 0; i < ring.size(); ++i ) {
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


    m_movedPointCoords.set( lon, lat );
    // We loop through all regions from the list, including the last one, which
    // is the entire polygon. This will be useful in mouseMoveEvent to know
    // whether to move a node or the whole polygon.
    for ( int i = 0; i < regionList.size(); ++i ) {
        if ( regionList.at(i).contains( event->pos()) ) {

            if ( event->button() == Qt::LeftButton ) {
                m_movedNodeIndex = i;
                return true;

            } else if ( event->button() == Qt::RightButton ) {
                if ( i < regionList.size() - 1 ) {
                    m_rightClickedNode = i;
                } else {
                    m_rightClickedNode = -1;
                }

                // Return false because we cannot fully deal with this event within this class.
                // We need to have access to the marble widget to show a menu of options on the
                // screen as well as control of the object since one of the options will be
                // "remove polygon".
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

    if ( m_movedNodeIndex < 0 ) {
        return false;
    }

    QList<QRegion> regionList = regions();
    qreal lon, lat;
    // There is no need to check whether the coordinates are on the globe because
    // this is already checked by the caller function (AnnotatePlugin::eventFilter).
    m_viewport->geoCoordinates( event->pos().x(),
                                event->pos().y(),
                                lon, lat,
                                GeoDataCoordinates::Radian );
    GeoDataCoordinates const coords( lon, lat );

    if ( m_movedNodeIndex >= 0 && m_movedNodeIndex < regionList.size() - 1 ) {
        if ( placemark()->geometry()->nodeType() == GeoDataTypes::GeoDataPolygonType ) {
            GeoDataPolygon *polygon = static_cast<GeoDataPolygon*>( placemark()->geometry() );
            GeoDataLinearRing &ring = polygon->outerBoundary();
            ring[m_movedNodeIndex] = coords;
            return true;
        } else {
            return false;
        }
    }

    Q_ASSERT( m_movedNodeIndex == regionList.size() - 1 );
    qreal bearing = coords.bearing( m_movedPointCoords );
    qreal distance = distanceSphere( coords, m_movedPointCoords );

    if ( placemark()->geometry()->nodeType() == GeoDataTypes::GeoDataPolygonType ) {

        GeoDataPolygon *poly = static_cast<GeoDataPolygon*>( placemark()->geometry() );
        GeoDataLinearRing ring( poly->outerBoundary() );
        poly->outerBoundary().clear();

        for ( int i = 0; i < ring.size(); ++i ) {
            qreal newLon = ring.at(i).longitude() - distance * sin( bearing );
            qreal newLat = ring.at(i).latitude() - distance * cos( bearing );

            GeoDataCoordinates::normalizeLonLat( newLon, newLat );
            poly->outerBoundary().append( GeoDataCoordinates( newLon, newLat ) );
        }

        m_movedPointCoords.set( lon, lat );
        return true;
    }

    return false;
}

bool AreaAnnotation::mouseReleaseEvent( QMouseEvent *event )
{
    QList<QRegion> regionList = regions();

    m_movedNodeIndex = -1;
    m_rightClickedNode = -1;

    qreal lon, lat;
    m_viewport->geoCoordinates( event->pos().x(),
                                event->pos().y(),
                                lon, lat,
                                GeoDataCoordinates::Radian );

    // The node gets selected only if it is clicked and not moved.
    // Is this value ok in order to avoid this?
    if ( qFabs(lon - m_movedPointCoords.longitude()) > 0.001 ||
         qFabs(lat - m_movedPointCoords.latitude()) > 0.001 ) {
        return true;
    }

    // Only loop untill size - 1 because we only want to mark nodes
    // as selected, and not the entire polygon.
    for ( int i = 0; i < regionList.size() - 1; ++i ) {
        if ( regionList.at(i).contains( event->pos()) ) {

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

    // We return true even if we get here, because it means that there were no nodes to
    // be marked (the interior of the polygon has been clicked) and we don't want to do
    // anything else than release it, so we tell caller that we handled the event.
    return true;
}

QList<int> &AreaAnnotation::selectedNodes()
{
    return m_selectedNodes;
}

const char *AreaAnnotation::graphicType() const
{
    return SceneGraphicTypes::SceneGraphicAreaAnnotation;
}

int AreaAnnotation::rightClickedNode() const
{
    return m_rightClickedNode;
}

}
