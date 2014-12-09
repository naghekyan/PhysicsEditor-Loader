#include "PhysicsLoader.h"
#include "Box2D/Box2D.h"


/**
 * Internal class to hold the fixtures
 */
class FixtureDef 
{
public:
    FixtureDef()
    : next(NULL) {}
    
    ~FixtureDef() 
    {
        delete next;
        delete fixture.shape;
    }
    
    FixtureDef *next;
    b2FixtureDef fixture;
    int callbackData;
};

class BodyDef 
{
public:
    BodyDef()
    : fixtures(NULL) {}
    
    ~BodyDef() 
    {
        if (fixtures)
            delete fixtures;
    }
    
    FixtureDef *fixtures;
    Vec2 anchorPoint;
};


PhysicsLoader::PhysicsLoader()
{
}

PhysicsLoader::~PhysicsLoader()
{
}


void PhysicsLoader::reset() 
{
    std::map<std::string, BodyDef *>::iterator iter;
    for (iter = m_shapeObjects.begin() ; iter != m_shapeObjects.end() ; ++iter) 
    {
        delete iter->second;
    }
    m_shapeObjects.clear();
}

void PhysicsLoader::addFixturesToBody(b2Body *body, const std::string &shape) 
{
    std::map<std::string, BodyDef *>::iterator pos = m_shapeObjects.find(shape);
    assert(pos != m_shapeObjects.end());
    
    BodyDef *so = (*pos).second;

    FixtureDef *fix = so->fixtures;
    while (fix) 
    {
        body->CreateFixture(&fix->fixture);
        fix = fix->next;
    }
}

Vec2 PhysicsLoader::getAnchorPointForShape(const std::string &shape) const
{
    std::map<std::string, BodyDef *>::const_iterator pos = m_shapeObjects.find(shape);
    assert(pos != m_shapeObjects.end());
    
    BodyDef *bd = (*pos).second;
    return bd->anchorPoint;
}


void PhysicsLoader::addShapesWithFile(const std::string &plist) 
{
    std::string fullName = FileUtils::getInstance()->fullPathForFilename(plist);
    ValueMap dict =  FileUtils::getInstance()->getValueMapFromFile(fullName);
    
    CCAssert(dict.size() != 0, "plist file empty or not existing");
    
    ValueMap metadataDict = dict["metadata"].asValueMap();
    int format = Value(metadataDict["format"]).asInt();
    m_ptmRatio = Value(metadataDict["ptm_ratio"]).asFloat();
    CCAssert(format == 1, "Format not supported");

    ValueMap bodyDict = dict["bodies"].asValueMap();

    b2Vec2 vertices[b2_maxPolygonVertices];
   

    for (auto pElement : bodyDict)
    {
        ValueMap bodyData = pElement.second.asValueMap();
        std::string bodyName = pElement.first;
         
        BodyDef *bodyDef = new BodyDef();
        std::string content = Value(bodyData["anchorpoint"]).asString();
        bodyDef->anchorPoint = PointFromString(content);
        
        ValueVector fixtureList = bodyData["fixtures"].asValueVector();
        FixtureDef **nextFixtureDef = &(bodyDef->fixtures);

        
        for (auto pObj : fixtureList)
        {
            ValueMap fixtureData = pObj.asValueMap(); 
            b2FixtureDef basicData;
             
            
            basicData.filter.categoryBits = Value(fixtureData["filter_categoryBits"]).asInt();
            basicData.filter.maskBits = Value(fixtureData["filter_maskBits"]).asInt();
            basicData.filter.groupIndex = Value(fixtureData["filter_groupIndex"]).asInt();
            basicData.friction = Value(fixtureData["friction"]).asFloat();
            basicData.density = Value(fixtureData["density"]).asFloat();
            basicData.restitution = Value(fixtureData["restitution"]).asFloat();
            basicData.isSensor = (bool)Value(fixtureData["isSensor"]).asBool();

            Value cb = Value(fixtureData["userdataCbValue"]);
            
            int callbackData = 0;
            
            if (!cb.isNull())
                callbackData = cb.asInt();
            
            std::string fixtureType = Value(fixtureData["fixture_type"]).asString();


            if (fixtureType == "POLYGON") 
            {
                ValueVector polygonsArray = fixtureData["polygons"].asValueVector();
                

                for (auto pObjPolygons : polygonsArray)
                {
                    FixtureDef *fix = new FixtureDef();
                    fix->fixture = basicData; // copy basic data
                    fix->callbackData = callbackData;
                    
                    b2PolygonShape *polyshape = new b2PolygonShape();
                    int vindex = 0;
                    ValueVector polygonArray = pObjPolygons.asValueVector(); 
                     
                     
                    assert(polygonArray.size() <= b2_maxPolygonVertices);

                    
                    for (auto pObjPolygon : polygonArray)
                    {
                        
                        std::string polygonValue = Value(pObjPolygon).asString();
                        Vec2 offset = PointFromString(polygonValue);
                        vertices[vindex].x = (offset.x / m_ptmRatio) ; 
                        vertices[vindex].y = (offset.y / m_ptmRatio) ; 
                        vindex++;
                    }
                    
                    polyshape->Set(vertices, vindex);
                    fix->fixture.shape = polyshape;
                    
                    // create a list
                    *nextFixtureDef = fix;
                    nextFixtureDef = &(fix->next);
                }
                
            } 
            else if (fixtureType == "CIRCLE") 
            {
                FixtureDef *fix = new FixtureDef();
                fix->fixture = basicData; // copy basic data
                fix->callbackData = callbackData;
                
                ValueMap circleData = fixtureData["circle"].asValueMap();
                
                b2CircleShape *circleShape = new b2CircleShape();
                
                circleShape->m_radius = Value(circleData["radius"]).asFloat() / m_ptmRatio;
                Vec2 p = PointFromString(Value(circleData["position"]).asString());
                circleShape->m_p = b2Vec2(p.x / m_ptmRatio, p.y / m_ptmRatio);
                fix->fixture.shape = circleShape;
                
                // create a list
                *nextFixtureDef = fix;
                nextFixtureDef = &(fix->next);

            } 
            else 
            {
                CCAssert(0, "Unknown fixtureType");
            }
            
            // add the body element to the hash
            m_shapeObjects[bodyName] = bodyDef;
        }
    }
}

float PhysicsLoader::getPtmRatio() const
{
    return m_ptmRatio;
}

