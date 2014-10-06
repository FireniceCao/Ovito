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

#ifndef __OVITO_COORDINATE_TRIPOD_OVERLAY_H
#define __OVITO_COORDINATE_TRIPOD_OVERLAY_H

#include <core/Core.h>
#include <core/gui/properties/PropertiesEditor.h>
#include "ViewportOverlay.h"

namespace Ovito {

/**
 * \brief A viewport overlay that displays the coordinate system orientation.
 */
class OVITO_CORE_EXPORT CoordinateTripodOverlay : public ViewportOverlay
{
public:

	/// \brief Constructor.
	Q_INVOKABLE CoordinateTripodOverlay(DataSet* dataset);

	/// \brief This method asks the overlay to paint its contents over the given viewport.
	virtual void render(Viewport* viewport, QPainter& painter, const ViewProjectionParameters& projParams, RenderSettings* renderSettings) override;

private:

	/// The corner the viewport.
	PropertyField<int> _alignment;

	/// Controls the display of the first axis.
	PropertyField<bool> _axis1Enabled;

	/// Controls the display of the second axis.
	PropertyField<bool> _axis2Enabled;

	/// Controls the display of the thrid axis.
	PropertyField<bool> _axis3Enabled;

	/// Controls the display of the fourth axis.
	PropertyField<bool> _axis4Enabled;

	/// The label of the first axis.
	PropertyField<QString> _axis1Label;

	/// The label of the second axis.
	PropertyField<QString> _axis2Label;

	/// The label of the third axis.
	PropertyField<QString> _axis3Label;

	/// The label of the fourth axis.
	PropertyField<QString> _axis4Label;

	/// The direction of the first axis.
	PropertyField<Vector3> _axis1Dir;

	/// The direction of the second axis.
	PropertyField<Vector3> _axis2Dir;

	/// The direction of the third axis.
	PropertyField<Vector3> _axis3Dir;

	/// The direction of the fourth axis.
	PropertyField<Vector3> _axis4Dir;

	/// The display color of the first axis.
	PropertyField<Color, QColor> _axis1Color;

	/// The display color of the second axis.
	PropertyField<Color, QColor> _axis2Color;

	/// The display color of the third axis.
	PropertyField<Color, QColor> _axis3Color;

	/// The display color of the fourth axis.
	PropertyField<Color, QColor> _axis4Color;

	DECLARE_PROPERTY_FIELD(_alignment);
	DECLARE_PROPERTY_FIELD(_axis1Enabled);
	DECLARE_PROPERTY_FIELD(_axis2Enabled);
	DECLARE_PROPERTY_FIELD(_axis3Enabled);
	DECLARE_PROPERTY_FIELD(_axis4Enabled);
	DECLARE_PROPERTY_FIELD(_axis1Label);
	DECLARE_PROPERTY_FIELD(_axis2Label);
	DECLARE_PROPERTY_FIELD(_axis3Label);
	DECLARE_PROPERTY_FIELD(_axis4Label);
	DECLARE_PROPERTY_FIELD(_axis1Dir);
	DECLARE_PROPERTY_FIELD(_axis2Dir);
	DECLARE_PROPERTY_FIELD(_axis3Dir);
	DECLARE_PROPERTY_FIELD(_axis4Dir);
	DECLARE_PROPERTY_FIELD(_axis1Color);
	DECLARE_PROPERTY_FIELD(_axis2Color);
	DECLARE_PROPERTY_FIELD(_axis3Color);
	DECLARE_PROPERTY_FIELD(_axis4Color);

	Q_CLASSINFO("DisplayName", "Coordinate tripod");

	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief A properties editor for the CoordinateTripodOverlay class.
 */
class CoordinateTripodOverlayEditor : public PropertiesEditor
{
public:

	/// Constructor.
	Q_INVOKABLE CoordinateTripodOverlayEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

private:

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_COORDINATE_TRIPOD_OVERLAY_H