#pragma once

#include <SequenceCapture.h>
#include <InotifySequenceCapture.h>
#include <VisualizationUtils.h>
#include <Visualizer.h>
#include <nlohmann/json.hpp>
#include <unordered_set>

namespace tomcat {

    class Sensor {
      public:
        Sensor()
            : visualizer(true, false, false, false),
              det_parameters(this->arguments) {}

        void initialize(std::string exp,
                        std::string trial,
                        std::string pname,
                        bool ind,
                        bool vis,
                        std::string file_path,
                        bool output_emotions,
                        int input_source,
                        int output_source,
                        std::string out_path,
                        std::string bus);

        void get_observation();

      private:
        std::vector<std::string> arguments = {"-au_static"};
        Utilities::Visualizer visualizer;
        cv::Mat rgb_image;
        Utilities::SequenceCapture sequence_reader;
        Utilities::InotifySequenceCapture inotify_reader;
        LandmarkDetector::CLNF face_model;
        LandmarkDetector::FaceModelParameters det_parameters;
        Utilities::FpsTracker fps_tracker;
        cv::Mat_<uchar> grayscale_image;
        std::string exp_id;
        std::string trial_id;
        std::string playername;
        bool indent;
        bool visual;
        bool output_emotions;
        std::unordered_set<std::string> get_emotions(nlohmann::json au);
        int input_source;
        int output_source;
        FILE* out_file;
    };

} // namespace tomcat
