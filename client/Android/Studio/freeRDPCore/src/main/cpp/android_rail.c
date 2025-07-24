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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <winpr/wlog.h>
#include <winpr/print.h>

#include "android_freerdp.h"

#define TAG CLIENT_TAG("android.rail")

static const char* error_code_names[] = { "RAIL_EXEC_S_OK",
	                                      "RAIL_EXEC_E_HOOK_NOT_LOADED",
	                                      "RAIL_EXEC_E_DECODE_FAILED",
	                                      "RAIL_EXEC_E_NOT_IN_ALLOWLIST",
	                                      "RAIL_EXEC_E_FILE_NOT_FOUND",
	                                      "RAIL_EXEC_E_FAIL",
	                                      "RAIL_EXEC_E_SESSION_LOCKED" };

/* RemoteApp Virtual Channel Extension */

void android_rail_enable_remoteapp_mode(androidContext* afc)
{
	if (!afc->remote_app)
	{
		afc->remote_app = TRUE;
	}
}

void android_rail_disable_remoteapp_mode(androidContext* afc)
{
	if (afc->remote_app)
	{
		afc->remote_app = FALSE;
	}
}

/**
 * Function description
 *
 * @return 0 on success, otherwise a Win32 error code
 */
static UINT android_rail_server_execute_result(RailClientContext* context,
                                               const RAIL_EXEC_RESULT_ORDER* execResult)
{
	androidContext* afc = (androidContext*)context->custom;

	#if 0
	if (execResult->execResult == RAIL_EXEC_E_MOUNTED_FAILED)
	{
		WLog_ERR(TAG, "RAIL exec error: execResult=%s appType=0x%d\n",
				error_code_names[execResult->execResult], execResult->rawResult);
		//freerdp_callback("OnDeleteOptimg", "(JI)V", (jlong)((rdpContext*)afc)->instance, execResult->rawResult);
	}
	else 
	#endif

	if (execResult->execResult != RAIL_EXEC_S_OK)
	{
		WLog_ERR(TAG, "RAIL exec error: execResult=%s NtError=0x%X\n",
			error_code_names[execResult->execResult], execResult->rawResult);
		freerdp_abort_connect(afc->common.context.instance);
	}
	else
	{
		android_rail_enable_remoteapp_mode(afc);
	}

	return CHANNEL_RC_OK;
}

/**
 * Function description
 *
 * @return 0 on success, otherwise a Win32 error code
 */
static UINT android_rail_server_system_param(RailClientContext* context,
                                             const RAIL_SYSPARAM_ORDER* sysparam)
{
	// androidContext* afc = (androidContext*)context->custom;

	// WLog_ERR(TAG, "sysparam.param = %d\n", sysparam->param);
	// WLog_ERR(TAG, "MinimizedRequested = %d, appType = %d\n", sysparam->setMinimized, sysparam->appType);
	// WLog_ERR(TAG, "OpenwpsRequested = %d\n", sysparam->setOpenwps);
	// if (sysparam->setMinimized == 1) {
	// 	// WLog_ERR(TAG, "MinimizedRequested = %d\n", sysparam.param);
	// 	freerdp_callback("OnMinimizeRequested", "(JIZ)V", (jlong)((rdpContext*)afc)->instance, sysparam->appType, true);
	// }
	// if (sysparam->setOpenwps == 1) {
	// 	freerdp_callback("OnOpenwpsRequested", "(JZ)V", (jlong)((rdpContext*)afc)->instance, true);
	// }
	// TODO: Actually apply param
	WLog_ERR("TODO", "TODO: implement");

	return CHANNEL_RC_OK;
}

