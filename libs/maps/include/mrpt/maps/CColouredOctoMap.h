/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */

#pragma once

#include <mrpt/maps/COctoMapBase.h>
#include <mrpt/obs/obs_frwds.h>

namespace octomap
{
class ColorOcTree;
}
namespace octomap
{
class ColorOcTreeNode;
}

namespace mrpt
{
namespace maps
{
/** A three-dimensional probabilistic occupancy grid, implemented as an
 * octo-tree with the "octomap" C++ library.
 *  This version stores both, occupancy information and RGB colour data at
 * each octree node. See the base class mrpt::maps::COctoMapBase.
 *
 * \sa CMetricMap, the example in "MRPT/samples/octomap_simple"
 * \ingroup mrpt_maps_grp
 */
class CColouredOctoMap
	: public COctoMapBase<octomap::ColorOcTree, octomap::ColorOcTreeNode>
{
	// This must be added to any CSerializable derived class:
	DEFINE_SERIALIZABLE(CColouredOctoMap, mrpt::maps)

   public:
	CColouredOctoMap(const double resolution = 0.10);  //!< Default constructor
	~CColouredOctoMap() override;  //!< Destructor

	/** This allows the user to select the desired method to update voxels
	   colour.
		SET = Set the colour of the voxel at (x,y,z) directly
		AVERAGE = Set the colour of the voxel at (x,y,z) as the mean of
	   its previous colour and the new observed one.
		INTEGRATE = Calculate the new colour of the voxel at (x,y,z) using
	   this formula: prev_color*node_prob +  new_color*(0.99-node_prob)
		If there isn't any previous color, any method is equivalent to
	   SET.
		INTEGRATE is the default option*/
	enum TColourUpdate
	{
		INTEGRATE = 0,
		SET,
		AVERAGE
	};

	/** Get the RGB colour of a point
	 * \return false if the point is not mapped, in which case the
	 * returned colour is undefined. */
	bool getPointColour(
		const float x, const float y, const float z, uint8_t& r, uint8_t& g,
		uint8_t& b) const;

	/** Manually update the colour of the voxel at (x,y,z) */
	void updateVoxelColour(
		const double x, const double y, const double z, const uint8_t r,
		const uint8_t g, const uint8_t b);

	/// Set the method used to update voxels colour
	void setVoxelColourMethod(TColourUpdate new_method)
	{
		m_colour_method = new_method;
	}

	/// Get the method used to update voxels colour
	TColourUpdate getVoxelColourMethod() { return m_colour_method; }
	void getAsOctoMapVoxels(
		mrpt::opengl::COctoMapVoxels& gl_obj) const override;

	MAP_DEFINITION_START(CColouredOctoMap)
	double resolution{
		0.10};	//!< The finest resolution of the octomap (default: 0.10
	//! meters)
	mrpt::maps::CColouredOctoMap::TInsertionOptions
		insertionOpts;	//!< Observations insertion options
	mrpt::maps::CColouredOctoMap::TLikelihoodOptions
		likelihoodOpts;	 //!< Probabilistic observation likelihood options
	MAP_DEFINITION_END(CColouredOctoMap)

	/** Returns true if the map is empty/no observation has been inserted */
	bool isEmpty() const override { return size() == 1; }
	/** @name Direct access to octomap library methods
	@{ */

	/** Just like insertPointCloud but with a single ray. */
	void insertRay(
		const float end_x, const float end_y, const float end_z,
		const float sensor_x, const float sensor_y, const float sensor_z);
	/** Manually updates the occupancy of the voxel at (x,y,z) as being occupied
	 * (true) or free (false), using the log-odds parameters in \a
	 * insertionOptions */
	void updateVoxel(
		const double x, const double y, const double z, bool occupied);
	/** Check whether the given point lies within the volume covered by the
	 * octomap (that is, whether it is "mapped") */
	bool isPointWithinOctoMap(
		const float x, const float y, const float z) const;
	double getResolution() const;
	unsigned int getTreeDepth() const;
	/// \return The number of nodes in the tree
	size_t size() const;
	/// \return Memory usage of the complete octree in bytes (may vary between
	/// architectures)
	size_t memoryUsage() const;
	/// \return Memory usage of the a single octree node
	size_t memoryUsageNode() const;
	/// \return Memory usage of a full grid of the same size as the OcTree in
	/// bytes (for comparison)
	size_t memoryFullGrid() const;
	double volume();
	/// Size of OcTree (all known space) in meters for x, y and z dimension
	void getMetricSize(double& x, double& y, double& z);
	/// Size of OcTree (all known space) in meters for x, y and z dimension
	void getMetricSize(double& x, double& y, double& z) const;
	/// minimum value of the bounding box of all known space in x, y, z
	void getMetricMin(double& x, double& y, double& z);
	/// minimum value of the bounding box of all known space in x, y, z
	void getMetricMin(double& x, double& y, double& z) const;
	/// maximum value of the bounding box of all known space in x, y, z
	void getMetricMax(double& x, double& y, double& z);
	/// maximum value of the bounding box of all known space in x, y, z
	void getMetricMax(double& x, double& y, double& z) const;
	/// Traverses the tree to calculate the total number of nodes
	size_t calcNumNodes() const;
	/// Traverses the tree to calculate the total number of leaf nodes
	size_t getNumLeafNodes() const;

	void setOccupancyThres(double prob) override;
	void setProbHit(double prob) override;
	void setProbMiss(double prob) override;
	void setClampingThresMin(double thresProb) override;
	void setClampingThresMax(double thresProb) override;
	double getOccupancyThres() const override;
	float getOccupancyThresLog() const override;
	double getProbHit() const override;
	float getProbHitLog() const override;
	double getProbMiss() const override;
	float getProbMissLog() const override;
	double getClampingThresMin() const override;
	float getClampingThresMinLog() const override;
	double getClampingThresMax() const override;
	float getClampingThresMaxLog() const override;
	/** @} */

   protected:
	void internal_clear() override;

	bool internal_insertObservation(
		const mrpt::obs::CObservation& obs,
		const std::optional<const mrpt::poses::CPose3D>& robotPose =
			std::nullopt) override;

	TColourUpdate m_colour_method{
		INTEGRATE};	 //! Method used to updated voxels colour.

};	// End of class def.
}  // namespace maps
}  // namespace mrpt
