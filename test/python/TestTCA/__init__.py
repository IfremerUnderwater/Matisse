# -*- coding: utf-8 -*-
"""
/***************************************************************************
 TestTCA
                                 A QGIS plugin
 Un premier test de plugin by TCA
                             -------------------
        begin                : 2013-10-30
        copyright            : (C) 2013 by Chrisar/TCA
        email                : thierry.cabassud@chrisar.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 This script initializes the plugin, making it known to QGIS.
"""

def classFactory(iface):
    # load TestTCA class from file TestTCA
    from testtca import TestTCA
    return TestTCA(iface)
