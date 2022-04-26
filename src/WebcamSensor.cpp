#include "WebcamSensor.h"
#include <FaceAnalyser.h>
#include <GazeEstimation.h>
#include <LandmarkCoreIncludes.h>
#include <RecorderOpenFace.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/date_time/time_facet.hpp>
#include <nlohmann/json.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string>

#define MAX_EVENT_MONITOR 2048
#define NAME_LEN 32
#define MONITOR_EVENT_SIZE (sizeof(struct inotify_event))
#define BUFFER_LEN MAX_EVENT_MONITOR*(MONITOR_EVENT_SIZE+NAME_LEN)

using namespace std;
using namespace nlohmann;
namespace pt = boost::posix_time;

typedef vector<pair<string, double>> au_vector;

namespace tomcat {

    void WebcamSensor::initialize(string exp,
                                  string trial,
                                  string pname,
                                  bool ind,
                                  bool vis,
                                  string file_path,
                                  bool output_emotions) {
        // Initialize the experiment ID, trial ID and player name
        this->exp_id = exp;
        this->trial_id = trial;
        this->playername = pname;
        this->indent = ind;
        this->visual = vis;
        this->output_emotions = output_emotions;
        if (file_path.compare("null") != 0) {
            this->arguments.insert(this->arguments.begin(), file_path);
            this->arguments.insert(this->arguments.begin(), "-f");
        }
        else {
            this->arguments.insert(this->arguments.begin(), "0");
            this->arguments.insert(this->arguments.begin(), "-device");
        }

        // The modules that are being used for tracking
        this->face_model = LandmarkDetector::CLNF();
        if (!this->face_model.loaded_successfully) {
            throw runtime_error("Could not load the landmark detector.");
        }

        if (!this->face_model.eye_model) {
            throw runtime_error("No eye model found");
        }

        this->fps_tracker.AddFrame();
    };

