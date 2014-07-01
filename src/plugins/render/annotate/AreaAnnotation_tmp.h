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

#ifndef AREAANNOTATION_H
#define AREAANNOTATION_H

#include "SceneGraphicsItem.h"


namespace Marble
{

class GeoDataPlacemark;
class PolygonNode;
class QPair;
class QColor;

class AreaAnnotation : public SceneGraphicsItem
{
public:
    explicit AreaAnnotation( GeoDataPlacemark *placemark );

    ~AreaAnnotation();

    enum EditingInteractingObject {
        InteractingNothing, // e.g. when hovering
        InteractingNode,
        InteractingPolygon
    }

    /**
     * @brief
     */
    virtual void paint( GeoPainter *painter, const ViewportParams *viewport );

    /**
     * @brief
     */
    virtual bool containsPoint( const QPoint &point ) const;

    /**
     * @brief
     */
    virtual void itemChanged( const SceneGraphicItem &other );

    /**
     * @brief Provides information for downcasting a SceneGraphicsItem.
     */
    virtual const char *graphicType() const;

protected:
    virtual bool mousePressEvent( QMouseEvent *event );
    virtual bool mouseMoveEvent( QMouseEvent *event );
    virtual bool mouseReleaseEvent( QMouseEvent *event );

private:
    /**
     * @brief
     */
    void setupRegionsLists( GeoPainter *painter );

    /**
     * @brief
     */
    void drawNodes( GeoPainter *painter );

    int outerNodeContains( const QPoint &point ) const;

    QPair<int, int> innerNodeContains( const QPoint &point ) const;

    int virtualNodeContains( const QPoint &point ) const;

    int innerBoundsContain( const QPoint &point ) const;

    bool polygonContains( const QPoint &point ) const;


    bool processEditingOnPress( QMouseEvent *mouseEvent );
    bool processEditingOnMove( QMouseEvent *mouseEvent );
    bool processEditingOnRelease( QMouseEvent *mouseEvent );

    bool processAddingHoleOnPress( QMouseEvent *mouseEvent );
    bool processAddingHoleOnMove( QMouseEvent *mouseEvent );
    bool processAddingHoleOnRelease( QMouseEvent *mouseEvent );

    bool processMergingOnPress( QMouseEvent *mouseEvent );
    bool processMergingOnMove( QMouseEvent *mouseEvent );
    bool processMergingOnRelease( QMouseEvent *mouseEvent );

    bool processAddingNodesOnPress( QMouseEvent *mouseEvent );
    bool processAddingNodesOnMove( QMouseEvent *mouseEvent );
    bool processAddingNodesOnRelease( QMouseEvent *mouseEvent );

    void updateBoundariesList();


    static const int regularDim;
    static const int selectedDim;
    static const int mergedDim;
    static const int hoveredDim;
    static const QColor regularColor;
    static const QColor selectedColor;
    static const QColor mergedColor;
    static const QColor hoveredColor;

    const GeoPainter     *m_geopainter;
    const ViewportParams *m_viewport;
    bool                  m_regionsInitialized;

    QList<PolygonNode>          m_outerNodesList;
    QList< QList<PolygonNode> > m_innerNodesList;
    QList<PolygonNode>          m_virtualNodesList;
    QList<QRegion>              m_boundariesList;

    // Used in the Editing state
    GeoDataCoordinates       m_movedPointCoords;
    QPair<int, int>          m_clickedNodeIndexes;
    EditingInteractingObject m_interactingObj;

}

}
