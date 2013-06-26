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

#include <core/Core.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportWindow.h>
#include <core/viewport/ViewportManager.h>
#include <core/animation/AnimManager.h>
#include <core/rendering/viewport/ViewportSceneRenderer.h>
#include <core/rendering/RenderSettings.h>
#include <core/dataset/DataSetManager.h>
#include <core/scene/objects/AbstractCameraObject.h>
#include "ViewportMenu.h"

/// The default field of view in world units used for orthogonal view types when the scene is empty.
#define DEFAULT_ORTHOGONAL_FIELD_OF_VIEW		200.0

/// The default field of view in radians used for perspective view types when the scene is empty.
#define DEFAULT_PERSPECTIVE_FIELD_OF_VIEW		(FLOATTYPE_PI/4.0)

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, Viewport, RefTarget);
DEFINE_FLAGS_REFERENCE_FIELD(Viewport, _viewNode, "ViewNode", ObjectNode, PROPERTY_FIELD_NO_UNDO|PROPERTY_FIELD_NEVER_CLONE_TARGET)
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _viewType, "ViewType", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _shadingMode, "ShadingMode", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _showGrid, "ShowGrid", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _gridMatrix, "GridMatrix", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _fieldOfView, "FieldOfView", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _cameraPosition, "CameraPosition", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _cameraDirection, "CameraDirection", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _showRenderFrame, "ShowRenderFrame", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _orbitCenter, "OrbitCenter", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _useOrbitCenter, "UseOrbitCenter", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _viewportTitle, "Title", PROPERTY_FIELD_NO_UNDO)

/******************************************************************************
* Constructor.
******************************************************************************/
Viewport::Viewport() :
		_widget(nullptr), _viewportWindow(nullptr),
		_viewType(VIEW_NONE), _shadingMode(SHADING_WIREFRAME), _showGrid(false),
		_fieldOfView(100),
		_showRenderFrame(false), _orbitCenter(Point3::Origin()), _useOrbitCenter(false),
		_glcontext(nullptr), _paintDevice(nullptr),
		_cameraPosition(Point3::Origin()), _cameraDirection(0,0,-1),
		_renderDebugCounter(0)
{
	INIT_PROPERTY_FIELD(Viewport::_viewNode);
	INIT_PROPERTY_FIELD(Viewport::_viewType);
	INIT_PROPERTY_FIELD(Viewport::_shadingMode);
	INIT_PROPERTY_FIELD(Viewport::_showGrid);
	INIT_PROPERTY_FIELD(Viewport::_gridMatrix);
	INIT_PROPERTY_FIELD(Viewport::_fieldOfView);
	INIT_PROPERTY_FIELD(Viewport::_cameraPosition);
	INIT_PROPERTY_FIELD(Viewport::_cameraDirection);
	INIT_PROPERTY_FIELD(Viewport::_showRenderFrame);
	INIT_PROPERTY_FIELD(Viewport::_orbitCenter);
	INIT_PROPERTY_FIELD(Viewport::_useOrbitCenter);
	INIT_PROPERTY_FIELD(Viewport::_viewportTitle);
}

/******************************************************************************
* Destructor
******************************************************************************/
Viewport::~Viewport()
{
	delete _widget;

	OVITO_ASSERT(!_widget);
	OVITO_ASSERT(!_viewportWindow);
}

/******************************************************************************
* Displays the context menu for this viewport.
******************************************************************************/
void Viewport::showViewportMenu(const QPoint& pos)
{
	// Create the context menu for the viewport.
	ViewportMenu contextMenu(this);

	// Show menu.
	contextMenu.show(pos);
}

