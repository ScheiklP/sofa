/******************************************************************************
*                 SOFA, Simulation Open-Framework Architecture                *
*                    (c) 2006 INRIA, USTL, UJF, CNRS, MGH                     *
*                                                                             *
* This program is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this program. If not, see <http://www.gnu.org/licenses/>.        *
*******************************************************************************
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#include <SofaBaseCollision/ContactListener.h>

#include <sofa/core/collision/NarrowPhaseDetection.h>

#include <sofa/core/CollisionModel.h>
#include <sofa/core/ObjectFactory.h>
#include <sofa/simulation/CollisionBeginEvent.h>
#include <sofa/simulation/CollisionEndEvent.h>
#include <sofa/defaulttype/Vec.h>

#include <tuple>

namespace sofa::core::collision
{

int ContactListenerClass = core::RegisterObject("ContactListener .. ").add< ContactListener >();

ContactListener::ContactListener(  CollisionModel* collModel1 , CollisionModel* collModel2 )
    :  m_NarrowPhase(nullptr)
{
    m_CollisionModel1 = collModel1;
    m_CollisionModel2 = collModel2;
}

ContactListener::~ContactListener()
{
}

void ContactListener::init(void)
{
    m_NarrowPhase = getContext()->get<core::collision::NarrowPhaseDetection>();
    if ( m_NarrowPhase != nullptr )
    {
        // add to the event listening
        f_listening.setValue(true);
    }
}

void ContactListener::handleEvent( core::objectmodel::Event* _event )
{
    if (simulation::CollisionBeginEvent::checkEventType(_event))
    {
        m_ContactsVectorBuffer = m_ContactsVector;
        m_ContactsVector.clear();
    }

    else if (simulation::CollisionEndEvent::checkEventType(_event))
    {

        const NarrowPhaseDetection::DetectionOutputMap& detectionOutputsMap = m_NarrowPhase->getDetectionOutputs();

        if ( detectionOutputsMap.size() == 0 )
        {
            endContact(nullptr);
            return;
        }

        if  ( m_CollisionModel2 == nullptr )
        {
            //// check only one collision model
            for (core::collision::NarrowPhaseDetection::DetectionOutputMap::const_iterator it = detectionOutputsMap.begin(); it!=detectionOutputsMap.end(); ++it )
            {
                const CollisionModel* collMod1 = it->first.first;
                const CollisionModel* collMod2 = it->first.second;

                if ( m_CollisionModel1 == collMod1 || m_CollisionModel1 == collMod2 )
                {
                    if ( const helper::vector<DetectionOutput>* contacts = dynamic_cast<helper::vector<DetectionOutput>*>(it->second) )
                    {
                        m_ContactsVector.push_back( contacts );
                    }
                }
            }
        }
        else
        {
            // check both collision models
            for (core::collision::NarrowPhaseDetection::DetectionOutputMap::const_iterator it = detectionOutputsMap.begin(); it!=detectionOutputsMap.end(); ++it )
            {
                const CollisionModel* collMod1 = it->first.first;
                const CollisionModel* collMod2 = it->first.second;

                if ( (m_CollisionModel1==collMod1 && m_CollisionModel2==collMod2) || (m_CollisionModel1==collMod2 && m_CollisionModel2==collMod1) )
                {
                    if ( const helper::vector<DetectionOutput>* contacts = dynamic_cast<helper::vector<DetectionOutput>*>(it->second) )
                    {
                        m_ContactsVector.push_back( contacts );
                    }
                }
            }
        }
        beginContact(m_ContactsVector);
    }
}

unsigned int ContactListener::getNumberOfContacts() const {
    if (m_ContactsVectorBuffer.size() != 0){
        unsigned int numberOfContacts = m_ContactsVectorBuffer[0][0].size();
        if (0 < numberOfContacts && ((numberOfContacts <= m_CollisionModel1->getSize()) || (numberOfContacts <= m_CollisionModel2->getSize()))){
            return numberOfContacts;
        }
        else {
            return 0;
        }
    }
    else {
        return 0;
    }
}

helper::vector<double> ContactListener::getDistances() const {
    helper::vector<double> distances;
    unsigned int numberOfContacts = getNumberOfContacts();
    if (0 < numberOfContacts){ // can be 0
        distances.reserve(numberOfContacts);
        for (const auto& c: m_ContactsVectorBuffer[0][0]){
            distances.emplace_back(c.value);
        }
    }
    return distances;
}

std::vector<std::tuple<unsigned int, sofa::defaulttype::Vector3, unsigned int, sofa::defaulttype::Vector3>> ContactListener::getContactPoints() const {
    std::vector<std::tuple<unsigned int, sofa::defaulttype::Vector3, unsigned int, sofa::defaulttype::Vector3>> contactPoints;
    const unsigned int numberOfContacts = getNumberOfContacts();
    if (0 < numberOfContacts){ // can be 0
        contactPoints.reserve(numberOfContacts);
        for (const auto& c: m_ContactsVectorBuffer[0][0]){
            unsigned int firstID = m_CollisionModel1 != c.elem.first.getCollisionModel();
            unsigned int secondID = !firstID;
            contactPoints.emplace_back(firstID, c.point[0], secondID, c.point[1]);
        }
    }
    return contactPoints;
}

std::vector<std::tuple<unsigned int, unsigned int, unsigned int, unsigned int>> ContactListener::getContactElements() const {
    std::vector<std::tuple<unsigned int, unsigned int, unsigned int, unsigned int>> contactElements;
    const unsigned int numberOfContacts = getNumberOfContacts();
    if (0 < numberOfContacts){ // can be 0
        contactElements.reserve(numberOfContacts);
        for (const auto& c: m_ContactsVectorBuffer[0][0]){
            unsigned int firstID = m_CollisionModel1 != c.elem.first.getCollisionModel();
            unsigned int secondID = !firstID;
            contactElements.emplace_back(firstID, c.elem.first.getIndex(), secondID, c.elem.second.getIndex());
        }
    }
    return contactElements;
}

helper::vector<const helper::vector<DetectionOutput>* > ContactListener::getContactsVector() const{
    return m_ContactsVector;
}

} // namespace sofa::core::collision
