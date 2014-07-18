//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2014       Calin Cruceru  <crucerucalincristian@gmail.com>
//

// Self
#include "MergingNodesAnimation.h"

// Marble
#include "AreaAnnotation.h"
#include "GeoDataCoordinates.h"
#include "GeoDataPolygon.h"
#include "GeoDataPlacemark.h"
#include "MarbleMath.h"

#include <QDebug>

namespace Marble {


MergingNodesAnimation::MergingNodesAnimation( AreaAnnotation *polygon ) :
    m_polygon( polygon ),
    m_timer( new QTimer( this ) )
{
    if ( m_polygon->m_firstMergedNode.second == -1 ) {
        Q_ASSERT( m_polygon->m_secondMergedNode.second == -1 );
        m_boundary = OuterBoundary;
    } else {
        Q_ASSERT( m_polygon->m_firstMergedNode.second != -1 &&
                  m_polygon->m_secondMergedNode.second != -1 );
        m_boundary = InnerBoundary;
    }

    connect( m_timer, SIGNAL(timeout()), this, SLOT(updateNodes()) );
}

MergingNodesAnimation::~MergingNodesAnimation()
{
    delete m_timer;
}

void MergingNodesAnimation::startAnimation()
{
    static const int timeOffset = 50;
    m_timer->start( timeOffset );
}

void MergingNodesAnimation::updateNodes()
{
    static const qreal distanceOffset = 0.005;
    static const qreal ratio = 0.05;

    GeoDataPolygon *polygon = static_cast<GeoDataPolygon*>( m_polygon->placemark()->geometry() );
    GeoDataLinearRing &outerRing = polygon->outerBoundary();
    QVector<GeoDataLinearRing> &innerBounds = polygon->innerBoundaries();

    int first_i = m_polygon->m_firstMergedNode.first;
    int first_j = m_polygon->m_firstMergedNode.second;
    int second_i = m_polygon->m_secondMergedNode.first;
    int second_j = m_polygon->m_secondMergedNode.second;

    if ( nodesDistance() <  distanceOffset ) {
        if ( m_boundary == OuterBoundary ) {
            outerRing[second_i] = newCoords();
            outerRing.remove( first_i );
        } else {
            innerBounds[second_i][second_j] = newCoords();
            innerBounds[second_i].remove( first_j );
        }

        emit animationFinished();
    } else {
        if ( m_boundary == OuterBoundary ) {
            GeoDataCoordinates first, second;
            first = outerRing.at(first_i).interpolate( outerRing.at(second_i), ratio );
            second = outerRing.at(second_i).interpolate( outerRing.at(first_i), ratio );

            outerRing[first_i] = first;
            outerRing[second_i] = second;
        } else {
            GeoDataCoordinates first, second;
            first = innerBounds.at(first_i).at(first_j).interpolate( innerBounds.at(second_i).at(second_j), ratio );
            second = innerBounds.at(second_i).at(second_j).interpolate( innerBounds.at(first_i).at(first_j), ratio );

            innerBounds[first_i][first_j] = first;
            innerBounds[second_i][second_j] = second;
        }

        emit nodesMoved();
    }
}

GeoDataCoordinates MergingNodesAnimation::newCoords()
{
    GeoDataPolygon *polygon = static_cast<GeoDataPolygon*>( m_polygon->placemark()->geometry() );
    GeoDataLinearRing &outerRing = polygon->outerBoundary();
    QVector<GeoDataLinearRing> &innerBounds = polygon->innerBoundaries();

    int first_i = m_polygon->m_firstMergedNode.first;
    int first_j = m_polygon->m_firstMergedNode.second;
    int second_i = m_polygon->m_secondMergedNode.first;
    int second_j = m_polygon->m_secondMergedNode.second;

    return ( m_boundary == OuterBoundary ) ?
        outerRing.at(first_i).interpolate( polygon->outerBoundary().at(second_i), 0.5 ) :
        innerBounds.at(first_i).at(first_j).interpolate( innerBounds.at(second_i).at(second_j), 0.5 );
}

qreal MergingNodesAnimation::nodesDistance()
{
    GeoDataPolygon *polygon = static_cast<GeoDataPolygon*>( m_polygon->placemark()->geometry() );
    GeoDataCoordinates first, second;

    int first_i = m_polygon->m_firstMergedNode.first;
    int first_j = m_polygon->m_firstMergedNode.second;
    int second_i = m_polygon->m_secondMergedNode.first;
    int second_j = m_polygon->m_secondMergedNode.second;

    if ( m_boundary == OuterBoundary ) {
        first = polygon->outerBoundary().at(first_i);
        second = polygon->outerBoundary().at(second_i);
    } else {
        first = polygon->innerBoundaries().at(first_i).at(first_j);
        second = polygon->innerBoundaries().at(second_i).at(second_j);
    }

    return distanceSphere( first, second );
}

} // namespace Marble

#include "MergingNodesAnimation.moc"
