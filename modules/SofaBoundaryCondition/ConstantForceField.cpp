/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, development version     *
*                (c) 2006-2018 INRIA, USTL, UJF, CNRS, MGH                    *
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
#define SOFA_COMPONENT_FORCEFIELD_CONSTANTFORCEFIELD_CPP

#include <SofaBoundaryCondition/ConstantForceField.inl>
#include <sofa/core/ObjectFactory.h>

#include <sofa/defaulttype/VecTypes.h>
#include <sofa/defaulttype/RigidTypes.h>

namespace sofa
{

namespace component
{

namespace forcefield
{

using namespace sofa::defaulttype;

template <> SOFA_BOUNDARY_CONDITION_API
SReal ConstantForceField<defaulttype::Rigid3Types>::getPotentialEnergy(const core::MechanicalParams*, const DataVecCoord& ) const { return 0; }
template <> SOFA_BOUNDARY_CONDITION_API
SReal ConstantForceField<defaulttype::Rigid2Types>::getPotentialEnergy(const core::MechanicalParams*, const DataVecCoord& ) const { return 0; }



int ConstantForceFieldClass = core::RegisterObject("Constant forces applied to given degrees of freedom")
        .add< ConstantForceField<Vec3Types> >()
        .add< ConstantForceField<Vec2Types> >()
        .add< ConstantForceField<Vec1Types> >()
        .add< ConstantForceField<Vec6Types> >()
        .add< ConstantForceField<Rigid3Types> >()
        .add< ConstantForceField<Rigid2Types> >()

        ;
template class SOFA_BOUNDARY_CONDITION_API ConstantForceField<Vec3Types>;
template class SOFA_BOUNDARY_CONDITION_API ConstantForceField<Vec2Types>;
template class SOFA_BOUNDARY_CONDITION_API ConstantForceField<Vec1Types>;
template class SOFA_BOUNDARY_CONDITION_API ConstantForceField<Vec6Types>;
template class SOFA_BOUNDARY_CONDITION_API ConstantForceField<Rigid3Types>;
template class SOFA_BOUNDARY_CONDITION_API ConstantForceField<Rigid2Types>;


} // namespace forcefield

} // namespace component

} // namespace sofa
