idf_component_register(SRCS "CMDControl.cpp" "CMDFunction.cpp" "parse_packet.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES WiFi esp_http_client esp-tls esp_event)