/******************************************************************************
* Changes the view type.
******************************************************************************/
void Viewport::setViewType(ViewType type)
{
	if(type == viewType())
		return;

	// Reset camera node.
	setViewNode(nullptr);

	// Setup default view.
	switch(type) {
		case VIEW_TOP:
			setCameraPosition(Point3::Origin());
			setCameraDirection(-ViewportSettings::getSettings().coordinateSystemOrientation().column(2));
			break;
		case VIEW_BOTTOM:
			setCameraPosition(Point3::Origin());
			setCameraDirection(ViewportSettings::getSettings().coordinateSystemOrientation().column(2));
			break;
		case VIEW_LEFT:
			setCameraPosition(Point3::Origin());
			setCameraDirection(ViewportSettings::getSettings().coordinateSystemOrientation().column(0));
			break;
		case VIEW_RIGHT:
			setCameraPosition(Point3::Origin());
			setCameraDirection(-ViewportSettings::getSettings().coordinateSystemOrientation().column(0));
			break;
		case VIEW_FRONT:
			setCameraPosition(Point3::Origin());
			setCameraDirection(ViewportSettings::getSettings().coordinateSystemOrientation().column(1));
			break;
		case VIEW_BACK:
			setCameraPosition(Point3::Origin());
			setCameraDirection(-ViewportSettings::getSettings().coordinateSystemOrientation().column(1));
			break;
		case VIEW_ORTHO:
			setCameraPosition(Point3::Origin());
			if(viewType() == VIEW_NONE)
				setCameraDirection(-ViewportSettings::getSettings().coordinateSystemOrientation().column(2));
			break;
		case VIEW_PERSPECTIVE:
			if(viewType() >= VIEW_TOP && viewType() <= VIEW_ORTHO) {
				setCameraPosition(cameraPosition() - (cameraDirection().normalized() * fieldOfView()));
			}
			else if(viewType() != VIEW_PERSPECTIVE) {
				setCameraPosition(ViewportSettings::getSettings().coordinateSystemOrientation() * Point3(0,0,-50));
				setCameraDirection(ViewportSettings::getSettings().coordinateSystemOrientation() * Vector3(0,0,1));
			}
			break;
		case VIEW_SCENENODE:
			break;
		case VIEW_NONE:
			break;
	}

	// Setup default zoom.
	if(type == VIEW_PERSPECTIVE) {
		if(viewType() != VIEW_PERSPECTIVE)
			setFieldOfView(DEFAULT_PERSPECTIVE_FIELD_OF_VIEW);
	}
	else {
		if(viewType() == VIEW_PERSPECTIVE || viewType() == VIEW_NONE)
			setFieldOfView(DEFAULT_ORTHOGONAL_FIELD_OF_VIEW);
	}

	_viewType = type;
}

/******************************************************************************
* Returns true if the viewport is using a perspective project;
* returns false if it is using an orthogonal projection.
******************************************************************************/
bool Viewport::isPerspectiveProjection() const
{
	return (viewType() == VIEW_PERSPECTIVE);
}

/******************************************************************************
* Computes the projection matrix and other parameters.
******************************************************************************/
ViewProjectionParameters Viewport::projectionParameters(TimePoint time, FloatType aspectRatio, const Box3& sceneBoundingBox)
{
	OVITO_ASSERT(aspectRatio > FLOATTYPE_EPSILON);

	ViewProjectionParameters params;
	params.aspectRatio = aspectRatio;
	params.validityInterval.setInfinite();

	// Get transformation from view scene node.
	if(viewType() == VIEW_SCENENODE && viewNode()) {
		PipelineFlowState state = viewNode()->evalPipeline(time);
		AbstractCameraObject* camera = state.findObject<AbstractCameraObject>();
		if(camera) {
			// Get camera transformation.
			params.inverseViewMatrix = viewNode()->getWorldTransform(time, params.validityInterval);
			params.viewMatrix = params.inverseViewMatrix.inverse();

			// Calculate znear and zfar clipping plane distances.
			Box3 bb = sceneBoundingBox.transformed(params.viewMatrix);
			params.zfar = -bb.minc.z();
			params.znear = std::max(-bb.maxc.z(), params.zfar * 1e-5f);

			// Get remaining parameters from camera object.
			camera->projectionParameters(time, params);
		}
	}
	else {
		params.viewMatrix = AffineTransformation::lookAlong(cameraPosition(), cameraDirection(), ViewportSettings::getSettings().upVector());
		params.fieldOfView = fieldOfView();

		// Transform scene bounding box to camera space.
		Box3 bb = sceneBoundingBox.transformed(params.viewMatrix).centerScale(1.01);

		if(viewType() == VIEW_PERSPECTIVE) {
			params.isPerspective = true;

			if(bb.minc.z() < -FLOATTYPE_EPSILON) {
				params.zfar = -bb.minc.z();
				params.znear = std::max(-bb.maxc.z(), params.zfar * 1e-5f);
			}
			else {
				params.zfar = sceneBoundingBox.size().length();
				params.znear = params.zfar * 1e-5f;
			}
			params.projectionMatrix = Matrix4::perspective(params.fieldOfView, 1.0 / params.aspectRatio, params.znear, params.zfar);
		}
		else {
			params.isPerspective = false;

			if(!bb.isEmpty()) {
				params.znear = -bb.maxc.z();
				params.zfar  = std::max(-bb.minc.z(), params.znear + 1.0f);
			}
			else {
				params.znear = 1;
				params.zfar = 100;
			}
			params.projectionMatrix = Matrix4::ortho(-params.fieldOfView / params.aspectRatio, params.fieldOfView / params.aspectRatio,
								-params.fieldOfView, params.fieldOfView,
								params.znear, params.zfar);
		}
		params.inverseViewMatrix = params.viewMatrix.inverse();
		params.inverseProjectionMatrix = params.projectionMatrix.inverse();
	}
	return params;
}

