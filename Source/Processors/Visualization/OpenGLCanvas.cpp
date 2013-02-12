/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "OpenGLCanvas.h"
#include <stdio.h>
#include <math.h>

OpenGLCanvas::OpenGLCanvas() : //OpenGLComponent(OpenGLComponent::OpenGLType::openGLDefault, true),
	scrollPix(0), animationIsActive(false), refreshMs(5000),
	scrollBarWidth(15), scrollDiff(0), originalScrollPix(0),
	scrollTime(0), showScrollTrack(true), PI(3.1415926)
{
	loadFonts();
}

OpenGLCanvas::~OpenGLCanvas()
{
	
}

void OpenGLCanvas::setUp2DCanvas()
{
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity();

	glOrtho (0, 1, 1, 0, 0, 1);
	
	glEnable(GL_TEXTURE_2D);
}

void OpenGLCanvas::activateAntiAliasing()
{

	// disable everything we don't need
	glDisable(GL_FOG);
	glDisable(GL_LIGHTING);
	glDisable(GL_DITHER);
	glDisable(GL_CULL_FACE);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);

	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glShadeModel(GL_FLAT);
}

void OpenGLCanvas::setClearColor(int colorCode)
{

	switch (colorCode)
	{
		case white:
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			break;
		case black:
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			break;
		case lightgrey:
			glClearColor (0.667f, 0.698f, 0.718f, 1.0f);
			break;
		case darkgrey:
			glClearColor (0.23f, 0.23f, 0.23f, 1.0f);
			break;
		default:
			glClearColor (0.23f, 0.23f, 0.23f, 1.0f);
	}

}


void OpenGLCanvas::loadFonts()
{

	const unsigned char* buffer = reinterpret_cast<const unsigned char*>(BinaryData::misoregular_ttf);
	size_t bufferSize = BinaryData::misoregular_ttfSize;

	FTPixmapFont* font1 = new FTPixmapFont(buffer, bufferSize);
	
	fontList.add(font1);

	buffer = reinterpret_cast<const unsigned char*>(BinaryData::misobold_ttf);
	bufferSize = BinaryData::misobold_ttfSize;

	FTPixmapFont* font2 = new FTPixmapFont(buffer, bufferSize);
	
	fontList.add(font2);

	buffer = reinterpret_cast<const unsigned char*>(BinaryData::misolight_ttf);
	bufferSize = BinaryData::misolight_ttfSize;

	FTPixmapFont* font3 = new FTPixmapFont(buffer, bufferSize);
	
	fontList.add(font3);

	buffer = reinterpret_cast<const unsigned char*>(BinaryData::BebasNeue_otf);
	bufferSize = BinaryData::BebasNeue_otfSize;

	FTPixmapFont* font4 = new FTPixmapFont(buffer, bufferSize);
	
	fontList.add(font4);

	buffer = reinterpret_cast<const unsigned char*>(BinaryData::ostrich_ttf);
	bufferSize = BinaryData::ostrich_ttfSize;

	FTPixmapFont* font5 = new FTPixmapFont(buffer, bufferSize);
	
	fontList.add(font5);

	buffer = reinterpret_cast<const unsigned char*>(BinaryData::cpmono_extra_light_otf);
	bufferSize = BinaryData::cpmono_extra_light_otfSize;

	FTPixmapFont* font6 = new FTPixmapFont(buffer, bufferSize);
	
	fontList.add(font6);

	buffer = reinterpret_cast<const unsigned char*>(BinaryData::cpmono_light_otf);
	bufferSize = BinaryData::cpmono_light_otfSize;

	FTPixmapFont* font7 = new FTPixmapFont(buffer, bufferSize);
	
	fontList.add(font7);

	buffer = reinterpret_cast<const unsigned char*>(BinaryData::cpmono_plain_otf);
	bufferSize = BinaryData::cpmono_plain_otfSize;

	FTPixmapFont* font8 = new FTPixmapFont(buffer, bufferSize);
	
	fontList.add(font8);

	buffer = reinterpret_cast<const unsigned char*>(BinaryData::cpmono_bold_otf);
	bufferSize = BinaryData::cpmono_bold_otfSize;

	FTPixmapFont* font9 = new FTPixmapFont(buffer, bufferSize);
	
	fontList.add(font9);

	buffer = reinterpret_cast<const unsigned char*>(BinaryData::nordic_ttf);
	bufferSize = BinaryData::nordic_ttfSize;

	FTPixmapFont* font10 = new FTPixmapFont(buffer, bufferSize);
	
	fontList.add(font10);

	buffer = reinterpret_cast<const unsigned char*>(BinaryData::silkscreen_ttf);
	bufferSize = BinaryData::silkscreen_ttfSize;

	FTPixmapFont* font11 = new FTPixmapFont(buffer, bufferSize);
	
	fontList.add(font11);

}

