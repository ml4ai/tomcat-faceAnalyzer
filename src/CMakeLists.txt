find_package(nlohmann_json REQUIRED)
find_package(Boost COMPONENTS program_options filesystem date_time REQUIRED)
add_executable(faceAnalyzer WebcamSensor.cpp FaceAnalyzer.cpp)
add_executable(test inotifyTest.c)
target_include_directories(faceAnalyzer PRIVATE ${openface_include_dirs})
target_link_libraries(faceAnalyzer PRIVATE OpenFace nlohmann_json::nlohmann_json
                                         ${Boost_LIBRARIES})