/******************************************************************************
* Zooms to the extents of the scene.
******************************************************************************/
void Viewport::zoomToSceneExtents()
{
	Box3 sceneBoundingBox = DataSetManager::instance().currentSet()->sceneRoot()->worldBoundingBox(AnimManager::instance().time());
	zoomToBox(sceneBoundingBox);
}

/******************************************************************************
* Zooms to the extents of the currently selected nodes.
******************************************************************************/
void Viewport::zoomToSelectionExtents()
{
	Box3 selectionBoundingBox;
	for(SceneNode* node : DataSetManager::instance().currentSelection()->nodes()) {
		selectionBoundingBox.addBox(node->worldBoundingBox(AnimManager::instance().time()));
	}
	if(selectionBoundingBox.isEmpty() == false)
		zoomToBox(selectionBoundingBox);
	else
		zoomToSceneExtents();
}

/******************************************************************************
* Zooms to the extents of the given bounding box.
******************************************************************************/
void Viewport::zoomToBox(const Box3& box)
{
	if(box.isEmpty())
		return;

	if(viewType() == VIEW_SCENENODE)
		return;	// Cannot reposition the view node.

	if(isPerspectiveProjection()) {
		FloatType dist = box.size().length() * 0.5 / tan(fieldOfView() * 0.5);
		setCameraPosition(box.center() - cameraDirection().resized(dist));
	}
	else {
		AffineTransformation viewMat = viewMatrix();

		FloatType minX =  FLOATTYPE_MAX, minY =  FLOATTYPE_MAX;
		FloatType maxX = -FLOATTYPE_MAX, maxY = -FLOATTYPE_MAX;
		for(int i = 0; i < 8; i++) {
			Point3 trans = viewMat * box[i];
			if(trans.x() < minX) minX = trans.x();
			if(trans.x() > maxX) maxX = trans.x();
			if(trans.y() < minY) minY = trans.y();
			if(trans.y() > maxY) maxY = trans.y();
		}
		FloatType w = std::max(maxX - minX, FloatType(1e-5));
		FloatType h = std::max(maxY - minY, FloatType(1e-5));
		if(_projParams.aspectRatio > h/w)
			setFieldOfView(w * _projParams.aspectRatio * 0.55);
		else
			setFieldOfView(h * 0.55);
		setCameraPosition(box.center());
	}
}


/******************************************************************************
* Is called when a RefTarget referenced by this object has generated an event.
******************************************************************************/
bool Viewport::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(source == viewNode() && event->type() == ReferenceEvent::TargetChanged) {
		// Update viewport when camera node has moved.
		updateViewport();
		return false;
	}
	else if(source == viewNode() && event->type() == ReferenceEvent::TitleChanged) {
		// Update viewport title when camera node has been renamed.
		updateViewportTitle();
		updateViewport();
		return false;
	}
	return RefTarget::referenceEvent(source, event);
}

/******************************************************************************
* Is called when the value of a reference field of this RefMaker changes.
******************************************************************************/
void Viewport::referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget)
{
	if(field == PROPERTY_FIELD(Viewport::_viewNode)) {
		// Switch to perspective mode when camera node has been deleted.
		if(viewType() == VIEW_SCENENODE && newTarget == nullptr) {
			setViewType(VIEW_PERSPECTIVE);
		}
		else {
			// Update viewport when the camera has been replaced by another scene node.
			updateViewportTitle();
		}
	}
	RefTarget::referenceReplaced(field, oldTarget, newTarget);
}

/******************************************************************************
* Is called when the value of a property field of this object has changed.
******************************************************************************/
void Viewport::propertyChanged(const PropertyFieldDescriptor& field)
{
	RefTarget::propertyChanged(field);
	if(field == PROPERTY_FIELD(Viewport::_viewType)) {
		updateViewportTitle();
	}
	updateViewport();
}

