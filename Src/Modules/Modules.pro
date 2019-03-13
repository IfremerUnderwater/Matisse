TEMPLATE = subdirs

SUBDIRS = DTPictureFileSetImageProvider \
#          RTVideoDirectShowProvider \
          RTPictureFileSetImageProvider \
          Flusher \
          Init2DMosaic \
          DrawBlend2DMosaic \
          GeoTiffImageWriter \
          ReconstructionLister \
#	  RTStillCameraProvider \
#	  RTSurveyPlotter \
          BridgeModule \
          Init3DRecon \
          Matching3D \
          SfmBundleAdjustment \
          Meshing3D \
          MeshingSparse3D \
          Texturing3D
