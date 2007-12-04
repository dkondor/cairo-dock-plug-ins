/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include "stdlib.h"

#include "rendering-config.h"
#include "rendering-caroussel.h"
#include "rendering-parabole.h"
#include "rendering-3D-plane.h"
#include "rendering-init.h"


#define MY_APPLET_CONF_FILE "rendering.conf"
#define MY_APPLET_USER_DATA_DIR "rendering"

double my_rendering_fInclinationOnHorizon;  // inclinaison de la ligne de fuite vers l'horizon.

double my_rendering_fForegroundRatio;  // fraction des icones presentes en avant-plan (represente donc l'etirement en profondeur de l'ellipse).
double my_rendering_iGapOnEllipse;  // regle la profondeur du caroussel.
gboolean my_rendering_bRotateIconsOnEllipse;  // tourner les icones de profil ou pas.

double my_rendering_fParabolePower = .5;
double my_rendering_fParaboleFactor = .33;


CairoDockVisitCard *pre_init (void)
{
	CairoDockVisitCard *pVisitCard = g_new0 (CairoDockVisitCard, 1);
	pVisitCard->cModuleName = g_strdup ("rendering");
	pVisitCard->cReadmeFilePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, MY_APPLET_README_FILE);
	pVisitCard->iMajorVersionNeeded = 1;
	pVisitCard->iMinorVersionNeeded = 4;
	pVisitCard->iMicroVersionNeeded = 5;
	return pVisitCard;
}


Icon *init (CairoDock *pDock, gchar **cConfFilePath, GError **erreur)
{
	//g_print ("%s ()\n", __func__);
	//\_______________ On verifie la presence des fichiers necessaires.
	*cConfFilePath = cairo_dock_check_conf_file_exists (MY_APPLET_USER_DATA_DIR, MY_APPLET_SHARE_DATA_DIR, MY_APPLET_CONF_FILE);
	
	
	//\_______________ On lit le fichier de conf.
	cd_rendering_read_conf_file (*cConfFilePath);
	
	
	//\_______________ On enregistre les vues.
	cd_rendering_register_caroussel_renderer ();
	
	cd_rendering_register_3D_plane_renderer ();
	
	//cd_rendering_register_parabole_renderer ();  // pas encore ...
	
	///cairo_dock_set_all_views_to_default ();
	
	return NULL;
}

void stop (void)
{
	cairo_dock_remove_renderer (MY_APPLET_CAROUSSEL_VIEW_NAME);
	cairo_dock_remove_renderer (MY_APPLET_3D_PLANE_VIEW_NAME);
	//cairo_dock_remove_renderer (MY_APPLET_PARABOLIC_VIEW_NAME);
	
	cairo_dock_reset_all_views ();
}


