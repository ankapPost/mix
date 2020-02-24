/******************************** web server **********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#include <config/Config.h>

#if defined(ENABLE_EWING_HTTP_SERVER)

#include "EwingsWebServer.h"


/**
 * start http server functionality. this requires wifi should work as access point
 */
void WebServer::start_server( ESP8266WiFiClass* _wifi ){

  this->wifi = _wifi;
  __web_resource.collect_resource( &this->server, this->wifi );

  this->home_controller.boot();
  this->login_controller.boot();
  this->dashboard_controller.boot();
  this->ota_controller.boot();
  this->wificonfig_controller.boot();
  #ifdef ENABLE_GPIO_SERVICE
  this->gpio_controller.boot();
  #endif
  #ifdef ENABLE_MQTT_SERVICE
  this->mqtt_controller.boot();
  #endif
  #ifdef ENABLE_EMAIL_SERVICE
  this->emailconfig_controller.boot();
  #endif

  //here the list of headers to be recorded
  // const char * headerkeys[] = {"User-Agent", "Cookie"} ;
  const char * headerkeys[] = {"Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
  //ask server to track these headers
  this->server.collectHeaders(headerkeys, headerkeyssize);
  this->server.begin();
  #ifdef EW_SERIAL_LOG
  Logln(F("HTTP server started"));
  #endif
}

/**
 * handle server functionality.
 */
void WebServer::handle_clients(){

  this->server.handleClient();
}

WebServer __web_server;

#endif
