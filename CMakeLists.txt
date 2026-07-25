########### requirements ###############

cmake_minimum_required (VERSION 3.16.0)

project ("cairo-dock4-plugins" C)

find_package (PkgConfig)
include (CheckLibraryExists)
include (CheckIncludeFiles)
include (CheckFunctionExists)
include (CheckSymbolExists)
include (GNUInstallDirs)

########### project ###############

set (VERSION "3.99.0")
# can be different from VERSION, e.g. if a new version of plugins does not depend on things added to core
set (CORE_REQUIRED_VERSION "3.99.0")

add_definitions (-std=gnu99 -Wall -Werror-implicit-function-declaration) # -Wextra -Wwrite-strings -Wuninitialized -Werror-implicit-function-declaration -Wstrict-prototypes -Wreturn-type -Wparentheses -Warray-bounds)
if (NOT DEFINED CMAKE_BUILD_TYPE)
	add_definitions (-O3)
endif()
add_definitions (-DGL_GLEXT_PROTOTYPES="1")

############ adjust prefix ##############
# (this is to make things work if the user is using
# CMAKE_INSTALL_PREFIX for both core and plugins to set
# and installation path)
if(NOT DEFINED CMAKE_PREFIX_PATH)
	set (CMAKE_PREFIX_PATH "${CMAKE_INSTALL_PREFIX}")
endif()

############ sources tarball #############

