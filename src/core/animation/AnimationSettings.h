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
 * \file AnimationSettings.h 
 * \brief Contains the definition of the Ovito::AnimationSettings class.
 */
 
#ifndef __OVITO_ANIMATION_SETTINGS_H
#define __OVITO_ANIMATION_SETTINGS_H

#include <core/Core.h>
#include <core/reference/RefTarget.h>
#include "TimeInterval.h"

namespace Ovito {

/**
 * \brief This class stores the animation settings for a scene.
 * 
 * Each DataSet has an instance of this class associated with it. It
 * can be accessed via DataSet::animationSettings().
 * 
 * To access the animation settings of the current scene you can also use
 * the methods of AnimManager.
 */
class OVITO_CORE_EXPORT AnimationSettings : public RefTarget
{
public:

	/// \brief Constructor that initializes the object with default values.
	/// \param dataset The context dataset.
	Q_INVOKABLE AnimationSettings(DataSet* dataset);

	/// \brief Gets the current animation time.
	/// \return The current time.
	/// 
	/// The state of the scene at this time is shown in the viewports.
	/// \sa setTime()
	TimePoint time() const { return _time; }

	/// \brief Sets the current animation time.
	/// \param time The new animation time.
	///
	/// The state of the scene at the given time will be shown in the viewports.
	/// \undoable
	/// \sa time()
	void setTime(TimePoint time) { _time = time; }

	/// \brief Gets the animation interval.
	/// \return The time interval of the animation.
	/// \sa setAnimationInterval() 
	const TimeInterval& animationInterval() const { return _animationInterval; }

	/// \brief Sets the animation interval.
	/// \param interval The new animation interval for the scene.
	/// \undoable
	/// \sa animationInterval()
	void setAnimationInterval(const TimeInterval& interval) { _animationInterval = interval; }
	
	/// \brief Returns the number of frames per second.
	/// \return The number of frames per second.
	/// 
	/// This setting controls the playback speed of the animation.
	///
	/// \sa setFramesPerSecond()
	int framesPerSecond() const { return TICKS_PER_SECOND / _ticksPerFrame; }

	/// \brief Sets the number of frames per second.
	/// \param fps The number of frames per second. Please note that not all
	///            values are allowed here because time is measured in integer ticks units.
	/// \undoable
	/// 
	/// This setting controls the playback speed of the animation.
	///
	/// \sa setFramesPerSecond()
	/// \sa framesPerSecond()
	/// \sa setTicksPerFrame()
	void setFramesPerSecond(int fps) { setTicksPerFrame(TICKS_PER_SECOND / fps); } 	

	/// \brief Returns the number of time ticks per frame.
	/// \return The number of time ticks per animation frame. One tick is 1/4800 of a second.
	/// 
	/// This setting controls the playback speed of the animation.
	///
	/// \sa setTicksPerFrame()
	int ticksPerFrame() const { return _ticksPerFrame; }

	/// \brief Sets the number of time ticks per frame.
	/// \param ticksPerFrame The number of time tick units per animation frame.
	///                      This must be a positive value.
	/// \undoable
	/// 
	/// This setting controls the playback speed of the animation.
	/// \sa ticksPerFrame()
	void setTicksPerFrame(int ticksPerFrame) { _ticksPerFrame = ticksPerFrame; }

	/// \brief Converts an animation frame number to a time value.
	/// \param frame A frame number starting at 0.
	/// \return The animation time at which the animation frame begins.
	TimePoint frameToTime(int frame) const { return frame * ticksPerFrame(); }

	/// \brief Converts a time value to an animation frame number.
	/// \param time A time in ticks units.
	/// \return The animation frame that corresponds to the given time.
	int timeToFrame(TimePoint time) const { return time / ticksPerFrame(); }

	/// \brief Returns the playback speed factor that is used for animation playback in the viewports.
	/// \return The playback speed factor. A value greater than 1 means that the animation is played at a speed higher
	///         than realtime whereas a value smaller than -1 means that the animation is played at a speed lower than realtime.
	/// \sa setPlaybackSpeed()
	int playbackSpeed() const { return _playbackSpeed; }

	/// \brief Sets the playback speed factor that is used for animation playback in the viewport.
	/// \param factor A value greater than 1 means that the animation is played at a speed higher
	///               than realtime. A value smaller than -1 that the animation is played at a speed lower than realtime.
	/// \undoable
	/// \sa playbackSpeed()
	void setPlaybackSpeed(int factor) { _playbackSpeed = factor; }
	
    /// \brief Returns the list of names assigned to animation frames.
    const QMap<int,QString>& namedFrames() const { return _namedFrames; }

    /// \brief Clears all names assigned to animation frames.
    void clearNamedFrames() { _namedFrames.clear(); }

    /// \brief Assigns a name to an animation frame.
    void assignFrameName(int frameIndex, const QString& name) { _namedFrames[frameIndex] = name; }

	/// \brief Returns whether animation recording is active and animation keys should be automatically generated.
	/// \return \c true if animating is currently turned on and not suspended; \c false otherwise.
	///
	/// When animating is turned on, controllers should automatically set keys when their value is changed.
	bool isAnimating() const { return autoKeyMode() && _animSuspendCount == 0; }

	/// \brief Returns whether Auto Key mode is active.
	/// \return \c true if the automatic generation of keys has been enabled.
	/// \note The automatic generation of animation keys may be suspended by a call to suspendAnim().
	///       This overrides the Auto Key mode. Controllers should use isAnimating() to check whether
	///       have to generate a key whenever their values is changed.
	bool autoKeyMode() const { return _autoKeyMode; }

