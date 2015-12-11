TEMPLATE = subdirs

SUBDIRS = DTPictureFileSetImageProvider \
          RTVideoDirectShowProvider \
          Flusher \
          GeoTiffImageWriter \
          Init2DMosaic \
          RTPictureFileSetImageProvider

# Following modules are deprecated and windows + MATLAB lib only
          #DrawAndWriteModule \
          #InitMatchModule \
          #OptimizationModule \
          #MatlabCppInterface \
