#include "ScriptBindings.h"

#include <vector>

#include <base/linalg/Point3.h>
#include <base/linalg/Vector3.h>
#include <base/utilities/FloatType.h>
#include <core/dataset/DataSetContainer.h>
#include <core/dataset/importexport/FileImporter.h>
#include <core/gui/app/Application.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/gui/widgets/rendering/FrameBufferWindow.h>
#include <core/rendering/RenderSettings.h>
#include <core/rendering/standard/StandardSceneRenderer.h>
#include <core/plugins/PluginManager.h>
#include <core/scene/SelectionSet.h>

namespace Scripting {

using namespace Ovito;


// Wrapping DataSet ////////////////////////////////////////////////////

DataSet* unwrapGlobalDataSet(QScriptEngine* engine) {
	QVariant ds = engine->property("_ovito_dataset");
	if (ds.canConvert< OORef<OvitoObject> >()) {
		DataSet* data = dynamic_object_cast<DataSet>(ds.value< OORef<OvitoObject> >()).get();
		if (data == nullptr)
			throw std::runtime_error("global DataSet is not a DataSet (but is an OvitoObject)!");
		return data;
	} else
		throw std::runtime_error("global DataSet is not a DataSet!");
}


// ViewportBinding /////////////////////////////////////////////////////

ViewportBinding::ViewportBinding(Viewport* viewport, QScriptEngine* engine,
								 DataSet* dataSet,
								 QObject* parent)
	: QObject(parent),
	  viewportConf_(dataSet->viewportConfig()),
	  dataSet_(dataSet),
	  viewport_(viewport),
	  engine_(engine)
{}

void ViewportBinding::perspective(double cam_pos_x, double cam_pos_y, double cam_pos_z,
                                  double cam_dir_x, double cam_dir_y, double cam_dir_z,
                                  double cam_angle) {
  Viewport* vp = getViewport();
  vp->setViewType(Viewport::VIEW_PERSPECTIVE);
  vp->setCameraPosition(Point3(FloatType(cam_pos_x),
                               FloatType(cam_pos_y),
                               FloatType(cam_pos_z)));
  vp->setCameraDirection(Vector3(FloatType(cam_dir_x),
                                 FloatType(cam_dir_y),
                                 FloatType(cam_dir_z)));
  vp->setFieldOfView(cam_angle);
}

void ViewportBinding::ortho(double cam_pos_x, double cam_pos_y, double cam_pos_z,
                            double cam_dir_x, double cam_dir_y, double cam_dir_z,
                            double fov) {
  Viewport* vp = getViewport();
  vp->setViewType(Viewport::VIEW_ORTHO);
  vp->setCameraPosition(Point3(FloatType(cam_pos_x),
                               FloatType(cam_pos_y),
                               FloatType(cam_pos_z)));
  vp->setCameraDirection(Vector3(FloatType(cam_dir_x),
                                 FloatType(cam_dir_y),
                                 FloatType(cam_dir_z)));
  vp->setFieldOfView(fov);
}


void ViewportBinding::maximize() {
	Viewport* vp = getViewport();
	viewportConf_->setMaximizedViewport(vp);
	viewportConf_->setActiveViewport(vp);
}

void ViewportBinding::restore() {
	viewportConf_->setMaximizedViewport(nullptr);
}

void ViewportBinding::setActive() const {
	viewportConf_->setActiveViewport(getViewport());
}


void ViewportBinding::render(const QString& filename,
							 const QScriptValue& options) const {
	// Clone current render settings of the data set.
	CloneHelper cloner;
	OORef<RenderSettings> settings = cloner.cloneObject(dataSet_->renderSettings(), true);
	// Output settings.
	settings->setImageFilename(filename);
	settings->setSaveToFile(true);
	/* TODO: override these from 'options'       
	settings->setRendererClass(&StandardSceneRenderer::OOType);
	settings->setRenderingRangeType(RenderSettings::CURRENT_FRAME);
	settings->setOutputImageWidth(800);
	settings->setOutputImageHeight(600);
	settings->setSkipExistingImages(false);
	*/

	try {
		// Prepare framebuffer.
		FrameBufferWindow* frameBufferWindow = nullptr; // TODO: remove this variable
		QSharedPointer<FrameBuffer> frameBuffer;
		/* not needed.
		if(Application::instance().guiMode()) {
			MainWindow* mainWindow = dataSet_->mainWindow();
			frameBufferWindow = mainWindow->frameBufferWindow();
			frameBuffer = frameBufferWindow->frameBuffer();
		}
		*/
		if(!frameBuffer)
			frameBuffer.reset(new FrameBuffer(settings->outputImageWidth(),
											  settings->outputImageHeight()));

		// Render.
		dataSet_->renderScene(settings.get(),
							  getViewport(),
							  frameBuffer,
							  frameBufferWindow);
	} catch (const Exception& e) {
		QScriptContext* context = engine_->currentContext();
		context->throwError(QString("Exception while rendering: ") + e.what());
	}
}


// DataSetBinding //////////////////////////////////////////////////////

DataSetBinding::DataSetBinding(ObjectNode* object, QObject* parent)
	: QObject(parent), object_(object)
{}

void DataSetBinding::appendModifier(const QScriptValue& modifier) {
	QScriptEngine* engine = modifier.engine();
	QScriptContext* context = engine->currentContext();
	try {
		QObject* data = modifier.data().toQObject();
		if (data == nullptr) {
			context->throwError("Not a valid modifier");
			return;
		}
		// TODO: this cast is not necessarily safe.   
		ScriptRef<Modifier> modWrapper = *dynamic_cast<ScriptRef<Modifier>*>(data);
		OORef<Modifier> mod = modWrapper.getReference();
		object_->applyModifier(mod);
	} catch (const Exception& e) {
		context->throwError(QString("Not a valid modifier (error: ") + e.what() + ")");
	}
}


// Helper functions ////////////////////////////////////////////////////

QScriptValue pwd(QScriptContext* context, QScriptEngine* engine) {
	// Process arguments.
	if (context->argumentCount() != 0)
		return context->throwError("This function takes no arguments.");
	// Return.
	return QScriptValue(QDir::currentPath());
}

QScriptValue cd(QScriptContext* context, QScriptEngine* engine) {
	// Process arguments.
	if (context->argumentCount() != 1)
		return context->throwError("This function takes one argument.");
	const QString newdir = context->argument(0).toString();

	// Change dir.
	const bool success = QDir::setCurrent(newdir);
	if (success)
		return QScriptValue(QDir::currentPath());
	else
		return context->throwError("Can't change directory to " + newdir);
}

QScriptValue loadFile(QScriptContext* context, QScriptEngine* engine) {
	// Process arguments.
	if (context->argumentCount() != 1)
		return context->throwError("This function takes one argument.");
	const QString path = context->argument(0).toString();

	// Load it.
	DataSet* dataSet = unwrapGlobalDataSet(engine);
	DataSetContainer* container = dataSet->container();
	try {
		container->importFile(QUrl::fromLocalFile(path),
							  nullptr,
							  FileImporter::AddToScene);
	} catch (const Exception& e) {
		return context->throwError(e.what());
	}

	// Return wrapper around the dataset representing the current file.
	// TODO: this is racy, perhaps? Is the new file guaranteed to be     
 	// the active selection?                                             
	// Is this a good way to do it?                                      
	ObjectNode* root = static_cast<ObjectNode*>(dataSet->selection()->firstNode());
	return engine->newQObject(new DataSetBinding(root),
							  QScriptEngine::ScriptOwnership);
}


QScriptValue listModifiers(QScriptContext* context, QScriptEngine* engine) {
	// Process arguments.
	if (context->argumentCount() != 0)
		return context->throwError("This function takes no arguments.");

	// Build array of modifier names.
	std::vector<QString> names;
	Q_FOREACH(const OvitoObjectType* clazz,
			  PluginManager::instance().listClasses(Modifier::OOType)) {
		names.push_back(clazz->name());
	}
	QScriptValue retval = engine->newArray(names.size());
	for (int i = 0; i != names.size(); ++i)
		retval.setProperty(i, QScriptValue(names[i]));
	return retval;
}


QScriptValue modifier(QScriptContext* context, QScriptEngine* engine) {
	// Process arguments.
	if (context->argumentCount() != 1)
		return context->throwError("This function takes one argument.");
	QString name = context->argument(0).toString();

	// Search modifier.
	const OvitoObjectType* searchResultClass = nullptr;
	Q_FOREACH(const OvitoObjectType* clazz,
			  PluginManager::instance().listClasses(Modifier::OOType)) {
		if (clazz->name() == name) {
			searchResultClass = clazz;
			break;
		}
	}

	// Return.
	if (searchResultClass == nullptr)
		return context->throwError("Modifier " + name + " not found.");
	else {
		// Get DataSet.
		DataSet* dataSet = unwrapGlobalDataSet(engine);
		// Get instance of modifier.
		OORef<Modifier> ptr =
			static_object_cast<Modifier>(searchResultClass->createInstance(dataSet));
		return wrapOORef<Modifier>(ptr, engine);
	}
}


// Helpers to convert values between JS and C++. ///////////////////////

QScriptValue fromFloatType(QScriptEngine *engine, const FloatType& x) {
	double xx = x;
	return QScriptValue(xx);
}

void toFloatType(const QScriptValue& obj, FloatType& x) {
	x = obj.toNumber();
}



// Factory function for script engine //////////////////////////////////

QScriptEngine* prepareEngine(DataSet* dataSet, QObject* parent) {
	// Set up engine.
	QScriptEngine* engine = new QScriptEngine(parent);

	// Store global DataSet.
	engine->setProperty("_ovito_dataset", QVariant::fromValue(OORef<OvitoObject>(dataSet)));

	// Register automatic conversions.
	const int FloatTypeTypeId = qRegisterMetaType<FloatType>("FloatType");
	qScriptRegisterMetaType<FloatType>(engine, fromFloatType, toFloatType);

	// Set up namespace. ///////////////////////////////////////////////

	ViewportConfiguration* viewportConf = dataSet->viewportConfig();

	// All viewports.
	const QVector<Viewport*>& allViewports = viewportConf->viewports();
	QScriptValue viewport = engine->newArray(allViewports.size());
	for (int i = 0; i != allViewports.size(); ++i)
		viewport.setProperty(i, engine->newQObject(new ViewportBinding(allViewports[i],
																	   engine,
																	   dataSet),
												   QScriptEngine::ScriptOwnership));
	engine->globalObject().setProperty("viewport", viewport);

	// Active viewport
	QScriptValue activeViewport =
		engine->newQObject(new ActiveViewportBinding(engine, dataSet),
						   QScriptEngine::ScriptOwnership);
	engine->globalObject().setProperty("activeViewport", activeViewport);

	// quit function.
	engine->globalObject().setProperty("quit",
									   engine->newFunction(quit, 0));

	// pwd function.
	engine->globalObject().setProperty("pwd",
									   engine->newFunction(pwd, 0));

	// cd function.
	engine->globalObject().setProperty("cd",
									   engine->newFunction(cd, 0));

	// loadFile function.
	engine->globalObject().setProperty("loadFile",
									   engine->newFunction(loadFile, 0));

	// listModifiers function.
	engine->globalObject().setProperty("listModifiers",
									   engine->newFunction(listModifiers, 0));

	// modifier function.
	engine->globalObject().setProperty("modifier",
									   engine->newFunction(modifier, 1));

	// Done.
	return engine;
}


}

// Local variables:
// indent-tabs-mode: t
// tab-width: 4
// c-basic-offset: 4
// End:
