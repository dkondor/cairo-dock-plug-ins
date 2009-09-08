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
#include <stdio.h>
#include <glib/gi18n.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-musicplayer.h"
#include "applet-mpris.h"
#include "applet-xmms2.h"

/* On enregistre notre lecteur.
 */
void cd_musicplayer_register_xmms2_handler (void)
{
	MusicPlayerHandeler *pXmms2 = cd_mpris_new_handler ();
	pXmms2->cMprisService = "org.xmms.xmms2";  /// trouver le nom ...
	pXmms2->appclass = "xmms2";  /// idem...
	pXmms2->launch = "xmms2";  /// idem...
	pXmms2->name = "XMMS 2";
	pXmms2->iPlayer = MP_XMMS2;
	cd_musicplayer_register_my_handler (pXmms2, "XMMS 2");
}