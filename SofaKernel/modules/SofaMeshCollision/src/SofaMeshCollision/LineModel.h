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
#pragma once
#include <SofaMeshCollision/config.h>

#include <sofa/core/CollisionModel.h>
#include <sofa/core/objectmodel/BaseObject.h>
#include <SofaBaseMechanics/MechanicalObject.h>
#include <sofa/core/topology/BaseMeshTopology.h>
#include <sofa/defaulttype/VecTypes.h>

namespace sofa::component::collision
{

template<class DataTypes>
class LineCollisionModel;

class LineLocalMinDistanceFilter;

template<class DataTypes>
class PointCollisionModel;

template<class TDataTypes>
class TLine : public core::TCollisionElementIterator<LineCollisionModel<TDataTypes> >
{
public:
    typedef TDataTypes DataTypes;
    typedef typename DataTypes::Coord Coord;
    typedef typename DataTypes::Deriv Deriv;
    typedef LineCollisionModel<DataTypes> ParentModel;

    TLine(ParentModel* model, Index index);
    TLine() {}

    explicit TLine(const core::CollisionElementIterator& i);

    Index i1() const;
    Index i2() const;
    int flags() const;

    const Coord& p1() const;
    const Coord& p2() const;
    const Coord& p(Index i) const;

    const Coord& p1Free() const;
    const Coord& p2Free() const;

    const Deriv& v1() const;
    const Deriv& v2() const;

    Deriv n() const;

    /// Return true if the element stores a free position vector
    bool hasFreePosition() const;
};

template<class TDataTypes>
class SOFA_SOFAMESHCOLLISION_API LineCollisionModel : public core::CollisionModel
{
public :
    SOFA_CLASS(SOFA_TEMPLATE(LineCollisionModel, TDataTypes), core::CollisionModel);
    
    enum LineFlag
    {
        FLAG_P1  = 1<<0, ///< Point 1  is attached to this line
        FLAG_P2  = 1<<1, ///< Point 2  is attached to this line
        FLAG_BP1 = 1<<2, ///< Point 1  is attached to this line and is a boundary
        FLAG_BP2 = 1<<3, ///< Point 2  is attached to this line and is a boundary
        FLAG_POINTS  = FLAG_P1|FLAG_P2,
        FLAG_BPOINTS = FLAG_BP1|FLAG_BP2,
    };

protected:
    struct LineData
    {
        Index p[2];
        // Triangles neighborhood
//		int tRight, tLeft;
    };

    sofa::helper::vector<LineData> elems;
    bool needsUpdate;
    virtual void updateFromTopology();

    LineCollisionModel();

public:
    typedef TDataTypes DataTypes;
    typedef DataTypes InDataTypes;
    typedef LineCollisionModel<DataTypes> ParentModel;
    typedef typename DataTypes::VecCoord VecCoord;
    typedef typename DataTypes::VecDeriv VecDeriv;
    typedef typename DataTypes::Coord Coord;
    typedef typename DataTypes::Deriv Deriv;
    typedef TLine<DataTypes> Element;
    friend class TLine<DataTypes>;

    void init() override;

    // -- CollisionModel interface

    void resize(Size size) override;

    void computeBoundingTree(int maxDepth=0) override;

    void computeContinuousBoundingTree(double dt, int maxDepth=0) override;

    void draw(const core::visual::VisualParams* vparams) override;

    void handleTopologyChange() override;

    bool canCollideWithElement(Index index, CollisionModel* model2, Index index2) override;

    core::behavior::MechanicalState<DataTypes>* getMechanicalState() { return mstate; }

    Deriv velocity(Index index)const;

    LineLocalMinDistanceFilter *getFilter() const;

    virtual Index getElemEdgeIndex(Index index) const { return index; }
    
    int getLineFlags(Index i);

    void setFilter(LineLocalMinDistanceFilter * /*lmdFilter*/);

    Data<bool> bothSide; ///< to activate collision on both-side of the both side of the line model (when surface normals are defined on these lines)

    /// Pre-construction check method called by ObjectFactory.
    /// Check that DataTypes matches the MechanicalState.
    template<class T>
    static bool canCreate(T*& obj, core::objectmodel::BaseContext* context, core::objectmodel::BaseObjectDescription* arg)
    {
        if (dynamic_cast<core::behavior::MechanicalState<DataTypes>*>(context->getMechanicalState()) == nullptr)
        {
            arg->logError(std::string("No mechanical state with the datatype '") + DataTypes::Name() +
                          "' found in the context node.");
            return false;
        }
        return BaseObject::canCreate(obj, context, arg);
    }

    sofa::core::topology::BaseMeshTopology* getCollisionTopology() override
    {
        return l_topology.get();
    }

    void computeBBox(const core::ExecParams* params, bool onlyVisible) override;

