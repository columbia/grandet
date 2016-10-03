#include <boost/program_options/option.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <signal.h>
#include <string>
#include <fstream>

#include "controller.h"
#include "config.h"
#include "log.h"

namespace po = boost::program_options;
using namespace std;

static Controller *controller;
static string profilingFile;

const string DEFAULT_CONFIG_FILE = "config.json";
const string DEFAULT_PROFILING_FILE = "profile.json";

void loadProfiling(int sigid) {
    controller->updateChoices(profilingFile);
}

int main(int argc, char **argv) {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "get help")
        ("conf", po::value<string>(), "config file")
        ("verbose", "verbose mode")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        Info() << desc << endl;
        return 1;
    }

    string confFile;
    if (vm.count("conf")) {
        confFile = vm["conf"].as<string>();
    } else {
        confFile = DEFAULT_CONFIG_FILE;
    }

    Config *config = GetConfigInstance();
    config->load(confFile);

    profilingFile = config->get<string>("server.profiling_file", DEFAULT_PROFILING_FILE);
    signal(SIGHUP, loadProfiling);

    controller = new Controller();
    controller->loadDefaultChoices();
    ifstream profFile(profilingFile);
    if (profFile.good()) {
        loadProfiling(SIGHUP);
    }
    profFile.close();

    if (vm.count("verbose")) {
        SetLogLevel(LOG_DBG);
    } else {
        SetLogLevel(LOG_INFO);
    }
    controller->start();
}