    void WebcamSensor::get_observation() {
        using cv::Point2f;
        using cv::Point3f;
        using GazeAnalysis::EstimateGaze;
        using LandmarkDetector::Calculate3DEyeLandmarks;
        using LandmarkDetector::CalculateAllEyeLandmarks;
        using LandmarkDetector::DetectLandmarksInVideo;
        using LandmarkDetector::FaceModelParameters;
        using LandmarkDetector::GetPose;

        // Load facial feature extractor and AU analyser
        FaceAnalysis::FaceAnalyserParameters face_analysis_params(
            this->arguments);
        FaceAnalysis::FaceAnalyser face_analyser(face_analysis_params);

        this->sequence_reader.Open(arguments);

        // Recorder open face parameters
        Utilities::RecorderOpenFaceParameters recording_params(
            this->arguments,
            true,
            this->sequence_reader.IsWebcam(),
            this->sequence_reader.fx,
            this->sequence_reader.fy,
            this->sequence_reader.cx,
            this->sequence_reader.cy,
            this->sequence_reader.fps);
	
	// This command gets the frame from the webcam (Uncomment to remove inotify functionality)
        //this->rgb_image = this->sequence_reader.GetNextFrame();

        //while (!this->rgb_image.empty()) {
	
	// -----------------------------------------
	// Inotify code
	// -----------------------------------------
	
	int fd, watch_desc;
	char buffer[BUFFER_LEN];
	fd = inotify_init();
	int testing_int = 1;

	if (fd < 0)
		printf("Notify did not initialize");
	
	// Change the destination to the location where images are going to be added
	string monitor_path = "/data/cat/lion";
	watch_desc = inotify_add_watch(fd, monitor_path.c_str(), IN_CREATE);

	if (watch_desc == -1)
		printf("Couldn't add watch to the path");
	else
		cout << "Monitoring path " << monitor_path << endl;
	while (1) {
	    cout << "Entered infinite while" << endl;
	    int total_read = read (fd, buffer, BUFFER_LEN);
	    if (total_read < 0)
		    printf("Read error");

	    int i = 0;
	    string filename = "";
	    
	    cout << "Reached here" << endl;
	    while(i < total_read) {
		    cout << "Looking for event" << endl;
		    struct inotify_event *event = (struct inotify_event*) &buffer[i];
		    if (event->len) {
			    if (event->mask & IN_CREATE) {
				    if (event->mask & IN_ISDIR)
					    cout << "This is a directory";
				    else
					    filename = event->name;
			    }
			    i += MONITOR_EVENT_SIZE + event->len;
		    }
	    }
	    cout << "After event found" << endl;
	    // Use the filename variable here to get the name of the file
	    if (filename[0] == '.')
		    continue;
	    filename = monitor_path + "/" + filename;

	    do {
	    	this->rgb_image = cv::imread(filename);
		usleep(50);
	    } while(this->rgb_image.empty());
	    
	    cout << "This is the name of the file: " << filename << endl;
	    // -----------------------------------
	    // OpenFace code begins
	    // -----------------------------------

            // Converting to grayscale
            this->grayscale_image = this->sequence_reader.GetGrayFrame();

            // The actual facial landmark detection / tracking
            bool detection_success =
                DetectLandmarksInVideo(this->rgb_image,
                                       this->face_model,
                                       this->det_parameters,
                                       this->grayscale_image);

            // Gaze tracking, absolute gaze direction
            Point3f gazeDirection0(0, 0, 0);
            Point3f gazeDirection1(0, 0, 0);
            cv::Vec2d gazeAngle(0, 0);

            // If tracking succeeded and we have an eye model, estimate gaze
            if (detection_success && this->face_model.eye_model) {

                EstimateGaze(this->face_model,
                             gazeDirection0,
                             this->sequence_reader.fx,
                             this->sequence_reader.fy,
                             this->sequence_reader.cx,
                             this->sequence_reader.cy,
                             true);

                EstimateGaze(this->face_model,
                             gazeDirection1,
                             this->sequence_reader.fx,
                             this->sequence_reader.fy,
                             this->sequence_reader.cx,
                             this->sequence_reader.cy,
                             false);

                gazeAngle =
                    GazeAnalysis::GetGazeAngle(gazeDirection0, gazeDirection1);
            }

            // Do face alignment
            cv::Mat sim_warped_img;
            cv::Mat_<double> hog_descriptor;
            int num_hog_rows = 0, num_hog_cols = 0;

            // Perform AU detection and HOG feature extraction
            // Note: As this can be expensive, only compute it if needed by
            // output or visualization
            if (recording_params.outputAlignedFaces() ||
                recording_params.outputHOG() || recording_params.outputAUs() ||
                this->visualizer.vis_align || this->visualizer.vis_hog ||
                this->visualizer.vis_aus) {
                face_analyser.AddNextFrame(this->rgb_image,
                                           this->face_model.detected_landmarks,
                                           this->face_model.detection_success,
                                           this->sequence_reader.time_stamp,
                                           this->sequence_reader.IsWebcam());
                face_analyser.GetLatestAlignedFace(sim_warped_img);
                face_analyser.GetLatestHOG(
                    hog_descriptor, num_hog_rows, num_hog_cols);
            }

            // Work out the pose of the head from the tracked model
            cv::Vec6d pose_estimate = GetPose(this->face_model,
                                              this->sequence_reader.fx,
                                              this->sequence_reader.fy,
                                              this->sequence_reader.cx,
                                              this->sequence_reader.cy);

            // Keeping track of fps
            this->fps_tracker.AddFrame();

            // Displaying the tracking visualizations if visualization is set
            if (this->visual) {
                this->visualizer.SetImage(this->rgb_image,
                                          this->sequence_reader.fx,
                                          this->sequence_reader.fy,
                                          this->sequence_reader.cx,
                                          this->sequence_reader.cy);

                this->visualizer.SetObservationFaceAlign(sim_warped_img);
                this->visualizer.SetObservationHOG(
                    hog_descriptor, num_hog_rows, num_hog_cols);

                this->visualizer.SetObservationLandmarks(
                    this->face_model.detected_landmarks,
                    this->face_model.detection_certainty,
                    this->face_model.GetVisibilities());

                this->visualizer.SetObservationPose(
                    pose_estimate, this->face_model.detection_certainty);

                this->visualizer.SetObservationGaze(
                    gazeDirection0,
                    gazeDirection1,
                    CalculateAllEyeLandmarks(this->face_model),
                    Calculate3DEyeLandmarks(this->face_model,
                                            this->sequence_reader.fx,
                                            this->sequence_reader.fy,
                                            this->sequence_reader.cx,
                                            this->sequence_reader.cy),
                    this->face_model.detection_certainty);

                this->visualizer.SetObservationActionUnits(
                    face_analyser.GetCurrentAUsReg(),
                    face_analyser.GetCurrentAUsClass());

                this->visualizer.SetFps(this->fps_tracker.GetFPS());

                // Detect key press
                char character_press = this->visualizer.ShowObservation();
                if (character_press == 'r') {
                    this->face_model.Reset();
                }
                else if (character_press == 'q' || character_press == 'Q') {
                    break;
                }
            }

            // JSON output
            json output;

            string timestamp =
            	pt::to_iso_extended_string(pt::microsec_clock::universal_time()) +
            	"Z";

            // Header block
            output["header"] = {
                {"timestamp", timestamp},
                {"message_type", "observation"},
                {"version", "0.1"},
            };

            // Message block
            output["msg"] = {{"experiment_id", this->exp_id},
                             {"trial_id", this->trial_id},
                             {"timestamp", timestamp},
                             {"source", "faceSensor"},
                             {"sub_type", "state"},
                             {"version", "0.1"}};

            // Data block
            stringstream ss;
            ss << fixed << setprecision(5)
               << this->face_model.detection_certainty;
            output["data"] = {
                {"playername", this->playername},
                {"landmark_detection_confidence", ss.str()},
                {"landmark_detection_success", detection_success},
                {"frame", this->sequence_reader.GetFrameNumber()}};

            au_vector AU_reg = face_analyser.GetCurrentAUsReg();
            au_vector AU_class = face_analyser.GetCurrentAUsClass();
            sort(AU_reg.begin(), AU_reg.end());
            sort(AU_class.begin(), AU_class.end());
            for (auto& [AU, occurrence] : AU_class) {
                output["data"]["action_units"][AU]["occurrence"] = occurrence;
            }

            // Only classify emotion if the user specifies through command line
            // option '--emotion'
            if (this->output_emotions) {
                json action_units = output["data"]["action_units"];
                unordered_set<string> emotion_labels =
                    get_emotions(action_units);
                output["data"]["emotions"] = emotion_labels;
            }

            for (auto& [AU, intensity] : AU_reg) {
                output["data"]["action_units"][AU]["intensity"] = intensity;
            }

            output["data"]["gaze"] = {
                {"eye_0",
                 {
                     {"x", gazeDirection0.x},
                     {"y", gazeDirection0.y},
                     {"z", gazeDirection0.z},
                 }},
                {"eye_1",
                 {
                     {"x", gazeDirection1.x},
                     {"y", gazeDirection1.y},
                     {"z", gazeDirection1.z},
                 }},
                {"gaze_angle", {{"x", gazeAngle[0]}, {"y", gazeAngle[1]}}}

            };

            vector<Point2f> eye_landmarks2d =
                CalculateAllEyeLandmarks(this->face_model);
            vector<Point3f> eye_landmarks3d =
                Calculate3DEyeLandmarks(this->face_model,
                                        this->sequence_reader.fx,
                                        this->sequence_reader.fy,
                                        this->sequence_reader.cx,
                                        this->sequence_reader.cy);

            output["data"]["gaze"]["eye_landmarks"]["2D"]["x"] = {};
            output["data"]["gaze"]["eye_landmarks"]["2D"]["y"] = {};

            output["data"]["gaze"]["eye_landmarks"]["3D"]["x"] = {};
            output["data"]["gaze"]["eye_landmarks"]["3D"]["y"] = {};
            output["data"]["gaze"]["eye_landmarks"]["3D"]["z"] = {};

            for (auto& landmark : eye_landmarks2d) {
                output["data"]["gaze"]["eye_landmarks"]["2D"]["x"].push_back(
                    landmark.x);
                output["data"]["gaze"]["eye_landmarks"]["2D"]["y"].push_back(
                    landmark.y);
            }

            for (auto& landmark : eye_landmarks3d) {
                output["data"]["gaze"]["eye_landmarks"]["3D"]["x"].push_back(
                    landmark.x);
                output["data"]["gaze"]["eye_landmarks"]["3D"]["y"].push_back(
                    landmark.y);
                output["data"]["gaze"]["eye_landmarks"]["3D"]["z"].push_back(
                    landmark.z);
            }

            output["data"]["pose"] = {{"location",
                                       {{"x", pose_estimate[0]},
                                        {"y", pose_estimate[1]},
                                        {"z", pose_estimate[2]}}},
                                      {"rotation",
                                       {{"x", pose_estimate[3]},
                                        {"y", pose_estimate[4]},
                                        {"z", pose_estimate[5]}}}};

            // Only indent if the user specifies through command line option
            // --indent
            if (this->indent)
                cout << output.dump(4) << endl;
            else
                cout << output.dump() << endl;

            // Grabbing the next frame in the sequence (removed for inotify)
            // this->rgb_image = this->sequence_reader.GetNextFrame();
        }
	
        this->sequence_reader.Close();

        // Reset the models for the next video
        face_analyser.Reset();
        face_model.Reset();
	
	// Close the inotify watch
	inotify_rm_watch(fd, watch_desc);
	close(fd);
    }

