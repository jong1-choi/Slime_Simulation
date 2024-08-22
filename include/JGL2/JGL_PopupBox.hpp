//
//  JGL_PopupBox.hpp
//  TextilePatternGeneration
//
//  Created by Hyun Joon Shin on 2020/09/26.
//  Copyright © 2020 Hyun Joon Shin. All rights reserved.
//

#ifndef JGL_PopupBox_hpp
#define JGL_PopupBox_hpp

#ifdef __APPLE__
#pragma GCC visibility push(default)
#endif

#include "JGL/JGL_Group.hpp"

namespace JGL {

struct PopupBox : Group {
	PopupBox(float w,float h, const std::string& label="");
	PopupBox(const glm::dim2& sz, const std::string& label="");

	static glm::vec2		popupPos(Widget* popup, Widget* parent);
	// TODO: Resizable popupbox
protected:
	virtual void	drawBox(NVGcontext* vg, const glm::rect& r ) override;
	virtual void	drawBoxOver(NVGcontext* vg, const glm::rect& r ) override;
};


} // namespace JGL

#ifdef __APPLE__
#pragma GCC visibility pop
#endif

#endif /* JGL_PopupBox_hpp */
