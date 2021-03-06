#include "cad/primitive/dimradial.h"
#include "dimradial.h"


using namespace lc;
using namespace entity;

DimRadial::DimRadial(geo::Coordinate definitionPoint,
                     geo::Coordinate middleOfText,
                     TextConst::AttachmentPoint attachmentPoint,
                     double angle,
                     double lineSpacingFactor,
                     TextConst::LineSpacingStyle lineSpacingStyle,
                     std::string explicitValue,
                     geo::Coordinate definitionPoint2,
                     double leader,
                     meta::Layer_CSPtr layer,
                     meta::MetaInfo_CSPtr metaInfo,
                     meta::Block_CSPtr block) :
    CADEntity(std::move(layer), std::move(metaInfo), std::move(block)),
    Dimension(std::move(definitionPoint),
              std::move(middleOfText),
              attachmentPoint,
              angle,
              lineSpacingFactor,
              lineSpacingStyle,
              std::move(explicitValue)
             ),
    _leader(leader),
    _definitionPoint2(std::move(definitionPoint2)) {
}

DimRadial::DimRadial(const DimRadial_CSPtr& other, bool sameID) :
    CADEntity(other, sameID),
    Dimension(*other),
    _leader(other->_leader),
    _definitionPoint2(other->_definitionPoint2) {
}

DimRadial::DimRadial(const lc::builder::DimRadialBuilder& builder) :
    CADEntity(builder),
    Dimension(builder),
    _leader(builder.leader()),
    _definitionPoint2(builder.definitionPoint2()) {
}

CADEntity_CSPtr DimRadial::move(const geo::Coordinate& offset) const {
    auto newDimRadial = std::make_shared<DimRadial>(this->definitionPoint() + offset,
                        this->middleOfText() + offset,
                        this->attachmentPoint(),
                        this->textAngle(),
                        this->lineSpacingFactor(),
                        this->lineSpacingStyle(),
                        this->explicitValue(),
                        this->_definitionPoint2 + offset,
                        this->_leader,
                        this->layer()
                        , metaInfo(), block()
                                                   );
    newDimRadial->setID(this->id());
    return newDimRadial;
}

CADEntity_CSPtr DimRadial::copy(const geo::Coordinate& offset) const {
    auto newDimRadial = std::make_shared<DimRadial>(this->definitionPoint() + offset,
                        this->middleOfText() + offset,
                        this->attachmentPoint(),
                        this->textAngle(),
                        this->lineSpacingFactor(),
                        this->lineSpacingStyle(),
                        this->explicitValue(),
                        this->_definitionPoint2 + offset,
                        this->_leader,
                        this->layer(), metaInfo(), block()
                                                   );
    return newDimRadial;
}

CADEntity_CSPtr DimRadial::rotate(const geo::Coordinate& rotation_center, double rotation_angle) const {
    auto newDimRadial = std::make_shared<DimRadial>(this->definitionPoint().rotate(rotation_center, rotation_angle),
                        this->middleOfText().rotate(rotation_center, rotation_angle),
                        this->attachmentPoint(),
                        this->textAngle(),
                        this->lineSpacingFactor(),
                        this->lineSpacingStyle(),
                        this->explicitValue(),
                        this->_definitionPoint2.rotate(rotation_center, rotation_angle),
                        this->_leader,
                        this->layer(), metaInfo(), block()
                                                   );
    newDimRadial->setID(this->id());
    return newDimRadial;
}

CADEntity_CSPtr DimRadial::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto newDimRadial = std::make_shared<DimRadial>(this->definitionPoint().scale(scale_center, scale_factor),
                        this->middleOfText().scale(scale_center, scale_factor),
                        this->attachmentPoint(),
                        this->textAngle(),
                        this->lineSpacingFactor(),
                        this->lineSpacingStyle(),
                        this->explicitValue(),
                        this->_definitionPoint2.scale(scale_center, scale_factor),
                        this->_leader,
                        this->layer(), metaInfo(), block()
                                                   );
    newDimRadial->setID(this->id());
    return newDimRadial;
}

