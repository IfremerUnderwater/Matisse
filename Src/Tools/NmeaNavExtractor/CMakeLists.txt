# add the executable
add_executable(NmeaNavExtractor
               main.cpp
			   nmea_extractor_wizard.cpp
			   nmea_files_reader.cpp
			   angle_interpolator.cpp
			   interpolator.cpp
			   nmea_extractor_wizard.ui)

set_target_properties(NmeaNavExtractor PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)

target_link_libraries(NmeaNavExtractor PUBLIC
     Qt5::Core
     Qt5::Widgets)