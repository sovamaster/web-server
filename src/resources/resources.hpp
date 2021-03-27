/*!
 * Copyright (c) 2015-2018, Andrei Chudinovich
 * Released under MIT license
 * Date: 03.09.2018
 */


#ifndef HTTP_RESOURCES_HPP
#define HTTP_RESOURCES_HPP

#include <utils/util.h>

#include "../net/httpServer.hpp"
#include "../net/httpClient.hpp"
#include "../config.hpp"

#include <jsoncons/json.hpp>
#include <pugixml/pugixml.hpp>
#include <libpq-fe.h>


#pragma once


namespace res
{
	struct gpsPoint
	{
		double lat = 0.0;
		double lon = 0.0;
		int timepoint = 0;
	};

	int getGpsDistance(gpsPoint& adv_point, gpsPoint& place_point)
	{
		int res = -1;
		try
		{
			//55.626490, 37.740955  //55.624641, 37.739850
			//double src_lat = 55.626490, dst_lat = 55.624641, src_lon = 37.740955, dst_lon = 37.739850;

			double PI = 3.14159265358979323846;
			res = (int)((6371 * 2 * asin(sqrt(pow(sin((adv_point.lat - fabs(place_point.lat)) * PI / 180 / 2), 2) + cos(adv_point.lat * PI / 180) * cos(fabs(place_point.lat) * PI / 180) * pow(sin((adv_point.lon - place_point.lon) * PI / 180 / 2), 2)))) * 1000);
		}
		catch (...)
		{
			util::writeLogLine("Exeption in getGpsDistance function", util::message_type.failure, config::SOURCE_TYPE.server);
			return -1;
		}
		return res;
	}

	jsc::json getResultItem(std::string status, std::string descr, std::string id)
	{
		jsc::json j_result_item = jsc::json::object();
		if (id != "") j_result_item["id"] = id;

		jsc::json j_result = jsc::json::object();
		j_result["status"] = status;
		j_result["descr"] = descr;

		j_result_item["result"] = j_result;

		return j_result_item;
	}
}
#endif