CADEntity_CSPtr DimRadial::mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const {
    auto newDimRadial = std::make_shared<DimRadial>(this->definitionPoint().mirror(axis1,axis2),
                        this->middleOfText().mirror(axis1,axis2),
                        this->attachmentPoint(),
                        this->textAngle(),
                        this->lineSpacingFactor(),
                        this->lineSpacingStyle(),
                        this->explicitValue(),
                        this->_definitionPoint2.mirror(axis1,axis2),
                        this->_leader,
                        this->layer(), metaInfo(), block()
                                                   );
    newDimRadial->setID(this->id());
    return newDimRadial;
}

const geo::Area DimRadial::boundingBox() const {
    return geo::Area(this->middleOfText(), this->definitionPoint2());
}

CADEntity_CSPtr DimRadial::modify(meta::Layer_CSPtr layer, meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const {
    auto newDimRadial = std::make_shared<DimRadial>(
                            this->definitionPoint(),
                            this->middleOfText(),
                            this->attachmentPoint(),
                            this->textAngle(),
                            this->lineSpacingFactor(),
                            this->lineSpacingStyle(),
                            this->explicitValue(),
                            this->_definitionPoint2,
                            this->_leader,
                            layer,
                            metaInfo, block
                        );
    newDimRadial->setID(this->id());
    return newDimRadial;
}

double DimRadial::leader() const {
    return _leader;
}

geo::Coordinate DimRadial::definitionPoint2() const {
    return _definitionPoint2;
}

std::map<unsigned int, geo::Coordinate> DimRadial::dragPoints() const {
    std::map<unsigned int, geo::Coordinate> dragPoints;

    dragPoints[0] = definitionPoint();
    dragPoints[1] = middleOfText();
    dragPoints[2] = _definitionPoint2;

    return dragPoints;
}


CADEntity_CSPtr DimRadial::setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const {
    try {
        auto newEntity = std::make_shared<DimRadial>(dragPoints.at(0),
                         dragPoints.at(1),
                         attachmentPoint(),
                         textAngle(),
                         lineSpacingFactor(),
                         lineSpacingStyle(),
                         explicitValue(),
                         dragPoints.at(2),
                         leader(),
                         layer(), metaInfo(), block());
        newEntity->setID(id());
        return newEntity;
    }
    catch(std::out_of_range& e) {
        return shared_from_this();
    }
}

PropertiesMap DimRadial::availableProperties() const {
    PropertiesMap propertyValues;

    getDimensionProperties(propertyValues);
    propertyValues["definitionPoint2"] = this->definitionPoint2();
    propertyValues["leader"] = this->leader();

    return propertyValues;
}

CADEntity_CSPtr DimRadial::setProperties(const PropertiesMap& propertiesMap) const {
    lc::geo::Coordinate definitionPointp, middleOfTextp;
    double textAnglep, lineSpacingFactorp;
    std::string explicitValuep;
    lc::geo::Coordinate definitionPoint2p = this->definitionPoint2();
    double leaderp = this->leader();

    setDimensionProperties(propertiesMap, definitionPointp, middleOfTextp, textAnglep, lineSpacingFactorp, explicitValuep);

    for (auto iter = propertiesMap.begin(); iter != propertiesMap.end(); ++iter)
    {
        if (iter->first == "definitionPoint2") {
            definitionPoint2p = boost::get<lc::geo::Coordinate>(iter->second);
        }

        if (iter->first == "leader") {
            leaderp = boost::get<double>(iter->second);
        }
    }

    auto newDimRad = std::make_shared<DimRadial>(definitionPointp, middleOfTextp, attachmentPoint(), textAnglep, lineSpacingFactorp,
                     lineSpacingStyle(), explicitValuep, definitionPoint2p, leaderp, layer(), metaInfo(), block());
    newDimRad->setID(this->id());
    return newDimRad;
}
