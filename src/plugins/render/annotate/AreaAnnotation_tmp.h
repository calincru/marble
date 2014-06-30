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
    bool processMergingOnPress( QMouseEvent *mouseEvent );
    bool processAddingNodesOnPress( QMouseEvent *mouseEvent );


    const static int regularDim = 10;
    const static int selectedDim = 10;
    const static int mergedDim = 15;
    const static int hoveredDim = 15;
    const static QColor regularColor = Oxygen::aluminumGray3;
    const static QColor selectedColor = Oxygen::aluminumGray6;
    const static QColor mergedColor = Oxygen::emeraldGreen6;
    const static QColor hoveredColor = Oxygen::burgundyPurple4;

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
