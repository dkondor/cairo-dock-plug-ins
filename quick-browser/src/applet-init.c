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
#include "applet-menu.h"
#include "applet-init.h"


CD_APPLET_DEFINE2_BEGIN (N_("Quick Browser"),
	CAIRO_DOCK_MODULE_DEFAULT_FLAGS,
	CAIRO_DOCK_CATEGORY_APPLET_FILES,
	N_("This applet lets you browse a folder and its sub-folders very quickly.\n"
	"You can set up a shortkey to pop up the menu.\n"
	"Midlle-click will open the main folder.\n"
	"This applet can be instanciated several times, if you want to browse different folders."),
	"Fabounet (Fabrice Rey)")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	CD_APPLET_ALLOW_EMPTY_TITLE
CD_APPLET_DEFINE2_END


//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
	}
	
	CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
	
	if (myIcon->cName == NULL)
	{
		gchar *cDirName = g_path_get_basename (myConfig.cDirPath);
		CD_APPLET_SET_NAME_FOR_MY_ICON (cDirName);
		g_free (cDirName);
	}
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	
	// keyboard events
	myData.cKeyBinding = CD_APPLET_BIND_KEY (myConfig.cMenuShortkey,
		D_("Show/hide the folder menu"),
		"Configuration", "menu shortkey",
		(CDBindkeyHandler) cd_quick_browser_on_shortkey_menu);
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;

	gldi_object_unref (GLDI_OBJECT(myData.cKeyBinding));
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		}
		
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
		
		if (myIcon->cName == NULL)
		{
			gchar *cDirName = g_path_get_basename (myConfig.cDirPath);
			CD_APPLET_SET_NAME_FOR_MY_ICON (cDirName);
			g_free (cDirName);
		}
		
		gldi_shortkey_rebind (myData.cKeyBinding, myConfig.cMenuShortkey, NULL);
	}
CD_APPLET_RELOAD_END
