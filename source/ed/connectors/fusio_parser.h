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

#pragma once
#include "gtfs_parser.h"

/**
  * Read CanalTP custom transportation files
  *
  * The format is based on GTFS but additional data have been added
  *
  * In most case the FusioHandler will inherit from the GTFS handler and add additional data to the created object
  *
  * Note: In case of errors don't forget to clean the created object since
  * it will have been added in the data structure during the GTFSHandler::handle_line
  */
namespace ed { namespace connectors {

struct AgencyFusioHandler : public AgencyGtfsHandler {
    AgencyFusioHandler(GtfsData& gdata, CsvReader& reader) : AgencyGtfsHandler(gdata, reader) {}
    int ext_code_c,
        sort_c,
        agency_url_c;
    void init(Data& data);
    void handle_line(Data& data, const csv_row& line, bool is_first_line);
    const std::vector<std::string> required_headers() const { return {"agency_id", "agency_name", "agency_url", "agency_timezone"}; }
};

struct StopsFusioHandler : public StopsGtfsHandler {
    StopsFusioHandler(GtfsData& gdata, CsvReader& reader) : StopsGtfsHandler(gdata, reader) {}
    int ext_code_c,
        property_id_c,
        comment_id_c,
        visible_c;

    void init(Data& data);
    stop_point_and_area handle_line(Data& data, const csv_row& line, bool is_first_line);
    void handle_stop_point_without_area(Data& data);
};

struct RouteFusioHandler : public GenericHandler {
    RouteFusioHandler(GtfsData& gdata, CsvReader& reader) : GenericHandler(gdata, reader) {}
    int route_id_c,
        ext_code_c,
        route_name_c,
        is_forward_c,
        line_id_c,
        comment_id_c,
        commercial_mode_id_c,
        contributor_id_c;
    int ignored;
    void init(Data&);
    void handle_line(Data& data, const csv_row& line, bool is_first_line);
    const std::vector<std::string> required_headers() const { return {"route_id", "route_name", "line_id"}; }
};

struct TransfersFusioHandler : public TransfersGtfsHandler {
    TransfersFusioHandler(GtfsData& gdata, CsvReader& reader) : TransfersGtfsHandler(gdata, reader) {}
    int real_time_c,
    property_id_c;
    void init(Data&);
    virtual void fill_stop_point_connection(ed::types::StopPointConnection* connection, const csv_row& row) const;
};

struct TripsFusioHandler : public GenericHandler {
    TripsFusioHandler(GtfsData& gdata, CsvReader& reader) : GenericHandler(gdata, reader) {}
    int route_id_c,
        service_c,
        trip_c,
        headsign_c,
        block_id_c,
        comment_id_c,
        trip_propertie_id_c,
        odt_type_c,
        company_id_c,
        odt_condition_id_c,
        physical_mode_c,
        ext_code_c;

