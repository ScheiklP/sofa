/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, version 1.0 beta 4      *
*                (c) 2006-2009 MGH, INRIA, USTL, UJF, CNRS                    *
*                                                                             *
* This library is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This library is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this library; if not, write to the Free Software Foundation,     *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.          *
*******************************************************************************
*                              SOFA :: Framework                              *
*                                                                             *
* Authors: M. Adam, J. Allard, B. Andre, P-J. Bensoussan, S. Cotin, C. Duriez,*
* H. Delingette, F. Falipou, F. Faure, S. Fonteneau, L. Heigeas, C. Mendoza,  *
* M. Nesme, P. Neumann, J-P. de la Plata Alcade, F. Poyer and F. Roy          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#ifndef SOFA_CORE_OBJECTMODEL_BASENODE_H
#define SOFA_CORE_OBJECTMODEL_BASENODE_H

#include "BaseContext.h"
#include "BaseObject.h"

namespace sofa
{

namespace core
{

// forward declaration of classes accessible from the node
namespace behavior
{
class BaseAnimationLoop;
class OdeSolver;
}
namespace collision
{
class Pipeline;
}
namespace visual
{
class VisualLoop;
}

namespace objectmodel
{

/**
 *  \brief Base class for simulation nodes.
 *
 *  A Node is a class defining the main scene data structure of a simulation.
 *  It defined hierarchical relations between elements.
 *  Each node can have parent and child nodes (potentially defining a tree),
 *  as well as attached objects (the leaves of the tree).
 *
 * \author Jeremie Allard
 */
class SOFA_CORE_API BaseNode : public virtual Base
{
public:
    SOFA_ABSTRACT_CLASS(BaseNode, Base);

protected:
    BaseNode();
    virtual ~BaseNode();
public:
    /// @name Scene hierarchy
    /// @{

    typedef sofa::helper::vector< BaseNode* > Children;
    /// Get a list of child node
    virtual const Children getChildren() const = 0;

    /// Add a child node
    virtual void addChild(BaseNode::SPtr node) = 0;

    /// Remove a child node
    virtual void removeChild(BaseNode::SPtr node) = 0;

    /// Move a node from another node
    virtual void moveChild(BaseNode::SPtr node) = 0;

    /// Add a generic object
    virtual bool addObject(BaseObject::SPtr obj) = 0;

    /// Remove a generic object
    virtual bool removeObject(BaseObject::SPtr obj) = 0;

    /// Move an object from a node to another node
    virtual void moveObject(BaseObject::SPtr obj) = 0;

    /// Test if the given node is a parent of this node.
    virtual bool hasParent(const BaseNode* node) const = 0;

    /// Test if the given node is an ancestor of this node.
    /// An ancestor is a parent or (recursively) the parent of an ancestor.
    virtual bool hasAncestor(const BaseNode* node) const = 0;

    /// Remove the current node from the graph: depending on the type of Node, it can have one or several parents.
    virtual void detachFromGraph() = 0;

    /// Get this node context
    virtual BaseContext* getContext() = 0;

    /// Get this node context
    virtual const BaseContext* getContext() const = 0;

    /// Return the full path name of this node
    virtual std::string getPathName() const=0;

    /// @}

    /// @name Solvers and main algorithms
    /// @{

    virtual core::behavior::BaseAnimationLoop* getAnimationLoop() const;
    virtual core::behavior::OdeSolver* getOdeSolver() const;
    virtual core::collision::Pipeline* getCollisionPipeline() const;
    virtual core::visual::VisualLoop* getVisualLoop() const;

    /// @}

};

} // namespace objectmodel

} // namespace core

} // namespace sofa

#endif
