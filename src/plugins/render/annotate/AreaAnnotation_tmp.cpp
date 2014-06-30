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


/**
 * NOTES:
 * -> metoda PAINT seteaza doar pentru prima data nodurile, cand se deseneaza poligonul, iar apoi eu ma
 *  ocup de acestea in mod dinamic, in timp ce modific poly->outerBoundary si innerBoundary schimb si
 *  listele de regiuni.
 * -> voi schimba modul in care se face inner boundary adding si nu voi mai desena nodurile in timp ce
 *  dau click ci le voi tine intr-un buffer, intr-un GeoDataLinearRing temporar pana cand iese din starea
 *  de adding polygon holes. Dupa, pur si simplu dau poly->innerBoundaries.append( buffer ). Face mult mai
 *  mult sens din punctul de vedere al utilizatorului.
 * -> modul in care e PAINT implementat in momentul de fata permite foarte usor extinderea pentru alte stari.
 *  Spre exemplu, pentru a face hover pe nodurile virtuale si pe inner boundaries trb sa adaug doar 2 foruri
 *  imbricate in drawNodes. INSA!! Eu acum, in implementare, trebuie sa ma asigur de urmatoarele lucruri:
 *      -> regiunile sunt conform cu outerboundary/inner boundary. Asta inseamna ca primul element din
 *  polygon->outerBoundary() corespunde regiunii pe care am creat-o pentru acesta in m_outerNodesList.
 *      -> Dupa fiecare operatie in functiile de event handling, trebuie ca nodurile sa aiba setate flagurile
 *  corecte. Spre exemplu, doar un nod la un moment dat din cele virtuale POATE avea flagul de virtual hovered.
 * -> in AnnotatePlugin sa am un approach de genul: pastrez ultimul SceneGraphicsItem cu care am interactionat
 *  si la fiecare event nou, verific daca itemul este altul. Daca este altul, apelez o functie care anunte toate
 *  itemele ca s-a schimbat itemul (sau poate pe cel vechi? pt moment). Asta ar fi folositoare, spre exemplu:
 *  in adding polygon holes, cand da click pe interiorul unui poligon handeluiesc aici (tin intr-un buffer, etc,
 *  am explicat mai sus), dar daca schimba poligonul pe care da click, cel vechi trebuie sa stie pentru a-l face
 *  real inner boundary-ul sau pentru a-l sterge daca are sub 3 noduri.
 */

namespace Marble {

class PolygonNode {

public:
    PolygonNode( QRegion region );
    ~PolygonNode();

    enum PolyNodeFlag {
        NoOption = 0x0,
        NodeIsSelected = 0x1,
        NodeIsMerged = 0x2,
        NodeIsVirtualHovered = 0x4
    };

    Q_DECLARE_FLAGS(PolyNodeFlags, PolyNodeFlag)

    bool operator==( const PolygonNode &other ) const;
    bool operator!=( const PolygonNode &other ) const;

    bool isSelected() const;
    bool isBeingMerged() const;
    bool isVirtualHovered() const;

    void setFlag( PolyNodeFlag flag, bool enabled = true );
    void setFlags( PolyNodeFlags flags );
    void setRegion( QRegion newRegion );

