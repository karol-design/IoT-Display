# CMake file for the Main component

idf_component_register( SRCS "main.c"
		INCLUDE_DIRS "."
		PRIV_REQUIRES config nvs_flash provisioning data_scraping ui tm1637)

