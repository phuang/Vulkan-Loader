/**************************************************************************
 *
 * Copyright 2014 Valve Software. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *************************************************************************/
#pragma once
#include "glvdebug_trace_file_utils.h"
#include "glvdebug_view.h"

extern "C"
{
GLVTRACER_EXPORT glv_trace_packet_header* GLVTRACER_CDECL glvdebug_controller_interpret_trace_packet(glv_trace_packet_header* pHeader);

GLVTRACER_EXPORT bool GLVTRACER_CDECL glvdebug_controller_load_trace_file(glvdebug_trace_file_info* pTraceFileInfo, glvdebug_view* pView);

GLVTRACER_EXPORT void GLVTRACER_CDECL glvdebug_controller_unload_trace_file(void);

// entrypoints that must be exposed by each controller library
typedef glv_trace_packet_header* (GLVTRACER_CDECL *funcptr_glvdebug_controller_interpret_trace_packet)(glv_trace_packet_header* pHeader);
typedef bool (GLVTRACER_CDECL *funcptr_glvdebug_controller_load_trace_file)(glvdebug_trace_file_info* pTraceFileInfo, glvdebug_view* pView);
typedef void (GLVTRACER_CDECL *funcptr_glvdebug_controller_unload_trace_file)(void);

struct glvdebug_controller
{
    void* pLibrary;
    funcptr_glvdebug_controller_interpret_trace_packet InterpretTracePacket;
    funcptr_glvdebug_controller_load_trace_file LoadTraceFile;
    funcptr_glvdebug_controller_unload_trace_file UnloadTraceFile;
};

}
