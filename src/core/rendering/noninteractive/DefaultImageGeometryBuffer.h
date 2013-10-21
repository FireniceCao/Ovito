///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2013) Alexander Stukowski
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
 * \file DefaultImageGeometryBuffer.h
 * \brief Contains the definition of the Ovito::DefaultImageGeometryBuffer class.
 */

#ifndef __OVITO_DEFAULT_IMAGE_GEOMETRY_BUFFER_H
#define __OVITO_DEFAULT_IMAGE_GEOMETRY_BUFFER_H

#include <core/Core.h>
#include <core/rendering/ImageGeometryBuffer.h>

namespace Ovito {

/**
 * \brief Buffer object that stores an image to be rendered by a non-interactive renderer.
 */
class OVITO_CORE_EXPORT DefaultImageGeometryBuffer : public ImageGeometryBuffer
{
public:

	/// Constructor.
	DefaultImageGeometryBuffer() {}

	/// \brief Returns true if the geometry buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) override;

	/// \brief Renders the image in a rectangle given in pixel coordinates.
	virtual void renderWindow(SceneRenderer* renderer, const Point2& pos, const Vector2& size) override;

	/// \brief Renders the image in a rectangle given in viewport coordinates.
	virtual void renderViewport(SceneRenderer* renderer, const Point2& pos, const Vector2& size) override;

private:

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_DEFAULT_IMAGE_GEOMETRY_BUFFER_H