	/// \brief Suspends the animation mode temporarily.
	///
	/// Automatic generation of animation keys is suspended by this method until a call to resumeAnim().
	/// If suspendAnim() is called multiple times then resumeAnim() must be called the same number of
	/// times until animation mode is enabled again.
	///
	/// It is recommended to use the AnimationSuspender helper class to suspend animation mode because
	/// this is more exception save than the suspendAnim()/resumeAnim() combination.
	void suspendAnim() { _animSuspendCount++; }

	/// \brief Resumes the automatic generation of animation keys.
	///
	/// This re-enables animation mode after it had
	/// been suspended by a call to suspendAnim().
	void resumeAnim() {
		OVITO_ASSERT_MSG(_animSuspendCount > 0, "AnimationSettings::resumeAnim()", "resumeAnim() has been called more often than suspendAnim().");
		_animSuspendCount--;
	}

	/// \brief Converts a time value to its string representation.
	/// \param time Some time value.
	/// \return A human-readable representation of the time value.
	QString timeToString(TimePoint time);

	/// \brief Converts a string to a time value.
	/// \param stringValue The  human-readable representation of a time value.
	/// \return The parsed time value.
	/// \throw Exception when a parsing error occurs.
	TimePoint stringToTime(const QString& stringValue);

	/// \brief Indicates whether the animation has recently been changed, and the scene
	///        is still being prepared for display of the new frame.
	bool isTimeChanging() const { return _timeIsChanging != 0; }

public Q_SLOTS:

	/// \brief Enables or disables animation mode (i.e. automatic creation of animation keys).
	/// \param on Flag indicating whether Auto Key mode should turned on or off.
	/// \note The automatic generation of animation keys may be temporarily suspended by a call to suspendAnim()
	///       even if Auto Key is active. Controllers should use isAnimating() to check whether
	///       they have to generate a key when their value is changed.
	void setAutoKeyMode(bool on);

	/// \brief  Sets the current animation time to the start of the animation interval.
	void jumpToAnimationStart();

	//// \brief Sets the current animation time to the end of the animation interval.
	void jumpToAnimationEnd();

	/// \brief Jumps to the next animation frame.
	void jumpToNextFrame();

	/// \brief Jumps to the previous animation frame.
	void jumpToPreviousFrame();

	/// \brief Starts playback of the animation in the viewports.
	void startAnimationPlayback();

	/// \brief Stops playback of the animation in the viewports.
	void stopAnimationPlayback();

private Q_SLOTS:

	/// \brief Is called when the current animation time has changed.
	void onTimeChanged(TimePoint newTime);

	/// \brief Is called whenever the active animation interval has changed.
	void onIntervalChanged(TimeInterval newAnimationInterval);

	/// \brief Timer callback used during animation playback.
	void onPlaybackTimer();

protected:

	/// \brief Is called when the value of a non-animatable property field of this RefMaker has changed.
	virtual void propertyChanged(const PropertyFieldDescriptor& field) override;

	/// \brief Saves the class' contents to an output stream.
	virtual void saveToStream(ObjectSaveStream& stream) override;

	/// \brief Loads the class' contents from an input stream.
	virtual void loadFromStream(ObjectLoadStream& stream) override;

	/// \brief Creates a copy of this object.
	virtual OORef<RefTarget> clone(bool deepCopy, CloneHelper& cloneHelper) override;

Q_SIGNALS:

	/// This signal is emitted by this object when its current animation time has changed.
	void timeChanged(TimePoint newTime);
	
	/// This signal is emitted by this object when its animation interval has changed.
	void intervalChanged(TimeInterval newAnimationInterval);
	
	/// This signal is emitted by this object when its animation speed has changed.
	void speedChanged(int ticksPerFrame);
	
	/// This signal is emitted by the AnimManager when the time to string conversion format has changed.
	void timeFormatChanged();

	/// This signal is emitted when the Auto Key mode has been activated or deactivated.
	void autoKeyModeChanged(bool active);

private:

	/// The current animation time.
    PropertyField<TimePoint> _time;

	/// The start and end times of the animation.
    PropertyField<TimeInterval> _animationInterval;

	/// The number of time ticks per frame.
	/// This controls the animation speed.
    PropertyField<int> _ticksPerFrame;
	
	/// The playback speed factor that is used for animation playback in the viewport.
	/// A value greater than 1 means that the animation is played at a speed higher
	/// than realtime.
	/// A value smaller than -1 that the animation is played at a speed lower than realtime.	
    PropertyField<int> _playbackSpeed;

    /// List of names assigned to animation frames.
    QMap<int,QString> _namedFrames;

	/// Counts the number of times the animation modes has been suspended.
	int _animSuspendCount;

	/// Indicates whether animation recording mode is active.
	bool _autoKeyMode;

	/// Indicates that the animation has been changed, and the scene is still being prepared for display of the new frame.
	int _timeIsChanging;

	/// Indicates that the animation is currently being played back in the viewports.
	bool _isPlaybackActive;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_time);
	DECLARE_PROPERTY_FIELD(_animationInterval);
	DECLARE_PROPERTY_FIELD(_ticksPerFrame);
	DECLARE_PROPERTY_FIELD(_playbackSpeed);
};

/**
 * \brief A small helper object that suspends generation of animation keys while it
 *        exists. It can be used to make your code exception-safe.
 *
 * The constructor of this class calls AnimationSettings::suspendAnim() and
 * the destructor calls AnimationSettings::resumeAnim().
 */
class AnimationSuspender {
public:
	AnimationSuspender(AnimationSettings* animSettings) : _animSettings(*animSettings) { _animSettings.suspendAnim(); }
	~AnimationSuspender() { _animSettings.resumeAnim(); }
private:
	AnimationSettings& _animSettings;
};

};

#endif // __OVITO_ANIMATION_SETTINGS_H
