#pragma once
#include "itineraires.h"
#include "type/type.pb.h"
namespace itineraires {
void to_proto(etape et, pbnavitia::Parcours_Etape *etpb);
void to_proto(parcours pa, pbnavitia::Parcours *papb);
void to_proto(itineraire it,std::map<uint32_t, itineraires::parcours> &map_parcours, navitia::type::Data &data, pbnavitia::Itineraire *itpb) ;
}
