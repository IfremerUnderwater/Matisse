TEMPLATE = subdirs

SUBDIRS = MatisseCommon \
          MatisseTools \
          MatisseServer \
          #Modules/MatlabCppInterface \
          Modules/DTPictureFileSetImageProvider \
          #Modules/DrawAndWriteModule \
          #Modules/Flusher \
          Modules/GeoTiffImageWriter \
          #Modules/InitMatchModule \
          #Modules/OptimizationModule \
          Modules/RTPictureFileSetImageProvider \
          Modules/RTVideoDirectShowProvider \
          libOpticalMapping
          
          
          
CONFIG += ordered
