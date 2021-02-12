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
#include <SofaBaseTopology/config.h>

#include <SofaBaseTopology/MeshTopology.h>
#include <SofaBaseTopology/RegularGridTopology.h>
#include <sofa/helper/MarchingCubeUtility.h>
#include <sofa/defaulttype/Vec.h>

#include <sofa/helper/io/Mesh.h>
#include <stack>
#include <string>

namespace sofa::core::loader
{
    class VoxelLoader;
}

namespace sofa::component::topology
{

class RegularGridTopology;

/** A sparse grid topology. Like a sparse FFD building from the bounding box of the object. Starting from a RegularGrid, only valid cells containing matter (ie intersecting the original surface mesh or totally inside the object) are considered.
 * Valid cells are tagged by a Type BOUNDARY or INSIDE
 * WARNING: the corresponding node in the XML file has to be placed BEFORE the MechanicalObject node, in order to excute its init() before the MechanicalObject one in order to be able to give dofs
 */
class SOFA_SOFABASETOPOLOGY_API SparseGridTopology : public MeshTopology
{
public:
    SOFA_CLASS(SparseGridTopology,MeshTopology);
    typedef sofa::defaulttype::Vector3 Vector3;
    typedef sofa::defaulttype::Vec3i   Vec3i;
    typedef sofa::helper::fixed_array<Vector3,8> CubeCorners;
    typedef sofa::defaulttype::BoundingBox BoundingBox;
    typedef enum {OUTSIDE,INSIDE,BOUNDARY} Type; ///< each cube has a type depending on its filling ratio
protected:
    SparseGridTopology(bool _isVirtual=false);

    /// Define using the resolution and the spatial size. The resolution corresponds to the number of points if all the cells were filled.
    SparseGridTopology(Vec3i numVertices, BoundingBox box, bool _isVirtual=false);
public:
    static const float WEIGHT27[8][27];
    static const Index cornerIndicesFromFineToCoarse[8][8];

    void init() override;

    /// building from a mesh file
    virtual void buildAsFinest();

    /// building by condensating a finer sparse grid (used if setFinerSparseGrid has initializated _finerSparseGrid before calling init() )
    virtual void buildFromFiner();

    /// building eventual virtual finer levels (cf _nbVirtualFinerLevels)
    virtual void buildVirtualFinerLevels();

    typedef std::map<Vector3, Index> MapBetweenCornerPositionAndIndice;///< a vertex indice for a given vertex position in space

    /// connexion between several coarsened levels
    typedef std::vector<helper::fixed_array<Index,8> > HierarchicalCubeMap; ///< a cube indice -> corresponding 8 child indices on the potential _finerSparseGrid
    HierarchicalCubeMap _hierarchicalCubeMap;
    typedef helper::vector<Index> InverseHierarchicalCubeMap; ///< a fine cube indice -> corresponding coarser cube indice
    InverseHierarchicalCubeMap _inverseHierarchicalCubeMap;

    typedef std::map<Index,float> AHierarchicalPointMap;
    typedef helper::vector< AHierarchicalPointMap > HierarchicalPointMap; ///< a point indice -> corresponding 27 child indices on the potential _finerSparseGrid with corresponding weight
    HierarchicalPointMap _hierarchicalPointMap;
    typedef helper::vector< AHierarchicalPointMap > InverseHierarchicalPointMap; ///< a fine point indice -> corresponding some parent points for interpolation
    InverseHierarchicalPointMap _inverseHierarchicalPointMap;
    typedef helper::vector< Index > PointMap;
    PointMap _pointMap; ///< a coarse point indice -> corresponding point in finer level
    PointMap _inversePointMap;  ///< a fine point indice -> corresponding point in coarser level


    enum {UP,DOWN,RIGHT,LEFT,BEFORE,BEHIND,NUM_CONNECTED_NODES};
    typedef helper::vector< helper::fixed_array<Index,NUM_CONNECTED_NODES> > NodeAdjacency; ///< a node -> its 6 neighboors
    NodeAdjacency _nodeAdjacency;
    typedef helper::vector< helper::vector<Index> >NodeCubesAdjacency; ///< a node -> its 8 neighboor cells
    NodeCubesAdjacency _nodeCubesAdjacency;
    typedef helper::vector< helper::vector<Index> >NodeCornersAdjacency; ///< a node -> its 8 corners of neighboor cells
    NodeCornersAdjacency _nodeCornersAdjacency;


    helper::vector< SparseGridTopology::SPtr > _virtualFinerLevels; ///< saving the virtual levels (cf _nbVirtualFinerLevels)
    int getNbVirtualFinerLevels() const { return _nbVirtualFinerLevels.getValue();}
    void setNbVirtualFinerLevels(int n) {_nbVirtualFinerLevels.setValue(n);}


    /// Resolution
    sofa::defaulttype::Vec<3, int> getN() const { return n.getValue();}
    int getNx() const { return n.getValue()[0]; }
    int getNy() const { return n.getValue()[1]; }
    int getNz() const { return n.getValue()[2]; }

