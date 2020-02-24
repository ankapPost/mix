/****************************** Gpio service **********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/
#include <config/Config.h>

#if defined(ENABLE_GPIO_SERVICE)

#include "GpioServiceProvider.h"

__gpio_alert_track_t __gpio_alert_track = {
  false, 0
};

/**
 * start gpio services if enabled
 */
void GpioServiceProvider::begin( ESP8266WiFiClass* _wifi, WiFiClient* _wifi_client ){

  this->wifi = _wifi;
  this->wifi_client = _wifi_client;

  this->handleGpioModes();
  this->virtual_gpio_configs = __database_service.get_gpio_config_table();

  __task_scheduler.setInterval( [&]() { this->handleGpioOperations(); }, GPIO_OPERATION_DURATION );
  __task_scheduler.setInterval( [&]() { this->enable_update_gpio_table_from_virtual(); }, GPIO_TABLE_UPDATE_DURATION );
}

/**
 * post gpio data to server specified in gpio configs
 */
void GpioServiceProvider::handleGpioHttpRequest( ){

  memset( __http_service.host, 0, HTTP_HOST_ADDR_MAX_SIZE);
  strcpy( __http_service.host, this->virtual_gpio_configs.gpio_host );
  __http_service.port = this->virtual_gpio_configs.gpio_port;

  #ifdef EW_SERIAL_LOG
  Logln( F("Handling GPIO Http Request") );
  #endif

  if( strlen( __http_service.host ) > 5 && __http_service.port > 0 &&
    this->virtual_gpio_configs.gpio_post_frequency > 0 &&
    __http_service.client.begin( *this->wifi_client, __http_service.host )
  ){

    String _payload = "{\"data\":{";

    for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS; _pin++) {

      if( !this->is_exceptional_gpio_pin(_pin) ){

        _payload += "\"D";
        _payload += _pin;
        _payload += "\":{\"mode\":";
        _payload += this->virtual_gpio_configs.gpio_mode[_pin];
        _payload += ",\"val\":";
        _payload += this->virtual_gpio_configs.gpio_readings[_pin];
        _payload += "},";
      }
    }
    _payload += "\"A0\":{\"mode\":";
    _payload += this->virtual_gpio_configs.gpio_mode[MAX_NO_OF_GPIO_PINS];
    _payload += ",\"val\":";
    _payload += this->virtual_gpio_configs.gpio_readings[MAX_NO_OF_GPIO_PINS];
    _payload += "}}}";

    __http_service.client.setUserAgent("Ewings");
    __http_service.client.addHeader("Content-Type", "application/json");
    __http_service.client.setAuthorization("user", "password");
    __http_service.client.setTimeout(2000);

    int _httpCode = __http_service.client.POST( _payload );

    if( __http_service.followHttpRequest( _httpCode ) ) this->handleGpioHttpRequest();

  }else{
    #ifdef EW_SERIAL_LOG
    Logln( F("GPIO Http Request not initializing or failed or Not Configured Correctly") );
    #endif
  }
}

#ifdef ENABLE_EMAIL_SERVICE
/**
 * handle gpio email alert
 * @return bool
 */
bool GpioServiceProvider::handleGpioEmailAlert(){

  #ifdef EW_SERIAL_LOG
  Logln( F("Handling GPIO email alert") );
  #endif
  String _payload = "";
  for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS; _pin++) {

    if( !this->is_exceptional_gpio_pin(_pin) ){

      _payload += "D";
      _payload += _pin;
      _payload += " ( mode : ";
      _payload += this->virtual_gpio_configs.gpio_mode[_pin];
      _payload += ", val : ";
      _payload += this->virtual_gpio_configs.gpio_readings[_pin];
      _payload += ")\n";
    }
  }
  _payload += "A0 ( mode : ";
  _payload += this->virtual_gpio_configs.gpio_mode[MAX_NO_OF_GPIO_PINS];
  _payload += ", val : ";
  _payload += this->virtual_gpio_configs.gpio_readings[MAX_NO_OF_GPIO_PINS];
  _payload += ")\n\n";
  _payload += "Hello from Esp\n";
  _payload += this->wifi->macAddress();

  return __email_service.sendMail( _payload );
}
#endif