    Data<bool> m_displayFreePosition; ///< Display Collision Model Points free position(in green)

    /// Link to be set to the topology container in the component graph.
    SingleLink<LineCollisionModel<DataTypes>, sofa::core::topology::BaseMeshTopology, BaseLink::FLAG_STOREPATH | BaseLink::FLAG_STRONGLINK> l_topology;
    
protected:
    core::behavior::MechanicalState<DataTypes>* mstate;
    Topology* topology;
    PointCollisionModel<sofa::defaulttype::Vec3Types>* mpoints;
    int meshRevision;
    LineLocalMinDistanceFilter *m_lmdFilter;

};


template<class DataTypes>
inline TLine<DataTypes>::TLine(ParentModel* model, Index index)
    : core::TCollisionElementIterator<ParentModel>(model, index)
{
}

template<class DataTypes>
inline TLine<DataTypes>::TLine(const core::CollisionElementIterator& i)
    : core::TCollisionElementIterator<ParentModel>(static_cast<ParentModel*>(i.getCollisionModel()), i.getIndex())
{
}

template<class DataTypes>
inline sofa::Index TLine<DataTypes>::i1() const { return this->model->elems[this->index].p[0]; }

template<class DataTypes>
inline sofa::Index TLine<DataTypes>::i2() const { return this->model->elems[this->index].p[1]; }

template<class DataTypes>
inline const typename DataTypes::Coord& TLine<DataTypes>::p1() const { return this->model->mstate->read(core::ConstVecCoordId::position())->getValue()[this->model->elems[this->index].p[0]]; }

template<class DataTypes>
inline const typename DataTypes::Coord& TLine<DataTypes>::p2() const { return this->model->mstate->read(core::ConstVecCoordId::position())->getValue()[this->model->elems[this->index].p[1]]; }

template<class DataTypes>
inline const typename DataTypes::Coord& TLine<DataTypes>::p(Index i) const {
    return this->model->mstate->read(core::ConstVecCoordId::position())->getValue()[this->model->elems[this->index].p[i]];
}

template<class DataTypes>
inline const typename DataTypes::Coord& TLine<DataTypes>::p1Free() const
{
    if (hasFreePosition())
        return this->model->mstate->read(core::ConstVecCoordId::freePosition())->getValue()[this->model->elems[this->index].p[0]];
    else
        return p1();
}

template<class DataTypes>
inline const typename DataTypes::Coord& TLine<DataTypes>::p2Free() const
{
    if (hasFreePosition())
        return this->model->mstate->read(core::ConstVecCoordId::freePosition())->getValue()[this->model->elems[this->index].p[1]];
    else
        return p2();
}

template<class DataTypes>
inline const typename DataTypes::Deriv& TLine<DataTypes>::v1() const { return this->model->mstate->read(core::ConstVecDerivId::velocity())->getValue()[this->model->elems[this->index].p[0]]; }

template<class DataTypes>
inline const typename DataTypes::Deriv& TLine<DataTypes>::v2() const { return this->model->mstate->read(core::ConstVecDerivId::velocity())->getValue()[this->model->elems[this->index].p[1]]; }

template<class DataTypes>
inline typename DataTypes::Deriv TLine<DataTypes>::n() const {return (this->model->mpoints->getNormal(this->i1()) + this->model->mpoints->getNormal( this->i2())).normalized();}

template<class DataTypes>
inline typename LineCollisionModel<DataTypes>::Deriv LineCollisionModel<DataTypes>::velocity(Index index) const { return (mstate->read(core::ConstVecDerivId::velocity())->getValue()[elems[index].p[0]] + mstate->read(core::ConstVecDerivId::velocity())->getValue()[elems[index].p[1]])/((Real)(2.0)); }

template<class DataTypes>
inline int TLine<DataTypes>::flags() const { return this->model->getLineFlags(this->index); }

template<class DataTypes>
inline bool TLine<DataTypes>::hasFreePosition() const { return this->model->mstate->read(core::ConstVecCoordId::freePosition())->isSet(); }

template <class TDataTypes> using TLineModel [[deprecated("The TLineModel is now deprecated, please use LineCollisionModel instead. Compatibility stops at v20.06")]] = LineCollisionModel<TDataTypes>;
using  LineModel [[deprecated("The LineModel is now deprecated, please use LineCollisionModel<sofa::defaulttype::Vec3Types> instead. Compatibility stops at v20.06")]] = LineCollisionModel<sofa::defaulttype::Vec3Types>;
using Line = TLine<sofa::defaulttype::Vec3Types>;

#if  !defined(SOFA_COMPONENT_COLLISION_LINECOLLISIONMODEL_CPP)
extern template class SOFA_SOFAMESHCOLLISION_API LineCollisionModel<defaulttype::Vec3Types>;

#endif

} //namespace sofa::component::collision