    void setN(Vec3i _n) {n.setValue(_n);}
    void setNx(int _n) { n.setValue(Vec3i(_n             ,n.getValue()[1],n.getValue()[2])); }
    void setNy(int _n) { n.setValue(Vec3i(n.getValue()[0],_n             ,n.getValue()[2])); }
    void setNz(int _n) { n.setValue(Vec3i(n.getValue()[0],n.getValue()[1],_n)             ); }

    void setMin(Vector3 val) {_min.setValue(val);}
    void setXmin(SReal val) { _min.setValue(Vector3(val             ,_min.getValue()[1],_min.getValue()[2])); }
    void setYmin(SReal val) { _min.setValue(Vector3(_min.getValue()[0],val             ,_min.getValue()[2])); }
    void setZmin(SReal val) { _min.setValue(Vector3(_min.getValue()[0],_min.getValue()[1],val)             ); }

    void setMax(Vector3 val) {_max.setValue(val);}
    void setXmax(SReal val) { _max.setValue(Vector3(val             ,_max.getValue()[1],_max.getValue()[2])); }
    void setYmax(SReal val) { _max.setValue(Vector3(_max.getValue()[0],val             ,_max.getValue()[2])); }
    void setZmax(SReal val) { _max.setValue(Vector3(_max.getValue()[0],_max.getValue()[1],val)             ); }

    Vector3 getMin() {return _min.getValue();}
    SReal getXmin() { return _min.getValue()[0]; }
    SReal getYmin() { return _min.getValue()[1]; }
    SReal getZmin() { return _min.getValue()[2]; }

    Vector3 getMax() {return _max.getValue();}
    SReal getXmax() { return _max.getValue()[0]; }
    SReal getYmax() { return _max.getValue()[1]; }
    SReal getZmax() { return _max.getValue()[2]; }

    bool hasPos()  const override { return true; }

    /// return the cube containing the given point (or -1 if not found),
    /// as well as deplacements from its first corner in terms of dx, dy, dz (i.e. barycentric coordinates).
    virtual Index findCube(const Vector3& pos, SReal& fx, SReal &fy, SReal &fz);

    /// return the cube containing the given point (or -1 if not found),
    /// as well as deplacements from its first corner in terms of dx, dy, dz (i.e. barycentric coordinates).
    virtual Index findNearestCube(const Vector3& pos, SReal& fx, SReal &fy, SReal &fz);

    /// return indices of 6 neighboor cubes
    virtual helper::fixed_array<Index,6> findneighboorCubes(Index indice );

    /// return the type of the i-th cube
    virtual Type getType(Index i );

    /// return the stiffness coefficient of the i-th cube
    virtual float getStiffnessCoef(Index elementIdx);
    /// return the mass coefficient of the i-th cube
    virtual float getMassCoef(Index elementIdx);

    SparseGridTopology *getFinerSparseGrid() const {return _finerSparseGrid;}
    void setFinerSparseGrid( SparseGridTopology *fsp ) {_finerSparseGrid=fsp;}
    SparseGridTopology *getCoarserSparseGrid() const {return _coarserSparseGrid;}
    void setCoarserSparseGrid( SparseGridTopology *csp ) {_coarserSparseGrid=csp;}

    void updateMesh();

    sofa::core::sptr<RegularGridTopology> _regularGrid; ///< based on a corresponding RegularGrid
    helper::vector< Index > _indicesOfRegularCubeInSparseGrid; ///< to redirect an indice of a cube in the regular grid to its indice in the sparse grid
    helper::vector< Index > _indicesOfCubeinRegularGrid; ///< to redirect an indice of a cube in the sparse grid to its indice in the regular grid

    Vector3 getPointPos(Index i ) { return Vector3( seqPoints.getValue()[i][0],seqPoints.getValue()[i][1],seqPoints.getValue()[i][2] ); }

    void getMesh( sofa::helper::io::Mesh &m);

    void setDimVoxels( int a, int b, int c) { dataResolution.setValue(Vec3i(a,b,c));}
    void setSizeVoxel( float a, float b, float c) { voxelSize.setValue(Vector3(a,b,c));}

    bool getVoxel(unsigned int x, unsigned int y, unsigned int z)
    {
        return getVoxel(dataResolution.getValue()[0]*dataResolution.getValue()[1]*z + dataResolution.getValue()[0]*y + x);
    }

    bool getVoxel(unsigned int index) const
    {
        return dataVoxels.getValue()[index]==1;
    }

    Data< helper::vector< unsigned char > >     dataVoxels;
    Data<bool> _fillWeighted; ///< is quantity of matter inside a cell taken into account?