static UINT android_rail_server_start_cmd(RailClientContext* context)
{
	UINT status;
	RAIL_EXEC_ORDER exec = { 0 };
	RAIL_SYSPARAM_ORDER sysparam = { 0 };
	RAIL_CLIENT_STATUS_ORDER clientStatus = { 0 };
	androidContext* afc = (androidContext*)context->custom;
	rdpSettings* settings = afc->common.context.settings;
	clientStatus.flags = TS_RAIL_CLIENTSTATUS_ALLOWLOCALMOVESIZE;

	if (settings->AutoReconnectionEnabled)
		clientStatus.flags |= TS_RAIL_CLIENTSTATUS_AUTORECONNECT;

	clientStatus.flags |= TS_RAIL_CLIENTSTATUS_ZORDER_SYNC;
	clientStatus.flags |= TS_RAIL_CLIENTSTATUS_WINDOW_RESIZE_MARGIN_SUPPORTED;
	clientStatus.flags |= TS_RAIL_CLIENTSTATUS_APPBAR_REMOTING_SUPPORTED;
	clientStatus.flags |= TS_RAIL_CLIENTSTATUS_POWER_DISPLAY_REQUEST_SUPPORTED;
	clientStatus.flags |= TS_RAIL_CLIENTSTATUS_BIDIRECTIONAL_CLOAK_SUPPORTED;
	status = context->ClientInformation(context, &clientStatus);

	if (status != CHANNEL_RC_OK)
		return status;

	if (settings->RemoteAppLanguageBarSupported)
	{
		RAIL_LANGBAR_INFO_ORDER langBarInfo;
		langBarInfo.languageBarStatus = 0x00000008; /* TF_SFT_HIDDEN */
		status = context->ClientLanguageBarInfo(context, &langBarInfo);

		/* We want the language bar, but the server might not support it. */
		switch (status)
		{
			case CHANNEL_RC_OK:
			case ERROR_BAD_CONFIGURATION:
				break;
			default:
				return status;
		}
	}

	sysparam.params = 0;
	sysparam.params |= SPI_MASK_SET_HIGH_CONTRAST;
	sysparam.highContrast.colorScheme.string = NULL;
	sysparam.highContrast.colorScheme.length = 0;
	sysparam.highContrast.flags = 0x7E;
	sysparam.params |= SPI_MASK_SET_MOUSE_BUTTON_SWAP;
	sysparam.mouseButtonSwap = FALSE;
	sysparam.params |= SPI_MASK_SET_KEYBOARD_PREF;
	sysparam.keyboardPref = FALSE;
	sysparam.params |= SPI_MASK_SET_DRAG_FULL_WINDOWS;
	sysparam.dragFullWindows = FALSE;
	sysparam.params |= SPI_MASK_SET_KEYBOARD_CUES;
	sysparam.keyboardCues = FALSE;
	sysparam.params |= SPI_MASK_SET_WORK_AREA;
	sysparam.workArea.left = 0;
	sysparam.workArea.top = 0;
	sysparam.workArea.right = settings->DesktopWidth;
	sysparam.workArea.bottom = settings->DesktopHeight;
	sysparam.dragFullWindows = FALSE;
	status = context->ClientSystemParam(context, &sysparam);

	if (status != CHANNEL_RC_OK) {
		WLog_ERR(TAG, "$$$$$$ ClientSystemParam failed code %u\n", status);
		return status;
	}

	// freerdp_callback("OnRailChannelReady", "(JZ)V", (jlong)((rdpContext*)afc)->instance, ((status == CHANNEL_RC_OK) ? JNI_TRUE : JNI_FALSE));
	// return status;

	exec.RemoteApplicationProgram = settings->RemoteApplicationProgram;
	exec.RemoteApplicationWorkingDir = settings->ShellWorkingDirectory;
	exec.RemoteApplicationArguments = settings->RemoteApplicationCmdLine;
	WLog_ERR(TAG, "$$$$$ start to launch app %s\n", settings->RemoteApplicationProgram ? settings->RemoteApplicationProgram : "NULL");
	return context->ClientExecute(context, &exec);
}
/**
 * Function description
 *
 * @return 0 on success, otherwise a Win32 error code
 */
static UINT android_rail_server_handshake(RailClientContext* context,
                                          const RAIL_HANDSHAKE_ORDER* handshake)
{
	return android_rail_server_start_cmd(context);
}

/**
 * Function description
 *
 * @return 0 on success, otherwise a Win32 error code
 */
static UINT android_rail_server_handshake_ex(RailClientContext* context,
                                             const RAIL_HANDSHAKE_EX_ORDER* handshakeEx)
{
	return android_rail_server_start_cmd(context);
}

/**
 * Function description
 *
 * @return 0 on success, otherwise a Win32 error code
 */
