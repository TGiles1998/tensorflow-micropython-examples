// Include MicroPython API.
#include <string.h>
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/objstr.h"
#include "py/mperrno.h"
#include "esp_wifi.h"
#include "esp_log.h"

// This is the function which will be called from Python as enterprise.encrypt_key(a, b).
//STATIC mp_obj_t enterprise_connect(const mp_obj_t ssid, const mp_obj_t username, const mp_obj_t identity, const mp_obj_t password) {
//    // Extract the ints from the micropython input objects.
////    #define SSID "eduroam"
////    #define WPA2_USERNAME "username@ed.ac.uk"
////    #define WPA2_IDENTITY WPA2_USERNAME
////    #define WPA2_PASSWORD "password"
//
//    mp_check_self(mp_obj_is_str_or_bytes(ssid));
//    GET_STR_DATA_LEN(ssid, str, str_len);
//    printf("WPA_SSID length: %lu\n", str_len);
//    char WPA_SSID[str_len];
//    strcpy(WPA_SSID, (char *)str);
//
//    mp_check_self(mp_obj_is_str_or_bytes(username));
//    GET_STR_DATA_LEN(username, str, str_len);
//    printf("WPA2_USERNAME length: %lu\n", str_len);
//    char WPA_SSID[str_len];
//    strcpy(WPA_SSID, (char *)str);
//
//    mp_check_self(mp_obj_is_str_or_bytes(identity));
//    GET_STR_DATA_LEN(identity, str, str_len);
//    printf("WPA2_IDENTITY length: %lu\n", str_len);
//    char WPA_SSID[str_len];
//    strcpy(WPA_SSID, (char *)str);
//
//    mp_check_self(mp_obj_is_str_or_bytes(password));
//    GET_STR_DATA_LEN(password, str, str_len);
//    printf("WPA2_PASSWORD length: %lu\n", str_len);
//    char WPA_SSID[str_len];
//    strcpy(WPA_SSID, (char *)str);
//
//    // Disconnect from wifi
//    WiFi.disconnect(true);
//
//    // Initialise wifi mode
//    WiFi.mode(WIFI_STA);
//    //    modespnetwork
//    // Initialise serial
//    Serial.begin(115200);
//    delay(10);
//
//    Serial.print("Trying to connect to ");
//    Serial.println(WPA_SSID);
//
//    // Set wpa2 details
//    esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)WPA2_IDENTITY, strlen(WPA2_IDENTITY));
//    esp_wifi_sta_wpa2_ent_set_username((uint8_t *)WPA2_USERNAME, strlen(WPA2_USERNAME));
//    esp_wifi_sta_wpa2_ent_set_password((uint8_t *)WPA2_PASSWORD, strlen(WPA2_PASSWORD));
//
//    // Set wpa2 wifi configuration to default settings
//    esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT();
//
//    // Set configuration settings to enable function
//    esp_wifi_sta_wpa2_ent_enable(&config);
//
//    // Connect to wifi
//    WiFi.begin(SSID);
//
//    while (WiFi.status() != WL_CONNECTED) {
//      delay(500);
//      Serial.print(".");
//      counter++;
//      if(counter>=60){
//        ESP.restart();
//      }
//    }
//
//    Serial.println("");
//    Serial.println("WiFi connected");
//    Serial.println("IP address set: ");
//    Serial.println(WiFi.localIP());
//
//
////    mp_check_self(mp_obj_is_str_or_bytes(ssid));
////    GET_STR_DATA_LEN(o_in, str, str_len);
////    printf("string length: %lu\n", str_len);
////    char out_str[str_len];
////    strcpy(out_str, (char *)str);
////    for(size_t i=0; i < (str_len-1)/2; i++) {
////        byteswap(out_str[i], out_str[str_len-i-1]);
////    }
////
////    const char * key_string = mp_obj_get_type_str(input);
////    int size_input_int = mp_obj_get_int(size_input);
////    const char * extern_pubkey_string = mp_obj_get_type_str(extern_pubkey);
////    const char * wrapped_key_string = mp_obj_get_type_str(output);
////
////    // Convert from string to key string array
////    uint8_t key_string_array[32];
////    memcpy(key_string_array, key_string, 32);
////
////    // Convert from string to key string array
////    uint8_t extern_pubkey_string_array[64];
////    memcpy(extern_pubkey_string_array, extern_pubkey_string, 64);
////
////    // Convert from string to key string array
////    uint8_t wrapped_key_string_array[132];
////    memcpy(wrapped_key_string_array, wrapped_key_string, 132);
////
////
////    // Calculate the addition and convert to MicroPython object.
////    // return mp_obj_new_int(a + b);
////
////    //    uint8_t * input, unsigned size_input, uint8_t * extern_pubkey, uint8_t * output
////    // key, 32, extern_pubkey, wraped_key
////    return ECIES_encrypt_key(key_string_array, size_input_int, extern_pubkey_string_array, wrapped_key_string_array);
//}
// Define a Python reference to the function above.Tes
//STATIC MP_DEFINE_CONST_FUN_OBJ_2(enterprise_connect_obj, enterprise_connect);


