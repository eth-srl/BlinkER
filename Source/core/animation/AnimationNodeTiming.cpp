// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "config.h"
#include "core/animation/AnimationNodeTiming.h"

#include "bindings/core/v8/UnionTypesCore.h"
#include "core/animation/Animation.h"
#include "core/animation/AnimationNode.h"
#include "platform/animation/TimingFunction.h"

namespace blink {

PassRefPtrWillBeRawPtr<AnimationNodeTiming> AnimationNodeTiming::create(AnimationNode* parent)
{
    return adoptRefWillBeNoop(new AnimationNodeTiming(parent));
}

AnimationNodeTiming::AnimationNodeTiming(AnimationNode* parent)
    : m_parent(parent)
{
}

double AnimationNodeTiming::delay()
{
    return m_parent->specifiedTiming().startDelay * 1000;
}

double AnimationNodeTiming::endDelay()
{
    return m_parent->specifiedTiming().endDelay * 1000;
}

String AnimationNodeTiming::fill()
{
    return Timing::fillModeString(m_parent->specifiedTiming().fillMode);
}

double AnimationNodeTiming::iterationStart()
{
    return m_parent->specifiedTiming().iterationStart;
}

double AnimationNodeTiming::iterations()
{
    return m_parent->specifiedTiming().iterationCount;
}

// This logic was copied from the example in bindings/tests/idls/TestInterface.idl
// and bindings/tests/results/V8TestInterface.cpp.
// FIXME: It might be possible to have 'duration' defined as an attribute in the idl.
// If possible, fix will be in a follow-up patch.
// http://crbug.com/240176
void AnimationNodeTiming::getDuration(String propertyName, DoubleOrString& returnValue)
{
    if (propertyName != "duration")
        return;

    if (std::isnan(m_parent->specifiedTiming().iterationDuration)) {
        returnValue.setString("auto");
        return;
    }
    returnValue.setDouble(m_parent->specifiedTiming().iterationDuration * 1000);
    return;
}

double AnimationNodeTiming::playbackRate()
{
    return m_parent->specifiedTiming().playbackRate;
}

String AnimationNodeTiming::direction()
{
    return Timing::playbackDirectionString(m_parent->specifiedTiming().direction);
}

String AnimationNodeTiming::easing()
{
    return m_parent->specifiedTiming().timingFunction->toString();
}

void AnimationNodeTiming::setDelay(double delay)
{
    Timing timing = m_parent->specifiedTiming();
    TimingInput::setStartDelay(timing, delay);
    m_parent->updateSpecifiedTiming(timing);
}

void AnimationNodeTiming::setEndDelay(double endDelay)
{
    Timing timing = m_parent->specifiedTiming();
    TimingInput::setEndDelay(timing, endDelay);
    m_parent->updateSpecifiedTiming(timing);
}

void AnimationNodeTiming::setFill(String fill)
{
    Timing timing = m_parent->specifiedTiming();
    TimingInput::setFillMode(timing, fill);
    m_parent->updateSpecifiedTiming(timing);
}

void AnimationNodeTiming::setIterationStart(double iterationStart)
{
    Timing timing = m_parent->specifiedTiming();
    TimingInput::setIterationStart(timing, iterationStart);
    m_parent->updateSpecifiedTiming(timing);
}

void AnimationNodeTiming::setIterations(double iterations)
{
    Timing timing = m_parent->specifiedTiming();
    TimingInput::setIterationCount(timing, iterations);
    m_parent->updateSpecifiedTiming(timing);
}

bool AnimationNodeTiming::setDuration(String name, double duration)
{
    if (name != "duration")
        return false;
    Timing timing = m_parent->specifiedTiming();
    TimingInput::setIterationDuration(timing, duration);
    m_parent->updateSpecifiedTiming(timing);
    return true;
}

void AnimationNodeTiming::setPlaybackRate(double playbackRate)
{
    Timing timing = m_parent->specifiedTiming();
    TimingInput::setPlaybackRate(timing, playbackRate);
    m_parent->updateSpecifiedTiming(timing);
}

void AnimationNodeTiming::setDirection(String direction)
{
    Timing timing = m_parent->specifiedTiming();
    TimingInput::setPlaybackDirection(timing, direction);
    m_parent->updateSpecifiedTiming(timing);
}

void AnimationNodeTiming::setEasing(String easing)
{
    Timing timing = m_parent->specifiedTiming();
    TimingInput::setTimingFunction(timing, easing);
    m_parent->updateSpecifiedTiming(timing);
}

void AnimationNodeTiming::trace(Visitor* visitor)
{
    visitor->trace(m_parent);
}

} // namespace blink
