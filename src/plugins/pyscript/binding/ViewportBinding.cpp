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

#include <plugins/pyscript/PyScript.h>
#include <core/viewport/ViewportConfiguration.h>
#include <core/viewport/Viewport.h>
#include <core/scene/SceneNode.h>
#include "PythonBinding.h"

namespace PyScript {

using namespace boost::python;
using namespace Ovito;

BOOST_PYTHON_MODULE(PyScriptViewport)
{
	docstring_options docoptions(true, false);

	ovito_class<Viewport, RefTarget>(
			"A viewport window showing the contents of the three-dimensional scene.")
		.add_property("isRendering", &Viewport::isRendering)
		.add_property("isPerspective", &Viewport::isPerspectiveProjection, "A boolean flag indicating whether this viewport uses a perspective projection (read-only).")
		.add_property("viewType", &Viewport::viewType, (void (*)(Viewport&,Viewport::ViewType))([](Viewport& vp, Viewport::ViewType vt) { vp.setViewType(vt); }),
				"The projection type of the viewport."
				"\n\n"
				"Possible types are:"
				"\n\n"
				"  * ``ovito.view.ViewType.TOP``\n"
				"  * ``ovito.view.ViewType.BOTTOM``\n"
				"  * ``ovito.view.ViewType.FRONT``\n"
				"  * ``ovito.view.ViewType.BACK``\n"
				"  * ``ovito.view.ViewType.LEFT``\n"
				"  * ``ovito.view.ViewType.RIGHT``\n"
				"  * ``ovito.view.ViewType.ORTHO``\n"
				"  * ``ovito.view.ViewType.PERSPECTIVE``\n"
				"\n"
				"The last two types, ``ORTHO`` and ``PERSPECTIVE``, allow you to set up custom views of the scene. For example, to set up a camera with "
				"perspective projection that is looking from a point in space toward the origin::"
				"\n\n"
				"    vp = dataset.viewports.activeViewport\n"
				"    vp.viewType = ovito.view.ViewType.PERSPECTIVE\n"
				"    vp.cameraPosition = (100, 50, 50)\n"
				"    vp.cameraDirection = (-100, -50, -50)\n"
				"    vp.fieldOfView = math.radians(60.0)\n")
		.add_property("fieldOfView", &Viewport::fieldOfView, &Viewport::setFieldOfView, "The field of view of the viewport's camera. "
				"For perspective projections this is the camera's angle in the vertical direction (in radians). For orthogonal projections this is the visible range in the vertical direction (in world units).")
		.add_property("cameraTransformation", make_function(&Viewport::cameraTransformation, return_value_policy<copy_const_reference>()), &Viewport::setCameraTransformation)
		.add_property("cameraDirection", &Viewport::cameraDirection, &Viewport::setCameraDirection, "The viewing direction vector of the viewport's camera.")
		.add_property("cameraPosition", &Viewport::cameraPosition, &Viewport::setCameraPosition,
				"\nThe position of the viewport's camera. For example, to move the camera of the active viewport to a new location in space::"
				"\n\n"
				"    dataset.viewports.activeViewport.cameraPosition = (100, 80, -30)\n"
				"\n\n")
		.add_property("viewMatrix", make_function(&Viewport::viewMatrix, return_value_policy<copy_const_reference>()))
		.add_property("inverseViewMatrix", make_function(&Viewport::inverseViewMatrix, return_value_policy<copy_const_reference>()))
		.add_property("projectionMatrix", make_function(&Viewport::projectionMatrix, return_value_policy<copy_const_reference>()))
		.add_property("inverseProjectionMatrix", make_function(&Viewport::inverseProjectionMatrix, return_value_policy<copy_const_reference>()))
		.add_property("renderFrameShown", &Viewport::renderFrameShown, &Viewport::setRenderFrameShown)
		.add_property("gridVisible", &Viewport::isGridVisible, &Viewport::setGridVisible)
		.add_property("viewNode", make_function(&Viewport::viewNode, return_value_policy<ovito_object_reference>()), &Viewport::setViewNode)
		.add_property("gridMatrix", make_function(&Viewport::gridMatrix, return_value_policy<copy_const_reference>()), &Viewport::setGridMatrix)
		.add_property("title", make_function(&Viewport::viewportTitle, return_value_policy<copy_const_reference>()), "The title string of the viewport shown in its top left corner (read-only).")
		.def("updateViewport", &Viewport::updateViewport)
		.def("redrawViewport", &Viewport::redrawViewport)
		.def("nonScalingSize", &Viewport::nonScalingSize)
		.def("zoomToSceneExtents", &Viewport::zoomToSceneExtents, "Repositions the viewport camera such that all objects in the scene become completely visible.")
		.def("zoomToSelectionExtents", &Viewport::zoomToSelectionExtents)
		.def("zoomToBox", &Viewport::zoomToBox)
	;

	enum_<Viewport::ViewType>("ViewType")
		.value("NONE", Viewport::VIEW_NONE)
		.value("TOP", Viewport::VIEW_TOP)
		.value("BOTTOM", Viewport::VIEW_BOTTOM)
		.value("FRONT", Viewport::VIEW_FRONT)
		.value("BACK", Viewport::VIEW_BACK)
		.value("LEFT", Viewport::VIEW_LEFT)
		.value("RIGHT", Viewport::VIEW_RIGHT)
		.value("ORTHO", Viewport::VIEW_ORTHO)
		.value("PERSPECTIVE", Viewport::VIEW_PERSPECTIVE)
		.value("SCENENODE", Viewport::VIEW_SCENENODE)
	;

	ovito_class<ViewportConfiguration, RefTarget>(
			"Manages OVITO's viewports."
			"\n\n"
			"This object can be accessed through the :py:attr:`~ovito.app.DataSet.viewports` attribute of the :py:attr:`~ovito.app.DataSet` class. "
			"It behaves like a list of that contains the :py:class:`Viewport` objects::"
			"\n\n"
			"    for viewport in dataset.viewports:\n"
			"        print viewport.title\n"
			"\n"
			"The ``ViewportConfiguration`` object contains four predefined viewports by default. It also manages the active and the maximized viewport.")
		.add_property("activeViewport", make_function(&ViewportConfiguration::activeViewport, return_value_policy<ovito_object_reference>()), &ViewportConfiguration::setActiveViewport,
				"The viewport that is currently active. It is marked with a colored border in OVITO's main window.")
		.add_property("maximizedViewport", make_function(&ViewportConfiguration::maximizedViewport, return_value_policy<ovito_object_reference>()), &ViewportConfiguration::setMaximizedViewport,
				"The viewport that is currently maximized; or ``None`` if no viewport is maximized.\n"
				"Assign a viewport to this attribute to maximize it, e.g.::"
				"\n\n"
				"    dataset.viewports.maximizedViewport = dataset.viewports.activeViewport\n")
		.def("zoomToSelectionExtents", &ViewportConfiguration::zoomToSelectionExtents)
		.def("zoomToSceneExtents", &ViewportConfiguration::zoomToSceneExtents)
		.def("updateViewports", &ViewportConfiguration::updateViewports)
		.add_property("viewports", make_function(&ViewportConfiguration::viewports, return_internal_reference<>()))
	;
}

OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE(PyScriptViewport);

};