//Set up EAP
//STATIC mp_obj_t enterprise_connect(mp_obj_t username, mp_obj_t password) { // mp_obj_t self_in,
//    size_t Ilen;
//    size_t Plen;
//    const char *EAP_IDENTITY = mp_obj_str_get_data(username, &Ilen);
//    const char *EAP_PASSWORD = mp_obj_str_get_data(password, &Plen);
//
//    printf(EAP_IDENTITY);
//    printf(EAP_PASSWORD);
//
//    // Initialise wifi mode
////    WiFi.mode(WIFI_STA);
//
////    esp_exceptions();
//    esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
//
////    esp_exceptions();
//    esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
//
//    //esp_exceptions(esp_wifi_sta_wpa2_ent_set_new_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD)));
//
////    esp_exceptions();
//    esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
//
//    // Set wpa2 wifi configuration to default settings
////    esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT();
//
//    // Set configuration settings to enable function
////    esp_exceptions();
//    esp_wifi_sta_wpa2_ent_enable();//&config
//
//    return mp_const_none;
//}
//
//STATIC MP_DEFINE_CONST_FUN_OBJ_2(enterprise_connect_obj, enterprise_connect);

NORETURN void my_esp_exceptions_helper(esp_err_t e) {
    switch (e) {
        case ESP_ERR_WIFI_NOT_INIT:
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Wifi Not Initialized"));
        case ESP_ERR_WIFI_NOT_STARTED:
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Wifi Not Started"));
        case ESP_ERR_WIFI_NOT_STOPPED:
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Wifi Not Stopped"));
        case ESP_ERR_WIFI_IF:
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Wifi Invalid Interface"));
        case ESP_ERR_WIFI_MODE:
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Wifi Invalid Mode"));
        case ESP_ERR_WIFI_STATE:
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Wifi Internal State Error"));
        case ESP_ERR_WIFI_CONN:
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Wifi Internal Error"));
        case ESP_ERR_WIFI_NVS:
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Wifi Internal NVS Error"));
        case ESP_ERR_WIFI_MAC:
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Wifi Invalid MAC Address"));
        case ESP_ERR_WIFI_SSID:
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Wifi SSID Invalid"));
        case ESP_ERR_WIFI_PASSWORD:
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Wifi Invalid Password"));
        case ESP_ERR_WIFI_TIMEOUT:
            mp_raise_OSError(MP_ETIMEDOUT);
        case ESP_ERR_WIFI_WAKE_FAIL:
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Wifi Wakeup Failure"));
        case ESP_ERR_WIFI_WOULD_BLOCK:
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Wifi Would Block"));
        case ESP_ERR_WIFI_NOT_CONNECT:
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Wifi Not Connected"));
        case ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS:
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("TCP/IP Invalid Parameters"));
        case ESP_ERR_TCPIP_ADAPTER_IF_NOT_READY:
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("TCP/IP IF Not Ready"));
        case ESP_ERR_TCPIP_ADAPTER_DHCPC_START_FAILED:
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("TCP/IP DHCP Client Start Failed"));
        case ESP_ERR_TCPIP_ADAPTER_NO_MEM:
            mp_raise_OSError(MP_ENOMEM);
        default:
            mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("Wifi Unknown Error 0x%04x"), e);
    }
}

//then add...
//Set up EAP
STATIC mp_obj_t enterprise_connect(mp_obj_t username, mp_obj_t password){// mp_obj_t self_in,
    mp_printf(MICROPY_ERROR_PRINTER, "\n esp_seteap \n");
    size_t Ilen;
    size_t Plen;
    const char *EAP_IDENTITY = mp_obj_str_get_data(username, &Ilen);
    const char *EAP_PASSWORD = mp_obj_str_get_data(password, &Plen);
    mp_printf(MICROPY_ERROR_PRINTER, EAP_IDENTITY);
    mp_printf(MICROPY_ERROR_PRINTER, EAP_PASSWORD);


    my_esp_exceptions_helper(esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY)));
    mp_printf(MICROPY_ERROR_PRINTER, "\n esp_wifi_sta_wpa2_ent_set_identity \n");
    my_esp_exceptions_helper(esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY)));
    mp_printf(MICROPY_ERROR_PRINTER, "\n esp_wifi_sta_wpa2_ent_set_username \n");
    //esp_wifi_sta_wpa2_ent_set_new_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
    my_esp_exceptions_helper(esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD)));
    mp_printf(MICROPY_ERROR_PRINTER, "\n esp_wifi_sta_wpa2_ent_set_password \n");
    my_esp_exceptions_helper(esp_wifi_sta_wpa2_ent_enable()));
    mp_printf(MICROPY_ERROR_PRINTER, "\n esp_wifi_sta_wpa2_ent_enable \n");
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(enterprise_connect_obj, enterprise_connect);


STATIC const mp_rom_map_elem_t enterprise_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_enterprise) },
    { MP_ROM_QSTR(MP_QSTR_set_username_pass), MP_ROM_PTR(&enterprise_connect_obj) },
};

STATIC MP_DEFINE_CONST_DICT(enterprise_module_globals, enterprise_module_globals_table);

// Define module object.
const mp_obj_module_t enterprise_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&enterprise_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_enterprise, enterprise_module);
