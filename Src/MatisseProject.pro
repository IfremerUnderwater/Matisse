TEMPLATE = subdirs
SUBDIRS = MatisseServer/MatisseServer.pro \
		  Modules/Modules.pro \
		  Libraries/Libraries.pro \
		  Tests/Tests.pro
		  

OTHER_FILES += .qmake.conf
OTHER_FILES += ./Scripts/Deploy.pro \
               ./Scripts/Deploy_Debug.pro
