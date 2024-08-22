//
//  JGL_RadioButtonGroup.hpp
//  TextilePatternGeneration
//
//  Created by Hyun Joon Shin on 2020/09/25.
//  Copyright © 2020 Hyun Joon Shin. All rights reserved.
//

#ifndef JGL_RadioButtonGroup_hpp
#define JGL_RadioButtonGroup_hpp

#ifdef __APPLE__
#pragma GCC visibility push(default)
#endif

#include "JGL/JGL_ButtonGroup.hpp"
#include "JGL/JGL__Valued.hpp"

namespace JGL {

struct RadioButtonGroup: public ButtonGroup, public _Valued<int> {
	RadioButtonGroup(float x,float y,float w,float h, const std::string& label="" );
	RadioButtonGroup(const glm::vec2& pos, const glm::dim2& sz, const std::string& label="" );
	virtual void	end() override;
	virtual void	add(Widget* w) override;

	virtual int		value() const override { return _selected; }
	virtual void	value(const int& k) override;

protected:
	virtual void buttonValueChanged( Widget* );
	static  void buttonValueChangedCB( Widget*, void* );

	int		_selected = 0;
};

} // namespace JGL

#ifdef __APPLE__
#pragma GCC visibility pop
#endif

#endif /* JGL_RadioButtonGroup_hpp */
