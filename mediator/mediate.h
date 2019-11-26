/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#ifndef _MEDIATE_H_GUARD
#define _MEDIATE_H_GUARD

//----------------------------------------------------------
// Overview
//----------------------------------------------------------
//
// From the perspective of the interface it does not matter whether the user interface
// is a GUI or a command-line application. We use 'GUI' as a general term to refer to
// the user interface. To refer to the core DOAS functionality we use the term 'engine'.
//
// The engine is responsible for all file-level operations on data files (spectral
// records, references, slit functions, instrument response functions, cross-sections,
// etc.)
// The GUI is responsible for all file level operations on configuration files. Any
// file accessed by the engine will be specified with the full filesystem path if
// provided by the GUI.
//
// A software layer will manage all interacton between the GUI and the engine. This
// will be a C language function, struct and native data type level interface. We
// will refer to this layer as the 'mediator'.
//
// Since the GUI/engine will typically be C++/C respectively, and different dynamic
// memory management schemes are standard in these languages, the mediator layer
// will have a 'data-copy' policy. Responsibility for freeing dynamically allocated
// data will NEVER pass through the mediator from GUI to engine or visa versa.
//
// The data structures in the interface will NOT include any data types specific to
// either the GUI or the engine. It must be possible to compile the GUI code without
// any reference to the engine header files. Likewise, it must be possible to compile
// the engine code without any reference to the GUI header files.
//
// The functions in the mediator will be separated into two groups. The first group
// will provide the GUI with an interface to control the activities of the engine.
// The second group will provide the engine with an interface to return data to the
// GUI. The first group will have names that begin with 'mediateRequest'. The second
// group will have names that begin with 'mediateResponse'.
// The implementation of the 'mediateResponse*' functions will be associated with the
// GUI. The implementation is the 'mediateRequest*' functions will be associated with
// the engine. Since the control of the engine is driven by the GUI, the implementation
// of the 'mediateRequest*' functions will typically involve calls to 'mediateResponse*'.
// The mediateRequest* and mediateResponse* functions will always include the argument
// responseHandle. This provides a context for the GUI and the implementation of the
// mediateRequest functions must use this argument blindly. The mediateRequest* functions
// always include the argument engineContext. This is to provide context for the engine
// and the caller (GUI) of the mediateRequest* functions must use this blindly.
//
// (NOTE: While only one engine instance is being used at a time, and since the current
// engine implementation is heavlily dependent on global variables, the engine context is
// not likely to be used by the current DOAS engine beyond ensuring that only one context
// is every requested. However, making it part of the interface now provides some level
// of future-proofing for the interface.


//----------------------------------
// request interface - called by GUI
//----------------------------------
#include "mediate_request.h"

//--------------------------------------
// response interface - called by engine
//--------------------------------------
#include "mediate_response.h"

#endif