    // Reference: Friesen, W. V., & Ekman, P. (1983). EMFACS-7: Emotional facial
    // action coding system. Unpublished manuscript, University of California at
    // San Francisco, 2(36), 1.
    // Refer to: https://en.wikipedia.org/wiki/Facial_Action_Coding_System
    // and https://imotions.com/blog/facial-action-coding-system/

    unordered_set<string> WebcamSensor::get_emotions(json au) {
        unordered_set<string> labels;

        if (au["AU06"]["occurrence"] == 1 && au["AU12"]["occurrence"] == 1) {
            labels.insert("happiness");
        }
        if (au["AU01"]["occurrence"] == 1 && au["AU04"]["occurrence"] == 1 &&
            au["AU15"]["occurrence"] == 1) {
            labels.insert("sadness");
        }
        if (au["AU01"]["occurrence"] == 1 && au["AU02"]["occurrence"] == 1 &&
            au["AU05"]["occurrence"] == 1 && au["AU26"]["occurrence"] == 1) {
            labels.insert("surprise");
        }
        if (au["AU01"]["occurrence"] == 1 && au["AU02"]["occurrence"] == 1 &&
            au["AU04"]["occurrence"] == 1 && au["AU05"]["occurrence"] == 1 &&
            au["AU07"]["occurrence"] == 1 && au["AU20"]["occurrence"] == 1 &&
            au["AU26"]["occurrence"] == 1) {
            labels.insert("fear");
        }
        if (au["AU04"]["occurrence"] == 1 && au["AU05"]["occurrence"] == 1 &&
            au["AU07"]["occurrence"] == 1 && au["AU23"]["occurrence"] == 1) {
            labels.insert("anger");
        }
        if (au["AU09"]["occurrence"] == 1 && au["AU15"]["occurrence"] == 1 &&
            au["AU17"]["occurrence"] == 1) {
            labels.insert("disgust");
        }
        if (au["AU12"]["occurrence"] == 1 && au["AU14"]["occurrence"] == 1) {
            labels.insert("contempt");
        }

        return labels;
    }

} // namespace tomcat