    Data<bool> d_bOnlyInsideCells; ///< Select only inside cells (exclude boundary cells)


protected:
    bool isVirtual;
    /// cutting number in all directions
    Data< sofa::defaulttype::Vec< 3, int > > n;
    Data< Vector3 > _min; ///< Min
    Data< Vector3 > _max; ///< Max
    Data< SReal > _cellWidth; ///< if > 0 : dimension of each cell in the created grid
    Data< int > _nbVirtualFinerLevels; ///< create virtual (not in the animation tree) finer sparse grids in order to dispose of finest information (usefull to compute better mechanical properties for example)

public:
    Data< Vec3i >			dataResolution; ///< Dimension of the voxel File
    Data< Vector3 >         voxelSize; ///< Dimension of one voxel
    Data< unsigned int >    marchingCubeStep; ///< Step of the Marching Cube algorithm
    Data< unsigned int >    convolutionSize; ///< Dimension of the convolution kernel to smooth the voxels. 0 if no smoothing is required.

    Data< helper::vector < helper::vector <Index> > >facets; ///< Input mesh facets

    /** Create the data structure based on resolution, size and filling.
          \param numPoints  Number of points in the x,y,and z directions
          \param box  Volume occupied by the grid
          \param filling Voxel filling: true if the cell is defined, false if the cell is empty. Voxel order is: for(each z){ for(each y){ for(each x) }}}
          */
    void buildFromData( Vec3i numPoints, BoundingBox box, const helper::vector<bool>& filling );

protected:
    virtual void updateEdges();
    virtual void updateQuads();

    sofa::helper::MarchingCubeUtility                 marchingCubes;
    bool                                _usingMC;

    helper::vector<Type> _types; ///< BOUNDARY or FULL filled cells

    helper::vector< float > _stiffnessCoefs; ///< a stiffness coefficient per hexa (BOUNDARY=.5, FULL=1)
    helper::vector< float > _massCoefs; ///< a stiffness coefficient per hexa (BOUNDARY=.5, FULL=1)

    /// start from a seed cell (i,j,k) the OUTSIDE filling is propagated to neighboor cells until meet a BOUNDARY cell (this function is called from all border cells of the RegularGrid)
    void launchPropagationFromSeed(const Vec3i& point,
            sofa::core::sptr<RegularGridTopology> regularGrid,
            helper::vector<Type>& regularGrdidTypes,
            helper::vector<bool>& alreadyTested,
            std::stack<Vec3i>& seed) const;

    void propagateFrom(  const Vec3i& point,
            sofa::core::sptr<RegularGridTopology> regularGrid,
            helper::vector<Type>& regularGridTypes,
            helper::vector<bool>& alreadyTested,
            std::stack< sofa::defaulttype::Vec<3,int> > &seed) const;

    void computeBoundingBox(const helper::vector<Vector3>& vertices,
            SReal& xmin, SReal& xmax,
            SReal& ymin, SReal& ymax,
            SReal& zmin, SReal& zmax) const;

    void voxelizeTriangleMesh(helper::io::Mesh* mesh,
            sofa::core::sptr<RegularGridTopology> regularGrid,
            helper::vector<Type>& regularGridTypes) const;

    void buildFromTriangleMesh(sofa::helper::io::Mesh* mesh);

    void buildFromRegularGridTypes(sofa::core::sptr<RegularGridTopology> regularGrid, const helper::vector<Type>& regularGridTypes);



    /** Create a sparse grid from a .voxel file
    .voxel file format (ascii):
    512  // num voxels x
    512  // num voxels y
    246  // num voxels z
    0.7  // voxels size x [mm]
    0.7  // voxels size y [mm]
    2    // voxels size z [mm]
    0 0 255 0 0 ... // data
    */
    void buildFromVoxelFile(const std::string& filename);
    void buildFromRawVoxelFile(const std::string& filename);
    void buildFromVoxelLoader(sofa::core::loader::VoxelLoader * loader);

    template< class T>
    void constructCollisionModels(const sofa::helper::vector< sofa::core::topology::BaseMeshTopology * > &list_mesh,
            const helper::vector< Data< helper::vector< sofa::defaulttype::Vec<3,T> > >* >            &list_X) ;

    SparseGridTopology* _finerSparseGrid; ///< an eventual finer sparse grid that can be used to built this coarser sparse grid
    SparseGridTopology* _coarserSparseGrid; ///< an eventual coarser sparse grid

    void setVoxel(int index, unsigned char value)
    {
        if (value)
        {
            (*dataVoxels.beginEdit())[index] = 1;
        }
        else
        {
            (*dataVoxels.beginEdit())[index] = 0;
        }
        dataVoxels.beginEdit();
    };



    bool _alreadyInit;

public :


    const SeqHexahedra& getHexahedra() override
    {
        if( !_alreadyInit ) init();
        return sofa::component::topology::MeshTopology::getHexahedra();
    }

    Size getNbPoints() const override
    {
        if( !_alreadyInit ) const_cast<SparseGridTopology*>(this)->init();
        return sofa::component::topology::MeshTopology::getNbPoints();
    }

    /// TODO 2018-07-23 epernod: check why this method is override to return the same result as parent class.
    Size getNbHexahedra() override { return Size(this->getHexahedra().size());}
};

} //namespace sofa::component::topology