#include <boost/foreach.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/timer/timer.hpp>
#include <boost/program_options/option.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
#include <boost/regex.hpp>
#include <sstream>
#include <string>
#include "config.h"
#include "key.h"
#include "profile.h"
#include "s3actor.h"
#include "log.h"

namespace po = boost::program_options;
using namespace std;

void print_pt(const boost::property_tree::ptree& pt) {
    for (auto it = pt.begin(); it != pt.end(); ++it) {
        cout << it->first << ": " << it->second.get_value<string>() << endl;
        print_pt(it->second);
    }
}

void fetch(string url, boost::property_tree::ptree& pt) {
    curlpp::Cleanup myCleanup;

    Info() << "Fetching " << url << endl;

    ostringstream os;
    os << curlpp::Options::Url(url);
    string raw = os.str();

    boost::regex trim_re("(.|\n)*callback\\((.*)\\);$");
    string trimmed = regex_replace(raw, trim_re, "$2");
    boost::regex add_quote_re("(\\w+):");
    string json = regex_replace(trimmed, add_quote_re, "\"$1\":");

    istringstream is(json);
    read_json(is, pt);
}

int main(int argc, char **argv) {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "get help")
        ("out", po::value<string>(), "output file")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        Info() << desc << endl;
        return 1;
    }

    string filename;
    if (vm.count("out")) {
        filename = vm["out"].as<string>();
        Info() << "writing to " << filename << endl;
    } else {
        Err() << "output file not specified" << endl;
        return 1;
    }

    boost::property_tree::ptree pt;
    double s3_storage = 0.0;
    double s3_ia_storage = 0.0;
    double s3_rr_storage = 0.0;
    double s3_put = 0.0;
    double s3_get = 0.0;
    double s3_ia_put = 0.0;
    double s3_ia_get = 0.0;
    double s3_rr_put = 0.0;
    double s3_rr_get = 0.0;
    double ebs_ssd_storage = 0.0;
    double ebs_magnetic_storage = 0.0;
    double ebs_magnetic_req = 0.0;
    boost::timer::nanosecond_type s3_normal_rtt = 0;
    boost::timer::nanosecond_type s3_tail_rtt = 0;

    fetch("http://a0.awsstatic.com/pricing/1/s3/pricing-storage-s3.min.js", pt);
    BOOST_FOREACH(auto& region, pt.get_child("config.regions")) {
        if (region.second.get("region", "") == "us-east-1") {
            BOOST_FOREACH(auto& tier, region.second.get_child("tiers")) {
                if (tier.second.get("name", "") == "firstTBstorage") {
                    BOOST_FOREACH(auto& type, tier.second.get_child("storageTypes")) {
                        if (type.second.get("type", "") == "storage") {
                            s3_storage = type.second.get("prices.USD", 0.0);
                        } else if (type.second.get("type", "") == "infrequentAccessStorage") {
                            s3_ia_storage = type.second.get("prices.USD", 0.0);
    }}}}}}

    fetch("http://a0.awsstatic.com/pricing/1/s3/pricing-storage-s3-rrs.min.js", pt);
    BOOST_FOREACH(auto& region, pt.get_child("config.regions")) {
        if (region.second.get("region", "") == "us-east-1") {
            BOOST_FOREACH(auto& tier, region.second.get_child("tiers")) {
                if (tier.second.get("name", "") == "firstTBstorage") {
                    BOOST_FOREACH(auto& type, tier.second.get_child("storageTypes")) {
                        if (type.second.get("type", "") == "reducedRedundancyStorage") {
                            s3_rr_storage = type.second.get("prices.USD", 0.0);
    }}}}}}

    fetch("http://a0.awsstatic.com/pricing/1/s3/pricing-requests-s3.min.js", pt);
    BOOST_FOREACH(auto& region, pt.get_child("config.regions")) {
        if (region.second.get("region", "") == "us-east-1") {
            BOOST_FOREACH(auto& type, region.second.get_child("types")) {
                if (type.second.get("name", "") == "requestNotSpecified") {
                    BOOST_FOREACH(auto& tier, type.second.get_child("tiers")) {
                        if (tier.second.get("name", "") == "putcopypost") {
                            s3_put = tier.second.get("prices.USD", 0.0) / stoi(tier.second.get("rate", "").substr(3));
                        } else if (tier.second.get("name", "") == "getEtc") {
                            s3_get = tier.second.get("prices.USD", 0.0) / stoi(tier.second.get("rate", "").substr(3));
                }}} else if (type.second.get("name", "") == "infrequentAccessRequests") {
                    BOOST_FOREACH(auto& tier, type.second.get_child("tiers")) {
                        if (tier.second.get("name", "") == "putCopyOrPost") {
                            s3_ia_put = tier.second.get("prices.USD", 0.0) / stoi(tier.second.get("rate", "").substr(3));
                        } else if (tier.second.get("name", "") == "getEtc") {
                            s3_ia_get = tier.second.get("prices.USD", 0.0) / stoi(tier.second.get("rate", "").substr(3));
    }}}}}}

    fetch("http://a0.awsstatic.com/pricing/1/s3/pricing-requests-s3-rrs.min.js", pt);
    BOOST_FOREACH(auto& region, pt.get_child("config.regions")) {
        if (region.second.get("region", "") == "us-east-1") {
            BOOST_FOREACH(auto& tier, region.second.get_child("tiers")) {
                if (tier.second.get("name", "") == "putcopypost") {
                    s3_rr_put = tier.second.get("prices.USD", 0.0) / stoi(tier.second.get("rate", "").substr(3));
                } else if (tier.second.get("name", "") == "getEtc") {
                    s3_rr_get = tier.second.get("prices.USD", 0.0) / stoi(tier.second.get("rate", "").substr(3));
    }}}}

    fetch("http://a0.awsstatic.com/pricing/1/ebs/pricing-ebs.min.js", pt);
    BOOST_FOREACH(auto& region, pt.get_child("config.regions")) {
        if (region.second.get("region", "") == "us-east-1") {
            BOOST_FOREACH(auto& type, region.second.get_child("types")) {
                if (type.second.get("name", "") == "Amazon EBS General Purpose (SSD) volumes") {
                    BOOST_FOREACH(auto& value, type.second.get_child("values")) {
                        if (value.second.get("rate", "") == "perGBmoProvStorage") {
                            ebs_ssd_storage = value.second.get("prices.USD", 0.0);
                }}} else if (type.second.get("name", "") == "Amazon EBS Magnetic volumes") {
                    BOOST_FOREACH(auto& value, type.second.get_child("values")) {
                        if (value.second.get("rate", "") == "perGBmoProvStorage") {
                            ebs_magnetic_storage = value.second.get("prices.USD", 0.0);
                        } else if (value.second.get("rate", "") == "perMMIOreq") {
                            ebs_magnetic_req = value.second.get("prices.USD", 0.0) / 1000;
    }}}}}}

    Config* config = GetConfigInstance();
    config->load("config.json");
    Info() << "S3 Storage: " << s3_storage << " per GB" << endl;
    Info() << "S3 Infrequent Access Storage: " << s3_ia_storage << " per GB" << endl;
    Info() << "S3 Reduced Redundancy Storage: " << s3_rr_storage << " per GB" << endl;
    Info() << "S3 PUT: " << s3_put << " per 1000 requests" << endl;
    Info() << "S3 GET: " << s3_get << " per 1000 requests" << endl;
    Info() << "S3 Infrequent Access PUT: " << s3_ia_put << " per 1000 requests" << endl;
    Info() << "S3 Infrequent Access GET: " << s3_ia_get << " per 1000 requests" << endl;
    Info() << "S3 Reduced Redundancy PUT: " << s3_rr_put << " per 1000 requests" << endl;
    Info() << "S3 Reduced Redundancy GET: " << s3_rr_get << " per 1000 requests" << endl;
    Info() << "EBS SSD Storage: " << ebs_ssd_storage << " per GB" << endl;
    Info() << "EBS Magnetic Storage: " << ebs_magnetic_storage << " per GB" << endl;
    Info() << "EBS Magnetic I/O: " << ebs_magnetic_req << " per 1000 requests" << endl;

    boost::property_tree::ptree ptOut;

    int choiceCount = config->getSize("server.choices");
    for (int i=1; i<=choiceCount; i++) {
        boost::property_tree::ptree ptChoice;

        string path = string("server.choices.") + to_string(i-1) + ".";
        string sActor = config->get<string>(path + "actor");
        string name = config->get<string>(path + "name");
        string actorPath = string("server.choices.") + name + ".";
        ptOut.put(actorPath + "actor", sActor);
        ptOut.put(actorPath + "id", config->get<int>(path + "id"));
        ptOut.put(actorPath + "name", name);
        ptOut.put(actorPath + "bandwidth", 1000);
        ptOut.put(actorPath + "durability", config->get<int>(path + "durability"));

        Profile profile;
        int profileSize = config->getSize(path + "profile");
        for (int i=0; i<profileSize; i++) {
            string profileKey = config->getName(path + "profile." + to_string(i));
            string profileValue = config->get<string>(path + "profile." + to_string(i));
            profile.put(profileKey, profileValue);
            ptOut.put(actorPath + "profile." + profileKey, profileValue);
        }
        if (name.find("s3") != string::npos) {
            if (sActor == "S3") {
                S3Actor actor;
                Key key("nonexistent_key");
                const int count = 10;
                s3_normal_rtt = 0;
                s3_tail_rtt = 0;
                for (int i = 0; i < count; ++i) {
                    boost::timer::cpu_timer timer;
                    actor.del(key, profile);
                    timer.stop();
                    boost::timer::nanosecond_type rtt = timer.elapsed().wall;
                    s3_normal_rtt += rtt;
                    if (rtt > s3_tail_rtt) {
                        s3_tail_rtt = rtt;
                    }
                }
                s3_normal_rtt = (s3_normal_rtt - s3_tail_rtt) / (count - 1);
                Info() << "S3 normal RTT: " << s3_normal_rtt << " nanoseconds" << endl;
                Info() << "S3 tail RTT: " << s3_tail_rtt << " nanoseconds" << endl;

                ptOut.put(actorPath + "latency", s3_normal_rtt / 1000000);
            } else {
                ptOut.put(actorPath + "latency", 0);
            }

            string isIA;
            string isRR;
            if (profile.get("infrequentAccess", &isIA) && isIA == "true") {
                ptOut.put(actorPath + "costPerByte", s3_ia_storage * 1000);
                ptOut.put(actorPath + "costPerGet", s3_ia_get * 1000000000);
                ptOut.put(actorPath + "costPerPut", s3_ia_put * 1000000000);
                ptOut.put(actorPath + "minObjectSize", 131072);
            } else if (profile.get("reducedRedundancy", &isRR) && isRR == "true") {
                ptOut.put(actorPath + "costPerByte", s3_rr_storage * 1000);
                ptOut.put(actorPath + "costPerGet", s3_rr_get * 1000000000);
                ptOut.put(actorPath + "costPerPut", s3_rr_put * 1000000000);
                ptOut.put(actorPath + "minObjectSize", 0);
            } else {
                ptOut.put(actorPath + "costPerByte", s3_storage * 1000);
                ptOut.put(actorPath + "costPerGet", s3_get * 1000000000);
                ptOut.put(actorPath + "costPerPut", s3_put * 1000000000);
                ptOut.put(actorPath + "minObjectSize", 0);
            }
        } else if (name.find("fs") != string::npos || name.find("redis") != string::npos) {
            ptOut.put(actorPath + "minObjectSize", 0);
            string isSSD;
            if (profile.get("ssd", &isSSD) && isSSD == "true") {
                // SSD case
                ptOut.put(actorPath + "latency", 0);
                ptOut.put(actorPath + "costPerByte", ebs_ssd_storage * 1000);
                ptOut.put(actorPath + "costPerGet", 0);
                ptOut.put(actorPath + "costPerPut", 0);
            } else {
                ptOut.put(actorPath + "latency", 0);
                ptOut.put(actorPath + "costPerByte", ebs_magnetic_storage * 1000);
                // estimate: 10 IO per GET
                ptOut.put(actorPath + "costPerGet", 10 * ebs_magnetic_req * 1000000000);
                // estimate: 10 IO per PUT
                ptOut.put(actorPath + "costPerPut", 10 * ebs_magnetic_req * 1000000000);
            }
        } else {
            continue;
        }
    }

    ofstream outf(filename);
    write_json(outf, ptOut);

    return 0;
}
