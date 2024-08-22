//
//  JGL_Check.hpp
//  TextilePatternGeneration
//
//  Created by Hyun Joon Shin on 2020/10/02.
//  Copyright © 2020 Hyun Joon Shin. All rights reserved.
//

#ifndef JGL_Check_hpp
#define JGL_Check_hpp

#ifdef __APPLE__
#pragma GCC visibility push(default)
#endif

#include "JGL/JGL_Property.hpp"
#include "JGL/JGL__Targettable.hpp"
#include "JGL/JGL__Valued.hpp"

namespace JGL {

struct Check: public Property, public _Targettable, public _Valued<bool> {
	Check(float x, float y, float w, float h, const std::string& label="" );
	Check(float x, float y, float w, const std::string& label="" );
	Check(const glm::vec2& pos, const glm::dim2& sz, const std::string& label="" );
	Check(const glm::vec2& pos, float w, const std::string& label="" );

	virtual bool		value() const override { return _checked; }
	virtual void		value(const bool& i) override;
	
protected:
	virtual bool 		handle(int event) override;

	virtual glm::dim2	minActionSize(NVGcontext*vg) const override;

	virtual void		drawAction(NVGcontext* vg, const glm::rect& r, int alignment) override;
	virtual int			shadeState() const override;

	bool	_checked = false;
	bool	_pressed = false;
};

} // namespace JGL

#ifdef __APPLE__
#pragma GCC visibility pop
#endif

#endif /* JGL_Check_hpp */
