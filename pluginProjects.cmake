#/**********************************************************\ 
#Original Author: Richard Bateman (taxilian)
#
#Created:    Nov 20, 2009
#License:    Dual license model; choose one of two:
#            New BSD License
#            http://www.opensource.org/licenses/bsd-license.php
#            - or -
#            GNU Lesser General Public License, version 2.1
#            http://www.gnu.org/licenses/lgpl-2.1.html
#            
#Copyright 2009 PacketPass, Inc and the Firebreath development team
#\**********************************************************/


add_subdirectory(${PLUGINCOMMON_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/PluginCommon)

if (WIN32)  # ActiveX projects
  add_subdirectory(${ACTIVEXPLUGIN_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/ActiveXPlugin)
  add_subdirectory(${ACTIVEXPLUGINTEST_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/ActiveXPluginTest)
endif(WIN32)

