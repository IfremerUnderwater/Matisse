TEMPLATE = subdirs

SUBDIRS = DTPictureFileSetImageProvider \
          RTVideoDirectShowProvider \
          RTPictureFileSetImageProvider \
          Flusher \
          Init2DMosaic \
          DrawBlend2DMosaic \
          GeoTiffImageWriter \
	  RTStillCameraProvider \
	  RTSurveyPlotter \
	  BridgeModule

# Following modules are deprecated and windows + MATLAB lib only
          #DrawAndWriteModule \
          #InitMatchModule \
          #OptimizationModule \
          #MatlabCppInterface \
