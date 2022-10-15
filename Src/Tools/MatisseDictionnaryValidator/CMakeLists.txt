# add the executable
add_executable(MatisseDictionnaryValidator
               main.cpp
			   dictionnary_validator.cpp
			   dictionnary_validator.ui)

set_target_properties(MatisseDictionnaryValidator PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/$<0:>)

target_link_libraries(MatisseDictionnaryValidator PUBLIC
     Qt5::Core
     Qt5::Widgets
	 Qt5::Xml
	 Qt5::XmlPatterns)