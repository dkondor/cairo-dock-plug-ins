/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-netspeed.h"


CD_APPLET_ON_CLICK_BEGIN
	gldi_dialogs_remove_on_icon (myIcon);
	if (myData.bAcquisitionOK)
	{
		gldi_dialog_show_temporary_with_icon_printf ("%s :\n  %s : %.2f%s\n  %s : %.2f%s",
			myIcon, myContainer, 6e3,
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE,
			D_("Total amount of data"),
			D_("downloaded"), (double) myData.iReceivedBytes / (1024*1024), D_("MB"),
			D_("uploaded"), (double) myData.iTransmittedBytes / (1024*1024), D_("MB"));
	}
	else
	{
		gchar *cQuestion;
		if (myConfig.iStringLen == 0)
			cQuestion = g_strdup (D_("No interface found.\n"
				"Please be sure that at least one interface is available\n"
				" and that you have the right to monitor it"));
		else
			cQuestion = g_strdup_printf (D_("Interface '%s' doesn't seem to exist or is not readable.\n"
				"You may have to set up the interface you wish to monitor.\n"
				"Do you want to do it now?"), myConfig.cInterface);
		gldi_dialog_show_with_question (cQuestion, myIcon, myContainer,
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE,
			(CairoDockActionOnAnswerFunc) cairo_dock_open_module_config_on_demand,
			myApplet, NULL);
		g_free (cQuestion);
	}
CD_APPLET_ON_CLICK_END


static gboolean bWarning = FALSE;

static void _nm_got_enabled (GObject *pObj, GAsyncResult *pRes, gpointer ptr)
{
	GldiModuleInstance *myApplet = (GldiModuleInstance*)ptr;
	CD_APPLET_ENTER;
	
	GError *err = NULL;
	GDBusConnection *pConn = G_DBUS_CONNECTION (pObj);
	GVariant *res = g_dbus_connection_call_finish (pConn, pRes, &err);
	if (err)
	{
		if (! g_error_matches (err, G_IO_ERROR, G_IO_ERROR_CANCELLED) && !bWarning)
		{
			bWarning = TRUE; // only show this warning once, likely the service is not present
			cd_warning ("Cannot get network state: %s", err->message);
		}
		g_error_free (err);
	}
	else
	{
		GVariant *tmp1 = g_variant_get_child_value (res, 0);
		GVariant *tmp2 = g_variant_get_variant (tmp1);
		if (g_variant_is_of_type (tmp2, G_VARIANT_TYPE ("b")))
		{
			gboolean bActive = g_variant_get_boolean (tmp2);
			g_dbus_connection_call (pConn,
				"org.freedesktop.NetworkManager",
				"/org/freedesktop/NetworkManager",
				"org.freedesktop.NetworkManager",
				"Enable",
				g_variant_new ("(b)", !bActive),
				NULL,
				G_DBUS_CALL_FLAGS_NO_AUTO_START | G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION,
				-1, NULL, NULL, NULL); // not interested in the result
		}
		else cd_warning ("Unexpected type for 'NetworkingEnabled' property: %s", g_variant_get_type_string (tmp2));
		g_variant_unref (tmp2);
		g_variant_unref (tmp1);
	}
	g_variant_unref (res);
	
	CD_APPLET_LEAVE ();
}

static void _nm_sleep (GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	
	GDBusConnection *pConn = g_bus_get_sync (G_BUS_TYPE_SYSTEM, NULL, NULL);
	g_return_if_fail (pConn != NULL);
	
	if (! myData.pCancel)
		myData.pCancel = g_cancellable_new ();
	
	g_dbus_connection_call (pConn,
		"org.freedesktop.NetworkManager",
		"/org/freedesktop/NetworkManager",
		"org.freedesktop.DBus.Properties",
		"Get",
		g_variant_new ("(ss)", "org.freedesktop.NetworkManager", "NetworkingEnabled"),
		G_VARIANT_TYPE ("(v)"),
		G_DBUS_CALL_FLAGS_NO_AUTO_START,
		-1,
		myData.pCancel,
		_nm_got_enabled,
		myApplet);
	g_object_unref (G_OBJECT (pConn)); // refed by the above call
	
	CD_APPLET_LEAVE ();
}
static void _netspeed_sleep (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	_nm_sleep (myApplet);
}
static void _netspeed_recheck (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	gldi_task_stop (myData.pPeriodicTask);
	gldi_task_launch (myData.pPeriodicTask);
}
static void _show_system_monitor (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	if (myConfig.cSystemMonitorCommand != NULL)
	{
		cairo_dock_launch_command_full (myConfig.cSystemMonitorCommand, NULL, GLDI_LAUNCH_GUI | GLDI_LAUNCH_SLICE);
	}
	else
	{
		cairo_dock_fm_show_system_monitor ();
	}
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	gchar *cLabel = g_strdup_printf ("%s (%s)", D_("Enable/disable network"), D_("middle-click"));
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, GLDI_ICON_NAME_MEDIA_PAUSE, _netspeed_sleep, CD_APPLET_MY_MENU);
	g_free (cLabel);
	
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Open the System-Monitor"), GLDI_ICON_NAME_EXECUTE, _show_system_monitor, CD_APPLET_MY_MENU);
	
	if (! myData.bAcquisitionOK)
	{
		CD_APPLET_ADD_IN_MENU (D_("Re-check interface"), _netspeed_recheck, CD_APPLET_MY_MENU);
	}

CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	
	_nm_sleep (myApplet);
	
CD_APPLET_ON_MIDDLE_CLICK_END
