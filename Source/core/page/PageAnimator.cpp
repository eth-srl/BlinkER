// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "config.h"
#include "core/page/PageAnimator.h"

#include "core/animation/DocumentAnimations.h"
#include "core/frame/FrameView.h"
#include "core/frame/LocalFrame.h"
#include "core/page/Chrome.h"
#include "core/page/ChromeClient.h"
#include "core/page/Page.h"
#include "core/svg/SVGDocumentExtensions.h"
#include "platform/Logging.h"

namespace blink {

PageAnimator::PageAnimator(Page& page)
    : m_page(page)
    , m_animationFramePending(false)
    , m_servicingAnimations(false)
    , m_updatingLayoutAndStyleForPainting(false)
{
}

PassRefPtrWillBeRawPtr<PageAnimator> PageAnimator::create(Page& page)
{
    return adoptRefWillBeNoop(new PageAnimator(page));
}

void PageAnimator::trace(Visitor* visitor)
{
    visitor->trace(m_page);
}

void PageAnimator::serviceScriptedAnimations(double monotonicAnimationStartTime)
{
    RefPtrWillBeRawPtr<PageAnimator> protector(this);
    m_animationFramePending = false;
    TemporaryChange<bool> servicing(m_servicingAnimations, true);

    WillBeHeapVector<RefPtrWillBeMember<Document>> documents;
    for (Frame* frame = m_page->mainFrame(); frame; frame = frame->tree().traverseNext()) {
        if (frame->isLocalFrame())
            documents.append(toLocalFrame(frame)->document());
    }

    for (size_t i = 0; i < documents.size(); ++i) {
        if (documents[i]->frame()) {
            documents[i]->view()->serviceScrollAnimations(monotonicAnimationStartTime);

            if (const FrameView::ScrollableAreaSet* animatingScrollableAreas = documents[i]->view()->animatingScrollableAreas()) {
                // Iterate over a copy, since ScrollableAreas may deregister
                // themselves during the iteration.
                Vector<ScrollableArea*> animatingScrollableAreasCopy;
                copyToVector(*animatingScrollableAreas, animatingScrollableAreasCopy);
                for (ScrollableArea* scrollableArea : animatingScrollableAreasCopy)
                    scrollableArea->serviceScrollAnimations(monotonicAnimationStartTime);
            }
        }
    }

    for (size_t i = 0; i < documents.size(); ++i) {
        DocumentAnimations::updateAnimationTimingForAnimationFrame(*documents[i], monotonicAnimationStartTime);
        SVGDocumentExtensions::serviceOnAnimationFrame(*documents[i], monotonicAnimationStartTime);
    }

    for (size_t i = 0; i < documents.size(); ++i)
        documents[i]->serviceScriptedAnimations(monotonicAnimationStartTime);
}

void PageAnimator::scheduleVisualUpdate(LocalFrame* frame)
{
    // FIXME: also include m_animationFramePending here. It is currently not there due to crbug.com/353756.
    if (m_servicingAnimations || m_updatingLayoutAndStyleForPainting)
        return;
    // FIXME: The frame-specific version of scheduleAnimation() is for
    // out-of-process iframes. Passing 0 or the top-level frame to this method
    // causes scheduleAnimation() to be called for the page, which still uses
    // a page-level WebWidget (the WebViewImpl).
    if (frame && !frame->isMainFrame() && frame->isLocalRoot()) {
        m_page->chrome().scheduleAnimationForFrame(frame);
    } else {
        m_page->chrome().scheduleAnimation();
    }
}

void PageAnimator::updateLayoutAndStyleForPainting(LocalFrame* rootFrame)
{
    RefPtrWillBeRawPtr<FrameView> view = rootFrame->view();

    TemporaryChange<bool> servicing(m_updatingLayoutAndStyleForPainting, true);

    // In order for our child HWNDs (NativeWindowWidgets) to update properly,
    // they need to be told that we are updating the screen. The problem is that
    // the native widgets need to recalculate their clip region and not overlap
    // any of our non-native widgets. To force the resizing, call
    // setFrameRect(). This will be a quick operation for most frames, but the
    // NativeWindowWidgets will update a proper clipping region.
    view->setFrameRect(view->frameRect());

    // setFrameRect may have the side-effect of causing existing page layout to
    // be invalidated, so layout needs to be called last.
    view->updateLayoutAndStyleForPainting();
}

}
