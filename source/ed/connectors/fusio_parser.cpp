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

#include "fusio_parser.h"

namespace ed { namespace connectors {

void AgencyFusioHandler::init(Data& data) {
    AgencyGtfsHandler::init(data);
    ext_code_c = csv.get_pos_col("external_code");
    sort_c = csv.get_pos_col("agency_sort");
    agency_url_c = csv.get_pos_col("agency_url");
}

void AgencyFusioHandler::handle_line(Data& data, const csv_row& row, bool) {

    if(! is_valid(id_c, row)){
        LOG4CPLUS_WARN(logger, "AgencyFusioHandler : Invalid agency id " << row[id_c]);
        return;
    }

    ed::types::Network * network = new ed::types::Network();
    network->uri = row[id_c];

    if (is_valid(ext_code_c, row)) {
        network->external_code = row[ext_code_c];
    }

    network->name = row[name_c];

    if (is_valid(sort_c, row)) {
        network->sort =  boost::lexical_cast<int>(row[sort_c]);
    }

    if (is_valid(agency_url_c, row)) {
        network->website = row[agency_url_c];
    }

    data.networks.push_back(network);
    gtfs_data.agency_map[network->uri] = network;
}

void StopsFusioHandler::init(Data& data) {
        StopsGtfsHandler::init(data);
        ext_code_c = csv.get_pos_col("external_code");
        property_id_c = csv.get_pos_col("property_id");
        comment_id_c =  csv.get_pos_col("comment_id");
        visible_c =  csv.get_pos_col("visible");
}

//in fusio we want to delete all stop points without stop area
void StopsFusioHandler::handle_stop_point_without_area(Data& data) {
    //Deletion of the stop point without stop areas
    std::vector<size_t> erase_sp;
    for (int i = data.stop_points.size()-1; i >=0;--i) {
        if (data.stop_points[i]->stop_area == nullptr) {
            erase_sp.push_back(i);
        }
    }
    int num_elements = data.stop_points.size();
    for (size_t to_erase : erase_sp) {
        gtfs_data.stop_map.erase(data.stop_points[to_erase]->uri);
        delete data.stop_points[to_erase];
        data.stop_points[to_erase] = data.stop_points[num_elements - 1];
        num_elements--;
    }
    data.stop_points.resize(num_elements);
    LOG4CPLUS_INFO(logger, "Deletion of " << erase_sp.size() << " stop_point wihtout stop_area");
}

StopsGtfsHandler::stop_point_and_area StopsFusioHandler::handle_line(Data& data, const csv_row& row, bool is_first_line) {
    auto return_wrapper = StopsGtfsHandler::handle_line(data, row, is_first_line);

    if (is_valid(ext_code_c, row)) {
        if ( return_wrapper.second != nullptr )
            return_wrapper.second->external_code = row[ext_code_c];
        else if ( return_wrapper.first != nullptr )
                return_wrapper.first->external_code = row[ext_code_c];
    }

    if (is_valid(property_id_c, row)) {
        auto it_property = gtfs_data.hasProperties_map.find(row[property_id_c]);
        if(it_property != gtfs_data.hasProperties_map.end()){
            if( return_wrapper.first != nullptr){
                return_wrapper.first->set_properties(it_property->second.properties());
            }
            if( return_wrapper.second != nullptr){
                return_wrapper.second->set_properties(it_property->second.properties());
            }
        }
    }

    if (is_valid(comment_id_c, row)) {
        auto it_comment = gtfs_data.comment_map.find(row[comment_id_c]);
        if(it_comment != gtfs_data.comment_map.end()){
            if( return_wrapper.first != nullptr){
                return_wrapper.first->comment = it_comment->second;
            }
            if( return_wrapper.second != nullptr){
                return_wrapper.second->comment = it_comment->second;
            }
        }
    }

    if (return_wrapper.second != nullptr && is_valid(visible_c, row)) {
        return_wrapper.second->visible = (row[visible_c] == "1");
    }
    return return_wrapper;
}

void RouteFusioHandler::init(Data& ) {
    ext_code_c = csv.get_pos_col("external_code");
    route_id_c = csv.get_pos_col("route_id");
    route_name_c = csv.get_pos_col("route_name");
    is_forward_c = csv.get_pos_col("is_forward");
    line_id_c = csv.get_pos_col("line_id");
    comment_id_c = csv.get_pos_col("comment_id");
    contributor_id_c = csv.get_pos_col("contributor_id");
    ignored = 0;
}

void RouteFusioHandler::handle_line(Data& data, const csv_row& row, bool) {
    if(gtfs_data.route_map.find(row[route_id_c]) != gtfs_data.route_map.end()) {
            ignored++;
            LOG4CPLUS_WARN(logger, "dupplicate on route line " + row[route_id_c]);
            return;
     }
    ed::types::Line* ed_line = nullptr;
    auto it_line = gtfs_data.line_map.find(row[line_id_c]);
    if(it_line != gtfs_data.line_map.end()){
        ed_line = it_line->second;
    }else{
        ignored++;
        LOG4CPLUS_WARN(logger, "Route orphan " + row[route_id_c]);
        return;
    }
    ed::types::Route* ed_route = new ed::types::Route();
    ed_route->line = ed_line;
    ed_route->uri = row[route_id_c];

    if ( is_valid(ext_code_c, row) ){
        ed_route->external_code = row[ext_code_c];
    }

    ed_route->name = row[route_name_c];

    if ( is_valid(comment_id_c, row) ){
        auto it_comment = gtfs_data.comment_map.find(row[comment_id_c]);
        if(it_comment != gtfs_data.comment_map.end()){
            ed_route->comment = it_comment->second;
        }
    }

    gtfs_data.route_map[row[route_id_c]] = ed_route;
    data.routes.push_back(ed_route);
}

void TransfersFusioHandler::init(Data& d) {
    TransfersGtfsHandler::init(d);
    real_time_c = csv.get_pos_col("real_min_transfer_time"),
            property_id_c = csv.get_pos_col("property_id");
}

void TransfersFusioHandler::fill_stop_point_connection(ed::types::StopPointConnection* connection, const csv_row& row) const {
    TransfersGtfsHandler::fill_stop_point_connection(connection, row);

    if(is_valid(property_id_c, row)) {
        auto it_property = gtfs_data.hasProperties_map.find(row[property_id_c]);
        if(it_property != gtfs_data.hasProperties_map.end()){
            connection->set_properties(it_property->second.properties());
        }
    }

    if(is_valid(real_time_c, row)) {
        try {
            connection->duration = boost::lexical_cast<int>(row[real_time_c]);
        } catch (const boost::bad_lexical_cast&) {
            LOG4CPLUS_INFO(logger, "impossible to parse real transfers time duration " << row[real_time_c]);
        }
    }
}

void StopTimeFusioHandler::init(Data& data) {
    StopTimeGtfsHandler::init(data);
    itl_c = csv.get_pos_col("stop_times_itl"),
               desc_c = csv.get_pos_col("stop_desc"),
            date_time_estimated_c = csv.get_pos_col("date_time_estimated");
}

ed::types::StopTime* StopTimeFusioHandler::handle_line(Data& data, const csv_row& row, bool is_first_line) {
    auto stop_time = StopTimeGtfsHandler::handle_line(data, row, is_first_line);

    if (! stop_time) {
        return nullptr;
    }
    if (is_valid(date_time_estimated_c, row))
        stop_time->date_time_estimated = (row[date_time_estimated_c] == "1");
    else
        stop_time->date_time_estimated = false;

    if ( is_valid(desc_c, row) ){
        auto it_comment = gtfs_data.comment_map.find(row[desc_c]);
        if(it_comment != gtfs_data.comment_map.end()){
            stop_time->comment = it_comment->second;
        }
    }

    if(is_valid(itl_c, row)){
        uint16_t local_traffic_zone =  boost::lexical_cast<uint16_t>(row[itl_c]);
        if (local_traffic_zone > 0)
            stop_time->local_traffic_zone = local_traffic_zone;
        else
            stop_time->local_traffic_zone = std::numeric_limits<uint16_t>::max();
    }
    else
        stop_time->local_traffic_zone = std::numeric_limits<uint16_t>::max();
    return stop_time;

}

void TripsFusioHandler::init(Data& d) {
    d.vehicle_journeys.reserve(350000);

    route_id_c = csv.get_pos_col("route_id");
    service_c = csv.get_pos_col("service_id");
    trip_c = csv.get_pos_col("trip_id");
    headsign_c = csv.get_pos_col("trip_headsign");
    block_id_c = csv.get_pos_col("block_id");
    comment_id_c = csv.get_pos_col("comment_id");
    trip_propertie_id_c = csv.get_pos_col("trip_property_id");
    odt_type_c = csv.get_pos_col("odt_type");
    company_id_c = csv.get_pos_col("company_id");
    odt_condition_id_c = csv.get_pos_col("odt_condition_id");
    physical_mode_c = csv.get_pos_col("physical_mode_id");
    ext_code_c = csv.get_pos_col("external_code");
}

ed::types::VehicleJourney* TripsFusioHandler::get_vj(Data& data, const csv_row& row, bool){
    auto it = gtfs_data.route_map.find(row[route_id_c]);
    if (it == gtfs_data.route_map.end()) {
        LOG4CPLUS_WARN(logger, "Impossible to find the route " + row[route_id_c]
                       + " referenced by trip " + row[trip_c]);
        ignored++;
        return nullptr;
    }

    ed::types::Route* route = it->second;

    auto vp_it = gtfs_data.vp_map.find(row[service_c]);
    if(vp_it == gtfs_data.vp_map.end()) {
        LOG4CPLUS_WARN(logger, "Impossible to find the service " + row[service_c]
                       + " referenced by trip " + row[trip_c]);
        ignored++;
        return nullptr;
    }
    ed::types::ValidityPattern* vp_xx = vp_it->second;

    auto vj_it = gtfs_data.vj_map.find(row[trip_c]);
    if(vj_it != gtfs_data.vj_map.end()) {
        ignored_vj++;
        return nullptr;
    }
    ed::types::VehicleJourney* vj = new ed::types::VehicleJourney();
    vj->uri = row[trip_c];
    if(is_valid(ext_code_c, row)){
        vj->external_code = row[ext_code_c];
    }
    if(is_valid(headsign_c, row))
        vj->name = row[headsign_c];
    else
        vj->name = vj->uri;

    vj->validity_pattern = vp_xx;
    vj->adapted_validity_pattern = vp_xx;
    vj->journey_pattern = 0;
    vj->tmp_route = route;
    vj->tmp_line = vj->tmp_route->line;
    if(is_valid(block_id_c, row))
        vj->block_id = row[block_id_c];
    else
        vj->block_id = "";

    gtfs_data.vj_map[vj->uri] = vj;

    data.vehicle_journeys.push_back(vj);
    return vj;
}

ed::types::VehicleJourney* TripsFusioHandler::handle_line(Data& data, const csv_row& row, bool is_first_line) {
    ed::types::VehicleJourney* vj = TripsFusioHandler::get_vj(data, row, is_first_line);

    if (! vj)
        return nullptr;

    if (is_valid(ext_code_c, row))
        vj->external_code = row[ext_code_c];

    //if a physical_mode is given we override the value
    vj->physical_mode = nullptr;
    if (is_valid(physical_mode_c, row)){
        auto itm = gtfs_data.physical_mode_map.find(row[physical_mode_c]);
        if (itm == gtfs_data.physical_mode_map.end()) {
            LOG4CPLUS_WARN(logger, "TripsFusioHandler : Impossible to find the physical mode " << row[physical_mode_c]
                           << " referenced by trip " << row[trip_c]);
        }else{
            vj->physical_mode = itm->second;
        }
    }

    if(vj->physical_mode == nullptr){
        auto itm = gtfs_data.physical_mode_map.find("default_physical_mode");
        vj->physical_mode = itm->second;
    }

    if (is_valid(odt_condition_id_c, row)){
        auto it_odt_condition = gtfs_data.odt_conditions_map.find(row[odt_condition_id_c]);
        if(it_odt_condition != gtfs_data.odt_conditions_map.end()){
            vj->odt_message = it_odt_condition->second;
        }
    }

    if (is_valid(trip_propertie_id_c, row)) {
        auto it_property = gtfs_data.hasVehicleProperties_map.find(row[trip_propertie_id_c]);
        if(it_property != gtfs_data.hasVehicleProperties_map.end()){
            vj->set_vehicles(it_property->second.vehicles());
        }
    }

    if (is_valid(comment_id_c, row)) {
        auto it_comment = gtfs_data.comment_map.find(row[comment_id_c]);
        if(it_comment != gtfs_data.comment_map.end()){
            vj->comment = it_comment->second;
        }
    }

    if(is_valid(odt_type_c, row)){
        vj->vehicle_journey_type = static_cast<nt::VehicleJourneyType>(boost::lexical_cast<int>(row[odt_type_c]));
    }

    vj->company = nullptr;
    if(is_valid(company_id_c, row)){
        auto it_company = gtfs_data.company_map.find(row[company_id_c]);
        if(it_company == gtfs_data.company_map.end()){
            LOG4CPLUS_WARN(logger, "TripsFusioHandler : Impossible to find the company " << row[company_id_c]
                           << " referenced by trip " << row[trip_c]);
        }else{
            vj->company = it_company->second;
        }
    }

    if(vj->company == nullptr){
        auto it_company = gtfs_data.company_map.find("default_company");
        vj->company = it_company->second;
    }
    return vj;
}

void ContributorFusioHandler::init(Data&) {
    id_c = csv.get_pos_col("contributor_id");
    name_c = csv.get_pos_col("contributor_name");
}

void ContributorFusioHandler::handle_line(Data& data, const csv_row& row, bool is_first_line) {
    if(! is_first_line && ! has_col(id_c, row)) {
        LOG4CPLUS_FATAL(logger, "Error while reading " + csv.filename +
                        "  file has more than one contributor and no contributor_id column");
        throw InvalidHeaders(csv.filename);
    }
    ed::types::Contributor * contributor = new ed::types::Contributor();
    if (is_valid(id_c, row)) {
        contributor->uri = row[id_c];
    } else {
        contributor->uri = "default_contributor";
    }
    contributor->name = row[name_c];
    contributor->idx = data.contributors.size() + 1;
    data.contributors.push_back(contributor);
    gtfs_data.contributor_map[contributor->uri] = contributor;
}

void LineFusioHandler::init(Data &){
    id_c = csv.get_pos_col("line_id");
    name_c = csv.get_pos_col("line_name");
    external_code_c = csv.get_pos_col("external_code");
    code_c =  csv.get_pos_col("line_code");
    forward_name_c =  csv.get_pos_col("forward_line_name");
    backward_name_c =  csv.get_pos_col("backward_line_name");
    color_c = csv.get_pos_col("line_color");
    network_c = csv.get_pos_col("network_id");
    comment_c = csv.get_pos_col("comment_id");
    commercial_mode_c = csv.get_pos_col("commercial_mode_id");
    sort_c = csv.get_pos_col("line_sort");
}
void LineFusioHandler::handle_line(Data& data, const csv_row& row, bool is_first_line){
    if(! is_first_line && ! has_col(id_c, row)) {
        LOG4CPLUS_FATAL(logger, "Error while reading " + csv.filename +
                        "  file has more than one line and no line_id column");
        throw InvalidHeaders(csv.filename);
    }
    ed::types::Line * line = new ed::types::Line();
    line->uri = row[id_c];
    line->name = row[name_c];

    if (is_valid(external_code_c, row)) {
        line->external_code = row[external_code_c];
    }
    if (is_valid(code_c, row)) {
        line->code = row[code_c];
    }
    if (is_valid(forward_name_c, row)) {
        line->forward_name = row[forward_name_c];
    }
    if (is_valid(backward_name_c, row)) {
        line->backward_name = row[backward_name_c];
    }
    if (is_valid(color_c, row)) {
        line->color = row[color_c];
    }

    line->network = nullptr;
    if (is_valid(network_c, row)) {
        auto itm = gtfs_data.agency_map.find(row[network_c]);
        if(itm == gtfs_data.agency_map.end()){
            line->network = nullptr;
            LOG4CPLUS_WARN(logger, "LineFusioHandler : Impossible to find the network " << row[network_c]
                           << " referenced by line " << row[id_c]);
        }else{
            line->network = itm->second;
        }
    }

    if(line->network == nullptr){
        auto itm = gtfs_data.agency_map.find("default_network");
        line->network = itm->second;
    }

    if (is_valid(comment_c, row)) {
        auto itm = gtfs_data.comment_map.find(row[comment_c]);
        if(itm != gtfs_data.comment_map.end()){
            line->comment = itm->second;
        }
    }

    line->commercial_mode = nullptr;
    if (is_valid(commercial_mode_c, row)) {
        auto itm = gtfs_data.commercial_mode_map.find(row[commercial_mode_c]);
        if(itm == gtfs_data.commercial_mode_map.end()){
            LOG4CPLUS_WARN(logger, "LineFusioHandler : Impossible to find the commercial_mode " << row[commercial_mode_c]
                           << " referenced by line " << row[id_c]);
        }else{
            line->commercial_mode = itm->second;
        }
    }

    if(line->commercial_mode == nullptr){
        auto itm = gtfs_data.commercial_mode_map.find("default_commercial_mode");
        line->commercial_mode = itm->second;
    }
    if (is_valid(sort_c, row)) {
        line->sort =  boost::lexical_cast<int>(row[sort_c]);
    }

    data.lines.push_back(line);
    gtfs_data.line_map[line->uri] = line;
    gtfs_data.line_map_by_external_code[line->external_code] = line;
}

void CompanyFusioHandler::init(Data&) {
    id_c = csv.get_pos_col("company_id"), name_c = csv.get_pos_col("company_name"),
            company_address_name_c = csv.get_pos_col("company_address_name"),
            company_address_number_c = csv.get_pos_col("company_address_number"),
            company_address_type_c = csv.get_pos_col("company_address_type"),
            company_url_c = csv.get_pos_col("company_url"),
            company_mail_c = csv.get_pos_col("company_mail"),
            company_phone_c = csv.get_pos_col("company_phone"),
            company_fax_c = csv.get_pos_col("company_fax");
}

void CompanyFusioHandler::handle_line(Data& data, const csv_row& row, bool is_first_line) {
    if(! is_first_line && ! has_col(id_c, row)) {
        LOG4CPLUS_FATAL(logger, "Error while reading " + csv.filename +
                        "  file has more than one company and no company_id column");
        throw InvalidHeaders(csv.filename);
    }
    ed::types::Company * company = new ed::types::Company();
    if(! is_valid(id_c, row)){
        LOG4CPLUS_WARN(logger, "CompanyFusioHandler : Invalid company id " << row[id_c]);
        return;
    }
    company->uri = row[id_c];
    company->name = row[name_c];
    if (is_valid(company_address_name_c, row))
        company->address_name = row[company_address_name_c];
    if (is_valid(company_address_number_c, row))
        company->address_number = row[company_address_number_c];
    if (is_valid(company_address_type_c, row))
        company->address_type_name = row[company_address_type_c];
    if (is_valid(company_url_c, row))
        company->website = row[company_url_c];
    if (is_valid(company_mail_c, row))
        company->mail = row[company_mail_c];
    if (is_valid(company_phone_c, row))
        company->phone_number = row[company_phone_c];
    if (is_valid(company_fax_c, row))
        company->fax = row[company_fax_c];
    data.companies.push_back(company);
    gtfs_data.company_map[company->uri] = company;
}

void PhysicalModeFusioHandler::init(Data&) {
    id_c = csv.get_pos_col("physical_mode_id");
    name_c = csv.get_pos_col("physical_mode_name");
}

void PhysicalModeFusioHandler::handle_line(Data& data, const csv_row& row, bool is_first_line) {
    if(! is_first_line && ! has_col(id_c, row)) {
        LOG4CPLUS_FATAL(logger, "Error while reading " + csv.filename +
                        "  file has more than one physical mode and no physical_mode_id column");
        throw InvalidHeaders(csv.filename);
    }
    ed::types::PhysicalMode* mode = new ed::types::PhysicalMode();
    mode->name = row[name_c];
    mode->uri = row[id_c];
    data.physical_modes.push_back(mode);
    gtfs_data.physical_mode_map[mode->uri] = mode;
}

void CommercialModeFusioHandler::init(Data&) {
    id_c = csv.get_pos_col("commercial_mode_id");
    name_c = csv.get_pos_col("commercial_mode_name");
}

void CommercialModeFusioHandler::handle_line(Data& data, const csv_row& row, bool is_first_line) {
    if(! is_first_line && ! has_col(id_c, row)) {
        LOG4CPLUS_FATAL(logger, "Error while reading " + csv.filename +
                        "  file has more than one commercial mode and no commercial_mode_id column");
        throw InvalidHeaders(csv.filename);
    }
    ed::types::CommercialMode* commercial_mode = new ed::types::CommercialMode();
    commercial_mode->name = row[name_c];
    commercial_mode->uri = row[id_c];
    data.commercial_modes.push_back(commercial_mode);
    gtfs_data.commercial_mode_map[commercial_mode->uri] = commercial_mode;
}

void CommentFusioHandler::init(Data&){
    id_c = csv.get_pos_col("comment_id");
    comment_c = csv.get_pos_col("comment_name");
}

void CommentFusioHandler::handle_line(Data&, const csv_row& row, bool is_first_line){
    if(! is_first_line && ! has_col(id_c, row)) {
        LOG4CPLUS_FATAL(logger, "Error while reading " + csv.filename +
                        "  file has more than one comment and no comment_id column");
        throw InvalidHeaders(csv.filename);
    }
    gtfs_data.comment_map[row[id_c]] = row[comment_c];
}


void OdtConditionsFusioHandler::init(Data&){
    odt_condition_id_c = csv.get_pos_col("odt_condition_id");
    odt_condition_c = csv.get_pos_col("odt_condition");
}

void OdtConditionsFusioHandler::handle_line(Data& , const csv_row& row, bool is_first_line){
    if(! is_first_line && ! has_col(odt_condition_id_c, row)) {
        LOG4CPLUS_FATAL(logger, "Error while reading " + csv.filename +
                        "  file has more than one condition and no odt_condition_id_c column");
        throw InvalidHeaders(csv.filename);
    }
    gtfs_data.odt_conditions_map[row[odt_condition_id_c]] = row[odt_condition_c];
}

void StopPropertiesFusioHandler::init(Data&){
    id_c = csv.get_pos_col("property_id");
    wheelchair_boarding_c = csv.get_pos_col("wheelchair_boarding");
    sheltered_c = csv.get_pos_col("sheltered");
    elevator_c = csv.get_pos_col("elevator");
    escalator_c = csv.get_pos_col("escalator");
    bike_accepted_c = csv.get_pos_col("bike_accepted");
    bike_depot_c = csv.get_pos_col("bike_depot");
    visual_announcement_c = csv.get_pos_col("visual_announcement");
    audible_announcement_c = csv.get_pos_col("audible_announcement");
    appropriate_escort_c = csv.get_pos_col("appropriate_escort");
    appropriate_signage_c = csv.get_pos_col("appropriate_signage");
}

void StopPropertiesFusioHandler::handle_line(Data&, const csv_row& row, bool is_first_line){
    if(! is_first_line && ! has_col(id_c, row)) {
        LOG4CPLUS_FATAL(logger, "Error while reading " + csv.filename +
                        "  file has more than one stop_properties and no stop_propertie_id column");
        throw InvalidHeaders(csv.filename);
    }
    auto itm = gtfs_data.hasProperties_map.find(row[id_c]);
    if(itm == gtfs_data.hasProperties_map.end()){
        navitia::type::hasProperties has_properties;
        if(is_active(wheelchair_boarding_c, row))
            has_properties.set_property(navitia::type::hasProperties::WHEELCHAIR_BOARDING);
        if(is_active(sheltered_c, row))
            has_properties.set_property(navitia::type::hasProperties::SHELTERED);
        if(is_active(elevator_c, row))
            has_properties.set_property(navitia::type::hasProperties::ELEVATOR);
        if(is_active(escalator_c, row))
            has_properties.set_property(navitia::type::hasProperties::ESCALATOR);
        if(is_active(bike_accepted_c, row))
            has_properties.set_property(navitia::type::hasProperties::BIKE_ACCEPTED);
        if(is_active(bike_depot_c, row))
            has_properties.set_property(navitia::type::hasProperties::BIKE_DEPOT);
        if(is_active(visual_announcement_c, row))
            has_properties.set_property(navitia::type::hasProperties::VISUAL_ANNOUNCEMENT);
        if(is_active(audible_announcement_c, row))
            has_properties.set_property(navitia::type::hasProperties::AUDIBLE_ANNOUNVEMENT);
        if(is_active(appropriate_escort_c, row))
            has_properties.set_property(navitia::type::hasProperties::APPOPRIATE_ESCORT);
        if(is_active(appropriate_signage_c, row))
            has_properties.set_property(navitia::type::hasProperties::APPOPRIATE_SIGNAGE);
        gtfs_data.hasProperties_map[row[id_c]] = has_properties;
    }
}
void TripPropertiesFusioHandler::init(Data &){
    id_c = csv.get_pos_col("trip_property_id");
    wheelchair_accessible_c = csv.get_pos_col("wheelchair_accessible");
    bike_accepted_c = csv.get_pos_col("bike_accepted");
    air_conditioned_c = csv.get_pos_col("air_conditioned");
    visual_announcement_c = csv.get_pos_col("visual_announcement");
    audible_announcement_c = csv.get_pos_col("audible_announcement");
    appropriate_escort_c = csv.get_pos_col("appropriate_escort");
    appropriate_signage_c = csv.get_pos_col("appropriate_signage");
    school_vehicle_c = csv.get_pos_col("school_vehicle");
}

void TripPropertiesFusioHandler::handle_line(Data&, const csv_row& row, bool is_first_line){
    if(! is_first_line && ! has_col(id_c, row)) {
        LOG4CPLUS_FATAL(logger, "Error while reading " + csv.filename +
                        "  file has more than one stop_properties and no stop_propertie_id column");
        throw InvalidHeaders(csv.filename);
    }
    auto itm = gtfs_data.hasVehicleProperties_map.find(row[id_c]);
    if(itm == gtfs_data.hasVehicleProperties_map.end()){
        navitia::type::hasVehicleProperties has_properties;
        if(is_active(wheelchair_accessible_c, row))
            has_properties.set_vehicle(navitia::type::hasVehicleProperties::WHEELCHAIR_ACCESSIBLE);
        if(is_active(bike_accepted_c, row))
            has_properties.set_vehicle(navitia::type::hasVehicleProperties::BIKE_ACCEPTED);
        if(is_active(air_conditioned_c, row))
            has_properties.set_vehicle(navitia::type::hasVehicleProperties::AIR_CONDITIONED);
        if(is_active(visual_announcement_c, row))
            has_properties.set_vehicle(navitia::type::hasVehicleProperties::VISUAL_ANNOUNCEMENT);
        if(is_active(audible_announcement_c, row))
            has_properties.set_vehicle(navitia::type::hasVehicleProperties::AUDIBLE_ANNOUNCEMENT);
        if(is_active(appropriate_escort_c, row))
            has_properties.set_vehicle(navitia::type::hasVehicleProperties::APPOPRIATE_ESCORT);
        if(is_active(appropriate_signage_c, row))
            has_properties.set_vehicle(navitia::type::hasVehicleProperties::APPOPRIATE_SIGNAGE);
        if(is_active(school_vehicle_c, row))
            has_properties.set_vehicle(navitia::type::hasVehicleProperties::SCHOOL_VEHICLE);
        gtfs_data.hasVehicleProperties_map[row[id_c]] = has_properties;
    }
}

boost::gregorian::date parse_date(const std::string& str) {
    auto logger = log4cplus::Logger::getInstance("log");
    try {
        return boost::gregorian::from_undelimited_string(str);
    } catch(const boost::bad_lexical_cast& ) {
        LOG4CPLUS_ERROR(logger, "Impossible to parse the begin date for " << str);
    } catch(const boost::gregorian::bad_day_of_month&) {
        LOG4CPLUS_ERROR(logger, "bad_day_of_month : Impossible to parse the begin date for " << str);
    } catch(const boost::gregorian::bad_day_of_year&) {
        LOG4CPLUS_ERROR(logger, "bad_day_of_year : Impossible to parse the begin date for " << str);
    } catch(const boost::gregorian::bad_month&) {
        LOG4CPLUS_ERROR(logger, "bad_month : Impossible to parse the begin date for " << str);
    } catch(const boost::gregorian::bad_year&) {
        LOG4CPLUS_ERROR(logger, "bad_year : Impossible to parse the begin date for " << str);
    }
    return boost::gregorian::date(boost::gregorian::not_a_date_time);
}

namespace grid_calendar {

void PeriodFusioHandler::init(Data&) {
    calendar_c = csv.get_pos_col("calendar_id");
    begin_c = csv.get_pos_col("begin_date");
    end_c = csv.get_pos_col("end_date");
}

void PeriodFusioHandler::handle_line(Data&, const csv_row& row, bool is_first_line) {
    if(! is_first_line && ! has_col(calendar_c, row)) {
        LOG4CPLUS_FATAL(logger, "Error while reading " << csv.filename <<
                        "  file has more than one period and no calendar_id column");
        throw InvalidHeaders(csv.filename);
    }
    auto cal = gtfs_data.calendars_map.find(row[calendar_c]);
    if (cal == gtfs_data.calendars_map.end()) {
        LOG4CPLUS_ERROR(logger, "GridCalPeriodFusioHandler : Impossible to find the calendar " << row[calendar_c]);
        return;
    }

    boost::gregorian::date begin_date(parse_date(row[begin_c]));
    boost::gregorian::date end_date(parse_date(row[end_c]));

    if (begin_date.is_not_a_date() || end_date.is_not_a_date()) {
        LOG4CPLUS_ERROR(logger, "period invalid, not added for calendar " << row[calendar_c]);
        return;
    }

    boost::gregorian::date_period period(begin_date, end_date);
    cal->second->period_list.push_back(period);
}

void GridCalendarFusioHandler::init(Data&) {
    id_c = csv.get_pos_col("id");
    name_c = csv.get_pos_col("name");
    monday_c = csv.get_pos_col("monday");
    tuesday_c = csv.get_pos_col("tuesday");
    wednesday_c = csv.get_pos_col("wednesday");
    thursday_c = csv.get_pos_col("thursday");
    friday_c = csv.get_pos_col("friday");
    saturday_c = csv.get_pos_col("saturday");
    sunday_c = csv.get_pos_col("sunday");
}

void GridCalendarFusioHandler::handle_line(Data& data, const csv_row& row, bool is_first_line) {
    if(! is_first_line && ! has_col(id_c, row)) {
        LOG4CPLUS_FATAL(logger, "Error while reading " + csv.filename +
                        "  file has more than one calendar and no id column");
        throw InvalidHeaders(csv.filename);
    }
    ed::types::Calendar* calendar = new ed::types::Calendar();
    calendar->uri = row[id_c];
    calendar->external_code = row[id_c];
    calendar->name =  row[name_c];
    calendar->week_pattern[navitia::Monday] = is_active(monday_c, row);
    calendar->week_pattern[navitia::Tuesday] = is_active(tuesday_c, row);
    calendar->week_pattern[navitia::Wednesday] = is_active(wednesday_c, row);
    calendar->week_pattern[navitia::Thursday] = is_active(thursday_c, row);
    calendar->week_pattern[navitia::Friday] = is_active(friday_c, row);
    calendar->week_pattern[navitia::Saturday] = is_active(saturday_c, row);
    calendar->week_pattern[navitia::Sunday] = is_active(sunday_c, row);
    calendar->idx = data.calendars.size() + 1;
    data.calendars.push_back(calendar);
    gtfs_data.calendars_map[calendar->uri] = calendar;
}

void ExceptionDatesFusioHandler::init(Data &){
    calendar_c = csv.get_pos_col("calendar_id");
    datetime_c = csv.get_pos_col("date");
    type_c = csv.get_pos_col("type");
}

void ExceptionDatesFusioHandler::handle_line(Data&, const csv_row& row, bool is_first_line){
    if(! is_first_line && ! has_col(calendar_c, row)) {
        LOG4CPLUS_FATAL(logger, "Error while reading " + csv.filename +
                        "  file has more than one calendar_id and no id column");
        throw InvalidHeaders(csv.filename);
    }
    auto cal = gtfs_data.calendars_map.find(row[calendar_c]);
    if (cal == gtfs_data.calendars_map.end()) {
        LOG4CPLUS_WARN(logger, "ExceptionDatesFusioHandler : Impossible to find the calendar " << row[calendar_c]);
        return;
    }
    if (row[type_c] != "0" && row[type_c] != "1") {
        LOG4CPLUS_WARN(logger, "ExceptionDatesFusioHandler : unknown type " << row[type_c]);
        return;
    }

    boost::gregorian::date date(parse_date(row[datetime_c]));
    if(date.is_not_a_date()) {
        LOG4CPLUS_ERROR(logger, "date format not valid, we do not add the exception " <<
                       row[type_c] << " for " << row[calendar_c]);
        return;
    }
    navitia::type::ExceptionDate exception_date;
    exception_date.date = date;
    exception_date.type = static_cast<navitia::type::ExceptionDate::ExceptionType>(boost::lexical_cast<int>(row[type_c]));
    cal->second->exceptions.push_back(exception_date);
}

void CalendarLineFusioHandler::init(Data&){
    calendar_c = csv.get_pos_col("calendar_id");
    line_c = csv.get_pos_col("line_external_code");
}

void CalendarLineFusioHandler::handle_line(Data&, const csv_row& row, bool is_first_line){
    if(! is_first_line && ! has_col(calendar_c, row)) {
        LOG4CPLUS_FATAL(logger, "Error while reading " + csv.filename +
                        "  file has more than one calendar_id and no id column");
        throw InvalidHeaders(csv.filename);
    }

    auto cal = gtfs_data.calendars_map.find(row[calendar_c]);
    if (cal == gtfs_data.calendars_map.end()) {
        LOG4CPLUS_ERROR(logger, "CalendarLineFusioHandler : Impossible to find the calendar " << row[calendar_c]);
        return;
    }

    auto it = gtfs_data.line_map_by_external_code.find(row[line_c]);

    if (it == gtfs_data.line_map_by_external_code.end()) {
        LOG4CPLUS_ERROR(logger, "CalendarLineFusioHandler : Impossible to find the line " << row[line_c]);
        return;
    }
    cal->second->line_list.push_back(it->second);
}
}

void AdminStopAreaFusioHandler::init(Data&){
    admin_c = csv.get_pos_col("admin_id");
    stop_area_c = csv.get_pos_col("station_id");
    for (const auto& stop_area : gtfs_data.stop_area_map){
        tmp_stop_area_map[stop_area.second->external_code] = stop_area.second;
    }
}

void AdminStopAreaFusioHandler::handle_line(Data&, const csv_row& row, bool is_first_line){
    if(! is_first_line && ! has_col(stop_area_c, row)) {
        LOG4CPLUS_FATAL(logger, "Error while reading " + csv.filename +
                        "  file has no stop_area_c column");
        throw InvalidHeaders(csv.filename);
    }

    auto sa = tmp_stop_area_map.find(row[stop_area_c]);
    if (sa == tmp_stop_area_map.end()) {
        LOG4CPLUS_ERROR(logger, "AdminStopAreaFusioHandler : Impossible to find the stop_area " << row[stop_area_c]);
        return;
    }

    ed::types::AdminStopArea* admin_stop_area = new ed::types::AdminStopArea();
    admin_stop_area->admin = row[admin_c];
    admin_stop_area->stop_area.push_back(sa->second);
    gtfs_data.admin_stop_area_vector.push_back(admin_stop_area);

}

void FusioParser::fill_default_agency(Data & data){
    // création d'un réseau par defaut
    ed::types::Network * network = new ed::types::Network();
    network->uri = "default_network";
    network->name = "réseau par défaut";
    data.networks.push_back(network);
    gtfs_data.agency_map[network->uri] = network;
}

void FusioParser::fill_default_commercial_mode(Data & data){
    ed::types::CommercialMode* commercial_mode = new ed::types::CommercialMode();
    commercial_mode->name = "mode commercial par défaut";
    commercial_mode->uri = "default_commercial_mode";
    data.commercial_modes.push_back(commercial_mode);
    gtfs_data.commercial_mode_map[commercial_mode->uri] = commercial_mode;
}

void FusioParser::fill_default_physical_mode(Data & data){
    ed::types::PhysicalMode* mode = new ed::types::PhysicalMode();
    mode->name = "mode physique par défaut";
    mode->uri = "default_physical_mode";
    data.physical_modes.push_back(mode);
    gtfs_data.physical_mode_map[mode->uri] = mode;
}

void FusioParser::parse_files(Data& data) {

    fill_default_company(data);
    fill_default_agency(data);
    fill_default_commercial_mode(data);
    fill_default_physical_mode(data);
    parse<AgencyFusioHandler>(data, "agency.txt", true);
    parse<ContributorFusioHandler>(data, "contributors.txt");
    parse<CompanyFusioHandler>(data, "company.txt");
    parse<PhysicalModeFusioHandler>(data, "physical_modes.txt");
    parse<CommercialModeFusioHandler>(data, "commercial_modes.txt");
    parse<CommentFusioHandler>(data, "comments.txt");
    parse<LineFusioHandler>(data, "lines.txt");
    parse<StopPropertiesFusioHandler>(data, "stop_properties.txt");
    parse<StopsFusioHandler>(data, "stops.txt", true);
    parse<RouteFusioHandler>(data, "routes.txt", true);
    parse<TransfersFusioHandler>(data, "transfers.txt");
    parse<CalendarGtfsHandler>(data, "calendar.txt");
    parse<CalendarDatesGtfsHandler>(data, "calendar_dates.txt");
    parse<TripPropertiesFusioHandler>(data, "trip_properties.txt");
    parse<OdtConditionsFusioHandler>(data, "odt_conditions.txt");
    parse<TripsFusioHandler>(data, "trips.txt", true);
    parse<StopTimeFusioHandler>(data, "stop_times.txt", true);
    parse<FrequenciesGtfsHandler>(data, "frequencies.txt");
    parse<grid_calendar::GridCalendarFusioHandler>(data, "grid_calendars.txt");
    parse<grid_calendar::PeriodFusioHandler>(data, "grid_periods.txt");
    parse<grid_calendar::ExceptionDatesFusioHandler>(data, "grid_exception_dates.txt");
    parse<grid_calendar::CalendarLineFusioHandler>(data, "grid_rel_calendar_line.txt");
    parse<AdminStopAreaFusioHandler>(data, "admin_stations.txt");
}
}
}