/**
 * handle gpio operations as per gpio configs
 */
void GpioServiceProvider::handleGpioOperations(){

  for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS+1; _pin++) {

    switch ( this->is_exceptional_gpio_pin(_pin) ? OFF : this->virtual_gpio_configs.gpio_mode[_pin] ) {

      default:
      case OFF:{
        this->virtual_gpio_configs.gpio_readings[_pin] = LOW;
        break;
      }
      case DIGITAL_WRITE:{
        // Log( F("writing "));Log( this->getGpioFromPinMap( _pin ) );Log( F(" pin to "));Logln( this->virtual_gpio_configs.gpio_readings[_pin]);
        digitalWrite( this->getGpioFromPinMap( _pin ), this->virtual_gpio_configs.gpio_readings[_pin] );
        break;
      }
      case DIGITAL_READ:{
        this->virtual_gpio_configs.gpio_readings[_pin] = digitalRead( this->getGpioFromPinMap( _pin ) );
        break;
      }
      case ANALOG_WRITE:{
        analogWrite( this->getGpioFromPinMap( _pin ), this->virtual_gpio_configs.gpio_readings[_pin] );
        break;
      }
      case ANALOG_READ:{
        if( _pin == MAX_NO_OF_GPIO_PINS )
        this->virtual_gpio_configs.gpio_readings[_pin] = analogRead( A0 );
        break;
      }
    }

    if( !this->is_exceptional_gpio_pin(_pin) && this->virtual_gpio_configs.gpio_alert_channel[_pin] != NO_ALERT ){

      bool _is_alert_condition = false;

      switch ( this->virtual_gpio_configs.gpio_alert_comparator[_pin] ) {

        case EQUAL:{
          _is_alert_condition = ( this->virtual_gpio_configs.gpio_readings[_pin] == this->virtual_gpio_configs.gpio_alert_values[_pin] );
          break;
        }
        case GREATER_THAN:{
          _is_alert_condition = ( this->virtual_gpio_configs.gpio_readings[_pin] > this->virtual_gpio_configs.gpio_alert_values[_pin] );
          break;
        }
        case LESS_THAN:{
          _is_alert_condition = ( this->virtual_gpio_configs.gpio_readings[_pin] < this->virtual_gpio_configs.gpio_alert_values[_pin] );
          break;
        }
        default: break;
      }

      uint32_t _now = millis();
      if( _is_alert_condition && ( __gpio_alert_track.is_last_alert_succeed ?
        GPIO_ALERT_DURATION_FOR_SUCCEED < ( _now - __gpio_alert_track.last_alert_millis ) :
        GPIO_ALERT_DURATION_FOR_FAILED < ( _now - __gpio_alert_track.last_alert_millis )
      ) ){

        __gpio_alert_track.last_alert_millis = _now;
        switch ( this->virtual_gpio_configs.gpio_alert_channel[_pin] ) {

          #ifdef ENABLE_EMAIL_SERVICE
          case EMAIL:{
            __gpio_alert_track.is_last_alert_succeed = this->handleGpioEmailAlert();
            break;
          }
          #endif
          case NO_ALERT:
          default: break;
        }
      }
    }

  }

  if( this->update_gpio_table_from_virtual ){
    __database_service.set_gpio_config_table(&this->virtual_gpio_configs);
    this->update_gpio_table_from_virtual = false;
  }

}

/**
 * enable gpio data update to database from virtual table
 */
void GpioServiceProvider::enable_update_gpio_table_from_virtual(){
  this->update_gpio_table_from_virtual = true;
}

/**
 * handle gpio modes for their operations.
 */
