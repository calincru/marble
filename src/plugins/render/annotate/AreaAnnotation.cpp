//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Andrew Manson            <g.real.ate@gmail.com>
// Copyright 2013      Thibaut Gridel           <tgridel@free.fr>
// Copyright 2014      Calin-Cristian Cruceru   <crucerucalincristian@gmail.com>
//

// Self
#include "AreaAnnotation.h"

// Marble
#include "GeoDataPlacemark.h"
#include "GeoDataTypes.h"
#include "GeoPainter.h"
#include "ViewportParams.h"
#include "SceneGraphicsTypes.h"
#include "MarbleMath.h"
#include "GeoDataStyle.h"

// Qt
#include <qmath.h>
#include <QPair>
#include <QDebug>


namespace Marble {

class PolygonNode {

public:
    PolygonNode( QRegion region );
    ~PolygonNode();

    enum PolyNodeFlag {
        NoOption = 0x0,
        NodeIsSelected = 0x1,
        NodeIsInnerTmp = 0x2,
        NodeIsMerged = 0x4,
    };

    Q_DECLARE_FLAGS(PolyNodeFlags, PolyNodeFlag)

    bool isSelected() const;
    bool isInnerTmp() const;
    bool isBeingMerged() const;

    void setFlag( PolyNodeFlag flag, bool enabled = true );
    void setFlags( PolyNodeFlags flags );
    void setRegion( QRegion newRegion );

