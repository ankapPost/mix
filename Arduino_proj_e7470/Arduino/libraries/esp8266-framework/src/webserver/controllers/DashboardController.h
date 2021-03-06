/*************************** Dashboard Controller *****************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _EW_SERVER_DASHBOARD_CONTROLLER_
#define _EW_SERVER_DASHBOARD_CONTROLLER_

#include "Controller.h"
#include <webserver/pages/Dashboard.h>

/**
 * DashboardController class
 */
class DashboardController : public Controller {

	public:

		/**
		 * DashboardController constructor
		 */
		DashboardController(){
		}

		/**
		 * DashboardController destructor
		 */
		~DashboardController(){
		}

		/**
		 * register dashboard controller
		 *
		 */
		void boot( void ){

			this->route_handler->register_route( EW_SERVER_DASHBOARD_ROUTE, [&]() { this->handleDashboardRoute(); }, AUTH_MIDDLEWARE );
      this->route_handler->register_route( EW_SERVER_DASHBOARD_MONITOR_ROUTE, [&]() { this->handleDashboardMonitor(); } );
		}

		/**
		 * handle dashboard monitor calls. it provides live dashborad parameters.
		 */
		void handleDashboardMonitor( void ) {

      #ifdef EW_SERIAL_LOG
      Logln(F("Handling dashboard monitor route"));
      #endif

			struct station_info * stat_info = wifi_softap_get_station_info();
			int n = 1;

      String _response = "{\"nm\":\"";
      _response += this->web_resource->wifi->SSID();
			_response += "\",\"ip\":\"";
      _response += this->web_resource->wifi->localIP().toString();
      _response += "\",\"rs\":\"";
      _response += this->web_resource->wifi->RSSI();
      _response += "\",\"mc\":\"";
      _response += this->web_resource->wifi->macAddress();
      _response += "\",\"st\":";
      _response += this->web_resource->wifi->status();
			_response += ",\"nt\":";
      _response += __status_wifi.internet_available;
			_response += ",\"dl\":\"";

			while (stat_info != NULL) {
				char macStr[30];
				memset(macStr, 0, 30);
			  sprintf(
					macStr,
					"%02X:%02X:%02X:%02X:%02X:%02X",
					stat_info->bssid[0], stat_info->bssid[1], stat_info->bssid[2], stat_info->bssid[3], stat_info->bssid[4], stat_info->bssid[5]
				);
				_response += "<tr><td>";
				_response += String(macStr);
				_response += "</td><td>";
				_response += (uint8_t)stat_info->ip.addr;
				_response += ".";
				_response += (uint8_t)(stat_info->ip.addr>>8);
				_response += ".";
				_response += (uint8_t)(stat_info->ip.addr>>16);
				_response += ".";
				_response += (uint8_t)(stat_info->ip.addr>>24);
				_response += "</td></tr>";
				stat_info = STAILQ_NEXT(stat_info, next);
      }
      _response += "\",\"r\":";
      _response += !this->route_handler->has_active_session();
      _response += "}";

      this->web_resource->server->sendHeader("Cache-Control", "no-cache");
      this->web_resource->server->send( HTTP_OK, EW_HTML_CONTENT, _response );
    }

		/**
		 * handle dashboard page route. it build & send dashboard html to client.
		 */
    void handleDashboardRoute( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling dashboard route"));
      #endif
      bool _is_posted = false;

      char* _page = new char[EW_HTML_MAX_SIZE];
      memset( _page, 0, EW_HTML_MAX_SIZE );

      strcat_P( _page, EW_SERVER_HEADER_HTML );
      strcat_P( _page, EW_SERVER_DASHBOARD_PAGE );
      strcat_P( _page, EW_SERVER_FOOTER_WITH_DASHBOARD_MONITOR_HTML );

      this->web_resource->server->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;

    }

};

#endif
