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

esp_err_t driver_wpa2enterprise_init() { return ESP_OK; }

#define WIFI_SORT_ERRCHECK(err) do {int res = (err); if(res) {printf("Enterprise WiFi connection error: %s\n", esp_err_to_name(res)); } } while(0)

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
    int phase2 = mp_obj_get_int(args[4]);
    wifi_config_t wifi_config = {0};
    mp_printf(MICROPY_ERROR_PRINTER, "\n wifi_config \n");
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wstringop-truncation"
    strncpy((char*) wifi_config.sta.ssid, aSsid, 32);
    mp_printf(MICROPY_ERROR_PRINTER, "\n strncpy \n");
    #pragma GCC diagnostic pop
    WIFI_SORT_ERRCHECK(esp_wifi_stop());
    mp_printf(MICROPY_ERROR_PRINTER, "\n esp_wifi_stop \n");
    WIFI_SORT_ERRCHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    mp_printf(MICROPY_ERROR_PRINTER, "\n esp_wifi_set_mode \n");
    WIFI_SORT_ERRCHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    mp_printf(MICROPY_ERROR_PRINTER, "\n esp_wifi_set_config \n");
    // Set WPA2 ENT config.
    WIFI_SORT_ERRCHECK(esp_wifi_sta_wpa2_ent_set_identity((const uint8_t *) aAnonIdent, strlen(aAnonIdent)));
    mp_printf(MICROPY_ERROR_PRINTER, "\n esp_wifi_sta_wpa2_ent_set_identity \n");
    WIFI_SORT_ERRCHECK(esp_wifi_sta_wpa2_ent_set_username((const uint8_t *) aIdent, strlen(aIdent)));
    mp_printf(MICROPY_ERROR_PRINTER, "\n esp_wifi_sta_wpa2_ent_set_username \n");
    WIFI_SORT_ERRCHECK(esp_wifi_sta_wpa2_ent_set_password((const uint8_t *) aPassword, strlen(aPassword)));
    mp_printf(MICROPY_ERROR_PRINTER, "\n esp_wifi_sta_wpa2_ent_set_password \n");
    WIFI_SORT_ERRCHECK(esp_wifi_sta_wpa2_ent_set_ttls_phase2_method(phase2));
    mp_printf(MICROPY_ERROR_PRINTER, "\n esp_wifi_sta_wpa2_ent_set_ttls_phase2_method \n");
    // Enable enterprise auth.
    WIFI_SORT_ERRCHECK(esp_wifi_sta_wpa2_ent_enable());
    mp_printf(MICROPY_ERROR_PRINTER, "\n esp_wifi_sta_wpa2_ent_enable \n");
    // Disable 11b as NOC asked.
    WIFI_SORT_ERRCHECK(esp_wifi_config_11b_rate(WIFI_IF_STA, true));
    mp_printf(MICROPY_ERROR_PRINTER, "\n esp_wifi_config_11b_rate \n");
    // Start the connection.
    WIFI_SORT_ERRCHECK(esp_wifi_start());
    mp_printf(MICROPY_ERROR_PRINTER, "\n esp_wifi_start \n");
    WIFI_SORT_ERRCHECK(esp_wifi_connect());
    mp_printf(MICROPY_ERROR_PRINTER, "\n esp_wifi_connect \n");
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