    bool containsPoint( const QPoint &eventPos ) const;

private:
    QRegion m_region;
    PolyNodeFlags m_flags;
}

PolygonNode::PolygonNode( QRegion region ) :
    m_region( region ),
    m_flags( 0 )
{
    // nothing to do
}

PolygonNode::~PolygonNode()
{
    // nothing to do
}

bool PolygonNode::operator==( const PolygonNode &other ) const
{
    return m_region == other.m_region &&
           m_isSelected == other.m_isSelected &&
           m_isVirtual == other.m_isVirtual;
}

bool PolygonNode::operator!=( const PolygonNode &other ) const
{
    return !this->operator==( other );
}

bool PolygonNode::isSelected() const
{
    return m_flags & NodeIsSelected;
}

bool PolygonNode::isBeingMerged() const
{
    return m_flags & NodeIsMerged;
}

bool PolygonNode::isVirtualHovered() const
{
    return m_flags & NodeIsVirtualHovered;
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


AreaAnnotation::AreaAnnotation( GeoDataPlacemark *placemark ) :
    SceneGraphicsItem( placemark ),
    m_viewport( 0 ),
    m_regionsInitialized( false )
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
    Q_ASSERT( placemark()->geometry()->nodeType() == GeoDataTypes::GeoDataPolygonType );

    painter->save();
    if ( !m_regionsInitialized ) {
        setupRegionsLists( painter );
        m_regionsInitialized = true;
    }

    drawNodes( painter );
    painter->restore();
}

bool AreaAnnotation::containsPoint( const QPoint &point ) const
{
    if ( outerNodeContains( point ) != -1 || innerNodeContains( point ) != QPair(-1, -1) ||
         virtualNodeContains( point ) != -1 || boundaryContains( point ) != -1 ) {
        return true;
    }

    return false;
}

bool AreaAnnotation::mousePressEvent( QMouseEvent *event )
{
    SceneGraphicsItem::ActionState state = state();

    if ( state == SceneGraphicsItem::Editing ) {

    } else if ( state == SceneGraphicsItem::AddingPolygonHole ) {

    } else if ( state == SceneGraphicsItem::MergingPolygonNodes ) {

    } else if ( state == SceneGraphicsItem::AddingPolygonNodes ) {

    } else {
        return false;
    }

    return true;
}

bool AreaAnnotation::mouseMoveEvent( QMouseEvent *event )
{

}

bool AreaAnnotation::mouseReleaseEvent( QMouseEvent *event )
{

}

const char *AreaAnnotation::graphicType() const
{
    return SceneGraphicTypes::SceneGraphicAreaAnnotation;
}

void AreaAnnotation::setupRegionLists( QPainter *painter )
{
    const static int regDim = 20;
    const static int virtualsDim = 25;

    const GeoDataPolygon *polygon = static_cast<const GeoDataPolygon*>( placemark()->geometry() );
    const GeoDataLinearRing &outerRing = polygon->outerBoundary();

    // Add the outer boundary nodes
    QVector<GeoDataCoordinates>::ConstIterator itBegin = outerRing.begin();
    QVector<GeoDataCoordinates>::ConstIterator itEnd = outerRing.end();

    for ( ; itBegin != itEnd; ++itBegin ) {
        PolygonNode newNode = PolygonNode( painter->regionFromEllipse( *itBegin, regDim, regDim ) );
        m_outerNodesList.append( newNode );

        // Add at the same time the virtual nodes
        if ( !m_virtualNodesList.size() ) {
            GeoDataCoordinates coords = (*itBegin).interpolate( *itEnd, 0.5 );
            PolygonNode newNode = PolygonNode( painter->regionFromEllipse( coords, virtualsDim, virtualsDim ) );
            m_virtualNodesList.append( newNode );
        } else {
            GeoDataCoordinates coords = (*itBegin).interpolate( *(itBegin - 1), 0.5 );
            PolygonNode newNode = PolygonNode( painter->regionFromEllipse( coords, virtualsDim, virtualsDim ) );
            m_virtualNodesList.append( newNode );
        }
    }

    // Add the outer boundary to the boundaries list
    m_boundariesList.append( painter->regionFromPolygon( outerRing, Qt::OddEvenFill ) );
}

void AreaAnnotation::drawNodes( QPainter *painter )
{
    const static int regDim = 10;
    const static int selectedDim = 10;
    const static int mergedDim = 15;
    const static int hoveredDim = 15;
    const QColor regularColor = Oxygen::aluminumGray3;
    const QColor selectedColor = Oxygen::aluminumGray6;
    const QColor mergedColor = Oxygen::emeraldGreen6;
    const QColor hoveredColor = Oxygen::burgundyPurple4;

    const GeoDataPolygon *polygon = static_cast<const GeoDataPolygon*>( placemark()->geometry() );
    const GeoDataLinearRing &outerRing = polygon->outerBoundary();
    const QVector<GeoDataLinearRing> &innerRings = polygon->innerBoundaries();

    for ( int i = 0; i < outerRing.size(); ++i ) {
        // The order here is important, because a merged node can be at the same time selected.
        if ( m_outerNodesList.at(i).isMerged() ) {
            painter->setBrush( mergedColor );
            painter->drawEllipse( outerRing.at(i), mergedDim, mergedDim );
        } else if ( m_outerNodesList.at(i).isSelected() ) {
            painter->setBrush( selectedColor );
            painter->drawEllipse( outerRing.at(i), selectedDim, selectedDim );
        } else {
            painter->setBrush( regularColor );
            painter->drawEllipse( outerRing.at(i), regDim, regDim );
        }
    }

    for ( int i = 0; i < innerRings.size(); ++i ) {
        for ( int j = 0; j < innerRings.at(i).size(); ++i ) {
            if ( m_innerNodesList.at(i).at(j).isMerged() ) {
                painter->setBrush( mergedColor );
                painter->drawEllipse( innerRings.at(i).at(j), mergedDim, mergedDim );
            } else if ( m_innerNodesList.at(i).at(j).isMerged() ) {
                painter->setBrush( selectedColor );
                painter->drawEllipse( innerRings.at(i).at(j), selectedDim, selectedDim );
            } else {
                painter->setBrush( regularColor );
                painter->drawEllipse( innerRings.at(i).at(j), regDim, regDim );
            }
        }
    }

    for ( int i = 0; i < m_virtualNodesList.size(); ++i ) {
        if ( m_virtualNodesList.at(i).isVirtualHovered() ) {
            painter->setBrush( hoveredColor );

            GeoDataCoordinates coords;
            if ( i ) {
                coords = m_outerNodesList.at(i).interpolate( m_outerNodesList.at(i - 1), 0.5 );
            } else {
                coords = m_outerNodesList.at(i).interpolate(
                            m_outerNodesList.at(m_outerNodesList.size() - 1), 0.5 );
            }
            painter->drawEllipse( coords, hoveredDim, hoveredDim );
        }
    }
}

bool AreaAnnotation::isInnerBoundsPoint( const QPoint &point, bool restrictive = false ) const
{
    // Starting from 1 because on the first position is always stored the polygon's QRegion.
    // @see setupRegionLists
    for ( int i = 1; i < m_boundariesList.size(); ++i ) {
        if ( m_boundariesList.at(i).contains( point ) {
            if ( restrictive ) {
                foreach ( const QList<PolygonNode> &innerNodes, m_innerNodesList ) {
                    foreach ( const PolygonNode &node, innerNodes ) {
                        if ( node.containsPoint( point ) ) {
                            return false;
                        }
                    }
                }

                return true;
            } else {
                return true;
            }
        }
    }

    return false;
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
                return QPair(i, j);
            }
        }
    }

    return QPair(-1, -1);
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