/******************************************************************************
* Updates the title text of the viewport based on the current view type.
******************************************************************************/
void Viewport::updateViewportTitle()
{
	// Load viewport caption string.
	switch(viewType()) {
		case VIEW_TOP: _viewportTitle = tr("Top"); break;
		case VIEW_BOTTOM: _viewportTitle = tr("Bottom"); break;
		case VIEW_FRONT: _viewportTitle = tr("Front"); break;
		case VIEW_BACK: _viewportTitle = tr("Back"); break;
		case VIEW_LEFT: _viewportTitle = tr("Left"); break;
		case VIEW_RIGHT: _viewportTitle = tr("Right"); break;
		case VIEW_ORTHO: _viewportTitle = tr("Ortho"); break;
		case VIEW_PERSPECTIVE: _viewportTitle = tr("Perspective"); break;
		case VIEW_SCENENODE:
			if(viewNode() != nullptr)
				_viewportTitle = viewNode()->name();
			else
				_viewportTitle = tr("No view node");
		break;
		default: OVITO_ASSERT(false); _viewportTitle = QString(); // unknown viewport type
	}
}

/******************************************************************************
* Returns the widget that contains the viewport's rendering window.
******************************************************************************/
QWidget* Viewport::createWidget(QWidget* parent)
{
	OVITO_ASSERT(_widget == nullptr && _viewportWindow == nullptr);
	if(!_widget) {
		_viewportWindow = new ViewportWindow(this);
		_widget = QWidget::createWindowContainer(_viewportWindow, parent);
		_widget->setAttribute(Qt::WA_DeleteOnClose);
	}
	return _widget;
}

/******************************************************************************
* Puts an update request event for this viewport on the event loop.
******************************************************************************/
void Viewport::updateViewport()
{
	if(_viewportWindow)
		_viewportWindow->renderLater();
}

/******************************************************************************
* Immediately redraws the contents of this viewport.
******************************************************************************/
void Viewport::redrawViewport()
{
	if(_viewportWindow)
		_viewportWindow->renderNow();
}

/******************************************************************************
* If an update request is pending for this viewport, immediately processes it
* and redraw the viewport.
******************************************************************************/
void Viewport::processUpdateRequest()
{
	if(_viewportWindow)
		_viewportWindow->processUpdateRequest();
}

/******************************************************************************
* Renders the contents of the viewport.
******************************************************************************/
void Viewport::render(QOpenGLContext* context, QOpenGLPaintDevice* paintDevice)
{
	OVITO_ASSERT_MSG(_glcontext == NULL, "Viewport::render", "Viewport is already rendering.");
	_glcontext = context;
	_paintDevice = paintDevice;

	try {

		QSize vpSize = size();
		glViewport(0, 0, vpSize.width(), vpSize.height());

		// Request scene bounding box.
		Box3 boundingBox = DataSetManager::instance().currentSet()->sceneRoot()->worldBoundingBox(AnimManager::instance().time());
		if(boundingBox.isEmpty())
			boundingBox = Box3(Point3::Origin(), 100);

		// Setup projection.
		FloatType aspectRatio = (FloatType)vpSize.height() / vpSize.width();
		_projParams = projectionParameters(AnimManager::instance().time(), aspectRatio, boundingBox);

		// Set up the viewport renderer.
		ViewportManager::instance().renderer()->setTime(AnimManager::instance().time());
		ViewportManager::instance().renderer()->setProjParams(_projParams);
		ViewportManager::instance().renderer()->setViewport(this);
		ViewportManager::instance().renderer()->setDataset(DataSetManager::instance().currentSet());
		ViewportManager::instance().renderer()->beginRender();

		// Call the viewport renderer to render the scene objects.
		ViewportManager::instance().renderer()->renderFrame();

		// Render render frame.
		renderRenderFrame();

		// Render orientation tripod.
		renderOrientationIndicator();

#if 1
		// Render viewport caption.
		renderViewportTitle();
#endif

		// Stop rendering.
		ViewportManager::instance().renderer()->endRender();

		_glcontext = nullptr;
		_paintDevice = nullptr;
	}
	catch(Exception& ex) {
		ex.prependGeneralMessage(tr("An unexpected error occurred while rendering the viewport contents. The program will quit."));
		ViewportManager::instance().suspendViewportUpdates();
		QCoreApplication::removePostedEvents(nullptr, 0);
		ex.showError();
		QCoreApplication::instance()->quit();
	}
}

