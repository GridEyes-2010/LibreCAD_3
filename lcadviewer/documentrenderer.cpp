#include "documentrenderer.h"
#include <cad/document/document.h>
#include "cad/primitive/line.h"
#include "cad/primitive/circle.h"
#include <drawitems/lcdrawoptions.h>
#include <drawitems/lcvcircle.h>
#include <drawitems/lcvdrawitem.h>
#include <drawitems/lcvline.h>
#include <cad/dochelpers/quadtree.h>
#include "lcpainter.h"

#include <cad/const.h>

DocumentRenderer::DocumentRenderer(lc::Document* document) : _document(document), _zoomMin(0.05), _zoomMax(20.0) {

    document->addEntityEvent().connect<DocumentRenderer, &DocumentRenderer::on_addEntityEvent>(this);
    document->removeEntityEvent().connect<DocumentRenderer, &DocumentRenderer::on_removeEntityEvent>(this);
    document->commitProcessEvent().connect<DocumentRenderer, &DocumentRenderer::on_commitProcessEvent>(this);

}

DocumentRenderer::~DocumentRenderer() {
    _document->addEntityEvent().disconnect<DocumentRenderer, &DocumentRenderer::on_addEntityEvent>(this);
    _document->removeEntityEvent().disconnect<DocumentRenderer, &DocumentRenderer::on_removeEntityEvent>(this);
    _document->commitProcessEvent().disconnect<DocumentRenderer, &DocumentRenderer::on_commitProcessEvent>(this);

    for (auto i = _cachedPainters.begin(); i != _cachedPainters.end(); i++) {
        this->_deletePainterFunctor(i->second);
    }
}

void DocumentRenderer::newDeviceSize(unsigned int width, unsigned int height) {
    if (_deviceWidth != width || _deviceHeight != height) {
        _deviceWidth = width;
        _deviceHeight = height;

        for (auto i = _cachedPainters.begin(); i != _cachedPainters.end(); i++) {
            _deletePainterFunctor(i->second);
        }

        _cachedPainters.clear();
    }

}

LcPainter* DocumentRenderer::cachedPainter(PainterCacheType cacheType) {
    if (_cachedPainters.count(cacheType) == 0) {
        _cachedPainters[cacheType] = _createPainterFunctor(_deviceWidth, _deviceHeight);
    }
    return _cachedPainters[cacheType];
}


void DocumentRenderer::scrollTo(double factor, unsigned int deviceScrollX, unsigned int deviceScrollY) {
    LcPainter* painter = cachedPainter(VIEWER_DOCUMENT);

    // Test for minimum and maximum zoom levels
    if ((_zoomMax <= painter->scale() && factor > 1.) || (_zoomMin >= painter->scale() && factor < 1.)) {
        return;
    }

    painter->save();
    // Find mouse position in user space
    double userScrollX = deviceScrollX;
    double userScrollY = deviceScrollY;
    painter->device_to_user(&userScrollX, &userScrollY);

    painter->scale(factor);

    // Find out how much the offset was after scale
    double userCenterX = deviceScrollX;
    double userCenterY = deviceScrollY;
    painter->device_to_user(&userCenterX, &userCenterY);
    painter->restore();

    // Set translation
    for (auto i = _cachedPainters.begin(); i != _cachedPainters.end(); i++) {
        LcPainter* p = i->second;
        p->translate(-userScrollX + userCenterX, -userScrollY + userCenterY);
        p->scale(factor);
    }

    // Calculate visible area
    painter = cachedPainter(VIEWER_DOCUMENT);
    double x = 0.;
    double y = 0.;
    double w = _deviceWidth;
    double h = _deviceHeight;
    painter->device_to_user(&x, &y);
    painter->device_to_user_distance(&w, &h);
    _visibleUserArea = lc::geo::Area(lc::geo::Coordinate(x, y), w, h);
}