static UINT android_rail_server_local_move_size(RailClientContext* context,
                                                const RAIL_LOCALMOVESIZE_ORDER* localMoveSize)
{
	// TODO: 0222: window actions?
	return CHANNEL_RC_OK;
}

/**
 * Function description
 *
 * @return 0 on success, otherwise a Win32 error code
 */
static UINT android_rail_server_min_max_info(RailClientContext* context,
                                             const RAIL_MINMAXINFO_ORDER* minMaxInfo)
{
	// TODO: 0222: window actions?
	return CHANNEL_RC_OK;
}

/**
 * Function description
 *
 * @return 0 on success, otherwise a Win32 error code
 */
static UINT android_rail_server_language_bar_info(RailClientContext* context,
                                                  const RAIL_LANGBAR_INFO_ORDER* langBarInfo)
{
	return CHANNEL_RC_OK;
}

/**
 * Function description
 *
 * @return 0 on success, otherwise a Win32 error code
 */
static UINT android_rail_server_get_appid_response(RailClientContext* context,
                                                   const RAIL_GET_APPID_RESP_ORDER* getAppIdResp)
{
	return CHANNEL_RC_OK;
}

// TODO:
# if 0
UINT android_rail_open_app_cmd(RailClientContext* context, char* app, char* cmd)
{
	RAIL_EXEC_ORDER exec = { 0 };
	exec.RemoteApplicationProgram = app;
	exec.RemoteApplicationWorkingDir = NULL;
	exec.RemoteApplicationArguments = cmd;
	return context->ClientExecute(context, &exec);
}

UINT android_rail_send_window_event(RailClientContext* context, UINT32 winId, UINT32 cmdId)
{
	RAIL_SYSCOMMAND_ORDER syscmd_order = { 0 };
	syscmd_order.windowId = winId;
	syscmd_order.command = cmdId;
	return context->ClientSystemCommand(context, &syscmd_order);
}

UINT android_rail_send_window_focus_event(RailClientContext* context, UINT32 winId, BOOL focus)
{
	RAIL_ACTIVATE_ORDER activate_order = { 0 };
	activate_order.windowId = winId;
	activate_order.enabled = focus;
	return context->ClientActivate(context, &activate_order);
}
#endif

/*
 * add rail channel window callback for remote program
 * create/update.
 */

/* RemoteApp Core Protocol Extension */
char* title = NULL;

static BOOL android_rail_window_common(rdpContext* context, const WINDOW_ORDER_INFO* orderInfo,
	const WINDOW_STATE_ORDER* windowState)
{
	WLog_ERR("TODO", "TODO: implement");
	UINT32 fieldFlags = orderInfo->fieldFlags;
	bool title_print = false;

	if (fieldFlags & WINDOW_ORDER_STATE_NEW)
	{
		/* Ensure window always gets a window title */
		if (fieldFlags & WINDOW_ORDER_FIELD_TITLE)
		{
			union
			{
				WCHAR* wc;
				BYTE* b;
			} cnv;
			cnv.b = windowState->titleInfo.string;
			if (windowState->titleInfo.length > 0)
			{
				title = ConvertWCharNToUtf8Alloc(
					cnv.wc, windowState->titleInfo.length / sizeof(WCHAR), NULL);
				WLog_ERR(TAG, "$$$ create window for title %s id %u\n", title ? title : "RdpRailWindow", orderInfo->windowId);
				title_print = true;
			}
		}

		if (!title_print)
			WLog_ERR(TAG, "$$$ create window for title %s id %u\n", "RdpRailWindow", orderInfo->windowId);
	} else {
		WLog_ERR(TAG, "$$$ update window for title %s id %u\n", title ? title : "RdpRailWindow", orderInfo->windowId);
	}


	return TRUE;
}

static BOOL android_rail_window_delete(rdpContext* context, const WINDOW_ORDER_INFO* orderInfo)
{
	androidContext* afc = (androidContext*)context;
	WINPR_ASSERT(afc);
	WLog_ERR("TODO", "TODO: implement");

	WLog_ERR(TAG, "$$$ delete window for title %s id %u\n", title ? title : "RdpRailWindow", orderInfo->windowId);

	return TRUE;
}

