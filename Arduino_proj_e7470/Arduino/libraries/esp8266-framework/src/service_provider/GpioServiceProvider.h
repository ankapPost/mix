/****************************** Gpio service **********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _GPIO_SERVICE_PROVIDER_H_
#define _GPIO_SERVICE_PROVIDER_H_


#include <service_provider/ServiceProvider.h>
#include <service_provider/HttpServiceProvider.h>
#include <service_provider/EmailServiceProvider.h>
#include <WiFiClient.h>


/**
 * GpioServiceProvider class
 */
class GpioServiceProvider : public ServiceProvider {

  public:

    /**
     * GpioServiceProvider constructor.
     */
    GpioServiceProvider(){
    }

    /**
		 * GpioServiceProvider destructor
		 */
    ~GpioServiceProvider(){
    }

    /**
		 * @var gpio_config_table virtual_gpio_configs
		 */
    gpio_config_table virtual_gpio_configs;
    /**
		 * @var	int|0 _gpio_http_request_cb_id
		 */
    int _gpio_http_request_cb_id=0;
    /**
		 * @var	bool|true update_gpio_table_from_virtual
		 */
    bool update_gpio_table_from_virtual=true;


    void begin( ESP8266WiFiClass* _wifi, WiFiClient* _wifi_client );
    void enable_update_gpio_table_from_virtual( void );
    #ifdef ENABLE_EMAIL_SERVICE
    bool handleGpioEmailAlert( void );
    #endif
    void handleGpioOperations( void );
    void handleGpioModes( int _gpio_config_type=GPIO_MODE_CONFIG );
    uint8_t getGpioFromPinMap( uint8_t _pin );
    void handleGpioHttpRequest( void );
    #ifdef EW_SERIAL_LOG
    void printGpioConfigLogs( void );
    #endif
    bool is_exceptional_gpio_pin( uint8_t _pin );

  protected:

    /**
		 * @var	WiFiClient  wifi_client
		 */
    WiFiClient* wifi_client;
    ESP8266WiFiClass* wifi;

};

extern GpioServiceProvider __gpio_service;

#endif