void DocumentRenderer::render(std::function<void(LcPainter*)> wPainter) {

    LcPainter* painter = cachedPainter(VIEWER_DOCUMENT);



    // Render background
    // Cache these backgrounds
    painter = cachedPainter(VIEWER_BACKGROUND);

    if (_backgroundItems.size() == 0) {
        painter->clear(0., 0.1, 0.);
    }

    for (auto item : _backgroundItems) {
        item->draw(painter, nullptr, _visibleUserArea);
    }

    wPainter(painter);



    // Draw Document
    painter = cachedPainter(VIEWER_DOCUMENT);
    painter->clear(1., 1., 1., 0.);
    painter->source_rgb(1., 1., 1.);
    painter->lineWidthCompensation(0.5);

    const auto data = _entityContainer.allEntities();
    LcDrawOptions lcDrawOptions;

    for (auto item : data) {
        const std::shared_ptr<const LCVDrawItem> di = std::dynamic_pointer_cast<const LCVDrawItem>(item);

        if (di != nullptr) {
            di->draw(painter, &lcDrawOptions, _visibleUserArea);
        }
    }

    wPainter(painter);


    // Foreground
    painter = cachedPainter(VIEWER_DRAWING);
    painter->clear(1., 1., 1., 0.0);

    for (auto item : _foregroundItems) {
        item->draw(painter, nullptr, _visibleUserArea);
    }

    wPainter(painter);

    //    for (int i = 0; i < _cursorItems.size(); ++i) {
    //        this->_cursorItems.at(i)->draw(lcPainter, nullptr, rect);
    //    }


    /* Draw QuadTree (for debugging) */
    /*
        _painter->save();
        _painter->line_width(1.0);
        _painter->disable_antialias();
        _painter->source_rgba(0.7, 0.7, 1.0, .8);
        lc::QuadTree *t = _entityContainer.tree();
        LcPainter *painter=_painter;
        t->walkQuad(
            [painter]
                (const lc::QuadTreeSub &tree){
            lc::geo::Area a = tree.bounds();
           // painter->source_rgba(0.7, 0.7, 1.0, .8);
            painter->rectangle(a.minP().x(), a.minP().y(), a.width(), a.height());
           // painter->stroke();

            //painter->source_rgba(0.7, 1.0, .7, .8);
            //painter->rectangle(a.minP().x()+tree.level(), a.minP().y()+tree.level(), a.width()-+tree.level()*2, a.height()-+tree.level()*2);
            //painter->stroke();
        });
        painter->stroke();
        _painter->restore();*/



}

void DocumentRenderer::addBackgroundItem(std::shared_ptr<LCVDrawItem> item) {
    this->_backgroundItems.push_back(item);
}

void DocumentRenderer::addForegroundItem(std::shared_ptr<LCVDrawItem> item) {
    this->_foregroundItems.push_back(item);
}


void DocumentRenderer::on_commitProcessEvent(const lc::CommitProcessEvent&) {
    _entityContainer.optimise();
}

void DocumentRenderer::on_addEntityEvent(const lc::AddEntityEvent& event) {

    // Add a line
    const lc::Line_CSPtr line = std::dynamic_pointer_cast<const lc::Line>(event.entity());

    if (line != nullptr) {
        _entityContainer.insert(std::make_shared<LCVLine>(line));
        return;
    }

    // Add a circle
    const lc::Circle_CSPtr circle = std::dynamic_pointer_cast<const lc::Circle>(event.entity());

    if (circle != nullptr) {
        auto newCircle = std::make_shared<LCVCircle>(circle);
        newCircle->selected(true);
        _entityContainer.insert(newCircle);
        return;
    }

    /*

    // Add a Arc
    const std::shared_ptr<const lc::Arc> arc = std::dynamic_pointer_cast<const lc::Arc>(event.entity());

    if (arc != nullptr) {
        LCArcItem* foo = new LCArcItem(arc);
        foo->setFlags(QGraphicsItem::ItemIsSelectable);
        scene->addItem(foo);
        _activeGraphicsItems.insert(arc->id(), foo);
        return;
    }

    // Add Ellipse
    const std::shared_ptr<const lc::Ellipse> ellipse = std::dynamic_pointer_cast<const lc::Ellipse>(event.entity());

    if (ellipse != nullptr) {
        LCEllipseItem* foo = new LCEllipseItem(ellipse);
        foo->setFlags(QGraphicsItem::ItemIsSelectable);
        scene->addItem(foo);
        _activeGraphicsItems.insert(ellipse->id(), foo);
        return;
    }
    */
}

void DocumentRenderer::on_removeEntityEvent(const lc::RemoveEntityEvent& event) {
    _entityContainer.remove(event.entity());
}

void DocumentRenderer::createPainterFunctor(const std::function<LcPainter *(const unsigned int, const unsigned int)>& createPainterFunctor) {
    _createPainterFunctor = createPainterFunctor;
}

void DocumentRenderer::deletePainterFunctor(const std::function<void(LcPainter*)>& deletePainterFunctor) {
    _deletePainterFunctor = deletePainterFunctor;
}

lc::geo::Area DocumentRenderer::bounds() const {
    return _entityContainer.bounds();
}

void DocumentRenderer::makeSelection(double x, dobule y, double w, double y, bool occupies) {


}

void DocumentRenderer::removeSelectionArea() {

}

void DocumentRenderer::removeSelection() {

}
