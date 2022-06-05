find_package(nlohmann_json REQUIRED)
find_package(Boost COMPONENTS program_options filesystem date_time REQUIRED)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
add_executable(faceAnalyzer Sensor.cpp FaceAnalyzer.cpp)
target_include_directories(faceAnalyzer PRIVATE ${openface_include_dirs})
target_link_libraries(faceAnalyzer PRIVATE OpenFace nlohmann_json::nlohmann_json
                                         ${Boost_LIBRARIES})
