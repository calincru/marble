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

class AreaAnnotation : public SceneGraphicsItem
{
public:
    explicit AreaAnnotation( GeoDataPlacemark *placemark );

    ~AreaAnnotation();

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

    /**
     * @brief Checks whether the point parameter is contained by one of its inner
     * boundaries.
     *
     * @param restrictive If this parameter is set to false, only check if one of its
     * inner boundaries contains the point (using GeoDataLinerRing::contains). In
     * addition to this, when restrictive is set to true, also check that none of
     * the polygon nodes' regions contain the point (yes, these regions may 'intersect'
     * due to the way nodes are represented).
     */
    bool isInnerBoundsPoint( const QPoint &point, bool restrictive = false ) const;


    int outerNodeContains( const QPoint &point ) const; 

    QPair<int, int> innerNodeContains( const QPoint &point ) const;

    int virtualNodeContains( const QPoint &point ) const;

    int boundaryContains( const QPoint &point ) const;


    bool processEditingOnPress( QMouseEvent *mouseEvent );
    bool processAddingHoleOnPress( QMouseEvent *mouseEvent );
    bool processMergingOnPress( QMouseEvent *mouseEvent );
    bool processAddingNodesOnPress( QMouseEvent *mouseEvent );



    const ViewportParams *m_viewport;
    bool                  m_regionsInitialized;

    QList<PolygonNode>          m_outerNodesList;
    QList< QList<PolygonNode> > m_innerNodesList;
    QList<PolygonNode>          m_virtualNodesList;
    QList<QRegion>              m_boundariesList;

    // Used to store the coords of the node with which we have last interacted with.
    GeoDataCoordinates          m_clickedNodeCoords;


}

}
