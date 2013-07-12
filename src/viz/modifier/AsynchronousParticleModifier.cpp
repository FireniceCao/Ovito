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
#include <core/viewport/ViewportManager.h>
#include <core/animation/AnimManager.h>
#include <core/utilities/concurrent/Task.h>
#include <core/utilities/concurrent/ProgressManager.h>

#include "AsynchronousParticleModifier.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, AsynchronousParticleModifier, ParticleModifier)
DEFINE_PROPERTY_FIELD(AsynchronousParticleModifier, _autoUpdate, "AutoUpdate")
DEFINE_PROPERTY_FIELD(AsynchronousParticleModifier, _saveResults, "SaveResults")
SET_PROPERTY_FIELD_LABEL(AsynchronousParticleModifier, _autoUpdate, "Automatic update")
SET_PROPERTY_FIELD_LABEL(AsynchronousParticleModifier, _saveResults, "Save results in scene file")

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
AsynchronousParticleModifier::AsynchronousParticleModifier() : _autoUpdate(true), _saveResults(false),
	_cacheValidity(TimeInterval::empty()), _computationValidity(TimeInterval::empty())
{
	INIT_PROPERTY_FIELD(AsynchronousParticleModifier::_autoUpdate);
	INIT_PROPERTY_FIELD(AsynchronousParticleModifier::_saveResults);

	connect(&_backgroundOperationWatcher, &FutureWatcher::finished, this, &AsynchronousParticleModifier::backgroundJobFinished);
}

/******************************************************************************
* This method is called by the system when an item in the modification pipeline
* located before this modifier has changed.
******************************************************************************/
void AsynchronousParticleModifier::modifierInputChanged(ModifierApplication* modApp)
{
	ParticleModifier::modifierInputChanged(modApp);

	_cacheValidity.setEmpty();
	cancelBackgroundJob();
}

/******************************************************************************
* Cancels any running background job.
******************************************************************************/
void AsynchronousParticleModifier::cancelBackgroundJob()
{
	if(_backgroundOperation.isValid()) {
		try {
			_backgroundOperationWatcher.unsetFuture();
			_backgroundOperation.cancel();
			_backgroundOperation.waitForFinished();
		} catch(...) {}
		_backgroundOperation.reset();
		if(status().type() == ObjectStatus::Pending)
			setStatus(ObjectStatus());
	}
	_computationValidity.setEmpty();
}

/******************************************************************************
* This modifies the input object.
******************************************************************************/
ObjectStatus AsynchronousParticleModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	if(autoUpdateEnabled() && !_cacheValidity.contains(time) && input().status().type() != ObjectStatus::Pending) {

		if(!_computationValidity.contains(time)) {

			// Stop running job first.
			cancelBackgroundJob();

			// Start a background job to compute the modifier's results.
			_computationValidity.setInstant(time);

			std::shared_ptr<Engine> engine = createEngine(time);
			OVITO_ASSERT(engine);
			_backgroundOperation = runInBackground<std::shared_ptr<Engine>>(std::bind(&AsynchronousParticleModifier::performAnalysis, this, std::placeholders::_1, engine));
			ProgressManager::instance().addTask(_backgroundOperation);
			_backgroundOperationWatcher.setFuture(_backgroundOperation);
		}
	}

	if(!_cacheValidity.contains(time)) {
		if(!_computationValidity.contains(time))
			throw Exception(tr("The modifier results have not been computed yet."));
		else
			return ObjectStatus::Pending;
	}

	return ObjectStatus::Success;
}

/******************************************************************************
* This function is executed in a background thread to compute the modifier results.
******************************************************************************/
void AsynchronousParticleModifier::performAnalysis(FutureInterface<std::shared_ptr<Engine>>& futureInterface, std::shared_ptr<Engine> engine)
{
	// Let the engine object do the actual work.
	engine->compute(futureInterface);

	// Pass engine back to caller since it carries the results.
	if(!futureInterface.isCanceled())
		futureInterface.setResult(engine);
}

/******************************************************************************
* This is called when the background analysis task has finished.
******************************************************************************/
void AsynchronousParticleModifier::backgroundJobFinished()
{
	OVITO_ASSERT(!_computationValidity.isEmpty());
	ReferenceEvent::Type notificationType = ReferenceEvent::PendingOperationFailed;
	bool wasCanceled = _backgroundOperation.isCanceled();
	ObjectStatus newStatus = status();

	if(!wasCanceled) {
		try {
			std::shared_ptr<Engine> engine = _backgroundOperation.result();
			retrieveResults(engine.get());
			_cacheValidity = _computationValidity;

			// Notify dependents that the background operation has succeeded and new data is available.
			notificationType = ReferenceEvent::PendingOperationSucceeded;
			newStatus = ObjectStatus::Success;
		}
		catch(Exception& ex) {
			// Transfer exception message to evaluation status.
			newStatus = ObjectStatus(ObjectStatus::Error, ex.messages().join(QChar('\n')));
		}
	}
	else {
		newStatus = ObjectStatus(ObjectStatus::Error, tr("Operation has been canceled by the user."));
	}

	// Reset everything.
	_backgroundOperationWatcher.unsetFuture();
	_backgroundOperation.reset();
	_computationValidity.setEmpty();

	// Set the new modifier status.
	setStatus(newStatus);

	// Notify dependents that the evaluation request was satisfied or not satisfied.
	notifyDependents(notificationType);
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void AsynchronousParticleModifier::saveToStream(ObjectSaveStream& stream)
{
	ParticleModifier::saveToStream(stream);
	stream.beginChunk(0x01);
	stream << (storeResultsWithScene() ? _cacheValidity : TimeInterval::empty());
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void AsynchronousParticleModifier::loadFromStream(ObjectLoadStream& stream)
{
	ParticleModifier::loadFromStream(stream);
	stream.expectChunk(0x01);
	stream >> _cacheValidity;
	stream.closeChunk();
}

};	// End of namespace