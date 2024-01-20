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

#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"



CD_APPLET_DEFINE_BEGIN (N_("FPS monitor"),
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_APPLET_SYSTEM,
	N_("This applet shows the rendering rate of the dock it's attached to.\n"
	"It can be instanciated several times to monitor multiple docks."),
	"Daniel Kondor")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	CD_APPLET_ALLOW_EMPTY_TITLE
CD_APPLET_DEFINE_END


//~ CD_APPLET_DEFINITION (N_("fps-monitor"),
	//~ 3, 4, 99,
	//~ CAIRO_DOCK_CATEGORY_APPLET_ACCESSORY,
	//~ N_("Useful description\n"
	//~ "and manual"),
	//~ "Daniel Kondor")


void _draw_fps (GldiModuleInstance *myApplet, const char* fpsc)
{	
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);

	CD_APPLET_START_DRAWING_MY_ICON_OR_RETURN_CAIRO ();
	
	gldi_style_colors_set_text_color (myDrawContext);
	PangoFontDescription *pd = myData.td.fd;
	pango_font_description_set_absolute_size (pd, myIcon->fHeight * 72 / myData.fDpi * PANGO_SCALE);
	PangoLayout *pl = pango_cairo_create_layout (myDrawContext);
	pango_layout_set_font_description (pl, pd);
	pango_layout_set_text (pl, fpsc, -1);
	PangoRectangle log;
	pango_layout_get_pixel_extents (pl, NULL, &log);
	
	cairo_save (myDrawContext);
	double fZoomX = (double) iWidth / log.width;
	double fZoomY = (double) iHeight / log.height;
	
	fZoomX = MIN (fZoomX, fZoomY);
	fZoomY = fZoomX;
	
	cairo_translate (myDrawContext, (iWidth - fZoomX * log.width)/2,
		(iHeight - fZoomY * log.height)/2);
	cairo_scale (myDrawContext, fZoomX, fZoomY);
	pango_cairo_show_layout (myDrawContext, pl);
	cairo_restore (myDrawContext);
	
	CD_APPLET_FINISH_DRAWING_MY_ICON_CAIRO;
}


gboolean _fps_monitor_update (GldiModuleInstance *myApplet)
{
	char fpsc[8];
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	int64_t dt = 1000 * (ts.tv_sec - myData.tlast.tv_sec) + (ts.tv_nsec - myData.tlast.tv_nsec) / 1000000;
	if (dt > 0)
	{
		guint fps = (guint)round(myData.nframes * 1000.0 / (double)dt);
		if (fps > 999) fps = 999;
		snprintf (fpsc, 8, "%u fps", fps);
		_draw_fps (myApplet, fpsc);
	}
	myData.nframes = 0;
	myData.tlast = ts;
	return G_SOURCE_CONTINUE;
}

void _fps_monitor_update_timer (GldiModuleInstance *myApplet)
{
	if (myData.iTimer) g_source_remove (myData.iTimer);
	myData.iTimer = g_timeout_add_seconds (myConfig.iUpdateInterval, (GSourceFunc) _fps_monitor_update, (gpointer) myApplet);
}


gboolean _count_fps (GldiModuleInstance* myApplet, GldiContainer *pContainer, cairo_t *pCairoContext)
{
	myData.nframes++;
	return GLDI_NOTIFICATION_LET_PASS;
}

void _remove_notification (GldiModuleInstance* myApplet)
{
	if (myData.pCurrentDock)
	{
		gldi_object_remove_notification (myData.pCurrentDock, NOTIFICATION_RENDER, (GldiNotificationFunc) _count_fps, myApplet);
		myData.pCurrentDock = NULL;
	}
}

void _add_notification (GldiModuleInstance* myApplet)
{
	_remove_notification (myApplet);
	if (myDock)
	{
		gldi_object_register_notification (myDock, NOTIFICATION_RENDER, (GldiNotificationFunc) _count_fps, GLDI_RUN_AFTER, myApplet);
		myData.pCurrentDock = myDock;
		myData.nframes = 0;
		clock_gettime(CLOCK_MONOTONIC, &myData.tlast);
	}
}

//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (myDesklet) CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
	
	gldi_text_description_copy (&myData.td, &myStyleParam.textDescription);
	pango_font_description_set_weight (myData.td.fd, PANGO_WEIGHT_HEAVY);
	myData.fDpi = gdk_screen_get_resolution (gdk_screen_get_default());
	
	_add_notification (myApplet);
	_fps_monitor_update_timer (myApplet);
	
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	
	_remove_notification (myApplet);
	if (myData.iTimer)
	{
		g_source_remove (myData.iTimer);
		myData.iTimer = 0;
	}
	
	
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	fprintf(stderr, "fps-monitor: applet-reload\n");
	if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	if (myDock != myData.pCurrentDock)
	{
		_add_notification (myApplet);
	}
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		_fps_monitor_update_timer (myApplet);
	}
CD_APPLET_RELOAD_END
