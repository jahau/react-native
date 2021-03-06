/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TextInputShadowNode.h"

#include <react/attributedstring/AttributedStringBox.h>
#include <react/attributedstring/TextAttributes.h>
#include <react/core/LayoutConstraints.h>
#include <react/core/LayoutContext.h>
#include <react/core/conversions.h>

namespace facebook {
namespace react {

extern char const TextInputComponentName[] = "TextInput";

AttributedStringBox TextInputShadowNode::attributedStringBoxToMeasure() const {
  bool hasMeaningfulState = getState() && getStateData().revision != 0;

  if (hasMeaningfulState) {
    auto attributedStringBox = getStateData().attributedStringBox;
    if (attributedStringBox.getMode() ==
            AttributedStringBox::Mode::OpaquePointer ||
        !attributedStringBox.getValue().isEmpty()) {
      return getStateData().attributedStringBox;
    }
  }

  auto attributedString =
      hasMeaningfulState ? AttributedString{} : getAttributedString();

  if (attributedString.isEmpty()) {
    auto placeholder = getProps()->placeholder;
    // Note: `zero-width space` is insufficient in some cases (e.g. when we need
    // to measure the "hight" of the font).
    auto string = !placeholder.empty() ? placeholder : "I";
    auto textAttributes = getProps()->getEffectiveTextAttributes();
    attributedString.appendFragment({string, textAttributes, {}});
  }

  return AttributedStringBox{attributedString};
}

AttributedString TextInputShadowNode::getAttributedString() const {
  auto textAttributes = getProps()->getEffectiveTextAttributes();
  auto attributedString = AttributedString{};

  attributedString.appendFragment(
      AttributedString::Fragment{getProps()->text, textAttributes});

  attributedString.appendAttributedString(
      BaseTextShadowNode::getAttributedString(textAttributes, *this));
  return attributedString;
}

void TextInputShadowNode::setTextLayoutManager(
    TextLayoutManager::Shared const &textLayoutManager) {
  ensureUnsealed();
  textLayoutManager_ = textLayoutManager;
}

void TextInputShadowNode::updateStateIfNeeded() {
  ensureUnsealed();

  if (!getState() || getStateData().revision == 0) {
    auto state = TextInputState{};
    state.attributedStringBox = AttributedStringBox{getAttributedString()};
    state.paragraphAttributes = getProps()->paragraphAttributes;
    state.layoutManager = textLayoutManager_;
    state.revision = 1;
    setStateData(std::move(state));
  }
}

#pragma mark - LayoutableShadowNode

Size TextInputShadowNode::measure(LayoutConstraints layoutConstraints) const {
  return textLayoutManager_->measure(
      attributedStringBoxToMeasure(),
      getProps()->getEffectiveParagraphAttributes(),
      layoutConstraints);
}

void TextInputShadowNode::layout(LayoutContext layoutContext) {
  updateStateIfNeeded();
  ConcreteViewShadowNode::layout(layoutContext);
}

} // namespace react
} // namespace facebook
