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
#include "GeoDataCoordinates.h"
#include "GeoDataLineStyle.h"

namespace Marble
{

class AreaAnnotation : public SceneGraphicsItem
{
public:
    explicit AreaAnnotation( GeoDataPlacemark *placemark );

    virtual void paint( GeoPainter *painter, const ViewportParams *viewport );

    /**
     * @brief Returns the list of selected node indexes.
     */
    QList<int> &selectedNodes();

    /**
     * @brief Returns the node index on which the mouse press event (with the right
     * button) has been caught.
     */
    int rightClickedNode() const;

    /**
     * @brief Checks whether the point parameter is contained by one of its inner
     * boundaries.
     */
    bool isInnerBoundsPoint( const QPoint &point ) const;

    /**
     * @brief Checks if the polygon has a valid shape; an invalid shape would be, for
     * example, if one of its inner boundaries ring is intersected by its outer
     * boundary ring.
     */
    bool isValidPolygon() const;

    virtual const char *graphicType() const;

private:
    /**
     * @brief Sets the polygon as selected/deselected according to the @p enabled, by
     * modifying its style.
     */
    void setSelected( bool enabled );

    /**
     * @brief Returns the first index from the regionlist() which contains the
     * event->pos().
     */
    int firstIndexContains( QMouseEvent *event );

    QList<QRegion>     m_innerBoundariesList;

    bool               m_isSelected;
    int                m_movedNodeIndex;
    int                m_rightClickedNode;
    QList<int>         m_selectedNodes;

    /**
     * We need two coords because one (m_movedPointCoords) is used when moving the whole
     * polygon since we need to get the bearing of the move as well as the distance in
     * order to apply it to all nodes. The second (m_mousePressCoords) is used to get the
     * distance between the point where the mousePressEvent has been caught and the point
     * where the mouseReleaseEvent has been caught to decide as mouse release whether
     * before it there had been a mouse move or a mouse press.
     */
    GeoDataCoordinates m_movedPointCoords;
    GeoDataCoordinates m_mousePressCoords;

    /**
     * The style of the placemark before being selected.
     */
    GeoDataLineStyle m_previousLineStyle;

    const ViewportParams *m_viewport;

protected:
    /**
     * @brief In the implementation of these virtual functions, the following convention has  been
     * followed: if the event cannot be dealt with in this class (for example when right clicking
     * a node or polygon), the functions return false and AnnotatePlugin::eventFilter deals with it.
     */
    virtual bool mousePressEvent( QMouseEvent *event );
    virtual bool mouseMoveEvent( QMouseEvent *event );
    virtual bool mouseReleaseEvent( QMouseEvent *event );
};

}

#endif // AREAANNOTATION_H
