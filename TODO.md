To FIX:
- the problem regarding removing nodes when the polygon has inner boundaries: it
  is caused by the fact that GeoDataLinearRing::contains does not take into
  consideration that the polygon is Tessellated.
- the problem regarding moving the polygon around poles (and moving in general
  which changes polygon's shape).


To ADD:
- merging nodes - maybe highlight on the nodes which could be merged, but
  after review.
- adding nodes:
    -> when painting the nodes, maybe add in a new list the regions
    which contain the middle of each segment and their nearby surrounding area;
    -> modify AreaAnnotation::mouseMoveEvent as well as
    AnnotatePlugin::eventFilter to deal with mouse move events caught on
    polygons (not necessary when m_movedItem != nullptr) - search within the
    above mentioned list;
    -> show some "virtual node" as tackat mentioned and when being clicked,
    make it become real and have it selected (m_movedItem = that node).
- a more friendly user interface;
- redesigning the whole event filter in AnnotatePlugin: find a why to keep the
  code easier to upgrade and better documented as well as easier to be
  understood by newcomers (new contributors).