set (CPACK_SOURCE_GENERATOR "TGZ")
set (CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${VERSION}")
set (CPACK_SOURCE_IGNORE_FILES
	"/build/;/.bzr/;/.bzrignore$;/.git/;/.gitignore$;/misc/;~$;${CPACK_SOURCE_IGNORE_FILES}")
include (CPack)

add_custom_target(dist
	COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
add_custom_target(dist-bzr
	COMMAND bzr export ${CMAKE_PROJECT_NAME}-${VERSION}.tar.gz
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

############ uninstall #############

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake IMMEDIATE @ONLY)
add_custom_target (uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

########### global variables ###############

if( WIN32 )
	message(FATAL_ERROR "Cairo-Dock requires an air-conditioned room. Please close Windows!")
endif( WIN32 )

set (PACKAGE ${CMAKE_PROJECT_NAME})

if ("${PACKAGEMENT}" STREQUAL "") ## do not check if we're using the same version when packaging the application (e.g. if we just want to generate a tarball of a previous version)
	# we need GLDI which contains most global var: check it now to avoid many errors with regex
	pkg_check_modules (GLDI REQUIRED gldi4>=${CORE_REQUIRED_VERSION})
endif ()

set (dock_version ${VERSION}) # should be the same except if PACKAGEMENT is set

pkg_get_variable(pluginsdir         gldi4 pluginsdir)
pkg_get_variable(pluginsdatarootdir gldi4 pluginsdatarootdir)
pkg_get_variable(prefix             gldi4 plugins_prefix)
pkg_get_variable(localedir          gldi4 plugins_locale)
pkg_get_variable(gettext_domain     gldi4 plugins_gettext)
pkg_get_variable(gtkversion         gldi4 gtkversion)
pkg_get_variable(pkgdatadir         gldi4 pkgdatadir)
pkg_get_variable(HAVE_WAYLAND       gldi4 have_wayland)

# check that installation dir matches with the core
if (NOT "${CMAKE_INSTALL_PREFIX}" STREQUAL "${prefix}")
	if (NOT CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
		message (STATUS "It seems that the current CMAKE_INSTALL_{PREFIX,LIBDIR,DATAROOTDIR} flags are not set to what is expected by the core.")
		message (STATUS " It will be replaced by this value: ${prefix}")
		message (WARNING "Plug-ins should be installed in the directory defined when setting up the core, that is to say in ${pluginsdir}")
	endif()
	set (CMAKE_INSTALL_PREFIX "${prefix}")
endif()

# set internationalisation
set (GETTEXT_PLUGINS "${gettext_domain}")
# set base data directories
set (pluginsdatadir "${pluginsdatarootdir}/plug-ins")
set (gaugesdir "${pluginsdatarootdir}/gauges")
# needed for Turbo-night-fuel which is provided by core
set (gaugesdir_core "${pkgdatadir}/gauges")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")  # additionnal FindPackage files

set (MODULES_MISSING "")
set (MODULES_INTEGRATION_MISSING "")
set (PROGRAMS_MISSING "")


###########     Misc     ###############
macro (enable_if_not_defined MODULE1)
	if (NOT DEFINED ${MODULE1}) # true if not defined
		set (${MODULE1} TRUE)
	endif ()
endmacro (enable_if_not_defined)


##########     Config    ###############

enable_if_not_defined (force-icon-in-menus)
if (force-icon-in-menus)  # we believe that not showing icons in the menus by default is a terrible idea; unfortunately, it's not easily undoable for an end-user; so until this is fixed by a big player (Gnome, Ubuntu or other), we'll force the display, unless "-Dforce-icon-in-menus=yes" is provided in the cmake command.
	add_definitions (-DCAIRO_DOCK_FORCE_ICON_IN_MENUS=1)
else()
	add_definitions (-DCAIRO_DOCK_FORCE_ICON_IN_MENUS=0)
endif()


########### dependancies ###############

message ("")
message (STATUS "=====================")
message (STATUS "Check dependencies...")
message (STATUS "=====================")
message ("")

pkg_check_modules ("PACKAGE" REQUIRED "gldi4" "glib-2.0>=2.36")  # we check first the common libs that most plug-ins would need: basically, all the libs used by the core.
STRING (REGEX REPLACE "gldi4;" "" PACKAGE_LIBRARIES "${PACKAGE_LIBRARIES}")  # but we don't want to link to gldi, since we are dl-opened by it.

# add_definitions (-DGTK_DISABLE_DEPRECATED="1")


############# SHARED-FILES #################
set (shared_filesdatadir "${pluginsdatadir}/shared-files")
add_subdirectory (shared-files)



################################################################################
#                                   PLUG-INS                                   #
################################################################################


############# DESKLET_RENDERING #################
message (STATUS "> Desklet Rendering:")
set (GETTEXT_DESKLET_RENDERING ${GETTEXT_PLUGINS})
set (VERSION_DESKLET_RENDERING "1.5.8")
set (PACKAGE_DESKLET_RENDERING "cd-desklet-rendering")
set (desklet_renderingdatadir "${pluginsdatadir}/desklet-rendering")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/desklet-rendering/data/desklet-rendering.conf.in ${CMAKE_CURRENT_BINARY_DIR}/desklet-rendering/data/desklet-rendering.conf)
add_subdirectory (desklet-rendering)

############# DIALOG_RENDERING #################
message (STATUS "> Dialog Rendering:")
set (GETTEXT_DIALOG_RENDERING ${GETTEXT_PLUGINS})
set (VERSION_DIALOG_RENDERING "0.6.1")
set (PACKAGE_DIALOG_RENDERING "cd-dialog-rendering")
set (dialog_renderingdatadir "${pluginsdatadir}/dialog-rendering")
add_subdirectory (dialog-rendering)

############# DOCK RENDERING #################
message (STATUS "> Dock Rendering:")
set (GETTEXT_RENDERING ${GETTEXT_PLUGINS})
set (VERSION_RENDERING "1.5.11")
set (PACKAGE_RENDERING "cd-rendering")
set (renderingdatadir "${pluginsdatadir}/rendering")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/dock-rendering/data/rendering.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dock-rendering/data/rendering.conf)
add_subdirectory (dock-rendering)


message ("")
message (STATUS "===============")
message (STATUS "Language build:")
message (STATUS "===============")
message ("")
add_subdirectory (po)

message ("")
message (STATUS " Installation in : ${CMAKE_INSTALL_PREFIX}")
message ("")
