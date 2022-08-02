
#
# Main component of TF Micro project 'person_detection'.
#

idf_component_register(
    SRCS
        "enterprise.c"
    PRIV_REQUIRES wpa_supplicant
    INCLUDE_DIRS "")
