/***************************************************************************
 * Copyright 1998-2015 by authors (see AUTHORS.txt)                        *
 *                                                                         *
 *   This file is part of LuxRender.                                       *
 *                                                                         *
 * Licensed under the Apache License, Version 2.0 (the "License");         *
 * you may not use this file except in compliance with the License.        *
 * You may obtain a copy of the License at                                 *
 *                                                                         *
 *     http://www.apache.org/licenses/LICENSE-2.0                          *
 *                                                                         *
 * Unless required by applicable law or agreed to in writing, software     *
 * distributed under the License is distributed on an "AS IS" BASIS,       *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*
 * See the License for the specific language governing permissions and     *
 * limitations under the License.                                          *
 ***************************************************************************/

#include <iostream>

#include "luxcoreapp.h"

using namespace std;
using namespace luxrays;
using namespace luxcore;

LogWindow *LuxCoreApp::currentLogWindow = NULL;

//------------------------------------------------------------------------------
// LuxCoreApp
//------------------------------------------------------------------------------

LuxCoreApp::LuxCoreApp(luxcore::RenderConfig *renderConfig) : statsWindow(this) {
	config = renderConfig;
	session = NULL;
	window = NULL;

	optRealTimeMode = false;
	optMouseGrabMode = false;
	optMoveScale = 1.f;
	optMoveStep = .5f;
	optRotateStep = 4.f;

	mouseButton0 = false;
	mouseButton2 = false;
	mouseGrabLastX = 0.0;
	mouseGrabLastY = 0.0;
	lastMouseUpdate = 0.0;
	
	currentLogWindow = &logWindow;
}

LuxCoreApp::~LuxCoreApp() {
}

void LuxCoreApp::LogHandler(const char *msg) {
	cout << msg << endl;

	if (currentLogWindow)
		currentLogWindow->AddMsg(msg);
}