FTPixmapFont* OpenGLCanvas::getFont(int fontCode)
{

	return fontList[fontCode];

	//  if (fontCode == miso_regular)
	//  	return fontList[0];
	// else if (fontCode == miso_bold)
	// 	return fontList[1];
	// else if (fontCode == miso_light)
	// 	return fontList[2];
	// else if (fontCode == bebas_neue)
	// 	return fontList[3];
	// else if (fontCode == ostrich)
	// 	return fontList[4];
	// else if (fontCode == cpmono_extra_light)
	// 	return fontList[5];
	// else if (fontCode == cpmono_light)
	// 	return fontList[6];
	// else if (fontCode == cpmono_plain)
	// 	return fontList[7];
	// else if (fontCode == cpmono_bold)
	// 	return fontList[8];
	// else if (fontCode == nordic)
	// 	return fontList[9];
	// else if (fontCode == silkscreen)
	// 	return fontList[10];
	// else
	// 	return fontList[0]; // miso-regular is default font

}

void OpenGLCanvas::startCallbacks()
{
	startTimer(refreshMs);
	animationIsActive = true;
}

void OpenGLCanvas::stopCallbacks()
{
	stopTimer();
	animationIsActive = false;
}


void OpenGLCanvas::drawScrollBars()
{

	//std::cout << "Drawing scroll bars" << std::endl;
	
	float scrollBarY = float(getHeight())/float(getTotalHeight());
	float timeSinceScroll = timer.getMillisecondCounter()-scrollTime;
	
	if (scrollBarY < 1.0f && timeSinceScroll < 1300)
	{
		float alpha;

		if (timeSinceScroll < 1000)
			alpha = 1.0f;
		else
			alpha = 1.0f*(1-float(timeSinceScroll-1000)/300.0f);

		float Yoffset = float(scrollPix)/float(getTotalHeight());

		if (showScrollTrack)
			drawScrollBar(0.995f, 2.0f/getHeight(), alpha*0.2f);

		scrollBarBottom = scrollBarY + Yoffset - 2.0f/getHeight();
		scrollBarTop = Yoffset + 2.0f/getHeight();
		
		drawScrollBar(scrollBarBottom, scrollBarTop, alpha*0.5f);

	} else {
		if (!animationIsActive) {
			stopTimer(); 
		}
		showScrollTrack = false;
	}

}

void OpenGLCanvas::drawScrollBar(float y1, float y2, float alpha)
{

	glViewport(0, getFooterHeight(),
		       getWidth(),
		       getHeight()-getHeaderHeight()-getFooterHeight());

	float x1 = (getWidth()-8.0f)/getWidth();
	float x2 = (getWidth()-2.0f)/getWidth();

	glColor4f(0.0f, 0.0f, 0.0f, alpha);

	glBegin(GL_POLYGON);

	glVertex2f(x1,y1);
	glVertex2f(x2,y1);
	glVertex2f(x2,y2);
	glVertex2f(x1,y2);

	glEnd();

}

void OpenGLCanvas::showScrollBars()
{
	scrollTime = timer.getMillisecondCounter();
	startTimer(refreshMs);
}

void OpenGLCanvas::drawRoundedRect(float x,
 						   		  float y,
 						   		  float w,
 						   		  float h,
 						   		  float r,
 						   		  int n)
{
	//glLineWidth(3.0);
	GLint* params = new GLint[4];

	glGetIntegerv(GL_VIEWPORT, params);

	//std::cout << params[0] << " " << params[1] << " " << params[2] << " "
	//	      << params[3] << std::endl;

	float ratio = float(params[3])/float(params[2]);

	//std::cout << ratio << std::endl;

	glBegin(GL_LINE_LOOP);
	
	for (int side = 0; side < 4; side++)
	{
		float x0[2];
		float y0[2];
		float origin[2];
		float angle[2];
		
		switch (side) {
			case 0:
				x0[0] = 0; x0[1] = 0;
				y0[0] = r; y0[1] = h-r;
				origin[0] = r*ratio; origin[1] = h-r;
				angle[0] = PI/2; angle[1] = PI;
				break;

			case 1:
				x0[0] = r*ratio; x0[1] = w-r*ratio;
				y0[0] = h; y0[1] = h;
				origin[0] = w-r*ratio; origin[1] = h-r;
				angle[0] = 0; angle[1] = PI/2;
				break;

			case 2:
				x0[0] = w; x0[1] = w;
				y0[0] = h-r; y0[1] = r;
				origin[0] = w-r*ratio; origin[1] = r;
				angle[0] = 3*PI/2; angle[1] = 2*PI;
				break;

			case 3:
				x0[0] = w-r*ratio; x0[1] = r*ratio;
				y0[0] = 0; y0[1] = 0;
				origin[0] = r*ratio; origin[1] = r;
				angle[0] = PI; angle[1] = 3*PI/2;
				break;

			default:
				break;
		}

		//glLineWidth(2.0);
		glVertex2f(x0[0]+x,y0[1]+y);
		glVertex2f(x0[1]+x,y0[1]+y);

		//glLineWidth(1.0);
		for (float a = angle[1]; a > angle[0]; a -= (PI/2)/n)
		{
			glVertex2f(cos(a)*r*ratio+origin[0]+x,
			           sin(a)*r+origin[1]+y);
		}

	}

	glEnd();

}

