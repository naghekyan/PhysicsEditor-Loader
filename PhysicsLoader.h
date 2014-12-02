
#ifndef PhysicsLoader_x_h
#define PhysicsLoader_x_h

#include "cocos2d.h"

USING_NS_CC;

class BodyDef;
class b2Body;

class PhysicsLoader 
{
public:
    PhysicsLoader(float globalPtmRatio = 1.0f);
    void addShapesWithFile(const std::string &plist);
    void addFixturesToBody(b2Body *body, const std::string &shape);
    Vec2 getAnchorPointForShape(const std::string &shape) const;
    void reset();
    float getPtmRatio() const;
    ~PhysicsLoader();
        
private:
    std::map<std::string, BodyDef *>        m_shapeObjects;
    float                                   m_ptmRatio;
    // global PTM ratio is represented to arrange the vertex coordinates with 
    // director->getContentScaleFactor as for different scale factors Sprite->getContentSize()
    // returns texture size / ContentScaleFactor and this should be controlled
    float                                   m_globalPtmRatio;
};


#endif
