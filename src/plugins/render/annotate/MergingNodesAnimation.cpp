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

// TODOs:
// -> finish this

namespace Marble {

const int MergingNodesAnimation::timeOffset = 200;
const qreal MergingNodesAnimation::distanceOffset = 0.1;

MergingNodesAnimation::MergingNodesAnimation( AreaAnnotation *polygon ) :
    m_polygon( polygon ),
    m_timer( new QTimer() )
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
    m_timer->start( timeOffset );
}

void MergingNodesAnimation::updateNodes()
{
    GeoDataPolygon *polygon = static_cast<GeoDataPolygon*>( m_polygon->placemark()->geometry() );
    GeoDataLinearRing &outerRing = polygon->outerBoundary();
    QVector<GeoDataLinearRing> &innerBounds = polygon->innerBoundaries();

    if ( nodesDistance() <  distanceOffset ) {
        if ( m_boundary ) {
            outerRing[m_polygon->m_secondMergedNode.first] = newCoords();

            // TODO
        }

        emit animationFinished();
        m_timer->stop();
    } else {

        // TODO

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

    return m_boundary == OuterBoundary ?
        outerRing.at(first_i).interpolate( polygon->outerBoundary().at(second_i), 0.5 ) :
        innerBounds.at(first_i).at(second_i).interpolate( innerBounds.at(first_j).at(second_j), 0.5 );
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
