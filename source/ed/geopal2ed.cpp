/* Copyright © 2001-2014, Canal TP and/or its affiliates. All rights reserved.
  
This file is part of Navitia,
    the software to build cool stuff with public transport.
 
Hope you'll enjoy and contribute to this project,
    powered by Canal TP (www.canaltp.fr).
Help us simplify mobility and open public transport:
    a non ending quest to the responsive locomotion way of traveling!
  
LICENCE: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
   
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Affero General Public License for more details.
   
You should have received a copy of the GNU Affero General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
  
Stay tuned using
twitter @navitia 
IRC #navitia on freenode
https://groups.google.com/d/forum/navitia
www.navitia.io
*/

#include "conf.h"
#include <iostream>
#include "ed/connectors/geopal_parser.h"

#include "utils/timer.h"
#include "utils/init.h"

#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/program_options.hpp>
#include "utils/exception.h"
#include "ed_persistor.h"

namespace po = boost::program_options;
namespace pt = boost::posix_time;

int main(int argc, char * argv[])
{
    navitia::init_app();
    auto logger = log4cplus::Logger::getInstance("log");

    std::string input, connection_string;
    uint32_t coord_system;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Show this message")
        ("input,i", po::value<std::string>(&input), "Input directory")
        ("version,v", "Show version")
        ("config-file", po::value<std::string>(), "Path to config file")
        ("coord-system,c", po::value<uint32_t>(&coord_system)->default_value(27572),
             "Coordinate system: 27572 for \" lambert 2 Etendu\"")
        ("connection-string", po::value<std::string>(&connection_string)->required(),
             "Database connection parameters: host=localhost user=navitia "
             "dbname=navitia password=navitia");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if(vm.count("version")){
        std::cout << argv[0] << " V" << navitia::config::kraken_version << " "
                  << navitia::config::navitia_build_type << std::endl;
        return 0;
    }

    if(vm.count("config-file")){
        std::ifstream stream;
        stream.open(vm["config-file"].as<std::string>());
        if(!stream.is_open()) {
            throw navitia::exception("loading config file failed");
        } else {
            po::store(po::parse_config_file(stream, desc), vm);
        }
    }

    if(vm.count("help") || !vm.count("input")) {
        std::cout << "Reads and inserts geopal2ed file into a ed database" << std::endl;
        std::cout << desc << std::endl;
        return 1;
    }
    po::notify(vm);

    pt::ptime start;

    start = pt::microsec_clock::local_time();
    ed::connectors::GeopalParser geopal_parser(input);

    try {
        geopal_parser.fill();
    } catch(const ed::connectors::GeopalParserException& e) {
        LOG4CPLUS_FATAL(logger, "Erreur :" << std::string(e.what()) << "  backtrace :" << e.backtrace());
        return -1;
    }

    ed::EdPersistor p(connection_string);
    p.persist(geopal_parser.data);

    LOG4CPLUS_INFO(logger, "time to load geopal: " << to_simple_string(pt::microsec_clock::local_time() - start));

    return 0;
}
