/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * X11 RAIL
 *
 * Copyright 2011 Marc-Andre Moreau <marcandre.moreau@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FREERDP_CLIENT_ANDROID_RAIL_H
#define FREERDP_CLIENT_ANDROID_RAIL_H

#include "android_freerdp.h"

#include <freerdp/client/rail.h>

int android_rail_init(androidContext* afc, RailClientContext* rail);
int android_rail_uninit(androidContext* afc, RailClientContext* rail);
FREERDP_LOCAL UINT android_rail_open_app_cmd(RailClientContext* context, char* app, char* cmd);
FREERDP_LOCAL UINT android_rail_send_window_event(RailClientContext* context, UINT32 winId, UINT32 cmdId);
#endif /* FREERDP_CLIENT_X11_RAIL_H */