static BOOL android_rail_window_icon(rdpContext* context, const WINDOW_ORDER_INFO* orderInfo,
							    const WINDOW_ICON_ORDER* windowIcon)
{
	WLog_ERR("TODO", "TODO: implement id %u\n", orderInfo->windowId);
	return TRUE;
}

static BOOL android_rail_window_cached_icon(rdpContext* context, const WINDOW_ORDER_INFO* orderInfo,
	const WINDOW_CACHED_ICON_ORDER* windowCachedIcon)
{
	WLog_ERR("TODO", "TODO: implement");
	return TRUE;
}

static BOOL android_rail_notify_icon_create(rdpContext* context, const WINDOW_ORDER_INFO* orderInfo,
	const NOTIFY_ICON_STATE_ORDER* notifyIconState)
{
	WLog_ERR("TODO", "TODO: implement");
	return TRUE;
}

static BOOL android_rail_notify_icon_update(rdpContext* context, const WINDOW_ORDER_INFO* orderInfo,
	const NOTIFY_ICON_STATE_ORDER* notifyIconState)
{
	WLog_ERR("TODO", "TODO: implement");
	return TRUE;
}

static BOOL android_rail_notify_icon_delete(WINPR_ATTR_UNUSED rdpContext* context,
	WINPR_ATTR_UNUSED const WINDOW_ORDER_INFO* orderInfo)
{
	WLog_ERR("TODO", "TODO: implement");
	return TRUE;
}

static BOOL
android_rail_monitored_desktop(WINPR_ATTR_UNUSED rdpContext* context,
                          WINPR_ATTR_UNUSED const WINDOW_ORDER_INFO* orderInfo,
                          WINPR_ATTR_UNUSED const MONITORED_DESKTOP_ORDER* monitoredDesktop)
{
	WLog_ERR("TODO", "TODO: implement");
	return TRUE;
}

static BOOL android_rail_non_monitored_desktop(rdpContext* context,
                                          WINPR_ATTR_UNUSED const WINDOW_ORDER_INFO* orderInfo)
{
	androidContext* afc = (androidContext*)context;
	android_rail_disable_remoteapp_mode(afc);
	return TRUE;
}

 static void android_rail_register_update_callbacks(rdpUpdate* update)
 {
	 WINPR_ASSERT(update);

	 rdpWindowUpdate* window = update->window;
	 WINPR_ASSERT(window);

	 window->WindowCreate = android_rail_window_common;
	 window->WindowUpdate = android_rail_window_common;
	 window->WindowDelete = android_rail_window_delete;
	 window->WindowIcon = android_rail_window_icon;
	 window->WindowCachedIcon = android_rail_window_cached_icon;
	 window->NotifyIconCreate = android_rail_notify_icon_create;
	 window->NotifyIconUpdate = android_rail_notify_icon_update;
	 window->NotifyIconDelete = android_rail_notify_icon_delete;
	 window->MonitoredDesktop = android_rail_monitored_desktop;
	 window->NonMonitoredDesktop = android_rail_non_monitored_desktop;
 }

int android_rail_init(androidContext* afc, RailClientContext* rail)
{
	rdpContext* context = (rdpContext*)afc;

	WLog_ERR(TAG, "$$$$$$$ start to init android_rail channel\n");

	if (!afc || !rail)
		return 0;

	afc->rail = rail;
	android_rail_register_update_callbacks(context->update);
	rail->custom = (void*)afc;
	rail->ServerExecuteResult = android_rail_server_execute_result;
	rail->ServerSystemParam = android_rail_server_system_param;
	rail->ServerHandshake = android_rail_server_handshake;
	rail->ServerHandshakeEx = android_rail_server_handshake_ex;
	rail->ServerLocalMoveSize = android_rail_server_local_move_size;
	rail->ServerMinMaxInfo = android_rail_server_min_max_info;
	rail->ServerLanguageBarInfo = android_rail_server_language_bar_info;
	rail->ServerGetAppIdResponse = android_rail_server_get_appid_response;

	return 1;
}

int android_rail_uninit(androidContext* afc, RailClientContext* rail)
{
	WINPR_UNUSED(rail);

	if (afc->rail)
	{
		android_rail_disable_remoteapp_mode(afc);
		afc->rail->custom = NULL;
		afc->rail = NULL;
	}

	return 1;
}