    int ignored = 0;
    int ignored_vj = 0;
    void init(Data&);
    ed::types::VehicleJourney* get_vj(Data& data, const csv_row& row, bool is_first_line);
    ed::types::VehicleJourney* handle_line(Data& data, const csv_row& line, bool is_first_line);
    const std::vector<std::string> required_headers() const { return {"route_id", "service_id", "trip_id", "physical_mode_id", "company_id"}; }
};

struct StopTimeFusioHandler : public StopTimeGtfsHandler {
    StopTimeFusioHandler(GtfsData& gdata, CsvReader& reader) : StopTimeGtfsHandler(gdata, reader) {}
    int desc_c, itl_c, date_time_estimated_c;
    void init(Data&);
    ed::types::StopTime* handle_line(Data& data, const csv_row& line, bool is_first_line);
};

struct ContributorFusioHandler : public GenericHandler {
    ContributorFusioHandler(GtfsData& gdata, CsvReader& reader) : GenericHandler(gdata, reader) {}
    int id_c, name_c;
    void init(Data&);
    void handle_line(Data& data, const csv_row& line, bool is_first_line);
    const std::vector<std::string> required_headers() const { return {"contributor_name", "contributor_id"}; }
};

struct LineFusioHandler : public GenericHandler{
    LineFusioHandler(GtfsData& gdata, CsvReader& reader) : GenericHandler(gdata, reader) {}
    int id_c,
    name_c,
    external_code_c,
    code_c,
    forward_name_c,
    backward_name_c,
    color_c,
    sort_c,
    network_c,
    comment_c,
    commercial_mode_c,
    contributor_c;
    void init(Data &);
    void handle_line(Data& data, const csv_row& line, bool is_first_line);
    const std::vector<std::string> required_headers() const { return {"line_id", "line_name", "commercial_mode_id"}; }
};

struct CompanyFusioHandler : public GenericHandler {
    CompanyFusioHandler(GtfsData& gdata, CsvReader& reader) : GenericHandler(gdata, reader) {}
    int id_c,
    name_c,
    company_address_name_c,
    company_address_number_c,
    company_address_type_c,
    company_url_c,
    company_mail_c,
    company_phone_c,
    company_fax_c;
    void init(Data&);
    void handle_line(Data& data, const csv_row& line, bool is_first_line);
    const std::vector<std::string> required_headers() const { return {"company_name", "company_id"}; }
};

struct PhysicalModeFusioHandler : public GenericHandler {
    PhysicalModeFusioHandler(GtfsData& gdata, CsvReader& reader) : GenericHandler(gdata, reader) {}
    int id_c, name_c;
    void init(Data&);
    void handle_line(Data& data, const csv_row& line, bool is_first_line);
    const std::vector<std::string> required_headers() const { return {"physical_mode_id", "physical_mode_name"}; }
};

struct CommercialModeFusioHandler : public GenericHandler {
    CommercialModeFusioHandler(GtfsData& gdata, CsvReader& reader) : GenericHandler(gdata, reader) {}
    int id_c, name_c;
    void init(Data&);
    void handle_line(Data& data, const csv_row& line, bool is_first_line);
    const std::vector<std::string> required_headers() const { return {"commercial_mode_id", "commercial_mode_name"}; }
};

struct CommentFusioHandler: public GenericHandler{
    CommentFusioHandler(GtfsData& gdata, CsvReader& reader) : GenericHandler(gdata, reader) {}
    int id_c, comment_c;
    void init(Data&);
    void handle_line(Data& data, const csv_row& line, bool is_first_line);
    const std::vector<std::string> required_headers() const { return {"comment_id", "comment_name"}; }
};

struct OdtConditionsFusioHandler: public GenericHandler{
    OdtConditionsFusioHandler(GtfsData& gdata, CsvReader& reader) : GenericHandler(gdata, reader) {}
    int odt_condition_id_c,odt_condition_c;
    void init(Data&);
    void handle_line(Data& data, const csv_row& line, bool is_first_line);
    const std::vector<std::string> required_headers() const { return {"odt_condition_id", "odt_condition"}; }
};

struct StopPropertiesFusioHandler: public GenericHandler{
    StopPropertiesFusioHandler(GtfsData& gdata, CsvReader& reader) : GenericHandler(gdata, reader) {}
    int id_c,
    wheelchair_boarding_c,
    sheltered_c,
    elevator_c,
    escalator_c,
    bike_accepted_c,
    bike_depot_c,
    visual_announcement_c,
    audible_announcement_c,
    appropriate_escort_c,
    appropriate_signage_c;
    void init(Data&);
    void handle_line(Data&, const csv_row& row, bool is_first_line);
    const std::vector<std::string> required_headers() const { return {"property_id"}; }
};

struct TripPropertiesFusioHandler: public GenericHandler{
    TripPropertiesFusioHandler(GtfsData& gdata, CsvReader& reader) : GenericHandler(gdata, reader) {}
    int id_c,
    wheelchair_accessible_c,
    bike_accepted_c,
    air_conditioned_c,
    visual_announcement_c,
    audible_announcement_c,
    appropriate_escort_c,
    appropriate_signage_c,
    school_vehicle_c;
    void init(Data&);
    void handle_line(Data&, const csv_row& row, bool is_first_line);
    const std::vector<std::string> required_headers() const { return {"trip_property_id"}; }
};

namespace grid_calendar {
struct PeriodFusioHandler : public GenericHandler {
    PeriodFusioHandler(GtfsData& gdata, CsvReader& reader) : GenericHandler(gdata, reader) {}
    int calendar_c, begin_c, end_c;
    void init(Data&);
    void handle_line(Data& data, const csv_row& line, bool is_first_line);
    const std::vector<std::string> required_headers() const { return {"calendar_id", "begin_date", "end_date"}; }
};

struct GridCalendarFusioHandler : public GenericHandler {
    GridCalendarFusioHandler(GtfsData& gdata, CsvReader& reader) : GenericHandler(gdata, reader) {}
    int id_c, name_c, monday_c,
    tuesday_c, wednesday_c,
    thursday_c, friday_c,
    saturday_c, sunday_c;
    void init(Data&);
    void handle_line(Data& data, const csv_row& row, bool is_first_line);
    const std::vector<std::string> required_headers() const { return {"id", "name", "monday", "tuesday",
                                                                        "wednesday", "thursday", "friday", "saturday",
                                                                        "sunday" }; }
};

struct ExceptionDatesFusioHandler : public GenericHandler {
    ExceptionDatesFusioHandler(GtfsData& gdata, CsvReader& reader) : GenericHandler(gdata, reader) {}
    int calendar_c, datetime_c, type_c;
    void init(Data&);
    void handle_line(Data& data, const csv_row& row, bool is_first_line);
    const std::vector<std::string> required_headers() const { return {"calendar_id", "date", "type"}; }
};

struct CalendarLineFusioHandler : public GenericHandler {
    CalendarLineFusioHandler(GtfsData& gdata, CsvReader& reader) : GenericHandler(gdata, reader) {}
    int calendar_c, line_c;
    void init(Data&);
    void handle_line(Data& data, const csv_row& row, bool is_first_line);
    const std::vector<std::string> required_headers() const { return {"calendar_id", "line_external_code"}; }
};
}

struct AdminStopAreaFusioHandler : public GenericHandler {
    AdminStopAreaFusioHandler(GtfsData& gdata, CsvReader& reader) : GenericHandler(gdata, reader) {}
    int admin_c, stop_area_c;
    std::unordered_map<std::string, ed::types::StopArea*> tmp_stop_area_map;
    void init(Data&);
    void handle_line(Data& data, const csv_row& row, bool is_first_line);
    const std::vector<std::string> required_headers() const { return {"admin_id", "station_id"}; }

};

/**
 * custom parser
 * simply define the list of elemental parsers to use
 */
struct FusioParser : public GenericGtfsParser {
    void parse_files(Data&);
    FusioParser(const std::string & path) : GenericGtfsParser(path) {}
    /// Add default Agency
    void fill_default_agency(Data & data);
    /// Add default Commercial_mode
    void fill_default_commercial_mode(Data & data);
    /// Add default Physical_mode
    void fill_default_physical_mode(Data & data);
};
}
}
