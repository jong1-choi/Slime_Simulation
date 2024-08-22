//
//  AnimView.hpp
//  SpringMass
//
//  Created by Hyun Joon Shin on 2021/05/12.
//

#ifndef AnimView_h
#define AnimView_h

#include "ModelView.hpp"

struct AnimView: ModelView {
	float lastT = 0;
	bool animating = false;
	
	std::function<void()> initFunction=[](){};
	std::function<void(char)> controlFunction = [](char) {};
	std::function<void(float)> frameFunction = [](float){};
    std::function<void(int)> keyFunction = [](int){};

    
	AnimView(float x, float y, float w, float h, const std::string& name="")
	: ModelView(x,y,w,h,name){}
	
	bool handle(int e) override {
		if( e == JGL::EVENT_KEYDOWN ) {
            keyFunction(JGL::_JGL::eventKey());
			if( JGL::_JGL::eventKey() == '1' ) {
				animating = !animating;
				if( animating ) {
					lastT = glfwGetTime();
					animate();
				}
				return true;
			}
			else if( JGL::_JGL::eventKey() == '0' ) {
				animating = false;
				initFunction();
				redraw();
				return true;
			}
			else if (JGL::_JGL::eventKey() == ' ') {
				controlFunction(' ');
				return true;
			}
			else if (JGL::_JGL::eventKey() == 87) {
				controlFunction(87);
				return true;
			}
			else if (JGL::_JGL::eventKey() == 65) {
				controlFunction(65);
				return true;
			}
			else if (JGL::_JGL::eventKey() == 83) {
				controlFunction(83);
				return true;
			}
			else if (JGL::_JGL::eventKey() == 68) {
				controlFunction(68);
				return true;
			}
			else if (JGL::_JGL::eventKey() == '2') {
				controlFunction('2');
				return true;
			}
		}
		return ModelView::handle( e );
	}
	virtual void drawContents(NVGcontext* vg, const glm::rect&r, int a ) override {
		if( animating ) {
			float t = glfwGetTime();
			frameFunction(t-lastT);
			animate();
			lastT = t;
		}
	}
};

#endif /* AnimView_h */
