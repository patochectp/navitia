# Copyright (c) 2001-2019, Canal TP and/or its affiliates. All rights reserved.
#
# This file is part of Navitia,
#     the software to build cool stuff with public transport.
#
# Hope you'll enjoy and contribute to this project,
#     powered by Canal TP (www.canaltp.fr).
# Help us simplify mobility and open public transport:
#     a non ending quest to the responsive locomotion way of traveling!
#
# LICENCE: This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Stay tuned using
# twitter @navitia
# IRC #navitia on freenode
# https://groups.google.com/d/forum/navitia
# www.navitia.io

from __future__ import absolute_import, print_function, unicode_literals, division

from importlib import import_module
from itertools import chain
from navitiacommon import type_pb2

import logging


class EquipmentProviderManager(object):
    def __init__(self, equipment_providers_configuration):
        self.logger = logging.getLogger(__name__)
        self.providers_config = equipment_providers_configuration
        self._equipment_providers_legacy = {}
        self._equipment_providers = {}

    def init_providers(self, providers_keys):
        """
        Create equipment providers only if defined in the Jormungandr instance and not already created
        :param providers_keys: list of providers defined in the instance
        """
        for provider in self.providers_config:
            key = provider['key'].lower()
            if key in providers_keys and key not in dict(
                self._equipment_providers, **self._equipment_providers_legacy
            ):
                self._equipment_providers_legacy[key] = self._init_class(provider['class'], provider['args'])

    def _init_class(self, cls, arguments):
        """
        Create an instance of a provider according to config
        :param cls: provider class in Jormungandr found in config file
        :param arguments: parameters to set in the provider class
        :return: instance of provider
        """
        try:
            if '.' not in cls:
                self.log.warn('impossible to build, wrongly formated class: {}'.format(cls))

            module_path, name = cls.rsplit('.', 1)
            module = import_module(module_path)
            attr = getattr(module, name)
            return attr(**arguments)
        except ImportError:
            self.log.warn('impossible to build, cannot find class: {}'.format(cls))

    def manage_equipments(self, response):
        """
        Call equipment details provider to update response
        :param response: the pb response received
        :return: response: the pb response updated with equipment details
        """

        def get_from_to_stop_points_of_journeys(journeys):
            """
            :param journeys: the pb journey response
            :return: generator of stop points that can be 'origin' or 'destination'
            """
            places = chain(*[(s.origin, s.destination) for j in journeys for s in j.sections])
            return (
                place.stop_point
                for place in places
                if place.embedded_type == type_pb2.NavitiaType.Value('STOP_POINT')
                and place.HasField('stop_point')
            )

        stop_points = get_from_to_stop_points_of_journeys(response.journeys)

        for provider in dict(self._equipment_providers, **self._equipment_providers_legacy).values():
            provider.get_informations(stop_points)

        return response
