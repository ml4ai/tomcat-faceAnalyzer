#include "Sensor.h"
#include <csignal>
#include <iostream>
#include <string>

// Boost includes
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>

using namespace std;
using namespace tomcat;
namespace po = boost::program_options;

int main(int ac, char* av[]) {
    string exp_id, trial_id, playername, of_dir, path, out_path, bus;
    bool indent, visualize, emotion;
    int input_source, output_source;

    // Boost command line options
    try {

        po::options_description desc("Allowed options");
        desc.add_options()("help,h", "Show this help message")(
            "exp_id",
            po::value<string>(&exp_id)->default_value(""),
            "Set experiment ID")(
            "trial_id",
            po::value<string>(&trial_id)->default_value(""),
            "Set trial ID")("playername",
                            po::value<string>(&playername)->default_value(""),
                            "Set player name")("mloc",
                                               po::value<string>(&of_dir),
                                               "Set OpenFace models directory")(
	    "input_source",
	    po::value<int>(&input_source)->default_value(0),
	    "0 for webcam, 1 for nfs")(
	    "output_source",
	    po::value<int>(&output_source)->default_value(0),
	    "0 for stdout, 1 for file (need to specify out_path), 2 for mqtt (need to specify message bus)")(
            "indent",
            po::bool_switch(&indent)->default_value(false),
            "Indent output JSON by four spaces")(
            "visualize",
            po::bool_switch(&visualize)->default_value(false),
            "Enable visualization")(
            "path,p",
            po::value<string>(&path)->default_value("null"),
            "Specify an input video/image file")(
            "emotion",
            po::bool_switch(&emotion)->default_value(false),
            "Display discrete emotion")(
	    "out_path",
	    po::value<string>(&out_path)->default_value("null"),
	    "Path for the output file if output_source is file")(
	    "bus",
	    po::value<string>(&bus)->default_value("null"),
	    "Message bus to publish to if output_source is mqtt");

        po::variables_map vm;
        po::store(po::parse_command_line(ac, av, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            cout << desc << "\n";
            return 0;
        }

        if (getenv("OPENFACE_MODELS_DIR") != NULL) {
            if (vm.count("mloc")) {
                char* path = &of_dir[0];
                setenv("OPENFACE_MODELS_DIR", path, 1);
            }
        }
        else if (vm.count("mloc")) {
            char* path = &of_dir[0];
            setenv("OPENFACE_MODELS_DIR", path, 0);
        }
        else {
            throw runtime_error(
                "OPENFACE_MODELS_DIR is not set. Use the --mloc flag or "
                "set the environment variable to point to the directory "
                "containing the OpenFace models. Exiting now.");
        }
    }
    catch (exception const& e) {
        cerr << "error: " << e.what() << endl;
        return 1;
    }
    catch (...) {
        cerr << "Exception of unknown type!" << endl;
    }

    Sensor sensor;
    sensor.initialize(
        exp_id, 
        trial_id, 
        playername, 
        indent, 
        visualize, 
        path, 
        emotion, 
        input_source,
	      output_source,
	      out_path,
	      bus);

    sensor.get_observation();

    return 0;
}
