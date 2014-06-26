To FIX:
- the problem regarding removing nodes when the polygon has inner boundaries: it
  is caused by the fact that GeoDataLinearRing::contains does not take into
  consideration that the polygon is Tessellated.
- the problem regarding moving the polygon around poles (and moving in general
  which changes polygon's shape).


To ADD:
- adding nodes (the structure is done, there are still a few problems):
    -> in AreaAnnotation::mousePressEvent is a problem caused by the fact that 
    when clicking the virtual node "from the polygon's inside"
    firstRegionWhichContains is in fact the polygon which fucks up everything;
    -> skip mouse press release (keep m_movedItem != nullptr );
    -> maybe the most important, see how that virtual nod will be added;
    -> organize a little bit AreaAnnotation class (comments, maybe new methods
    due to repeating code sequences);
    -> in the end, maybe take into consideration showing the virtual node
    evetywhere on a polygon's side.
- a more friendly user interface;
- redesigning the whole event filter in AnnotatePlugin: find a why to keep the
  code easier to upgrade and better documented as well as easier to be
  understood by newcomers (new contributors).