void OpenGLCanvas::mouseMove(const MouseEvent& e)
{
	if (getTotalHeight() > getHeight()) {

		Point<int> pos = e.getPosition();
		int xcoord = pos.getX();
		if (xcoord > getWidth() - scrollBarWidth)
		{
			showScrollTrack = true; showScrollBars();
		}
	}

	mouseMoveInCanvas(e);
}

void OpenGLCanvas::mouseDown(const MouseEvent& e)
{

	if (getTotalHeight() > getHeight()) {

	Point<int> pos = e.getPosition();
	int xcoord = pos.getX();

	if (xcoord > getWidth()-scrollBarWidth) {

		int ycoord = pos.getY();

		float targetPoint = float(ycoord)/float(getHeight());

		if (targetPoint < scrollBarTop && targetPoint < scrollBarTop)
		{

			scrollPix = int(float(ycoord)/float(getHeight())*float(getTotalHeight()));

		} else if (targetPoint > scrollBarBottom && targetPoint > scrollBarBottom) {
			
			scrollPix = int(float(ycoord)/float(getHeight())*float(getTotalHeight())) -
						(scrollBarBottom-scrollBarTop)*float(getTotalHeight());

		}
		
		showScrollTrack = true;
		showScrollBars();
	}
	}

	mouseDownInCanvas(e);
}

void OpenGLCanvas::mouseDrag(const MouseEvent& e)
{

	if (getTotalHeight() > getHeight()) {
	if (e.getMouseDownX() > getWidth()-scrollBarWidth)
	{

		if (float(e.getMouseDownY()/float(getHeight())) > scrollBarTop &&
		    float(e.getMouseDownY()/float(getHeight())) < scrollBarBottom)
		{

			if (scrollDiff == 0)
			{
				originalScrollPix = scrollPix;
				scrollDiff = 1;
			}

		}

		if (scrollDiff == 1)
		{
			scrollPix = originalScrollPix + 
				float(e.getDistanceFromDragStartY())/float(getHeight())
				* float(getTotalHeight());

			if (scrollPix < 0)
				scrollPix = 0;
			
			if (scrollPix + getHeight() > getTotalHeight())
				scrollPix = getTotalHeight() - getHeight();
	
			scrollTime = timer.getMillisecondCounter();
			showScrollTrack = true;
			repaint();
		} 
	}
	}

	mouseDragInCanvas(e);
}

void OpenGLCanvas::mouseUp(const MouseEvent& e)
{
	scrollDiff = 0;

	mouseUpInCanvas(e);
}

void OpenGLCanvas::mouseWheelMove(const MouseEvent&e,
                                      float wheelIncrementX, float wheelIncrementY)

{
	if (getTotalHeight() > getHeight()) {

		if (wheelIncrementY > 0)
		{
			if (scrollPix + getHeight() < getTotalHeight())
			{
				scrollPix += int(100.0f*wheelIncrementY);
				if (scrollPix + getHeight() > getTotalHeight())
					scrollPix = getTotalHeight() - getHeight();
			}
		} else if (wheelIncrementY < 0)
		{
			if (scrollPix > 0)
			{
				scrollPix += int(100.0f*wheelIncrementY);
				if (scrollPix < 0)
					scrollPix = 0;
			}
		}

		repaint();

		showScrollBars();

	}

	mouseWheelMoveInCanvas(e, wheelIncrementX, wheelIncrementY);

}

void OpenGLCanvas::timerCallback()
{
	repaint();
}


void OpenGLCanvas::resized()
{

	if (scrollPix + getHeight() > getTotalHeight() && getTotalHeight() > getHeight())
		scrollPix = getTotalHeight() - getHeight();
	else
		scrollPix = 0;

	showScrollBars();

	canvasWasResized();
}
