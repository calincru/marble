//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2014      Calin Cruceru  <crucerucalincristian@gmail.com>
//

#ifndef POLYLINEANNOTATION_H
#define POLYLINEANNOTATION_H

#include <QColor>

#include "SceneGraphicsItem.h"
#include "GeoDataCoordinates.h"


namespace Marble
{

class PolylineNode;
class MergingNodesAnimation;

class PolylineAnnotation : public SceneGraphicsItem
{
public:
    explicit PolylineAnnotation( GeoDataPlacemark *placemark );
    ~PolylineAnnotation();

    virtual void paint( GeoPainter *painter, const ViewportParams *viewport );

    virtual bool containsPoint( const QPoint &eventPos ) const;

    virtual void dealWithItemChange( const SceneGraphicsItem *other );

    virtual void move( const GeoDataCoordinates &source, const GeoDataCoordinates &destination );

    void setBusy( bool enabled );

    /**
     * @brief Iterates through all nodes which form the polygon's outer boundary as well
     * as all its inner boundaries and sets the IsSelected flag to false.
     */
    void deselectAllNodes();

    /**
     * @brief Iterates through all nodes which form the polygon's outer boundary as well
     * as all its inner boundaries and deletes the selected ones.
     */
    void deleteAllSelectedNodes();

    /**
     * @brief Deletes the last clicked node while being in the Editing state.
     */
    void deleteClickedNode();

    /**
     * @brief If the last clicked node is selected, set its IsSelected flag to false and
     * vice versa.
     */
    void changeClickedNodeSelection();

    /**
     * @brief Tests if there are any selected nodes.
     */
    bool hasNodesSelected() const;

    /**
     * @brief Tests if the last clicked node is selected.
     */
    bool clickedNodeIsSelected() const;

    /**
     * @brief Provides information for downcasting a SceneGraphicsItem.
     */
    virtual const char *graphicType() const;

protected:
    virtual bool mousePressEvent( QMouseEvent *event );
    virtual bool mouseMoveEvent( QMouseEvent *event );
    virtual bool mouseReleaseEvent( QMouseEvent *event );

    virtual void dealWithStateChange( SceneGraphicsItem::ActionState previousState );

private:
    void setupRegionsLists( GeoPainter *painter );

    void updateRegions( GeoPainter *painter );

    void drawNodes( GeoPainter *painter );

    int nodeContains( const QPoint &point ) const;

    int virtualNodeContains( const QPoint &point ) const;

    bool polylineContains( const QPoint &point ) const;

    /**
     * @brief Since they are used in many functions, the size and color of nodes for each
     * state are static and have class scope.
     */
    static const int regularDim;
    static const int selectedDim;
    static const int mergedDim;
    static const int hoveredDim;
    static const QColor regularColor;
    static const QColor selectedColor;
    static const QColor mergedColor;
    static const QColor hoveredColor;

    const ViewportParams *m_viewport;
    bool m_regionsInitialized;
    bool m_busy;

    QList<PolylineNode> m_nodesList;
    QList<PolylineNode> m_virtualNodesList;
    QRegion             m_polylineRegion;

    // Used in Editing state
    GeoDataCoordinates m_movedPointCoords;
    int m_clickedNodeIndex;
    int m_hoveredNodeIndex;

    // Used in Merging Nodes state
    QPointer<MergingNodesAnimation> m_animation;
    int m_firstMergedNode;
    int m_secondMergedNode;

    // Used in Adding Nodes state
    int m_virtualHoveredNode;

    // FIXME: It won't have the same meaning here.
    // It can have the following values:
    //     -> -2 - means there is no node being adjusted;
    //     -> -1 - means the node which is being adjusted is a node from polygon's
    //             outer boundary (more exactly, the last; see below);
    //     -> i  - (i >= 0) means the node which is being adjusted is a node from
    //             the i'th inner boundary (more exactly, the last one; see below).
    // Due to the way the node appending is done (by rotating the vector which
    // contains the coordinates), we can be sure that the node we want to adjust
    // is everytime the last one.
    int             m_adjustedNode;

};

}

#endif
