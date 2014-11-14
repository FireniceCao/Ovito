///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
//
//  This file is part of OVITO (Open Visualization Tool).
//
//  OVITO is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  OVITO is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////

/**
 * \file IdentifyDiamondModifier.h
 * \brief Contains the definition of the Particles::IdentifyDiamondModifier class.
 */

#ifndef __OVITO_IDENTIFY_DIAMOND_MODIFIER_H
#define __OVITO_IDENTIFY_DIAMOND_MODIFIER_H

#include <plugins/particles/Particles.h>
#include <plugins/particles/modifier/analysis/StructureIdentificationModifier.h>

namespace Particles {

using namespace Ovito;

/**
 * \brief A modifier that identifies local diamond structures.
 */
class OVITO_PARTICLES_EXPORT IdentifyDiamondModifier : public StructureIdentificationModifier
{
public:

	/// The structure types recognized by the modifier.
	enum StructureType {
		OTHER = 0,					//< Unidentified structure
		CUBIC_DIAMOND,				//< Cubic diamond structure
		CUBIC_DIAMOND_FIRST_NEIGH,	//< First neighbor of a cubic diamond atom
		CUBIC_DIAMOND_SECOND_NEIGH,	//< Second neighbor of a cubic diamond atom
		HEX_DIAMOND,				//< Hexagonal diamond structure
		HEX_DIAMOND_FIRST_NEIGH,	//< First neighbor of a hexagonal diamond atom
		HEX_DIAMOND_SECOND_NEIGH,	//< Second neighbor of a hexagonal diamond atom

		NUM_STRUCTURE_TYPES 	//< This just counts the number of defined structure types.
	};
	Q_ENUMS(StructureType);

	/// Analysis engine that performs the structure identification
	class Engine : public StructureIdentificationModifier::StructureIdentificationEngine
	{
	public:

		/// Constructor.
		Engine(ParticleProperty* positions, const SimulationCellData& simCell) :
			StructureIdentificationModifier::StructureIdentificationEngine(positions, simCell) {}

		/// Computes the modifier's results and stores them in this object for later retrieval.
		virtual void compute(FutureInterfaceBase& futureInterface) override;
	};

public:

	/// Constructor.
	Q_INVOKABLE IdentifyDiamondModifier(DataSet* dataset);

protected:

	/// Creates and initializes a computation engine that will compute the modifier's results.
	virtual std::shared_ptr<AsynchronousParticleModifier::Engine> createEngine(TimePoint time, TimeInterval& validityInterval) override;

private:

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Identify diamond structure");
	Q_CLASSINFO("ModifierCategory", "Analysis");
};

/**
 * \brief A properties editor for the IdentifyDiamondModifier class.
 */
class IdentifyDiamondModifierEditor : public ParticleModifierEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE IdentifyDiamondModifierEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

private:

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __OVITO_IDENTIFY_DIAMOND_MODIFIER_H