/******************************************************************************
* Renders the viewport caption text.
******************************************************************************/
void Viewport::renderViewportTitle()
{
	Color captionColor = viewportColor(ViewportSettings::COLOR_VIEWPORT_CAPTION);
	QFontMetricsF metrics(ViewportManager::instance().viewportFont());
	QPointF pos(2, metrics.ascent() + 2);
	_contextMenuArea = QRect(0, 0, std::max(metrics.width(viewportTitle()), 30.0) + 2, metrics.height() + 2);
#ifndef OVITO_DEBUG
	renderText(viewportTitle(), pos, (QColor)captionColor);
#else
	renderText(QString("%1 [%2]").arg(viewportTitle()).arg(++_renderDebugCounter), pos, (QColor)captionColor);
#endif
}

/******************************************************************************
* Helper method that saves the current OpenGL rendering attributes on the
* stack and switches to flat shading.
******************************************************************************/
void Viewport::begin2DPainting()
{
	OVITO_CHECK_OPENGL(glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS));
	OVITO_CHECK_OPENGL(glPushAttrib(GL_ALL_ATTRIB_BITS));

	OVITO_CHECK_OPENGL(glDisable(GL_CULL_FACE));
	OVITO_CHECK_OPENGL(glDisable(GL_DEPTH_TEST));
	OVITO_CHECK_OPENGL(glEnable(GL_BLEND));
	OVITO_CHECK_OPENGL(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
}

/******************************************************************************
* Helper method that restores the OpenGL rendering attributes saved
* by begin2DPainting().
******************************************************************************/
void Viewport::end2DPainting()
{
	OVITO_CHECK_OPENGL(glPopAttrib());
	OVITO_CHECK_OPENGL(glPopClientAttrib());
}

/******************************************************************************
* Renders a text string into the GL context.
******************************************************************************/
void Viewport::renderText(const QString& str, const QPointF& pos, const QColor& color)
{
	OVITO_ASSERT_MSG(_paintDevice != NULL, "Viewport::renderText", "Viewport is not rendering.");

	if(str.isEmpty())
		return;

	begin2DPainting();
	{
		QPainter painter(_paintDevice);
		painter.setPen(color);
		painter.setFont(ViewportManager::instance().viewportFont());
		painter.drawText(pos, str);
	}
	end2DPainting();
}

/******************************************************************************
* Sets whether mouse grab should be enabled or not for this viewport window.
******************************************************************************/
bool Viewport::setMouseGrabEnabled(bool grab)
{
	if(_viewportWindow)
		return _viewportWindow->setMouseGrabEnabled(grab);
	else
		return false;
}

/******************************************************************************
* Sets the cursor shape for this viewport window.
******************************************************************************/
void Viewport::setCursor(const QCursor& cursor)
{
	if(_viewportWindow)
		_viewportWindow->setCursor(cursor);
}

/******************************************************************************
* Restores the default arrow cursor for this viewport window.
******************************************************************************/
void Viewport::unsetCursor()
{
	if(_viewportWindow)
		_viewportWindow->unsetCursor();
}

