#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "py/obj.h"
#include "py/objstr.h"
#include "py/runtime.h"
#include "mphalport.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_wpa2.h"
#include "esp_err.h"

#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* esp netif object representing the WIFI station */
static esp_netif_t *sta_netif = NULL;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;

static const char *TAG = "example";

esp_err_t driver_wpa2enterprise_init() { return ESP_OK; }

#define WIFI_SORT_ERRCHECK(err) do {int res = (err); if(res) {printf("Enterprise WiFi connection error: %s\n", esp_err_to_name(res)); } } while(0)

static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
    }
}

//------------------------------------------------------------------------
STATIC mp_obj_t mod_wpa2enterprise_connect (mp_uint_t n_args, const mp_obj_t *args) {
    const char* aSsid = mp_obj_str_get_str(args[0]);
    mp_printf(MICROPY_ERROR_PRINTER, aSsid);
    const char* aAnonIdent = mp_obj_str_get_str(args[1]);
    mp_printf(MICROPY_ERROR_PRINTER, aAnonIdent);
    const char* aIdent = mp_obj_str_get_str(args[2]);
    mp_printf(MICROPY_ERROR_PRINTER, aIdent);
    const char* aPassword = mp_obj_str_get_str(args[3]);
    mp_printf(MICROPY_ERROR_PRINTER, aPassword);
//    int phase2 = mp_obj_get_int(args[4]);

//    #define EXAMPLE_EAP_METHOD CONFIG_EXAMPLE_EAP_METHOD

//    #ifdef CONFIG_EXAMPLE_VALIDATE_SERVER_CERT
//    unsigned int ca_pem_bytes = ca_pem_end - ca_pem_start;
//    #endif /* CONFIG_EXAMPLE_VALIDATE_SERVER_CERT */

//    #ifdef CONFIG_EXAMPLE_EAP_METHOD_TLS
//    unsigned int client_crt_bytes = client_crt_end - client_crt_start;
//        unsigned int client_key_bytes = client_key_end - client_key_start;
//    #endif /* CONFIG_EXAMPLE_EAP_METHOD_TLS */

    ESP_ERROR_CHECK(esp_netif_init());
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );

    wifi_config_t wifi_config = {0};
    mp_printf(MICROPY_ERROR_PRINTER, "\n wifi_config \n");
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wstringop-truncation"
    mp_printf(MICROPY_ERROR_PRINTER, "\n pragmad this ting \n");
    strncpy((char*) wifi_config.sta.ssid, aSsid, 32);
    mp_printf(MICROPY_ERROR_PRINTER, "\n set strncpy \n");
//    wifi_config_t wifi_config = {
//            .sta = {
//                    .ssid = *aSsid,
//    #if defined (CONFIG_EXAMPLE_WPA3_192BIT_ENTERPRISE)
//                    .pmf_cfg = {
//                    .required = true
//                },
//    #endif
//            },
//    };

//    ESP_ERROR_CHECK( mp_print_str(MICROPY_ERROR_PRINTER, wifi_config.sta.ssid) );
    mp_printf(MICROPY_ERROR_PRINTER, "\n Ok gonna try Setting WiFi configuration SSID\n");
    ESP_ERROR_CHECK( mp_printf(MICROPY_ERROR_PRINTER, wifi_config.sta.ssid) );
//    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    mp_printf(MICROPY_ERROR_PRINTER, "\n running esp_wifi_set_mode\n");
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    mp_printf(MICROPY_ERROR_PRINTER, "\n ran esp_wifi_set_mode\n");

    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    mp_printf(MICROPY_ERROR_PRINTER, "\n ran esp_wifi_set_config\n");
    ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)aIdent, strlen(aIdent)) );
    mp_printf(MICROPY_ERROR_PRINTER, "\n ran esp_wifi_sta_wpa2_ent_set_identity\n");

    #if defined(CONFIG_EXAMPLE_VALIDATE_SERVER_CERT) || \
        defined(CONFIG_EXAMPLE_WPA3_ENTERPRISE) || \
        defined(CONFIG_EXAMPLE_WPA3_192BIT_ENTERPRISE)
    ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_ca_cert(ca_pem_start, ca_pem_bytes) );
    mp_printf(MICROPY_ERROR_PRINTER, "\n ran esp_wifi_sta_wpa2_ent_set_ca_cert\n");
    #endif /* CONFIG_EXAMPLE_VALIDATE_SERVER_CERT */ /* EXAMPLE_WPA3_ENTERPRISE */

    #ifdef CONFIG_EXAMPLE_EAP_METHOD_TLS
    ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_cert_key(client_crt_start, client_crt_bytes,\
                client_key_start, client_key_bytes, NULL, 0) );
    mp_printf(MICROPY_ERROR_PRINTER, "\n ran esp_wifi_sta_wpa2_ent_set_cert_key\n");
    #endif /* CONFIG_EXAMPLE_EAP_METHOD_TLS */

    #if defined CONFIG_EXAMPLE_EAP_METHOD_PEAP || CONFIG_EXAMPLE_EAP_METHOD_TTLS
    ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_username((uint8_t *)aAnonIdent, strlen(aAnonIdent)) );
        ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_password((uint8_t *)aPassword, strlen(aPassword)) );
        mp_printf(MICROPY_ERROR_PRINTER, "\n ran esp_wifi_sta_wpa2_ent_set_password\n");
    #endif /* CONFIG_EXAMPLE_EAP_METHOD_PEAP || CONFIG_EXAMPLE_EAP_METHOD_TTLS */

    #if defined CONFIG_EXAMPLE_EAP_METHOD_TTLS
    ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_ttls_phase2_method(TTLS_PHASE2_METHOD) );
    mp_printf(MICROPY_ERROR_PRINTER, "\n ran esp_wifi_sta_wpa2_ent_set_ttls_phase2_method\n");
    #endif /* CONFIG_EXAMPLE_EAP_METHOD_TTLS */
    #if defined (CONFIG_EXAMPLE_WPA3_192BIT_ENTERPRISE)
    ESP_LOGI(TAG, "Enabling 192 bit certification");
        ESP_ERROR_CHECK(esp_wifi_sta_wpa2_set_suiteb_192bit_certification(true));
        mp_printf(MICROPY_ERROR_PRINTER, "\n ran esp_wifi_sta_wpa2_set_suiteb_192bit_certification\n");
    #endif
    #ifdef CONFIG_EXAMPLE_USE_DEFAULT_CERT_BUNDLE
    ESP_ERROR_CHECK(esp_wifi_sta_wpa2_use_default_cert_bundle(true));
    mp_printf(MICROPY_ERROR_PRINTER, "\n ran esp_wifi_sta_wpa2_use_default_cert_bundle\n");
    #endif
    ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_enable() );
    mp_printf(MICROPY_ERROR_PRINTER, "\n ran esp_wifi_sta_wpa2_ent_enable\n");
    ESP_ERROR_CHECK( esp_wifi_start() );
    mp_printf(MICROPY_ERROR_PRINTER, "\n ran esp_wifi_start\n");
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_wpa2enterprise_connect_obj, 5, 5, mod_wpa2enterprise_connect);

//=========================================================

STATIC const mp_rom_map_elem_t enterprise_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_enterprise) },
    { MP_ROM_QSTR(MP_QSTR_set_username_pass), MP_ROM_PTR(&mod_wpa2enterprise_connect_obj) },
};

STATIC MP_DEFINE_CONST_DICT(enterprise_module_globals, enterprise_module_globals_table);

// Define module object.
const mp_obj_module_t enterprise_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&enterprise_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_enterprise, enterprise_module);