    bool containsPoint( const QPoint &eventPos ) const;

private:
    QRegion m_region;
    PolyNodeFlags m_flags;
};

PolygonNode::PolygonNode( QRegion region ) :
    m_region( region ),
    m_flags( NoOption )
{
    // nothing to do
}

PolygonNode::~PolygonNode()
{
    // nothing to do
}

bool PolygonNode::isSelected() const
{
    return m_flags & NodeIsSelected;
}

bool PolygonNode::isInnerTmp() const
{
    return m_flags & NodeIsInnerTmp;
}

bool PolygonNode::isBeingMerged() const
{
    return m_flags & NodeIsMerged;
}

void PolygonNode::setRegion( QRegion newRegion )
{
    m_region = newRegion;
}

void PolygonNode::setFlag( PolyNodeFlag flag, bool enabled )
{
    if ( enabled ) {
        m_flags |= flag;
    } else {
        m_flags &= ~flag;
    }
}

void PolygonNode::setFlags( PolyNodeFlags flags )
{
    m_flags = flags;
}

bool PolygonNode::containsPoint( const QPoint &eventPos ) const
{
    return m_region.contains( eventPos );
}


const int AreaAnnotation::regularDim = 15;
const int AreaAnnotation::selectedDim = 15;
const int AreaAnnotation::mergedDim = 20;
const int AreaAnnotation::hoveredDim = 22;
const QColor AreaAnnotation::regularColor = Oxygen::aluminumGray3;
const QColor AreaAnnotation::selectedColor = Oxygen::aluminumGray6;
const QColor AreaAnnotation::mergedColor = Oxygen::emeraldGreen6;
const QColor AreaAnnotation::hoveredColor = Oxygen::burgundyPurple4;

AreaAnnotation::AreaAnnotation( GeoDataPlacemark *placemark ) :
    SceneGraphicsItem( placemark ),
    m_geopainter( 0 ),
    m_viewport( 0 ),
    m_regionsInitialized( false ),
    m_request( NoRequest ),
    m_interactingObj( InteractingNothing ),
    m_virtualHovered( -1 )
{
    // nothing to do
}

AreaAnnotation::~AreaAnnotation()
{
    // nothing to do so far
}

void AreaAnnotation::paint( GeoPainter *painter, const ViewportParams *viewport )
{
    m_viewport = viewport;
    m_geopainter = painter;
    Q_ASSERT( placemark()->geometry()->nodeType() == GeoDataTypes::GeoDataPolygonType );

    painter->save();
    if ( !m_regionsInitialized ) {
        setupRegionsLists( painter );
        m_regionsInitialized = true;
    }

    applyChanges( painter );
    updateRegions( painter );
    drawNodes( painter );

    painter->restore();
}

bool AreaAnnotation::containsPoint( const QPoint &point ) const
{
    if ( state() == SceneGraphicsItem::Editing ) {
        return outerNodeContains( point ) != -1 || polygonContains( point ) ||
               innerNodeContains( point ) != QPair<int, int>(-1, -1);

    } else if ( state() == SceneGraphicsItem::AddingPolygonHole ) {
        return polygonContains( point ) && outerNodeContains( point ) == -1 &&
               innerNodeContains( point ) == QPair<int, int>(-1, -1);

    } else if ( state() == SceneGraphicsItem::MergingPolygonNodes ) {
        return outerNodeContains( point ) != -1 ||
               innerNodeContains( point ) != QPair<int, int>(-1, -1);

    } else if ( state() == SceneGraphicsItem::AddingPolygonNodes ) {
        return polygonContains( point ) || virtualNodeContains( point ) != -1;
    }

    return false;
}

void AreaAnnotation::itemChanged( const SceneGraphicsItem *other )
{
    Q_UNUSED( other );

    if ( state() == SceneGraphicsItem::Editing ) {
        return;
    } else if ( state() == SceneGraphicsItem::AddingPolygonHole ) {
        // Check if a polygon hole was being drawn before moving to other item.
        GeoDataPolygon *polygon = static_cast<GeoDataPolygon*>( placemark()->geometry() );
        QVector<GeoDataLinearRing> &innerBounds = polygon->innerBoundaries();

        if ( innerBounds.size() && innerBounds.last().size() &&
             m_innerNodesList.last().last().isInnerTmp() ) {
            // If only two nodes were added, remove all this inner boundary.
            if ( innerBounds.last().size() <= 2 ) {
                innerBounds.remove( innerBounds.size() - 1 );
                m_innerNodesList.removeLast();
                return;
            }

            // Remove the 'NodeIsInnerTmp' flag, to allow ::draw method to paint the nodes.
            for ( int i = 0; i < m_innerNodesList.last().size(); ++i ) {
                m_innerNodesList.last()[i].setFlag( PolygonNode::NodeIsInnerTmp, false );
            }
        }
    } else if ( state() == SceneGraphicsItem::MergingPolygonNodes ) {
        int i = m_firstMergedNode.first;
        int j = m_firstMergedNode.second;

        if ( i != -1 && j != -1 ) {
            m_innerNodesList[i][j].setFlag( PolygonNode::NodeIsMerged, false );
        } else if ( i != -1 && j == -1 ) {
            m_outerNodesList[i].setFlag( PolygonNode::NodeIsMerged, false );
        }

        m_firstMergedNode = QPair<int, int>( -1, -1 );
    } else if ( state() == SceneGraphicsItem::AddingPolygonNodes ) {
        m_virtualHovered = -1;
    }
}

AreaAnnotation::MarbleWidgetRequest AreaAnnotation::request() const
{
    return m_request;
}

void AreaAnnotation::deselectAllNodes()
{
    if ( state() != SceneGraphicsItem::Editing ) {
        return;
    }

    for ( int i = 0 ; i < m_outerNodesList.size(); ++i ) {
        m_outerNodesList[i].setFlag( PolygonNode::NodeIsSelected, false );
    }

    for ( int i = 0; i < m_innerNodesList.size(); ++i ) {
        for ( int j = 0; j < m_innerNodesList.at(i).size(); ++j ) {
            m_innerNodesList[i][j].setFlag( PolygonNode::NodeIsSelected, false );
        }
    }
}

void AreaAnnotation::deleteAllSelectedNodes()
{
    if ( state() != SceneGraphicsItem::Editing ) {
        return;
    }
     
    GeoDataPolygon *polygon = static_cast<GeoDataPolygon*>( placemark()->geometry() );
    GeoDataLinearRing &outerRing = polygon->outerBoundary();
    QVector<GeoDataLinearRing> &innerRings = polygon->innerBoundaries();

    // If it proves inefficient, try something different.
    GeoDataLinearRing initialOuterRing = polygon->outerBoundary();
    QVector<GeoDataLinearRing> initialInnerRings = polygon->innerBoundaries();
    QList<PolygonNode> initialOuterNodes = m_outerNodesList;
    QList< QList<PolygonNode> > initialInnerNodes = m_innerNodesList;

    for ( int i = 0; i < outerRing.size(); ++i ) {
        if ( m_outerNodesList.at(i).isSelected() ) {
            if ( m_outerNodesList.size() <= 3 ) {
                m_request = RemovePolygonRequest;
                return;
            }

            m_outerNodesList.removeAt( i );
            outerRing.remove( i );
            --i;
        }
    }

    for ( int i = 0; i < innerRings.size(); ++i ) {
        for ( int j = 0; j < innerRings.at(i).size(); ++j ) {
            if ( m_innerNodesList.at(i).at(j).isSelected() ) {
                if ( m_innerNodesList.at(i).size() <= 3 ) {
                    innerRings.remove( i );
                    m_innerNodesList.removeAt( i );
                    --i;
                    break;
                }

                innerRings[i].remove( j );
                m_innerNodesList[i].removeAt( j );
                --j;
            }
        }
    }

    if ( !isValidPolygon() ) {
        polygon->outerBoundary() = initialOuterRing;
        polygon->innerBoundaries() = initialInnerRings;
        m_outerNodesList = initialOuterNodes;
        m_innerNodesList = initialInnerNodes;
        m_request = InvalidShapeWarning;
    }
}

void AreaAnnotation::deleteClickedNode()
{
    if ( state() != SceneGraphicsItem::Editing ) {
        return;
    }

    GeoDataPolygon *polygon = static_cast<GeoDataPolygon*>( placemark()->geometry() );
    GeoDataLinearRing &outerRing = polygon->outerBoundary();
    QVector<GeoDataLinearRing> &innerRings = polygon->innerBoundaries();

    // If it proves inefficient, try something different.
    GeoDataLinearRing initialOuterRing = polygon->outerBoundary();
    QVector<GeoDataLinearRing> initialInnerRings = polygon->innerBoundaries();
    QList<PolygonNode> initialOuterNodes = m_outerNodesList;
    QList< QList<PolygonNode> > initialInnerNodes = m_innerNodesList;

    int i = m_clickedNodeIndexes.first;
    int j = m_clickedNodeIndexes.second;

    if ( i != -1 && j == -1 ) {
        if ( m_outerNodesList.size() <= 3 ) {
            m_request = RemovePolygonRequest;
            return;
        }

        outerRing.remove( i );
        m_outerNodesList.removeAt( i );
    } else if ( i != -1 && j != -1 ) {
        if ( m_innerNodesList.at(i).size() <= 3 ) {
            innerRings.remove( i );
            m_innerNodesList.removeAt( i );
            return;
        }

        innerRings[i].remove( j );
        m_innerNodesList[i].removeAt( j );
    }

    if ( !isValidPolygon() ) {
        polygon->outerBoundary() = initialOuterRing;
        polygon->innerBoundaries() = initialInnerRings;
        m_outerNodesList = initialOuterNodes;
        m_innerNodesList = initialInnerNodes;
        m_request = InvalidShapeWarning;
    }
}

void AreaAnnotation::changeClickedNodeSelection()
{
    if ( state() != SceneGraphicsItem::Editing ) {
        return;
    }

    int i = m_clickedNodeIndexes.first;
    int j = m_clickedNodeIndexes.second;

    if ( i != -1 && j == -1 ) {
        m_outerNodesList[i].setFlag( PolygonNode::NodeIsSelected,
                                     !m_outerNodesList.at(i).isSelected() );
    } else if ( i != -1 && j != -1 ) {
        m_innerNodesList[i][j].setFlag( PolygonNode::NodeIsSelected,
                                        !m_innerNodesList.at(i).at(j).isSelected() );
    }
}

bool AreaAnnotation::hasNodesSelected() const
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

bool AreaAnnotation::mousePressEvent( QMouseEvent *event )
{
    if ( !m_viewport || !m_geopainter ) {
        return false;
    }

    m_request = NoRequest;

    if ( state() == SceneGraphicsItem::Editing ) {
        return processEditingOnPress( event );
    } else if ( state() == SceneGraphicsItem::AddingPolygonHole ) {
        return processAddingHoleOnPress( event );
    } else if ( state() == SceneGraphicsItem::MergingPolygonNodes ) {
        return processMergingOnPress( event );
    } else if ( state() == SceneGraphicsItem::AddingPolygonNodes ) {
        return processAddingNodesOnPress( event );
    }

    return false;
}

bool AreaAnnotation::mouseMoveEvent( QMouseEvent *event )
{
    if ( !m_viewport || !m_geopainter ) {
        return false;
    }

    m_request = NoRequest;

    if ( state() == SceneGraphicsItem::Editing ) {
        return processEditingOnMove( event );
    } else if ( state() == SceneGraphicsItem::AddingPolygonHole ) {
        return processAddingHoleOnMove( event );
    } else if ( state() == SceneGraphicsItem::MergingPolygonNodes ) {
        return processMergingOnMove( event );
    } else if ( state() == SceneGraphicsItem::AddingPolygonNodes ) {
        return processAddingNodesOnMove( event );
    }

    return false;
}

bool AreaAnnotation::mouseReleaseEvent( QMouseEvent *event )
{
    if ( !m_viewport || !m_geopainter ) {
        return false;
    }

    m_request = NoRequest;

    if ( state() == SceneGraphicsItem::Editing ) {
        return processEditingOnRelease( event );
    } else if ( state() == SceneGraphicsItem::AddingPolygonHole ) {
        return processAddingHoleOnRelease( event );
    } else if ( state() == SceneGraphicsItem::MergingPolygonNodes ) {
        return processMergingOnRelease( event );
    } else if ( state() == SceneGraphicsItem::AddingPolygonNodes ) {
        return processAddingNodesOnRelease( event );
    }

    return false;
}

void AreaAnnotation::stateChanged( SceneGraphicsItem::ActionState previousState )
{
    // Dealing with cases when exiting a state has an effect on the scene graphic items.
    if ( previousState == SceneGraphicsItem::Editing ) {
        m_clickedNodeIndexes = QPair<int, int>( -1, -1 );
    } else if ( previousState == SceneGraphicsItem::AddingPolygonHole ) {
        // Check if a polygon hole was being drawn before moving to other item.
        GeoDataPolygon *polygon = static_cast<GeoDataPolygon*>( placemark()->geometry() );
        QVector<GeoDataLinearRing> &innerBounds = polygon->innerBoundaries();

        if ( innerBounds.size() && innerBounds.last().size() &&
             m_innerNodesList.last().last().isInnerTmp() ) {
            // If only two nodes were added, remove all this inner boundary.
            if ( innerBounds.last().size() <= 2 ) {
                innerBounds.remove( innerBounds.size() - 1 );
                m_innerNodesList.removeLast();
                return;
            }

            // Remove the 'NodeIsInnerTmp' flag, to allow ::draw method to paint the nodes.
            for ( int i = 0; i < m_innerNodesList.last().size(); ++i ) {
                m_innerNodesList.last()[i].setFlag( PolygonNode::NodeIsInnerTmp, false );
            }
        }
    } else if ( previousState == SceneGraphicsItem::MergingPolygonNodes ) {
        int i = m_firstMergedNode.first;
        int j = m_firstMergedNode.second;

        if ( i != -1 && j != -1 ) {
            m_innerNodesList[i][j].setFlag( PolygonNode::NodeIsMerged, false );
        } else if ( i != -1 && j == -1 ) {
            m_outerNodesList[i].setFlag( PolygonNode::NodeIsMerged, false );
        }

        m_firstMergedNode = QPair<int, int>( -1, -1 );
    } else if ( previousState == SceneGraphicsItem::AddingPolygonNodes ) {
        m_virtualNodesList.clear();
        m_virtualHovered = -1;
        m_adjustingNode = false;
    }

    // Dealing with cases when entering a state has an effect on scene graphic items, or
    // initializations are needed.
    if ( state() == SceneGraphicsItem::Editing ) {
        m_interactingObj = InteractingNothing;
        m_clickedNodeIndexes = QPair<int, int>( -1, -1 );
    } else if ( state() == SceneGraphicsItem::AddingPolygonHole ) {

    } else if ( state() == SceneGraphicsItem::MergingPolygonNodes ) {
        m_firstMergedNode = QPair<int, int>(-1, -1);
        m_secondMergedNode = QPair<int, int>(-1, -1);
    } else if ( state() == SceneGraphicsItem::AddingPolygonNodes ) {
        m_virtualHovered = -1;
        m_adjustingNode = false;
    }
}

const char *AreaAnnotation::graphicType() const
{
    return SceneGraphicsTypes::SceneGraphicAreaAnnotation;
}

bool AreaAnnotation::isValidPolygon() const
{
    const GeoDataPolygon *poly = static_cast<const GeoDataPolygon*>( placemark()->geometry() );
    const QVector<GeoDataLinearRing> &innerRings = poly->innerBoundaries();

    foreach ( const GeoDataLinearRing &innerRing, innerRings ) {
        for ( int i = 0; i < innerRing.size(); ++i ) {
            if ( !poly->outerBoundary().contains( innerRing.at(i) ) ) {
                return false;
            }
        }
    }

    return true;
}

void AreaAnnotation::setupRegionsLists( GeoPainter *painter )
{
    const GeoDataPolygon *polygon = static_cast<const GeoDataPolygon*>( placemark()->geometry() );
    const GeoDataLinearRing &outerRing = polygon->outerBoundary();

    // Add the outer boundary nodes
    QVector<GeoDataCoordinates>::ConstIterator itBegin = outerRing.begin();
    QVector<GeoDataCoordinates>::ConstIterator itEnd = outerRing.end();

    for ( ; itBegin != itEnd; ++itBegin ) {
        PolygonNode newNode = PolygonNode( painter->regionFromEllipse( *itBegin, regularDim, regularDim ) );
        m_outerNodesList.append( newNode );
    }

    // Add the outer boundary to the boundaries list
    m_boundariesList.append( painter->regionFromPolygon( outerRing, Qt::OddEvenFill ) );
}

void AreaAnnotation::applyChanges( GeoPainter *painter )
{
    const GeoDataPolygon *polygon = static_cast<const GeoDataPolygon*>( placemark()->geometry() );
    const GeoDataLinearRing &outerRing = polygon->outerBoundary();
    const QVector<GeoDataLinearRing> &innerRings = polygon->innerBoundaries();

    if ( state() == SceneGraphicsItem::Editing ) {
        // Only nodes update is needed, so there is nothing else to do so far.
    } else if ( state() == SceneGraphicsItem::AddingPolygonHole ) {
        // If meanwhile a new inner boundary have been added, append an empty list of PolygonNodes.
        if ( m_innerNodesList.size() < innerRings.size() ) {
            m_innerNodesList.append( QList<PolygonNode>() );
        }

        // If meanwhile a new node has been added, append it to the most recent inner boundary from
        // the inner boundaries regions list.
        if ( m_innerNodesList.size() &&
             m_innerNodesList.last().size() < innerRings.last().size() ) {
            PolygonNode newNode = PolygonNode( painter->regionFromEllipse(
                                               innerRings.last().last(), regularDim, regularDim ) );
            newNode.setFlag( PolygonNode::NodeIsInnerTmp );
            m_innerNodesList.last().append( newNode );
        }
    } else if ( state() == SceneGraphicsItem::MergingPolygonNodes ) {
        int ff = m_firstMergedNode.first;
        int fs = m_firstMergedNode.second;
        int sf = m_secondMergedNode.first;
        int ss = m_secondMergedNode.second;

        if ( ff != -1 && fs == -1 && sf != -1 && ss == -1 ) {
            // If the first clicked node is selected but the second one is not, make sure the
            // resulting node is selected as well.
            if ( m_outerNodesList.at(ff).isSelected() ) {
                m_outerNodesList[sf].setFlag( PolygonNode::NodeIsSelected );
            }

            if ( m_outerNodesList.at(sf).isSelected() ) {
                m_outerNodesList[sf].setRegion( painter->regionFromEllipse(
                                                m_resultingCoords, selectedDim, selectedDim ) );
            } else {
                m_outerNodesList[sf].setRegion( painter->regionFromEllipse(
                                                m_resultingCoords, regularDim, regularDim ) );
            }
            m_outerNodesList.removeAt( ff );

            m_firstMergedNode = QPair<int, int>( -1, -1 );
            m_secondMergedNode = QPair<int, int>( -1, -1 );
        } else if ( ff != -1 && fs != -1 && sf != -1 && ss != -1 ) {
            if ( m_innerNodesList.at(ff).at(fs).isSelected() ) {
                m_innerNodesList[sf][ss].setFlag( PolygonNode::NodeIsSelected );
            }

            if ( m_innerNodesList.at(sf).at(ss).isSelected() ) {
                m_innerNodesList[sf][ss].setRegion( painter->regionFromEllipse(
                                                    m_resultingCoords, selectedDim, selectedDim ) );
            } else {
                m_innerNodesList[sf][ss].setRegion( painter->regionFromEllipse(
                                                    m_resultingCoords, regularDim, regularDim ) );
            }
            m_innerNodesList[sf].removeAt( fs );

            m_firstMergedNode = QPair<int, int>( -1, -1 );
            m_secondMergedNode = QPair<int, int>( -1, -1 );
        }
    } else if ( state() == SceneGraphicsItem::AddingPolygonNodes ) {
        if ( m_outerNodesList.size() < outerRing.size() ) {
            m_outerNodesList.append( painter->regionFromEllipse( outerRing.last(), regularDim, regularDim ) );
        }

        m_virtualNodesList.clear();

        QRegion firstRegion( painter->regionFromEllipse( outerRing.at(0).interpolate(
                                             outerRing.last(), 0.5 ), hoveredDim, hoveredDim ) );
        m_virtualNodesList.append( PolygonNode( firstRegion ) );
        for ( int i = 0; i < outerRing.size() - 1; ++i ) {
            QRegion newRegion( painter->regionFromEllipse( outerRing.at(i).interpolate(
                                             outerRing.at(i+1), 0.5 ), hoveredDim, hoveredDim ) );
            m_virtualNodesList.append( PolygonNode( newRegion ) );
        }
    }
}

void AreaAnnotation::updateRegions( GeoPainter *painter )
{
    const GeoDataPolygon *polygon = static_cast<const GeoDataPolygon*>( placemark()->geometry() );
    const GeoDataLinearRing &outerRing = polygon->outerBoundary();
    const QVector<GeoDataLinearRing> &innerRings = polygon->innerBoundaries();

    // Update the boundaries list.
    m_boundariesList.clear();

    m_boundariesList.append( m_geopainter->regionFromPolygon( outerRing, Qt::OddEvenFill ) );
    foreach ( const GeoDataLinearRing &ring, innerRings ) {
        m_boundariesList.append( m_geopainter->regionFromPolygon( ring, Qt::OddEvenFill ) );
    }

    // Update the outer and inner nodes lists.
    for ( int i = 0; i < m_outerNodesList.size(); ++i ) {
        QRegion newRegion;
        if ( m_outerNodesList.at(i).isSelected() ) {
            newRegion = painter->regionFromEllipse( outerRing.at(i),
                                                    selectedDim, selectedDim );
        } else {
            newRegion = painter->regionFromEllipse( outerRing.at(i),
                                                    regularDim, regularDim );
        }
        m_outerNodesList[i].setRegion( newRegion );
    }

    for ( int i = 0; i < m_innerNodesList.size(); ++i ) {
        for ( int j = 0; j < m_innerNodesList.at(i).size(); ++j ) {
            QRegion newRegion;
            if ( m_innerNodesList.at(i).at(j).isSelected() ) {
                newRegion = painter->regionFromEllipse( innerRings.at(i).at(j),
                                                        selectedDim, selectedDim );
            } else {
                newRegion = painter->regionFromEllipse( innerRings.at(i).at(j),
                                                        regularDim, regularDim );
            }
            m_innerNodesList[i][j].setRegion( newRegion );
        }
    }
}

void AreaAnnotation::drawNodes( GeoPainter *painter )
{
    // These are the 'real' dimensions of the drawn nodes. The ones which have class scope are used
    // to generate the regions and they are a little bit larger, because, for example, it would be
    // a little bit too hard to select nodes.
    static const int d_regularDim = 10;
    static const int d_selectedDim = 10;
    static const int d_mergedDim = 20;
    static const int d_hoveredDim = 20;

    const GeoDataPolygon *polygon = static_cast<const GeoDataPolygon*>( placemark()->geometry() );
    const GeoDataLinearRing &outerRing = polygon->outerBoundary();
    const QVector<GeoDataLinearRing> &innerRings = polygon->innerBoundaries();

    for ( int i = 0; i < outerRing.size(); ++i ) {
        // The order here is important, because a merged node can be at the same time selected.
        if ( m_outerNodesList.at(i).isBeingMerged() ) {
            painter->setBrush( mergedColor );
            painter->drawEllipse( outerRing.at(i), d_mergedDim, d_mergedDim );
        } else if ( m_outerNodesList.at(i).isSelected() ) {
            painter->setBrush( selectedColor );
            painter->drawEllipse( outerRing.at(i), d_selectedDim, d_selectedDim );
        } else {
            painter->setBrush( regularColor );
            painter->drawEllipse( outerRing.at(i), d_regularDim, d_regularDim );
        }
    }

    for ( int i = 0; i < innerRings.size(); ++i ) {
        for ( int j = 0; j < innerRings.at(i).size(); ++j ) {
            if ( m_innerNodesList.at(i).at(j).isBeingMerged() ) {
                painter->setBrush( mergedColor );
                painter->drawEllipse( innerRings.at(i).at(j), d_mergedDim, d_mergedDim );
            } else if ( m_innerNodesList.at(i).at(j).isSelected() ) {
                painter->setBrush( selectedColor );
                painter->drawEllipse( innerRings.at(i).at(j), d_selectedDim, d_selectedDim );
            } else if ( m_innerNodesList.at(i).at(j).isInnerTmp() ) {
                // Do not draw inner nodes until the 'process' of adding these nodes ends
                // (aka while being in the 'Adding Polygon Hole').
                continue;
            } else {
                painter->setBrush( regularColor );
                painter->drawEllipse( innerRings.at(i).at(j), d_regularDim, d_regularDim );
            }
        }
    }

    if ( m_virtualHovered != -1 ) {
        painter->setBrush( hoveredColor );

        GeoDataCoordinates newCoords;
        if ( m_virtualHovered ) {
            newCoords = outerRing.at(m_virtualHovered).interpolate(
                                  outerRing.at(m_virtualHovered - 1), 0.5 );
        } else {
            newCoords = outerRing.at(0).interpolate( outerRing.last(), 0.5 );
        }
        painter->drawEllipse( newCoords, d_hoveredDim, d_hoveredDim );
    }
}

int AreaAnnotation::outerNodeContains( const QPoint &point ) const
{
    for ( int i = 0; i < m_outerNodesList.size(); ++i ) {
        if ( m_outerNodesList.at(i).containsPoint( point ) ) {
            return i;
        }
    }

    return -1;
}

QPair<int, int> AreaAnnotation::innerNodeContains( const QPoint &point ) const
{
    for ( int i = 0; i < m_innerNodesList.size(); ++i ) {
        for ( int j = 0; j < m_innerNodesList.at(i).size(); ++j ) {
            if ( m_innerNodesList.at(i).at(j).containsPoint( point ) ) {
                return QPair<int, int>(i, j);
            }
        }
    }

    return QPair<int, int>(-1, -1);
}

int AreaAnnotation::virtualNodeContains( const QPoint &point ) const
{
    for ( int i = 0; i < m_virtualNodesList.size(); ++i ) {
        if ( m_virtualNodesList.at(i).containsPoint( point ) ) {
            return i;
        }
    }

    return -1;
}

int AreaAnnotation::innerBoundsContain( const QPoint &point ) const
{
    // There are no inner boundaries.
    if ( m_boundariesList.size() == 1 ) {
        return -1;
    }

    // Starting from 1 because on index 0 is stored the region representing the whole polygon.
    for ( int i = 1; i < m_boundariesList.size(); ++i ) {
        if ( m_boundariesList.at(i).contains( point ) ) {
            return i;
        }
    }

    return -1;
}

bool AreaAnnotation::polygonContains( const QPoint &point ) const
{
    return m_boundariesList.at(0).contains( point ) && innerBoundsContain( point ) == -1;
}

bool AreaAnnotation::processEditingOnPress( QMouseEvent *mouseEvent )
{
    if ( mouseEvent->button() != Qt::LeftButton && mouseEvent->button() != Qt::RightButton ) {
        return false;
    }

    qreal lat, lon;
    m_viewport->geoCoordinates( mouseEvent->pos().x(),
                                mouseEvent->pos().y(),
                                lon, lat,
                                GeoDataCoordinates::Radian );
    m_movedPointCoords.set( lon, lat );

    // First check if one of the nodes from outer boundary has been clicked.
    int outerIndex = outerNodeContains( mouseEvent->pos() );
    if ( outerIndex != -1 ) {
        m_clickedNodeIndexes = QPair<int, int>( outerIndex, -1 );

        if ( mouseEvent->button() == Qt::RightButton ) {
            m_request = ShowNodeRmbMenu;
        } else {
            m_interactingObj = InteractingNode;
        }

        return true;
    }

    // Then check if one of the nodes which form an inner boundary has been clicked.
    QPair<int, int> innerIndexes = innerNodeContains( mouseEvent->pos() );
    if ( innerIndexes.first != -1 && innerIndexes.second != -1 ) {
        m_clickedNodeIndexes = innerIndexes;

        if ( mouseEvent->button() == Qt::RightButton ) {
            m_request = ShowNodeRmbMenu;
        } else {
            m_interactingObj = InteractingNode;
        }
        return true;
    }

    // If neither outer boundary nodes nor inner boundary nodes contain the event position,
    // then check if the interior of the polygon (excepting its 'holes') contains this point.
    if ( polygonContains( mouseEvent->pos() ) ) {
        if ( mouseEvent->button() == Qt::RightButton ) {
            m_request = ShowPolygonRmbMenu;
        } else {
            m_interactingObj = InteractingPolygon;
        }
        return true;
    }

    return false;
}

bool AreaAnnotation::processEditingOnMove( QMouseEvent *mouseEvent )
{
    if ( !m_viewport || !m_geopainter ) {
        return false;
    }

    qreal lon, lat;
    m_viewport->geoCoordinates( mouseEvent->pos().x(),
                                mouseEvent->pos().y(),
                                lon, lat,
                                GeoDataCoordinates::Radian );
    const GeoDataCoordinates newCoords( lon, lat );

    if ( m_interactingObj == InteractingNode ) {
        GeoDataPolygon *polygon = static_cast<GeoDataPolygon*>( placemark()->geometry() );
        GeoDataLinearRing &outerRing = polygon->outerBoundary();
        QVector<GeoDataLinearRing> &innerRings = polygon->innerBoundaries();

        int i = m_clickedNodeIndexes.first;
        int j = m_clickedNodeIndexes.second;

        if ( j == -1 ) {
            outerRing[i] = newCoords;
        } else {
            Q_ASSERT( i != -1 && j != -1 );
            innerRings[i].at(j) = newCoords;
        }

        return true;
    } else if ( m_interactingObj == InteractingPolygon ) {
        GeoDataPolygon *polygon = static_cast<GeoDataPolygon*>( placemark()->geometry() );
        GeoDataLinearRing outerRing = polygon->outerBoundary();
        QVector<GeoDataLinearRing> innerRings = polygon->innerBoundaries();

        const qreal bearing = m_movedPointCoords.bearing( newCoords );
        const qreal distance = distanceSphere( newCoords, m_movedPointCoords );

        polygon->outerBoundary().clear();
        polygon->innerBoundaries().clear();

        for ( int i = 0; i < outerRing.size(); ++i ) {
            GeoDataCoordinates movedPoint = outerRing.at(i).moveByBearing( bearing, distance );
            qreal lon = movedPoint.longitude();
            qreal lat = movedPoint.latitude();

            GeoDataCoordinates::normalizeLonLat( lon, lat );
            movedPoint.setLongitude( lon );
            movedPoint.setLatitude( lat );

            polygon->outerBoundary().append( movedPoint );
        }

        for ( int i = 0; i < innerRings.size(); ++i ) {
            GeoDataLinearRing newRing( Tessellate );
            for ( int j = 0; j < innerRings.at(i).size(); ++j ) {
                GeoDataCoordinates movedPoint = innerRings.at(i).at(j).moveByBearing( bearing, distance );
                qreal lon = movedPoint.longitude();
                qreal lat = movedPoint.latitude();

                GeoDataCoordinates::normalizeLonLat( lon, lat );
                movedPoint.setLongitude( lon );
                movedPoint.setLatitude( lat );

                newRing.append( movedPoint );
            }
            polygon->innerBoundaries().append( newRing );
        }

        m_movedPointCoords = newCoords;
        return true;
    } // Just need to add a new if ( m_interactingObj = InteractingNothing ) here if you one wants to
      // handle polygon hovers in Editing state.

    return false;
}

bool AreaAnnotation::processEditingOnRelease( QMouseEvent *mouseEvent )
{
    static const int mouseMoveOffset = 1;

    if ( mouseEvent->button() != Qt::LeftButton ) {
        return false;
    }

    if ( m_interactingObj == InteractingNode ) {
        qreal x, y;

        m_viewport->screenCoordinates( m_movedPointCoords.longitude(), m_movedPointCoords.latitude(), x, y );
        // The node gets selected only if it is clicked and not moved.
        if ( qFabs(mouseEvent->pos().x() - x) > mouseMoveOffset ||
             qFabs(mouseEvent->pos().y() - y) > mouseMoveOffset ) {
            m_interactingObj = InteractingNothing;
            return true;
        }

        int i = m_clickedNodeIndexes.first;
        int j = m_clickedNodeIndexes.second;

        if ( j == -1 ) {
            m_outerNodesList[i].setFlag( PolygonNode::NodeIsSelected,
                                         !m_outerNodesList[i].isSelected() );
        } else {
            m_innerNodesList[i][j].setFlag ( PolygonNode::NodeIsSelected,
                                             !m_innerNodesList.at(i).at(j).isSelected() );
        }

        m_interactingObj = InteractingNothing;
        return true;
    } else if ( m_interactingObj == InteractingPolygon ) {
        // Nothing special happens at polygon release.
        m_interactingObj = InteractingNothing;
        return true;
    }

    return false;
}

bool AreaAnnotation::processAddingHoleOnPress( QMouseEvent *mouseEvent )
{
    Q_UNUSED( mouseEvent );
    return true;
}

bool AreaAnnotation::processAddingHoleOnMove( QMouseEvent *mouseEvent )
{
    Q_UNUSED( mouseEvent );
    return true;
}

bool AreaAnnotation::processAddingHoleOnRelease( QMouseEvent *mouseEvent )
{
    if ( mouseEvent->button() != Qt::LeftButton ) {
        return false;
    }

    qreal lon, lat;
    m_viewport->geoCoordinates( mouseEvent->pos().x(),
                                    mouseEvent->pos().y(),
                                    lon, lat,
                                    GeoDataCoordinates::Radian );
    const GeoDataCoordinates newCoords( lon, lat );

    GeoDataPolygon *polygon = static_cast<GeoDataPolygon*>( placemark()->geometry() );
    QVector<GeoDataLinearRing> &innerBounds = polygon->innerBoundaries();

    // Check if this is the first node which is being added as a new polygon inner boundary.
    if ( !innerBounds.size() || !m_innerNodesList.last().last().isInnerTmp() ) {
       polygon->innerBoundaries().append( GeoDataLinearRing( Tessellate ) );
    }
    innerBounds.last().append( newCoords );

    return true;
}

bool AreaAnnotation::processMergingOnPress( QMouseEvent *mouseEvent )
{
    Q_UNUSED( mouseEvent );
    return true;
}

bool AreaAnnotation::processMergingOnMove( QMouseEvent *mouseEvent )
{
    Q_UNUSED( mouseEvent );
    return true;
}

bool AreaAnnotation::processMergingOnRelease( QMouseEvent *mouseEvent )
{
    // TODO: Verify if the size becomes smaller than 3.
    if ( mouseEvent->button() != Qt::LeftButton ) {
        return false;
    }

    GeoDataPolygon *polygon = static_cast<GeoDataPolygon*>( placemark()->geometry() );
    GeoDataLinearRing initialOuterRing = polygon->outerBoundary();
    QVector<GeoDataLinearRing> initialInnerRings = polygon->innerBoundaries();

    GeoDataLinearRing &outerRing = polygon->outerBoundary();
    QVector<GeoDataLinearRing> &innerRings = polygon->innerBoundaries();

    int outerIndex = outerNodeContains( mouseEvent->pos() );
    if ( outerIndex != -1 ) {
        if ( m_firstMergedNode.first == -1 && m_firstMergedNode.second == -1 ) {
            m_firstMergedNode = QPair<int, int>( outerIndex, -1 );
            m_outerNodesList[outerIndex].setFlag( PolygonNode::NodeIsMerged );
        } else if ( m_firstMergedNode.first != -1 && m_firstMergedNode.second != -1 ) {
            m_request = OuterInnerMergingWarning;
            m_innerNodesList[m_firstMergedNode.first][m_firstMergedNode.second].setFlag(
                                                        PolygonNode::NodeIsMerged, false );
            m_firstMergedNode = QPair<int, int>( -1, -1 );
        } else {
            Q_ASSERT( m_firstMergedNode.first != -1 && m_firstMergedNode.second == -1 );

            // Clicking two times the same node results in unmarking it for merging.
            if ( m_firstMergedNode.first == outerIndex ) {
                m_outerNodesList[outerIndex].setFlag( PolygonNode::NodeIsMerged, false );
                m_firstMergedNode = QPair<int, int>( -1, -1 );
                return true;
            }

            // If two nodes which form a triangle are merged, the whole triangle should be
            // destroyed.
            if ( outerRing.size() <= 3 ) {
                m_request = RemovePolygonRequest;
                return true;
            }

            m_resultingCoords = outerRing.at(m_firstMergedNode.first).interpolate(
                                                    outerRing.at(outerIndex), 0.5 );
            outerRing.at(outerIndex) = m_resultingCoords;
            outerRing.remove( m_firstMergedNode.first );

            if ( !isValidPolygon() ) {
                polygon->outerBoundary() = initialOuterRing;
                polygon->innerBoundaries() = initialInnerRings;
                m_firstMergedNode = QPair<int, int>( -1, -1 );
                m_request = InvalidShapeWarning;
                return true;
            }

            m_secondMergedNode = QPair<int, int>( outerIndex, -1 );
        }

        return true;
    }

    QPair<int, int> innerIndexes = innerNodeContains( mouseEvent->pos() );
    if ( innerIndexes.first != -1 && innerIndexes.second != -1 ) {
        int i = m_firstMergedNode.first;
        int j = m_firstMergedNode.second;

        if ( i == -1 && j == -1 ) {
            m_firstMergedNode = innerIndexes;
            m_innerNodesList[innerIndexes.first][innerIndexes.second].setFlag( PolygonNode::NodeIsMerged );
        } else if ( i != -1 && j == -1 ) {
            m_request = OuterInnerMergingWarning;
            m_outerNodesList[i].setFlag( PolygonNode::NodeIsMerged, false );
            m_firstMergedNode = QPair<int, int>( -1, -1 );
        } else {
            Q_ASSERT( i != -1 && j != -1 );
            if ( i != innerIndexes.first ) {
                m_request = InnerInnerMergingWarning;
                m_innerNodesList[i][j].setFlag( PolygonNode::NodeIsMerged, false );
                m_firstMergedNode = QPair<int, int>( -1, -1 );
                return true;
            }

            // Clicking two times the same node results in unmarking it for merging.
            if ( m_firstMergedNode == innerIndexes ) {
                m_innerNodesList[i][j].setFlag( PolygonNode::NodeIsMerged, false );
                m_firstMergedNode = QPair<int, int>( -1, -1 );
                return true;
            }

            // If two nodes which form an inner boundary of a polygon with a size smaller than
            // 3 are merged, remove the whole inner boundary.
            if ( innerRings.at(i).size() <= 3 ) {
                innerRings.remove( i );
                m_innerNodesList.removeAt( i );

                m_firstMergedNode = QPair<int, int>( -1, -1 );
                m_secondMergedNode = QPair<int, int>( -1, -1 );
                return true;
            }

            m_resultingCoords = innerRings.at(i).at(j).interpolate(
                                            innerRings.at(i).at(innerIndexes.second), 0.5 );
            innerRings[i][innerIndexes.second] = m_resultingCoords;
            innerRings[i].remove( j );

            m_secondMergedNode = innerIndexes;
        }

        return true;
    }

    return false;
}

bool AreaAnnotation::processAddingNodesOnPress( QMouseEvent *mouseEvent )
{
    if ( mouseEvent->button() != Qt::LeftButton ) {
        return false;
    }

    GeoDataPolygon *polygon = static_cast<GeoDataPolygon*>( placemark()->geometry() );
    GeoDataLinearRing &outerRing = polygon->outerBoundary();

    int index = virtualNodeContains( mouseEvent->pos() );
    if ( index != -1 && !m_adjustingNode ) {
        Q_ASSERT( m_virtualHovered == index );

        GeoDataLinearRing newRing;
        QList<PolygonNode> newList;
        for ( int i = index; i < index + outerRing.size(); ++i ) {
            newRing.append( outerRing.at(i % outerRing.size()) );
            newList.append( m_outerNodesList.at(i % m_outerNodesList.size()) );
        }
        GeoDataCoordinates newCoords = newRing.at(0).interpolate( newRing.last(), 0.5 );
        newRing.append( newCoords );

        polygon->outerBoundary() = newRing;
        m_outerNodesList = newList;

        m_virtualHovered = -1;
        m_adjustingNode = true;
        return true;
    }

    int outerIndex = outerNodeContains( mouseEvent->pos() );
    if ( outerIndex != -1 && m_adjustingNode ) {
        m_adjustingNode = false;
        return true;
    }

    return false;
}

bool AreaAnnotation::processAddingNodesOnMove( QMouseEvent *mouseEvent )
{
    Q_ASSERT( mouseEvent->button() == Qt::NoButton );

    if ( m_adjustingNode ) {
        // The virtual node which has just been added is always the last within
        // GeoDataLinearRing's container.
        qreal lon, lat;
        m_viewport->geoCoordinates( mouseEvent->pos().x(),
                                    mouseEvent->pos().y(),
                                    lon, lat,
                                    GeoDataCoordinates::Radian );
        const GeoDataCoordinates newCoords( lon, lat );

        GeoDataPolygon *polygon = static_cast<GeoDataPolygon*>( placemark()->geometry() );
        GeoDataLinearRing &outerRing = polygon->outerBoundary();

        outerRing.last() = newCoords;
        return true;
    } else {
        int index = virtualNodeContains( mouseEvent->pos() );
        m_virtualHovered = index;
        return true;
    }
    // This means that the interior of the polygon has been hovered. Let the event propagate
    // since there may be overlapping polygons.
    return false;
}

bool AreaAnnotation::processAddingNodesOnRelease( QMouseEvent *mouseEvent )
{
    Q_UNUSED( mouseEvent );
    return !m_adjustingNode;
}

}

