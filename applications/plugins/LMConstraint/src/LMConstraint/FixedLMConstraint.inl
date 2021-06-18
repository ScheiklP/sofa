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
#include <LMConstraint/FixedLMConstraint.h>
#include <sofa/core/visual/VisualParams.h>
#include <SofaBaseTopology/TopologySubsetData.inl>
#include <sofa/type/vector_algorithm.h>


namespace sofa::component::constraintset
{


// Define RemovalFunction
template< class DataTypes>
void FixedLMConstraint<DataTypes>::FCPointHandler::applyDestroyFunction(Index pointIndex, value_type &)
{
    if (fc)
    {
        fc->removeConstraint((Index) pointIndex);
    }
    return;
}

template <class DataTypes>
FixedLMConstraint<DataTypes>::FixedLMConstraint(MechanicalState *dof)
    : core::behavior::LMConstraint<DataTypes,DataTypes>(dof,dof)
    , f_indices(core::objectmodel::Base::initData(&f_indices, "indices", "List of the index of particles to be fixed"))
    , _drawSize(core::objectmodel::Base::initData(&_drawSize,0.0,"drawSize","0 -> point based rendering, >0 -> radius of spheres") )
    , l_topology(initLink("topology", "link to the topology container"))
    , m_pointHandler(nullptr)
{
    
}

template <class DataTypes>
void FixedLMConstraint<DataTypes>::clearConstraints()
{
    f_indices.beginEdit()->clear();
    f_indices.endEdit();
}

template <class DataTypes>
void FixedLMConstraint<DataTypes>::addConstraint(Index index)
{
    f_indices.beginEdit()->push_back(index);
    f_indices.endEdit();
}

template <class DataTypes>
void FixedLMConstraint<DataTypes>::removeConstraint(Index index)
{
    sofa::type::removeValue(*f_indices.beginEdit(),index);
    f_indices.endEdit();
}


template <class DataTypes>
void FixedLMConstraint<DataTypes>::initFixedPosition()
{
    this->restPosition.clear();
    const VecCoord& x =this->constrainedObject1->read(core::ConstVecCoordId::position())->getValue();
    const SetIndexArray & indices = this->f_indices.getValue();
    for (SetIndexArray::const_iterator it = indices.begin(); it != indices.end(); ++it)
    {
        Index index=*it;
        this->restPosition.insert(std::make_pair(index, x[index]));
    }
}

template <class DataTypes>
void FixedLMConstraint<DataTypes>::init()
{
    core::behavior::LMConstraint<DataTypes,DataTypes>::init();

    if (l_topology.empty())
    {
        msg_info() << "link to Topology container should be set to ensure right behavior. First Topology found in current context will be used.";
        l_topology.set(this->getContext()->getMeshTopologyLink());
    }

    sofa::core::topology::BaseMeshTopology* _topology = l_topology.get();

    if (_topology)
    {
        msg_info() << "Topology path used: '" << l_topology.getLinkedPath() << "'";

        // Initialize functions and parameters
        m_pointHandler = new FCPointHandler(this, &f_indices);
        f_indices.createTopologyHandler(_topology, m_pointHandler);
    }
    else
    {
        msg_info() << "No topology component found at path: " << l_topology.getLinkedPath() << ", nor in current context: " << this->getContext()->name;
    }


    X[0]=1; X[1]=0; X[2]=0;
    Y[0]=0; Y[1]=1; Y[2]=0;
    Z[0]=0; Z[1]=0; Z[2]=1;

    initFixedPosition();

}



template<class DataTypes>
void FixedLMConstraint<DataTypes>::buildConstraintMatrix(const core::ConstraintParams* /* cParams*/, core::MultiMatrixDerivId cId, unsigned int &cIndex)
{
    using namespace core::objectmodel;
    Data<MatrixDeriv>* dC = cId[this->constrainedObject1].write();
    helper::WriteAccessor<Data<MatrixDeriv> > c = *dC;
    idxX.clear();
    idxY.clear();
    idxZ.clear();
    const SetIndexArray &indices = f_indices.getValue();

    for (SetIndexArray::const_iterator it = indices.begin(); it != indices.end(); ++it)
    {
        const Index index=*it;

        //Constraint degree of freedom along X direction
        c->writeLine(cIndex).addCol(index,X);
        idxX.push_back(cIndex++);

        //Constraint degree of freedom along X direction
        c->writeLine(cIndex).addCol(index,Y);
        idxY.push_back(cIndex++);

        //Constraint degree of freedom along Z direction
        c->writeLine(cIndex).addCol(index,Z);
        idxZ.push_back(cIndex++);

        this->constrainedObject1->forceMask.insertEntry(index);
    }
}

template<class DataTypes>
void FixedLMConstraint<DataTypes>::writeConstraintEquations(unsigned int& lineNumber, core::MultiVecId id, ConstOrder Order)
{
    using namespace core;
    using namespace core::objectmodel;
    const SetIndexArray & indices = f_indices.getValue();

    unsigned int counter=0;
    for (SetIndexArray::const_iterator it = indices.begin(); it != indices.end(); ++it,++counter)
    {
        const Index index = *it;

        core::behavior::ConstraintGroup *constraint = this->addGroupConstraint(Order);
        SReal correctionX=0,correctionY=0,correctionZ=0;
        switch(Order)
        {
        case core::ConstraintParams::ACC :
        case core::ConstraintParams::VEL :
        {
            ConstVecId v1 = id.getId(this->constrainedObject1);
            correctionX = this->constrainedObject1->getConstraintJacobianTimesVecDeriv(idxX[counter],v1);
            correctionY = this->constrainedObject1->getConstraintJacobianTimesVecDeriv(idxY[counter],v1);
            correctionZ = this->constrainedObject1->getConstraintJacobianTimesVecDeriv(idxZ[counter],v1);
            break;
        }
        case core::ConstraintParams::POS :
        case core::ConstraintParams::POS_AND_VEL :
        {
            ConstVecId xid = id.getId(this->constrainedObject1);
            helper::ReadAccessor<Data<VecCoord> > x = *this->constrainedObject1->read((ConstVecCoordId)xid);

            //If a new particle has to be fixed, we add its current position as rest position
            if (restPosition.find(index) == this->restPosition.end())
            {
                restPosition.insert(std::make_pair(index, x[index]));
            }


            Coord v=x[index]-restPosition[index];
            correctionX=v[0];
            correctionY=v[1];
            correctionZ=v[2];
            break;
        }
        };

        constraint->addConstraint( lineNumber, idxX[counter], -correctionX);
        constraint->addConstraint( lineNumber, idxY[counter], -correctionY);
        constraint->addConstraint( lineNumber, idxZ[counter], -correctionZ);
    }
}



template <class DataTypes>
void FixedLMConstraint<DataTypes>::draw(const core::visual::VisualParams* vparams)
{
    if (!vparams->displayFlags().getShowBehaviorModels()) return;
    const VecCoord& x =this->constrainedObject1->read(core::ConstVecCoordId::position())->getValue();
    const SetIndexArray & indices = f_indices.getValue();

    std::vector< type::Vector3 > points;
    type::Vector3 point;
    for (unsigned int index : indices)
    {
        point = DataTypes::getCPos(x[index]);
        points.push_back(point);
    }
    if( _drawSize.getValue() == 0) // old classical drawing by points
    {
        vparams->drawTool()->drawPoints(points, 10, sofa::type::RGBAColor(1,0.5,0.5,1));
    }
    else
    {
        vparams->drawTool()->drawSpheres(points, (float)_drawSize.getValue(), sofa::type::RGBAColor(1.0f,0.35f,0.35f,1.0f));
    }
}

} //namespace sofa::component::constraintset
