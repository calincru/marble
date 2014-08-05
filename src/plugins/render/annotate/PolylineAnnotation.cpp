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
#include <qmath.h>

// Marble
#include "SceneGraphicsTypes.h"
#include "GeoPainter.h"
#include "PolylineNode.h"
#include "MarbleColors.h"
#include "GeoDataLineString.h"
#include "MergingNodesAnimation.h"
#include "GeoDataPlacemark.h"
#include "GeoDataTypes.h"


namespace Marble
{

const int PolylineAnnotation::regularDim = 15;
const int PolylineAnnotation::selectedDim = 15;
const int PolylineAnnotation::mergedDim = 20;
const int PolylineAnnotation::hoveredDim = 20;
const QColor PolylineAnnotation::regularColor = Oxygen::aluminumGray3;
const QColor PolylineAnnotation::selectedColor = Oxygen::aluminumGray6;
const QColor PolylineAnnotation::mergedColor = Oxygen::emeraldGreen6;
const QColor PolylineAnnotation::hoveredColor = Oxygen::grapeViolet6;


PolylineAnnotation::PolylineAnnotation( GeoDataPlacemark *placemark ) :
    SceneGraphicsItem( placemark ),
    m_viewport( 0 ),
    m_regionsInitialized( false ),
    m_busy( false ),
    m_hoveredNodeIndex( -1 ),
    m_virtualHoveredNode( -1 )

{
    // nothing to do
}

PolylineAnnotation::~PolylineAnnotation()
{
    delete m_animation;
}

void PolylineAnnotation::paint( GeoPainter *painter, const ViewportParams *viewport )
{
    m_viewport = viewport;
    Q_ASSERT( placemark()->geometry()->nodeType() == GeoDataTypes::GeoDataLineStringType );

    painter->save();
    if ( !m_regionsInitialized ) {
        setupRegionsLists( painter );
        m_regionsInitialized = true;
    } else {
        updateRegions( painter );
    }

    drawNodes( painter );
    painter->restore();
}

void PolylineAnnotation::setupRegionsLists( GeoPainter *painter )
{
    const GeoDataLineString line = static_cast<const GeoDataLineString>( *placemark()->geometry() );

    // Add the outer boundary nodes.
    QVector<GeoDataCoordinates>::ConstIterator itBegin = line.begin();
    QVector<GeoDataCoordinates>::ConstIterator itEnd = line.end();

    for ( ; itBegin != itEnd; ++itBegin ) {
        PolylineNode newNode = PolylineNode( painter->regionFromEllipse( *itBegin, regularDim, regularDim ) );
        m_nodesList.append( newNode );
    }

    // Add the outer boundary to the boundaries list.
    m_polylineRegion = painter->regionFromPolyline( line );
}

void PolylineAnnotation::updateRegions( GeoPainter *painter )
{
    if ( m_busy ) {
        return;
    }

    const GeoDataLineString line = static_cast<const GeoDataLineString>( *placemark()->geometry() );

    if ( state() == SceneGraphicsItem::MergingPolylineNodes ) {
        // Update the PolylineNodes lists after the animation has finished its execution.
        m_nodesList[m_secondMergedNode].setFlag( PolylineNode::NodeIsMergingHighlighted, false );
        m_hoveredNodeIndex = -1;

        // Remove the merging node flag and add the NodeIsSelected flag if either one of the
        // merged nodes had been selected before merging them.
        m_nodesList[m_secondMergedNode].setFlag( PolylineNode::NodeIsMerged, false );
        if ( m_nodesList[m_firstMergedNode].isSelected() ) {
            m_nodesList[m_secondMergedNode].setFlag( PolylineNode::NodeIsSelected );
        }
        m_nodesList.removeAt( m_firstMergedNode );

        m_firstMergedNode = -1;
        m_secondMergedNode = -1;
    } else if ( state() == SceneGraphicsItem::AddingPolylineNodes ) {
        // Create and update virtual nodes lists when being in the AddingPolgonNodes state, to
        // avoid overhead in other states.
        m_virtualNodesList.clear();
        QRegion firstRegion( painter->regionFromEllipse( line.at(0).interpolate( line.last(), 0.5 ),
                                                         hoveredDim, hoveredDim ) );
        m_virtualNodesList.append( PolylineNode( firstRegion ) );
        for ( int i = 0; i < line.size(); ++i ) {
            QRegion newRegion( painter->regionFromEllipse( line.at(i).interpolate( line.at(i+1), 0.5 ),
                                                           hoveredDim, hoveredDim ) );
            m_virtualNodesList.append( PolylineNode( newRegion ) );
        }
    }


    // Update the polyline region;
    m_polylineRegion = painter->regionFromPolyline( line );

    // Update the outer and inner nodes lists.
    for ( int i = 0; i < m_nodesList.size(); ++i ) {
        QRegion newRegion;
        if ( m_nodesList.at(i).isSelected() ) {
            newRegion = painter->regionFromEllipse( line.at(i), selectedDim, selectedDim );
        } else {
            newRegion = painter->regionFromEllipse( line.at(i), regularDim, regularDim );
        }
        m_nodesList[i].setRegion( newRegion );
    }
}

void PolylineAnnotation::drawNodes( GeoPainter *painter )
{
    // These are the 'real' dimensions of the drawn nodes. The ones which have class scope are used
    // to generate the regions and they are a little bit larger, because, for example, it would be
    // a little bit too hard to select nodes.
    static const int d_regularDim = 10;
    static const int d_selectedDim = 10;
    static const int d_mergedDim = 20;
    static const int d_hoveredDim = 20;

    const GeoDataLineString line = static_cast<const GeoDataLineString>( *placemark()->geometry() );

    for ( int i = 0; i < line.size(); ++i ) {
        // The order here is important, because a merged node can be at the same time selected.
        if ( m_nodesList.at(i).isBeingMerged() ) {
            painter->setBrush( mergedColor );
            painter->drawEllipse( line.at(i), d_mergedDim, d_mergedDim );
        } else if ( m_nodesList.at(i).isSelected() ) {
            painter->setBrush( selectedColor );
            painter->drawEllipse( line.at(i), d_selectedDim, d_selectedDim );

            if ( m_nodesList.at(i).isEditingHighlighted() ||
                 m_nodesList.at(i).isMergingHighlighted() ) {
                QPen defaultPen = painter->pen();
                QPen newPen;
                newPen.setWidth( defaultPen.width() + 3 );

                if ( m_nodesList.at(i).isEditingHighlighted() ) {
                    newPen.setColor( QColor( 0, 255, 255, 120 ) );
                } else {
                    newPen.setColor( QColor( 25, 255, 25, 180 ) );
                }

                painter->setBrush( Qt::NoBrush );
                painter->setPen( newPen );
                painter->drawEllipse( line.at(i), d_selectedDim + 2, d_selectedDim + 2 );

                painter->setPen( defaultPen );
            }
        } else {
            painter->setBrush( regularColor );
            painter->drawEllipse( line.at(i), d_regularDim, d_regularDim );

            if ( m_nodesList.at(i).isEditingHighlighted() ||
                 m_nodesList.at(i).isMergingHighlighted() ) {
                QPen defaultPen = painter->pen();
                QPen newPen;
                newPen.setWidth( defaultPen.width() + 3 );

                if ( m_nodesList.at(i).isEditingHighlighted() ) {
                    newPen.setColor( QColor( 0, 255, 255, 120 ) );
                } else {
                    newPen.setColor( QColor( 25, 255, 25, 180 ) );
                }

                painter->setPen( newPen );
                painter->setBrush( Qt::NoBrush );
                painter->drawEllipse( line.at(i), d_regularDim + 2, d_regularDim + 2 );

                painter->setPen( defaultPen );
            }
        }
    }

    if ( m_virtualHoveredNode != -1 ) {
        painter->setBrush( hoveredColor );

        GeoDataCoordinates newCoords;
        if ( m_virtualHoveredNode ) {
            newCoords = line.at(m_virtualHoveredNode).interpolate( line.at(m_virtualHoveredNode-1), 0.5 );
        } else {
            newCoords = line.at(0).interpolate( line.last(), 0.5 );
        }
        painter->drawEllipse( newCoords, d_hoveredDim, d_hoveredDim );
    }
}

bool PolylineAnnotation::containsPoint( const QPoint &point ) const
{
    if ( state() == SceneGraphicsItem::Editing ) {
        return nodeContains( point ) != -1 || polylineContains( point );
    } else if ( state() == SceneGraphicsItem::MergingPolylineNodes ) {
        return nodeContains( point ) != -1;
    } else if ( state() == SceneGraphicsItem::AddingPolylineNodes ) {
        return virtualNodeContains( point ) != -1 ||
               nodeContains( point ) != -1 ||
               polylineContains( point );
    }

    return false;
}

int PolylineAnnotation::nodeContains( const QPoint &point ) const
{
    for ( int i = 0; i < m_nodesList.size(); ++i ) {
        if ( m_nodesList.at(i).containsPoint( point ) ) {
            return i;
        }
    }

    return -1;
}

int PolylineAnnotation::virtualNodeContains( const QPoint &point ) const
{
    for ( int i = 0; i < m_virtualNodesList.size(); ++i ) {
        if ( m_virtualNodesList.at(i).containsPoint( point ) )
            return i;
    }

    return -1;
}

bool PolylineAnnotation::polylineContains( const QPoint &point ) const
{
    return m_polylineRegion.contains( point );
}

void PolylineAnnotation::dealWithItemChange( const SceneGraphicsItem *other )
{
    Q_UNUSED( other );

    // So far we only deal with item changes when hovering nodes, so that
    // they do not remain hovered when changing the item we interact with.
    if ( state() == SceneGraphicsItem::Editing ) {
        if ( m_hoveredNodeIndex != -1 ) {
            m_nodesList[m_hoveredNodeIndex].setFlag( PolylineNode::NodeIsEditingHighlighted, false );
        }

        m_hoveredNodeIndex = -1;
    } else if ( state() == SceneGraphicsItem::MergingPolylineNodes ) {
        if ( m_hoveredNodeIndex != -1 ) {
            m_nodesList[m_hoveredNodeIndex].setFlag( PolylineNode::NodeIsMergingHighlighted, false );
        }

        m_hoveredNodeIndex = -1;
    } else if ( state() == SceneGraphicsItem::AddingPolylineNodes ) {
        m_virtualHoveredNode = -1;
    }
}

void PolylineAnnotation::move( const GeoDataCoordinates &source, const GeoDataCoordinates &destination )
{
    Q_UNUSED( source );
    Q_UNUSED( destination );
}

void PolylineAnnotation::setBusy( bool enabled )
{
    m_busy = enabled;

    if ( !enabled ) {
        delete m_animation;
    }
}

void PolylineAnnotation::deleteAllSelectedNodes()
{
    if ( state() != SceneGraphicsItem::Editing ) {
        return;
    }

    GeoDataLineString line = static_cast<GeoDataLineString>( *placemark()->geometry() );

    for ( int i = 0; i < line.size(); ++i ) {
        if ( m_nodesList.at(i).isSelected() ) {
            if ( m_nodesList.size() <= 3 ) {
                setRequest( SceneGraphicsItem::RemovePolylineRequest );
                return;
            }

            m_nodesList.removeAt( i );
            line.remove( i );
            --i;
        }
    }
}

void PolylineAnnotation::deleteClickedNode()
{
    if ( state() != SceneGraphicsItem::Editing ) {
        return;
    }

    GeoDataLineString line = static_cast<GeoDataLineString>( *placemark()->geometry() );
    if ( m_nodesList.size() <= 3 ) {
        setRequest( SceneGraphicsItem::RemovePolylineRequest );
        return;
    }

    m_nodesList.removeAt( m_clickedNodeIndex );
    line.remove( m_clickedNodeIndex );
 }

void PolylineAnnotation::changeClickedNodeSelection()
{
    if ( state() != SceneGraphicsItem::Editing ) {
        return;
    }

    m_nodesList[m_clickedNodeIndex].setFlag( PolylineNode::NodeIsSelected, false );
}

bool PolylineAnnotation::hasNodesSelected() const
{
    for ( int i = 0; i < m_outerNodesList.size(); ++i ) {
        if ( m_outerNodesList.at(i).isSelected() ) {
            return true;
        }
    }

    for ( int i = 0; i < m_innerNodesList.size(); ++i ) {
        for ( int j = 0; j < m_innerNodesList.at(i).size(); ++j ) {
            if ( m_innerNodesList.at(i).at(j).isSelected() ) {
                return true;
            }
        }
    }

    return false;
}

bool AreaAnnotation::clickedNodeIsSelected() const
{
    int i = m_clickedNodeIndexes.first;
    int j = m_clickedNodeIndexes.second;

    return ( i != -1 && j == -1 && m_outerNodesList.at(i).isSelected() ) ||
           ( i != -1 && j != -1 && m_innerNodesList.at(i).at(j).isSelected() );
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
