add_executable(DataPreprocessingWizard
    main.cpp
    data_preprocessing_wizard.cpp
    interpolator.cpp
    angle_interpolator.cpp
    nav_file_reader.cpp)

set_target_properties(DataPreprocessingWizard PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)

target_link_libraries(DataPreprocessingWizard PRIVATE ImageProcessing BasicProcessing ${OpenCV_LIBS} Qt5::Core Qt5::Widgets)