void GpioServiceProvider::handleGpioModes( int _gpio_config_type ){

  gpio_config_table _gpio_configs = __database_service.get_gpio_config_table();

  for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS+1; _pin++) {

    switch ( _gpio_configs.gpio_mode[_pin] ) {

      case OFF:
      case DIGITAL_READ:
      default:
        pinMode( this->getGpioFromPinMap( _pin ), INPUT );
        break;
      case DIGITAL_WRITE:
      case ANALOG_WRITE:
        pinMode( this->getGpioFromPinMap( _pin ), OUTPUT );
        break;
      case ANALOG_READ:
      break;
    }

  }

  this->virtual_gpio_configs = _gpio_configs;

  if( strlen( this->virtual_gpio_configs.gpio_host ) > 5 && this->virtual_gpio_configs.gpio_port > 0 &&
    this->virtual_gpio_configs.gpio_post_frequency > 0
  ){
    this->_gpio_http_request_cb_id = __task_scheduler.updateInterval(
      this->_gpio_http_request_cb_id,
      [&]() { this->handleGpioHttpRequest(); },
      this->virtual_gpio_configs.gpio_post_frequency*MILLISECOND_DURATION_1000
    );
  }else{
    __task_scheduler.clearInterval( this->_gpio_http_request_cb_id );
    this->_gpio_http_request_cb_id = 0;
  }

}

#ifdef EW_SERIAL_LOG
/**
 * print gpio configs
 */
void GpioServiceProvider::printGpioConfigLogs(){


  Logln(F("\nGPIO Configs (mode) :"));
  // Logln(F("ssid\tpassword\tlocal\tgateway\tsubnet"));
  for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS+1; _pin++) {
    Log(this->virtual_gpio_configs.gpio_mode[_pin]); Log("\t");
  }
  Logln(F("\nGPIO Configs (readings) :"));
  for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS+1; _pin++) {
    Log(this->virtual_gpio_configs.gpio_readings[_pin]); Log("\t");
  }
  Logln(F("\nGPIO Configs (alert comparator) :"));
  for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS+1; _pin++) {
    Log(this->virtual_gpio_configs.gpio_alert_comparator[_pin]); Log("\t");
  }
  Logln(F("\nGPIO Configs (alert channels) :"));
  for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS+1; _pin++) {
    Log(this->virtual_gpio_configs.gpio_alert_channel[_pin]); Log("\t");
  }
  Logln(F("\nGPIO Configs (alert values) :"));
  for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS+1; _pin++) {
    Log(this->virtual_gpio_configs.gpio_alert_values[_pin]); Log("\t");
  }
  Logln(F("\nGPIO Configs (server) :"));
  Log(this->virtual_gpio_configs.gpio_host); Log("\t");
  Log(this->virtual_gpio_configs.gpio_port); Log("\t");
  Log(this->virtual_gpio_configs.gpio_post_frequency); Log("\n\n");
}
#endif

/**
 * get gpio mapped pin from its no.
 */
uint8_t GpioServiceProvider::getGpioFromPinMap( uint8_t _pin ){

  uint8_t mapped_pin;

  // Map
  switch ( _pin ) {

    case 0:
      mapped_pin = 16;
      break;
    case 1:
      mapped_pin = 5;
      break;
    case 2:
      mapped_pin = 4;
      break;
    case 3:
      mapped_pin = 0;
      break;
    case 4:
      mapped_pin = 2;
      break;
    case 5:
      mapped_pin = 14;
      break;
    case 6:
      mapped_pin = 12;
      break;
    case 7:
      mapped_pin = 13;
      break;
    case 8:
      mapped_pin = 15;
      break;
    case 9:
      mapped_pin = 3;
      break;
    case 10:
      mapped_pin = 1;
      break;
    default:
      mapped_pin = 0;
  }

  return mapped_pin;
}

bool GpioServiceProvider::is_exceptional_gpio_pin (uint8_t _pin) {

  for (uint8_t j = 0; j < sizeof(EXCEPTIONAL_GPIO_PINS); j++) {

    if( EXCEPTIONAL_GPIO_PINS[j] == _pin )return true;
  }
  return false;
}

// int getPinMode(uint8_t pin){
//
//   if (pin >= NUM_DIGITAL_PINS) return (-1);
//
//   uint8_t bit = digitalPinToBitMask(pin);
//   uint8_t port = digitalPinToPort(pin);
//   volatile uint8_t *reg = portModeRegister(port);
//   if (*reg & bit) return (OUTPUT);
//
//   volatile uint8_t *out = portOutputRegister(port);
//   return ((*out & bit) ? INPUT_PULLUP : INPUT);
// }


GpioServiceProvider __gpio_service;

#endif
