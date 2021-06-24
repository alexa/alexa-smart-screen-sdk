#
# Set up Config Validation specific configurations for the sample app.
#
# To build with Config Validation, run the following command,
#     cmake <path-to-source>
#       -DCONFIG_VALIDATION={ON|OFF}
#

option(CONFIG_VALIDATION "Enable configuration validation support." ON)

# Enable validation of configurations files only in debug mode and if user did not explicitly disable configuration validation
if ((CMAKE_BUILD_TYPE STREQUAL "DEBUG") AND NOT(CONFIG_VALIDATION STREQUAL "OFF"))
  add_definitions("-DENABLE_CONFIG_VALIDATION")
  # Encode the schema as hex in sampleApp
  file(READ ${CMAKE_SOURCE_DIR}/modules/Alexa/SampleApp/schemas/SmartScreenSDKConfigSchema.json schema_hex HEX)
  add_definitions("-DSCHEMA_HEX=\"${schema_hex}\"")
  message("Creating ${PROJECT_NAME} with configuration validation enabled")
endif()