/******************************************************************************
* Render the axis tripod symbol in the corner of the viewport that indicates
* the coordinate system orientation.
******************************************************************************/
void Viewport::renderOrientationIndicator()
{
	const FloatType tripodSize = 60.0f;			// pixels
	const FloatType tripodArrowSize = 0.17f; 	// percentage of the above value.

	// Save current rendering attributes and turn off depth-testing.
	begin2DPainting();

	// Setup projection matrix.
	ViewProjectionParameters projParams = _projParams;
	FloatType xscale = size().width() / tripodSize;
	FloatType yscale = size().height() / tripodSize;
	projParams.projectionMatrix = Matrix4::translation(Vector3(-1.0 + 1.3f/xscale, -1.0 + 1.3f/yscale, 0))
									* Matrix4::ortho(-xscale, xscale, -yscale, yscale, -2, 2);
	projParams.inverseProjectionMatrix = projParams.projectionMatrix.inverse();
	projParams.viewMatrix.setIdentity();
	projParams.inverseViewMatrix.setIdentity();
	ViewportManager::instance().renderer()->setProjParams(projParams);
	ViewportManager::instance().renderer()->setWorldTransform(AffineTransformation::Identity());

	// Create line buffer.
	static const Color axisColors[3] = { Color(1, 0, 0), Color(0, 1, 0), Color(0.2, 0.2, 1) };
	if(!_orientationTripodGeometry || !_orientationTripodGeometry->isValid(ViewportManager::instance().renderer())) {
		_orientationTripodGeometry = ViewportManager::instance().renderer()->createLineGeometryBuffer();
		_orientationTripodGeometry->setSize(18);
		ColorA vertexColors[18];
		for(int i = 0; i < 18; i++)
			vertexColors[i] = ColorA(axisColors[i / 6]);
		_orientationTripodGeometry->setVertexColors(vertexColors);
	}

	// Render arrows.
	Point3 vertices[18];
	for(int axis = 0, index = 0; axis < 3; axis++) {
		Vector3 dir = _projParams.viewMatrix.column(axis).normalized();
		vertices[index++] = Point3::Origin();
		vertices[index++] = Point3::Origin() + dir;
		vertices[index++] = Point3::Origin() + dir;
		vertices[index++] = Point3::Origin() + (dir + tripodArrowSize * Vector3(dir.y() - dir.x(), -dir.x() - dir.y(), dir.z()));
		vertices[index++] = Point3::Origin() + dir;
		vertices[index++] = Point3::Origin() + (dir + tripodArrowSize * Vector3(-dir.y() - dir.x(), dir.x() - dir.y(), dir.z()));
	}
	_orientationTripodGeometry->setVertexPositions(vertices);
	_orientationTripodGeometry->render();

	// Render x,y,z labels.
	static const QString labels[3] = { "x", "y", "z" };
	for(int axis = 0; axis < 3; axis++) {
		Point3 p = Point3::Origin() + _projParams.viewMatrix.column(axis).resized(1.2f);
		Point3 screenPoint = projParams.projectionMatrix * p;
		QPointF pos(( screenPoint.x() + 1.0) * size().width()  / 2,
					(-screenPoint.y() + 1.0) * size().height() / 2);
		pos += QPointF(-4, 3);
		renderText(labels[axis], pos, QColor(axisColors[axis]));
	}

	// Restore old rendering attributes.
	end2DPainting();
}

/******************************************************************************
* Renders the frame on top of the scene that indicates the visible rendering area.
******************************************************************************/
void Viewport::renderRenderFrame()
{
	if(!renderFrameShown())
		return;

	QSize vpSize = size();
	RenderSettings* renderSettings = DataSetManager::instance().currentSet()->renderSettings();
	if(!renderSettings || vpSize.width() == 0 || vpSize.height() == 0)
		return;

	// Save current rendering attributes.
	begin2DPainting();

	// Set identity projection matrices.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Compute a rectangle that has the same aspect ratio as the rendered image.
	FloatType renderAspectRatio = renderSettings->outputImageAspectRatio();
	FloatType windowAspectRatio = (FloatType)vpSize.height() / (FloatType)vpSize.width();
	FloatType frameWidth, frameHeight;
	if(renderAspectRatio < windowAspectRatio) {
		frameWidth = 0.9;
		frameHeight = frameWidth / windowAspectRatio * renderAspectRatio;
	}
	else {
		frameHeight = 0.9;
		frameWidth = frameHeight / renderAspectRatio * windowAspectRatio;
	}

	glColor4f(1, 1, 1, 0.5f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Render rectangle borders
	glBegin(GL_QUADS);

	glVertex3(-1, -1, 0);
	glVertex3(-frameWidth, -1, 0);
	glVertex3(-frameWidth, +1, 0);
	glVertex3(-1, +1, 0);

	glVertex3(+1, +1, 0);
	glVertex3(+frameWidth, +1, 0);
	glVertex3(+frameWidth, -1, 0);
	glVertex3(+1, -1, 0);

	glVertex3(-frameWidth, -1, 0);
	glVertex3(-frameWidth, -frameHeight, 0);
	glVertex3(+frameWidth, -frameHeight, 0);
	glVertex3(+frameWidth, -1, 0);

	glVertex3(+frameWidth, +1, 0);
	glVertex3(+frameWidth, +frameHeight, 0);
	glVertex3(-frameWidth, +frameHeight, 0);
	glVertex3(-frameWidth, +1, 0);

	glEnd();

	// Restore old rendering attributes.
	end2DPainting();
}

};