int AreaAnnotation::boundaryContains( const QPoint &point ) const
{
    for ( int i = 0; i < m_boundariesList.size(); ++i ) {
        if ( m_boundariesList.at(i).containsPoint( point ) ) {
            return i;
        }
    }

    return -1;

}

bool AreaAnnotation::processEditingOnPress( QMouseEvent *mouseEvent )
{
    const GeoDataPolygon *polygon = static_cast<const GeoDataPolygon*>( placemark()->geometry() );
    const GeoDataLinearRing &outerRing = polygon->outerBoundary();
    const QVector<GeoDataLinearRing> &innerRings = polygon->innerBoundaries();

    if ( mouseEvent->button() != Qt::LeftButton && mouseEvent->button() != Qt::RightButton ) {
        return false;
    }

    // First check if one of the nodes from outer boundary has been clicked.
    int outerIndex = outerNodeContains( mouseEvent.pos() );
    if ( outerIndex != -1 ) {
        m_clickedNodeCoords = outerRing.at(outerIndex);
        return true;
    }

    // Then check if one of the nodes which form an inner boundary has been clicked.
    QPair<int, int> innerIndexes = innerNodeContains( mouseEvent.pos() );
    if ( innerIndexes.first != -1 && innerIndexes.second != -1 ) {
        m_clickedNodeCoords = innerRings.at(innerIndexes.first).at(innerIndexes.second);
        return true;
    }

    if ( !boundaryContains( mouseEvent->pos() ) ) {
        
        return true;
    }

    return false;
}

bool AreaAnnotation::processAddingHoleOnPress( QMouseEvent *mouseEvent )
{

}

bool AreaAnnotation::processMergingOnPress( QMouseEvent *mouseEvent )
{

}

bool AreaAnnotation::processAddingNodesOnPress( QMouseEvent *mouseEvent )
{

}

}

