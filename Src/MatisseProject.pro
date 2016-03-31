TEMPLATE = subdirs

CONFIG += ordered

#SUBDIRS = MatisseServer/MatisseServer.pro \
#		  Modules/Modules.pro \
#		  Libraries/Libraries.pro \
#		  Tests/Tests.pro
SUBDIRS = Libraries \
            Modules \
            Tests \
            MatisseServer

Modules.depends = Libraries
Tests.depends = Libraries
MatisseServer.depends = Libraries Modules Tests

OTHER_FILES += .qmake.conf
OTHER_FILES += ./Scripts/Deploy_Release.pro \
               ./Scripts/Deploy_Debug.pro
