# cmake
cmake_minimum_required(VERSION 3.16)

# client
project(client)

# root path
set(ROOT_PATH ../..)

# globals
include(${ROOT_PATH}/global.cmake)

# dependencies
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets Network PrintSupport OpenGL REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets Network PrintSupport OpenGL REQUIRED)

if (QT_CONFIG_MAPPING)
  if (NOT QT_CONFIG_MAPPING_RELEASE)
    set(QT_CONFIG_MAPPING_RELEASE         ${QT_CONFIG_MAPPING})
  endif ()

  if (NOT QT_CONFIG_MAPPING_RELWITHDEBINFO)
    set(QT_CONFIG_MAPPING_RELWITHDEBINFO  ${QT_CONFIG_MAPPING})
  endif ()

  if (NOT QT_CONFIG_MAPPING_DEBUG)
    set(QT_CONFIG_MAPPING_DEBUG           ${QT_CONFIG_MAPPING})
  endif ()
endif()

if (NOT QT_CONFIG_MAPPING_RELEASE)
set(QT_CONFIG_MAPPING_RELEASE         "RELEASE")
endif ()

if (NOT QT_CONFIG_MAPPING_RELWITHDEBINFO)
set(QT_CONFIG_MAPPING_RELWITHDEBINFO  "RELEASE")
endif ()

if (NOT QT_CONFIG_MAPPING_DEBUG)
set(QT_CONFIG_MAPPING_DEBUG           "DEBUG")
endif ()

# set_target_properties(Qt${QT_VERSION_MAJOR}::Core PROPERTIES MAP_IMPORTED_CONFIG_RELEASE           QT_CONFIG_MAPPING_RELEASE)
# set_target_properties(Qt${QT_VERSION_MAJOR}::Core PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO    QT_CONFIG_MAPPING_RELWITHDEBINFO)
# set_target_properties(Qt${QT_VERSION_MAJOR}::Core PROPERTIES MAP_IMPORTED_CONFIG_DEUBG             QT_CONFIG_MAPPING_DEBUG)

# set_target_properties(Qt${QT_VERSION_MAJOR}::Gui PROPERTIES MAP_IMPORTED_CONFIG_RELEASE           QT_CONFIG_MAPPING_RELEASE)
# set_target_properties(Qt${QT_VERSION_MAJOR}::Gui PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO    QT_CONFIG_MAPPING_RELWITHDEBINFO)
# set_target_properties(Qt${QT_VERSION_MAJOR}::Gui PROPERTIES MAP_IMPORTED_CONFIG_DEUBG             QT_CONFIG_MAPPING_DEBUG)

# set_target_properties(Qt${QT_VERSION_MAJOR}::Widgets PROPERTIES MAP_IMPORTED_CONFIG_RELEASE           QT_CONFIG_MAPPING_RELEASE)
# set_target_properties(Qt${QT_VERSION_MAJOR}::Widgets PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO    QT_CONFIG_MAPPING_RELWITHDEBINFO)
# set_target_properties(Qt${QT_VERSION_MAJOR}::Widgets PROPERTIES MAP_IMPORTED_CONFIG_DEUBG             QT_CONFIG_MAPPING_DEBUG)

# set_target_properties(Qt${QT_VERSION_MAJOR}::Network PROPERTIES MAP_IMPORTED_CONFIG_RELEASE           QT_CONFIG_MAPPING_RELEASE)
# set_target_properties(Qt${QT_VERSION_MAJOR}::Network PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO    QT_CONFIG_MAPPING_RELWITHDEBINFO)
# set_target_properties(Qt${QT_VERSION_MAJOR}::Network PROPERTIES MAP_IMPORTED_CONFIG_DEUBG             QT_CONFIG_MAPPING_DEBUG)

# set_target_properties(Qt${QT_VERSION_MAJOR}::PrintSupport PROPERTIES MAP_IMPORTED_CONFIG_RELEASE           QT_CONFIG_MAPPING_RELEASE)
# set_target_properties(Qt${QT_VERSION_MAJOR}::PrintSupport PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO    QT_CONFIG_MAPPING_RELWITHDEBINFO)
# set_target_properties(Qt${QT_VERSION_MAJOR}::PrintSupport PROPERTIES MAP_IMPORTED_CONFIG_DEUBG             QT_CONFIG_MAPPING_DEBUG)

# set_target_properties(Qt${QT_VERSION_MAJOR}::OpenGL PROPERTIES MAP_IMPORTED_CONFIG_RELEASE           QT_CONFIG_MAPPING_RELEASE)
# set_target_properties(Qt${QT_VERSION_MAJOR}::OpenGL PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO    QT_CONFIG_MAPPING_RELWITHDEBINFO)
# set_target_properties(Qt${QT_VERSION_MAJOR}::OpenGL PROPERTIES MAP_IMPORTED_CONFIG_DEUBG             QT_CONFIG_MAPPING_DEBUG)

# qt version dependent
if (${QT_VERSION_MAJOR} EQUAL 6)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core5Compat REQUIRED)

# set_target_properties(Qt${QT_VERSION_MAJOR}::Core5Compat PROPERTIES MAP_IMPORTED_CONFIG_RELEASE           QT_CONFIG_MAPPING_RELEASE)
# set_target_properties(Qt${QT_VERSION_MAJOR}::Core5Compat PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO    QT_CONFIG_MAPPING_RELWITHDEBINFO)
# set_target_properties(Qt${QT_VERSION_MAJOR}::Core5Compat PROPERTIES MAP_IMPORTED_CONFIG_DEUBG             QT_CONFIG_MAPPING_DEBUG)
endif ()

if (QT_VERSION VERSION_LESS 6.4)
  add_compile_definitions(QT_6_3_2)
else()
  add_compile_definitions(QT_6_4_0)
endif()

if (NOT PLATFORM_WASM)
find_package(QT NAMES Qt6 Qt5 COMPONENTS DBus REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS DBus REQUIRED)

# set_target_properties(Qt${QT_VERSION_MAJOR}::DBus PROPERTIES MAP_IMPORTED_CONFIG_RELEASE           QT_CONFIG_MAPPING_RELEASE)
# set_target_properties(Qt${QT_VERSION_MAJOR}::DBus PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO    QT_CONFIG_MAPPING_RELWITHDEBINFO)
# set_target_properties(Qt${QT_VERSION_MAJOR}::DBus PROPERTIES MAP_IMPORTED_CONFIG_DEUBG             QT_CONFIG_MAPPING_DEBUG)
endif ()

if (PUBLIC)
  add_compile_definitions(PUBLIC)
endif ()


if (DEBUG)
  add_compile_definitions(DEBUG)
endif ()

if (DEFINED PLATFORM_WASM)
  # Add app keys for WASM
  add_compile_definitions(PLATFORM_WASM)

  if (DEFINED WASM_CLIENT_ID)
    add_compile_definitions(WASM_CLIENT_ID=${WASM_CLIENT_ID})
  endif ()

  if (DEFINED WASM_APP_URL)
    add_compile_definitions(WASM_APP_URL=${WASM_APP_URL})
  endif ()

  if (DEFINED WASM_KEYCLOAK_URL)
    add_compile_definitions(WASM_KEYCLOAK_URL=${WASM_KEYCLOAK_URL})
  endif ()
else()
  # Add app keys for Desktop if defined (for debug without config)
  if (DEFINED GENESIS_CLIENT_ID)
    add_compile_definitions(GENESIS_CLIENT_ID=${GENESIS_CLIENT_ID})
  endif ()

  if (DEFINED GENESIS_APP_URL)
    add_compile_definitions(GENESIS_APP_URL=${GENESIS_APP_URL})
  endif ()

  if (DEFINED GENESIS_KEYCLOAK_URL)
    add_compile_definitions(GENESIS_KEYCLOAK_URL=${GENESIS_KEYCLOAK_URL})
  endif ()

endif ()



# includes
include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}#for ui forms moc headers
)

# sources
set(
  SOURCES ${SOURCES}
  main.cpp
)
# headers
set(
  SOURCES ${SOURCES}
  genesis_colors.h
)

set(
  SOURCES ${SOURCES}
  graphicsplot/graphicsplot.cpp
  graphicsplot/graphicsplot_extended.cpp
  graphicsplot/graphicsplot_data_structures.cpp
  graphicsplot/settings_widgets/caption_edit_dialog.cpp
  graphicsplot/settings_widgets/curve_settings_widget.cpp
  graphicsplot/settings_widgets/font_settings_widget.cpp
  graphicsplot/settings_widgets/plot_settings_dialog.cpp
  graphicsplot/settings_widgets/settings_structs.cpp
  graphicsplot/settings_widgets/zones_amount_dialog.cpp
  graphicsplot/settings_widgets/zones_edit_dialog.cpp
  graphicsplot/graphics_items/common_item_fuctional.cpp
  graphicsplot/graphics_items/graphics_item_interactive_line.cpp
  graphicsplot/graphics_items/graphics_item_interactive_pixmap.cpp
  graphicsplot/graphics_items/graphics_item_interactive_polyline.cpp
  graphicsplot/graphics_items/graphics_item_interactive_text.cpp
)

set(
  HEADERS ${HEADERS}
  graphicsplot/graphicsplot.h
  graphicsplot/graphicsplot_extended.h
  graphicsplot/graphicsplot_data_structures.h
  graphicsplot/graphicsplot_global.h
  graphicsplot/settings_widgets/caption_edit_dialog.h
  graphicsplot/settings_widgets/curve_settings_widget.h
  graphicsplot/settings_widgets/font_settings_widget.h
  graphicsplot/settings_widgets/plot_settings_dialog.h
  graphicsplot/settings_widgets/settings_structs.h
  graphicsplot/settings_widgets/zones_amount_dialog.h
  graphicsplot/settings_widgets/zones_edit_dialog.h
  graphicsplot/graphics_items/common_item_fuctional.h
  graphicsplot/graphics_items/graphics_item_interactive_line.h
  graphicsplot/graphics_items/graphics_item_interactive_pixmap.h
  graphicsplot/graphics_items/graphics_item_interactive_polyline.h
  graphicsplot/graphics_items/graphics_item_interactive_text.h
)

set(
  UIS ${UIS}
  graphicsplot/settings_widgets/curve_settings_widget.ui
  graphicsplot/settings_widgets/font_settings_widget.ui
  graphicsplot/settings_widgets/plot_settings_dialog.ui
  graphicsplot/settings_widgets/zones_amount_dialog.ui
  graphicsplot/settings_widgets/zones_edit_dialog.ui
)


# item editors section
set(
  SOURCES ${SOURCES}
  ui/item_editors/expandable_default_item_editor_factory.cpp
)
set(
  SOURCES ${SOURCES}
  ui/item_editors/expandable_default_item_editor_factory.h
)

# sources / api / rest
set(
  HEADERS ${HEADERS}
  api/api_rest.h
  api/network_manager.h
)

set(
  SOURCES ${SOURCES}
  api/api_rest.cpp
  api/network_manager.cpp
)

# sources / logic
set(
  HEADERS ${HEADERS}
  logic/app.h
  logic/context.h
  logic/genesis_context_step_based_markup.h
  logic/known_context_names.h
  logic/known_json_tag_names.h
  logic/known_context_tag_names.h
  logic/istorage.h
  logic/iclass.h
  logic/inotify_object.h
  logic/context_root.h
  logic/notification.h
  logic/enums.h
)

set(
  SOURCES ${SOURCES}
  logic/app.cpp
  logic/context.cpp
  logic/genesis_context_step_based_markup.cpp
  logic/context_root.cpp
  logic/notification.cpp
)

# sources / logic / models base
set(
  HEADERS ${HEADERS}
  logic/tree_model.h
  logic/tree_model_dynamic.h
  logic/tree_model_item.h
  logic/tree_model_presentation.h
)

set(
  SOURCES ${SOURCES}
  logic/tree_model.cpp
  logic/tree_model_dynamic.cpp
  logic/tree_model_item.cpp
  logic/tree_model_presentation.cpp
)


# sources / logic / model
set(
    HEADERS ${HEADERS}
    logic/models/any_container.h
    logic/models/abstract_model.h
    logic/models/identity_model.h
    logic/models/analysis_identity_model.h
    logic/models/pca_analysis_identity_model.h
    logic/models/pls_analysis_identity_model.h
    logic/models/pls_fit_request_model.h
    logic/models/pls_analysis_list_model.h
    logic/models/pls_fit_response_model.h
    logic/models/pls_table_item_model.h
    logic/models/pls_predict_table_item_model.h
    logic/models/pls_predict_response_model.h
    logic/models/pls_predict_analysis_model.h
    logic/models/pls_predict_parent_analysis_list_model.h
    logic/models/mcr_analysis_request_model.h
    logic/models/mcr_analysis_response_model.h
    logic/models/mcr_analysis_identity_model.h
    logic/models/markup_reference_model.h
    logic/models/reference_analysis_data_table_model.h
    logic/models/coefficient_calculation_model.h
    logic/models/pca_analysis_request_model.h
    logic/models/pca_analysis_response_model.h
    logic/models/hotteling_request_model.h
    logic/models/hotteling_response_model.h
    logic/models/tree_confirmation_model.h
    logic/models/passport_combo_models.h
    logic/models/plot_template_model.h
    logic/models/shape_color_grouped_entity_plot_data_model.h
    logic/models/analysis_entity_model.h
    logic/models/analysis_plot_item_model.h
    logic/models/plot_template_items/line_item.h
    logic/models/plot_template_items/text_item.h
)

set(
    SOURCES ${SOURCES}
    logic/models/identity_model.cpp
    logic/models/analysis_identity_model.cpp
    logic/models/pca_analysis_identity_model.cpp
    logic/models/pls_analysis_identity_model.cpp
    logic/models/pls_fit_request_model.cpp
    logic/models/pls_fit_response_model.cpp
    logic/models/pls_table_item_model.cpp
    logic/models/pls_predict_table_item_model.cpp
    logic/models/pls_predict_response_model.cpp
    logic/models/pls_predict_analysis_model.cpp
    logic/models/pls_predict_parent_analysis_list_model.cpp
    logic/models/mcr_analysis_request_model.cpp
    logic/models/mcr_analysis_response_model.cpp
    logic/models/mcr_analysis_identity_model.cpp
    logic/models/markup_reference_model.cpp
    logic/models/reference_analysis_data_table_model.cpp
    logic/models/pca_analysis_request_model.cpp
    logic/models/pca_analysis_response_model.cpp
    logic/models/hotteling_request_model.cpp
    logic/models/hotteling_response_model.cpp
    logic/models/tree_confirmation_model.cpp
    logic/models/passport_combo_models.cpp
    logic/models/plot_template_model.cpp
    logic/models/shape_color_grouped_entity_plot_data_model.cpp
    logic/models/analysis_entity_model.cpp
    logic/models/analysis_plot_item_model.cpp
    logic/models/plot_template_items/line_item.cpp
    logic/models/plot_template_items/text_item.cpp
)

# sources / logic / serializers
set(
    HEADERS ${HEADERS}
    logic/serializers/json_combiner.h
    logic/serializers/analysis_identity_model_json_serializer.h
    logic/serializers/pca_analysis_identity_model_json_serializer.h
    logic/serializers/pls_analysis_identity_model_json_serializer.h
    logic/serializers/model_base_json_serializer.h
    logic/serializers/pls_analysis_list_json_serializer.h
    logic/serializers/pls_fit_request_json_serializer.h
    logic/serializers/pls_fit_response_json_serializer.h
    logic/serializers/pls_predict_response_json_serializer.h
    logic/serializers/pls_predict_analysis_json_serializer.h
    logic/serializers/pls_predict_parent_analysis_list_json_serializer.h
    logic/serializers/mcr_analysis_request_model_json_serializer.h
    logic/serializers/mcr_analysis_response_model_json_serializer.h
    logic/serializers/mcr_analysis_identity_model_json_serializer.h
    logic/serializers/mcr_analysis_common_json_serializer.h
    logic/serializers/markup_reference_model_json_serializer.h
    logic/serializers/reference_analysis_data_table_model_json_serializer.h
    logic/serializers/pca_analysis_request_model_json_serializer.h
    logic/serializers/pca_analysis_response_model_json_serializer.h
    logic/serializers/hotteling_request_json_serializer.h
    logic/serializers/hotteling_response_json_serializer.h
    logic/serializers/coefficient_calculation_model_json_serializer.h
    logic/serializers/entity_model_json_serializer.h
)

set(
    SOURCES ${SOURCES}
    logic/serializers/json_combiner.cpp
    logic/serializers/analysis_identity_model_json_serializer.cpp
    logic/serializers/pca_analysis_identity_model_json_serializer.cpp
    logic/serializers/pls_analysis_identity_model_json_serializer.cpp
    logic/serializers/model_base_json_serializer.cpp
    logic/serializers/pls_analysis_list_json_serializer.cpp
    logic/serializers/pls_fit_request_json_serializer.cpp
    logic/serializers/pls_fit_response_json_serializer.cpp
    logic/serializers/pls_predict_response_json_serializer.cpp
    logic/serializers/pls_predict_analysis_json_serializer.cpp
    logic/serializers/pls_predict_parent_analysis_list_json_serializer.cpp
    logic/serializers/mcr_analysis_request_model_json_serializer.cpp
    logic/serializers/mcr_analysis_response_model_json_serializer.cpp
    logic/serializers/mcr_analysis_identity_model_json_serializer.cpp
    logic/serializers/mcr_analysis_common_json_serializer.cpp
    logic/serializers/markup_reference_model_json_serializer.cpp
    logic/serializers/reference_analysis_data_table_model_json_serializer.cpp
    logic/serializers/pca_analysis_request_model_json_serializer.cpp
    logic/serializers/pca_analysis_response_model_json_serializer.cpp
    logic/serializers/hotteling_request_json_serializer.cpp
    logic/serializers/hotteling_response_json_serializer.cpp
    logic/serializers/coefficient_calculation_model_json_serializer.cpp
    logic/serializers/entity_model_json_serializer.cpp
)


# sources / logic / structures
set(
  HEADERS ${HEADERS}
  logic/structures/analysis_model_structures.h
  logic/structures/pls_model_structures.h
  logic/structures/common_structures.h
  logic/structures/peak_data_structure.h
  logic/structures/pca_data_structures.h
  logic/structures/passport_form_structures.h
  logic/structures/library/library_structures.h
)

# sources / logic / specific models
set(
  HEADERS ${HEADERS}
  logic/tree_model_dynamic_access.h
  logic/tree_model_dynamic_field_wells.h
  logic/tree_model_dynamic_fields.h
  logic/tree_model_dynamic_file_infos.h
  logic/tree_model_dynamic_file_types.h
  logic/tree_model_dynamic_markup_versions.h
  logic/tree_model_dynamic_job_functions.h
  logic/tree_model_dynamic_layers.h
  logic/tree_model_dynamic_other_users.h
  logic/tree_model_dynamic_project_children.h
  logic/tree_model_dynamic_project_imported_files.h
  logic/tree_model_dynamic_project_markups_list.h
  logic/tree_model_dynamic_project_markup_etalon_list.h
  logic/tree_model_dynamic_project_info.h
  logic/tree_model_dynamic_project_users.h
  logic/tree_model_dynamic_projects.h
  logic/tree_model_dynamic_roles.h
  logic/tree_model_dynamic_analyzes.h
  logic/tree_model_dynamic_project_chromatogramms.h
  logic/tree_model_dynamic_project_fragments.h
  logic/tree_model_dynamic_project_models_list.h
  logic/tree_model_dynamic_numeric_tables.h
  logic/tree_model_dynamic_library_compounds.h
  logic/tree_model_dynamic_library_compounds_additional.h
  logic/tree_model_dynamic_library_templates.h
  logic/tree_model_dynamic_calculation_coefficient_table.h
  logic/tree_model_presentation_library_compounds.h
  logic/tree_model_dynamic_identification_plots.h
)

set(
  SOURCES ${SOURCES}
  logic/tree_model_dynamic_access.cpp
  logic/tree_model_dynamic_field_wells.cpp
  logic/tree_model_dynamic_fields.cpp
  logic/tree_model_dynamic_file_infos.cpp
  logic/tree_model_dynamic_file_types.cpp
  logic/tree_model_dynamic_markup_versions.cpp
  logic/tree_model_dynamic_job_functions.cpp
  logic/tree_model_dynamic_layers.cpp
  logic/tree_model_dynamic_other_users.cpp
  logic/tree_model_dynamic_project_children.cpp
  logic/tree_model_dynamic_project_imported_files.cpp
  logic/tree_model_dynamic_project_markups_list.cpp
  logic/tree_model_dynamic_project_markup_etalon_list.cpp
  logic/tree_model_dynamic_project_info.cpp
  logic/tree_model_dynamic_project_users.cpp
  logic/tree_model_dynamic_projects.cpp
  logic/tree_model_dynamic_roles.cpp
  logic/tree_model_dynamic_analyzes.cpp
  logic/tree_model_dynamic_project_chromatogramms.cpp
  logic/tree_model_dynamic_project_fragments.cpp
  logic/tree_model_dynamic_project_models_list.cpp
  logic/tree_model_dynamic_numeric_tables.cpp
  logic/tree_model_dynamic_library_compounds.cpp
  logic/tree_model_dynamic_library_compounds_additional.cpp
  logic/tree_model_dynamic_library_templates.cpp
  logic/tree_model_dynamic_calculation_coefficient_table.cpp
  logic/tree_model_presentation_library_compounds.cpp
  logic/tree_model_dynamic_identification_plots.cpp
)

# sources / logic / numeric_data

set(
  HEADERS ${HEADERS}
  ui/numeric_data/view_page_numeric_data.h
  ui/numeric_data/tree_widget_select_numeric_data_table.h
  ui/numeric_data/join_numeric_dialog.h
  ui/numeric_data/tree_widget_height_ratio_table.h
  ui/numeric_data/tree_widget_imported_tables.h
  ui/numeric_data/tree_widget_imported_identification_tables.h
  ui/numeric_data/tree_widget_coefficient_tables.h
  ui/numeric_data/tree_widget_identified_tables.h
  ui/numeric_data/tree_widget_merged_tables.h
  ui/numeric_data/tree_widget_joined_tables.h
)

set(
  SOURCES ${SOURCES}
  ui/numeric_data/view_page_numeric_data.cpp
  ui/numeric_data/tree_widget_select_numeric_data_table.cpp
  ui/numeric_data/join_numeric_dialog.cpp
  ui/numeric_data/tree_widget_height_ratio_table.cpp
  ui/numeric_data/tree_widget_imported_tables.cpp
  ui/numeric_data/tree_widget_imported_identification_tables.cpp
  ui/numeric_data/tree_widget_coefficient_tables.cpp
  ui/numeric_data/tree_widget_identified_tables.cpp
  ui/numeric_data/tree_widget_merged_tables.cpp
  ui/numeric_data/tree_widget_joined_tables.cpp
)

set(
  UIS ${UIS}
  ui/numeric_data/tree_widget_imported_tables.ui
  ui/numeric_data/tree_widget_coefficient_tables.ui
  ui/numeric_data/tree_widget_merged_tables.ui
)

# sources / logic / service
set(
  HEADERS ${HEADERS}
  logic/service/iservice.h
  logic/service/service_locator.h
  logic/service/json_service_package.h
  logic/service/plot_service_package.h
  logic/service/ijson_serializer.h
  logic/service/ijson_combiner.h
  logic/service/igraphics_plot_axis_modificator.h
  logic/service/igraphics_plot_cortesian_transform.h
)

set(
  SOURCES ${SOURCES}
  logic/service/service_locator.cpp
  logic/service/json_service_package.cpp
  logic/service/plot_service_package.cpp
)

# sources / logic / plot_modificators
set(
  HEADERS ${HEADERS}
  logic/plot_modificators/triplot_axis_modificator.h
  logic/plot_modificators/triplot_cortesian_transform.h
)

set(
  SOURCES ${SOURCES}
  logic/plot_modificators/triplot_axis_modificator.cpp
  logic/plot_modificators/triplot_cortesian_transform.cpp
)

# sources / logic / markup
set(
  HEADERS ${HEADERS}
  logic/markup/abstract_data_model.h
  logic/markup/chromatogram_data_model.h
  logic/markup/markup_data_model.h
  logic/markup/json_serializable.h
  logic/markup/markup_step_controller.h
  logic/markup/genesis_markup_forward_declare.h
  logic/markup/chromatogram_passport_model.h
  logic/markup/genesis_markup_enums.h
  logic/markup/curve_data_model.h
  logic/markup/baseline_data_model.h
  logic/markup/commands.h
  logic/markup/sub_markup_step_controller.h
  logic/markup/i_markup_tab.h
  logic/markup/biomarkers_markup_step_controller.h
  logic/markup/biomarkers_sub_markup_step_controller.h
  logic/markup/item_models/covats_index_model_table.h
  logic/markup/item_models/identification_tree_model.h
  logic/markup/item_models/identification_presentation_model.h
)

set(
  SOURCES ${SOURCES}
  logic/markup/abstract_data_model.cpp
  logic/markup/chromatogram_data_model.cpp
  logic/markup/markup_data_model.cpp
  logic/markup/json_serializable.cpp
  logic/markup/markup_step_controller.cpp
  logic/markup/chromatogram_passport_model.cpp
  logic/markup/curve_data_model.cpp
  logic/markup/baseline_data_model.cpp
  logic/markup/commands.cpp
  logic/markup/sub_markup_step_controller.cpp
  logic/markup/i_markup_tab.cpp
  logic/markup/biomarkers_markup_step_controller.cpp
  logic/markup/biomarkers_sub_markup_step_controller.cpp
  logic/markup/item_models/covats_index_model_table.cpp
  logic/markup/item_models/identification_tree_model.cpp
  logic/markup/item_models/identification_presentation_model.cpp
)

# ui / widgets
set(
  SOURCES ${SOURCES}
  ui/widgets/curve_label_widget.cpp
  ui/widgets/pls_statistic_widget.cpp
  ui/widgets/tab_widget.cpp
  ui/widgets/pls_count_plot_widget.cpp
  ui/widgets/pls_load_plot_widget.cpp
  ui/widgets/plot_toolbar_widget.cpp
  ui/widgets/plot_widget.cpp
  ui/widgets/pls_slope_offset_plot_widget.cpp
  ui/widgets/pls_coefficient_plot_widget.cpp
  ui/widgets/multiple_plot_widget.cpp
  ui/widgets/pls_multiple_plot_widget.cpp
  ui/widgets/pls_rmse_plot_widget.cpp
  ui/widgets/pls_dispersion_plot_widget.cpp
  ui/widgets/table_view_widget.cpp
  ui/widgets/model_settings_widget.cpp
  ui/widgets/mcr_component_settings.cpp
  ui/widgets/mcr_plot_widget.cpp
  ui/widgets/mcr_calculation_plot.cpp
  ui/widgets/mcr_difference_plot.cpp
  ui/widgets/mcr_origin_restored_plot.cpp
  ui/widgets/parameter_table_view.cpp
  ui/widgets/markup_version_reference_widget.cpp
  ui/widgets/pca_plot_widget.cpp
  ui/widgets/pca_count_plot_widget.cpp
  ui/widgets/pca_load_plot_widget.cpp
  ui/widgets/pca_bars_plot_widget.cpp
  ui/widgets/mcr_point_info.cpp
  ui/widgets/plot_legend_widget.cpp
  ui/widgets/plot_legend_widget_refactor.cpp
  ui/widgets/filtered_table_widget.cpp
  ui/widgets/module_plate.cpp
  ui/widgets/main_page_with_splitter.cpp
  ui/widgets/menu_header_widget.cpp
  ui/widgets/project_info_menu_widget.cpp
  ui/widgets/plot_template_widget.cpp
  ui/widgets/plot_template_list_widget.cpp
  ui/widgets/coefficient_table_calc.cpp
  ui/widgets/title_comment_widget.cpp
  ui/widgets/edit_group_widget.cpp
  ui/widgets/selection_item.cpp
  ui/widgets/selected_tables.cpp
  ui/widgets/join_tables.cpp
  ui/widgets/editable_legend_widget.cpp
  ui/widgets/pca_plot_legend_widget.cpp
  ui/widgets/identification_plot_widget.cpp
  ui/widgets/identification_plot_list_widget.cpp
)

set(
  HEADERS ${HEADERS}
  ui/widgets/curve_label_widget.h
  ui/widgets/pls_statistic_widget.h
  ui/widgets/tab_widget.h
  ui/widgets/pls_count_plot_widget.h
  ui/widgets/pls_load_plot_widget.h
  ui/widgets/plot_toolbar_widget.h
  ui/widgets/plot_widget.h
  ui/widgets/pls_plot_widget.h
  ui/widgets/pls_slope_offset_plot_widget.h
  ui/widgets/pls_coefficient_plot_widget.h
  ui/widgets/multiple_plot_widget.h
  ui/widgets/pls_multiple_plot_widget.h
  ui/widgets/pls_rmse_plot_widget.h
  ui/widgets/pls_dispersion_plot_widget.h
  ui/widgets/table_view_widget.h
  ui/widgets/model_settings_widget.h
  ui/widgets/mcr_component_settings.h
  ui/widgets/mcr_plot_widget.h
  ui/widgets/mcr_calculation_plot.h
  ui/widgets/mcr_difference_plot.h
  ui/widgets/mcr_origin_restored_plot.h
  ui/widgets/parameter_table_view.h
  ui/widgets/markup_version_reference_widget.h
  ui/widgets/pca_plot_widget.h
  ui/widgets/pca_count_plot_widget.h
  ui/widgets/pca_load_plot_widget.h
  ui/widgets/pca_bars_plot_widget.h
  ui/widgets/mcr_point_info.h
  ui/widgets/plot_legend_widget.h
  ui/widgets/plot_legend_widget_refactor.h
  ui/widgets/filtered_table_widget.h
  ui/widgets/module_plate.h
  ui/widgets/main_page_with_splitter.h
  ui/widgets/menu_header_widget.h
  ui/widgets/project_info_menu_widget.h
  ui/widgets/plot_template_widget.h
  ui/widgets/plot_template_list_widget.h
  ui/widgets/coefficient_table_calc.h
  ui/widgets/title_comment_widget.h
  ui/widgets/edit_group_widget.h
  ui/widgets/selection_item.h
  ui/widgets/selected_tables.h
  ui/widgets/join_tables.h
  ui/widgets/editable_legend_widget.h
  ui/widgets/pca_plot_legend_widget.h
  ui/widgets/identification_plot_widget.h
  ui/widgets/identification_plot_list_widget.h
)

set(
  UIS ${UIS}
  ui/widgets/pls_statistic_widget.ui
  ui/widgets/tab_widget.ui
  ui/widgets/pls_statistic_widget.ui
  ui/widgets/tab_widget.ui
  ui/widgets/table_view_widget.ui
  ui/widgets/mcr_component_settings.ui
  ui/widgets/mcr_plot_widget.ui
  ui/widgets/web_dialog_ratio_matrix_settings.ui
  ui/widgets/parameter_table_view.ui
  ui/widgets/mcr_point_info.ui
  ui/widgets/plot_legend_widget.ui
  ui/widgets/filtered_table_widget.ui
  ui/widgets/module_plate.ui
  ui/widgets/menu_header_widget.ui
  ui/widgets/project_info_menu_widget.ui
  ui/widgets/plot_template_widget.ui
  ui/widgets/plot_template_list_widget.ui
  ui/widgets/coefficient_table_calc.ui
  ui/widgets/title_comment_widget.ui
  ui/widgets/edit_group_widget.ui
  ui/widgets/join_tables.ui
  ui/widgets/selection_item.ui
  ui/widgets/selected_tables.ui
  ui/widgets/identification_plot_widget.ui
  ui/widgets/identification_plot_list_widget.ui
)

# ui / widgets / markup
set(
  SOURCES ${SOURCES}
  ui/widgets/markup/chromatogramms_tab_widget.cpp
  ui/widgets/markup/chromatogram_widget_refactor.cpp
  ui/widgets/markup/stepper_widget.cpp
  ui/widgets/markup/table_marker_widget.cpp
  ui/widgets/markup/gp_stackedwidget.cpp
  ui/widgets/markup/numeric_data_table.cpp
  ui/widgets/markup/dialog_add_slave_chromatogramm.cpp
  ui/widgets/markup/readonly_simple_tree_model.cpp
  ui/widgets/markup/dialog_add_chromatogramms.cpp
  ui/widgets/markup/markup_widget.cpp
  ui/widgets/markup/reservoir_markup_widget.cpp
  ui/widgets/markup/identification_widget.cpp
  ui/widgets/markup/identification_legend_plate.cpp
)
set(
  HEADERS ${HEADERS}
  ui/widgets/markup/chromatogramms_tab_widget.h
  ui/widgets/markup/chromatogram_widget_refactor.h
  ui/widgets/markup/stepper_widget.h
  ui/widgets/markup/table_marker_widget.h
  ui/widgets/markup/gp_stackedwidget.h
  ui/widgets/markup/numeric_data_table.h
  ui/widgets/markup/dialog_add_slave_chromatogramm.h
  ui/widgets/markup/readonly_simple_tree_model.h
  ui/widgets/markup/dialog_add_chromatogramms.h
  ui/widgets/markup/markup_widget.h
  ui/widgets/markup/reservoir_markup_widget.h
  ui/widgets/markup/identification_widget.h
  ui/widgets/markup/identification_legend_plate.h
)
set(
  UIS ${UIS}
  ui/widgets/markup/table_marker_widget.ui
  ui/widgets/markup/numeric_data_table.ui
  ui/widgets/markup/dialog_add_slave_chromatogramm.ui
  ui/widgets/markup/chromatogram_widget_refactor.ui
  ui/widgets/markup/reservoir_markup_widget.ui
  ui/widgets/markup/identification_widget.ui
)

# ui / widgets / notifications
set(
  SOURCES ${SOURCES}
  ui/widgets/notifications/notification_widget.cpp
  ui/widgets/notifications/info_notification.cpp
  ui/widgets/notifications/headerless_success_notification.cpp
  ui/widgets/notifications/success_notification.cpp
  ui/widgets/notifications/error_notification.cpp
  ui/widgets/notifications/warning_notification.cpp
  ui/widgets/notifications/multi_notification.cpp
)
set(
  HEADERS ${HEADERS}
  ui/widgets/notifications/notification_widget.h
  ui/widgets/notifications/info_notification.h
  ui/widgets/notifications/headerless_success_notification.h
  ui/widgets/notifications/success_notification.h
  ui/widgets/notifications/error_notification.h
  ui/widgets/notifications/warning_notification.h
  ui/widgets/notifications/multi_notification.h
)
set(
  UIS ${UIS}
  ui/widgets/notifications/info_notification.ui
  ui/widgets/notifications/headerless_success_notification.ui
  ui/widgets/notifications/success_notification.ui
  ui/widgets/notifications/error_notification.ui
  ui/widgets/notifications/warning_notification.ui
  ui/widgets/notifications/multi_notification.ui
)

# ui / widgets / passport
set(
  SOURCES ${SOURCES}
  ui/widgets/passport/chemical_passport_form.cpp
  ui/widgets/passport/chromatogramm_item_widget.cpp
  ui/widgets/passport/component_control.cpp
  ui/widgets/passport/extended_form_control.cpp
  ui/widgets/passport/form_control.cpp
  ui/widgets/passport/header_form_control.cpp
  ui/widgets/passport/itab_order_list.cpp
  ui/widgets/passport/joint_layer_widget.cpp
  ui/widgets/passport/joint_layers_control.cpp
  ui/widgets/passport/joint_passport_form_control.cpp
  ui/widgets/passport/main_form.cpp
)
set(
  HEADERS ${HEADERS}
  ui/widgets/passport/chemical_passport_form.h
  ui/widgets/passport/chromatogramm_item_widget.h
  ui/widgets/passport/component_control.h
  ui/widgets/passport/extended_form_control.h
  ui/widgets/passport/form_control.h
  ui/widgets/passport/header_form_control.h
  ui/widgets/passport/itab_order_list.h
  ui/widgets/passport/joint_layer_widget.h
  ui/widgets/passport/joint_layers_control.h
  ui/widgets/passport/joint_passport_form_control.h
  ui/widgets/passport/main_form.h
)
set(
  UIS ${UIS}
  ui/widgets/passport/chemical_passport_form.ui
  ui/widgets/passport/component_control.ui
  ui/widgets/passport/extended_form_control.ui
  ui/widgets/passport/form_control.ui
  ui/widgets/passport/header_form_control.ui
  ui/widgets/passport/joint_layer_widget.ui
  ui/widgets/passport/joint_layers_control.ui
  ui/widgets/passport/joint_passport_form_control.ui
  ui/widgets/passport/main_form.ui
)

# ui / widgets / library
set(
  SOURCES ${SOURCES}
  ui/widgets/library/add_compound_widget.cpp
  ui/widgets/library/add_coefficient_widget.cpp
  ui/widgets/library/add_classifier_widget.cpp
  ui/widgets/library/library_compound_widget.cpp
  ui/widgets/library/library_coefficient_widget.cpp
  ui/widgets/library/library_plot_template_widget.cpp
  ui/widgets/library/compound_details_widget.cpp
  ui/widgets/library/coefficient_details_widget.cpp
  ui/widgets/library/modify_compound_widget.cpp
  ui/widgets/library/modify_coefficient_widget.cpp
  ui/widgets/library/plot_template_edit_widget.cpp
  ui/widgets/library/plot_template_view_widget.cpp
  ui/widgets/plot_template_tree_widget.cpp
)
set(
  HEADERS ${HEADERS}
  ui/widgets/library/add_compound_widget.h
  ui/widgets/library/add_coefficient_widget.h
  ui/widgets/library/add_classifier_widget.h
  ui/widgets/library/library_compound_widget.h
  ui/widgets/library/library_coefficient_widget.h
  ui/widgets/library/library_plot_template_widget.h
  ui/widgets/library/compound_details_widget.h
  ui/widgets/library/coefficient_details_widget.h
  ui/widgets/library/modify_compound_widget.h
  ui/widgets/library/modify_coefficient_widget.h
  ui/widgets/library/plot_template_edit_widget.h
  ui/widgets/library/plot_template_view_widget.h
  ui/widgets/plot_template_tree_widget.h
)
set(
  UIS ${UIS}
  ui/widgets/library/add_compound_widget.ui
  ui/widgets/library/add_coefficient_widget.ui
  ui/widgets/library/add_classifier_widget.ui
  ui/widgets/library/library_compound_widget.ui
  ui/widgets/library/library_coefficient_widget.ui
  ui/widgets/library/library_plot_template_widget.ui
  ui/widgets/library/compound_details_widget.ui
  ui/widgets/library/coefficient_details_widget.ui
  ui/widgets/library/modify_compound_widget.ui
  ui/widgets/library/modify_coefficient_widget.ui
  ui/widgets/library/plot_template_edit_widget.ui
  ui/widgets/library/plot_template_view_widget.ui
)

# widgets / legend
set(
  HEADERS ${HEADERS}
  ui/widgets/legend/legend_editor.h
  ui/widgets/legend/shape_widget.h
  ui/widgets/legend/legend_shape_item.h
  ui/widgets/legend/legend_group_editor.h
  ui/widgets/legend/legend_group_creator.h
  ui/widgets/legend/legend_parameter_table_model.h
)

set(
  SOURCES ${SOURCES}
  ui/widgets/legend/legend_editor.cpp
  ui/widgets/legend/shape_widget.cpp
  ui/widgets/legend/legend_shape_item.cpp
  ui/widgets/legend/legend_group_editor.cpp
  ui/widgets/legend/legend_group_creator.cpp
  ui/widgets/legend/legend_parameter_table_model.cpp
)

set(
  UIS ${UIS}
  ui/widgets/legend/legend_parameter_item.ui
  ui/widgets/legend/legend_editor.ui
  ui/widgets/legend/legend_shape_item.ui
  ui/widgets/legend/legend_group_editor.ui
  ui/widgets/legend/legend_group_creator.ui
)

# ui / item_models
set(
  SOURCES ${SOURCES}
  ui/item_models/statistic_table_model.cpp
  ui/item_models/object_with_model.cpp
  ui/item_models/checkable_string_list_model.cpp
  ui/item_models/concentration_table_model.cpp
  ui/item_models/pls_predict_table_model.cpp
  ui/item_models/mcr_table_model.cpp
  ui/item_models/markup_version_reference_model.cpp
  ui/item_models/numeric_data_table_model.cpp
  ui/item_models/analysis_data_table_model.cpp
  ui/item_models/bar_plot_table_model.cpp
  ui/item_models/hotteling_data_model.cpp
  ui/item_models/color_item_model.cpp
  ui/item_models/shape_item_model.cpp
)

set(
  HEADERS ${HEADERS}
  ui/item_models/statistic_table_model.h
  ui/item_models/object_with_model.h
  ui/item_models/checkable_string_list_model.h
  ui/item_models/concentration_table_model.h
  ui/item_models/pls_predict_table_model.h
  ui/item_models/mcr_table_model.h
  ui/item_models/markup_version_reference_model.h
  ui/item_models/numeric_data_table_model.h
  ui/item_models/analysis_data_table_model.h
  ui/item_models/bar_plot_table_model.h
  ui/item_models/hotteling_data_model.h
  ui/item_models/color_item_model.h
  ui/item_models/shape_item_model.h
)

# sources / ui
set(
  HEADERS ${HEADERS}
  ui/flow_layout.h
  ui/pin_layout.h
  ui/genesis_window.h
  ui/genesis_window_notifications.h
  ui/view.h
  ui/view_factory.h
  ui/view_factory_impls.h
  ui/view_factory_registry.h
)

set(
  SOURCES ${SOURCES}
  ui/flow_layout.cpp
  ui/pin_layout.cpp
  ui/genesis_window.cpp
  ui/genesis_window_notifications.cpp
  ui/view.cpp
  ui/view_factory.cpp
  ui/view_factory_impls.cpp
  ui/view_factory_registry.cpp
)

# sources / ui / view impls
set(
  HEADERS ${HEADERS}
  ui/view_login.h
  ui/view_login_create.h
  ui/view_page_create_identification_plot.h
  ui/view_page_identification.h
  ui/view_page_interpretation.h
  ui/view_page_login_edit.h
  ui/view_page_project_children.h
  ui/view_page_project_data.h
  ui/view_page_project_imported_files.h
  ui/view_page_project_markups_list.h
  ui/view_page_project_models_list.h
  ui/view_page_project_info.h
  ui/view_page_project_log.h
  ui/view_page_projects.h
  ui/view_root.h
  ui/view_root_layout_pages.h
  ui/view_root_layout_pages_menu.h
  ui/view_root_layout_welcome.h
  ui/view_root_layout_welcome_banners.h
  ui/view_root_toolbar.h
  ui/view_root_toolbar_account_menu.h
  ui/view_page_chromatogramms_fragments.h
  ui/known_view_names.h
  ui/view_page_analysis_data_table_page.h
  ui/view_page_project_pls_graphics_plot_page.h
  ui/view_page_project_mcr_graphics_plot_page.h
  ui/view_page_project_pls_predict.h
  ui/view_page_pls_model_page.h
  ui/view_page_project_markup_version_page.h
  ui/view_page_mcr_model_page.h
  ui/view_page_project_markup.h
  ui/view_page_project_pca_graphics_plot_page.h
  ui/view_page_module_selection.h
  ui/view_page_profile.h
  ui/view_page_library.h
  ui/view_page_pca_model.h
  ui/view_page_identification_plot_list.h
  ui/view_page_statistic.h
)

set(
  SOURCES ${SOURCES}
  ui/view_login.cpp
  ui/view_login_create.cpp
  ui/view_page_create_identification_plot.cpp
  ui/view_page_identification.cpp
  ui/view_page_login_edit.cpp
  ui/view_page_interpretation.cpp
  ui/view_page_project_children.cpp
  ui/view_page_project_data.cpp
  ui/view_page_project_imported_files.cpp
  ui/view_page_project_markups_list.cpp
  ui/view_page_project_models_list.cpp
  ui/view_page_project_info.cpp
  ui/view_page_project_log.cpp
  ui/view_page_projects.cpp
  ui/view_root.cpp
  ui/view_root_layout_pages.cpp
  ui/view_root_layout_pages_menu.cpp
  ui/view_root_layout_welcome.cpp
  ui/view_root_layout_welcome_banners.cpp
  ui/view_root_toolbar.cpp
  ui/view_root_toolbar_account_menu.cpp
  ui/view_page_chromatogramms_fragments.cpp
  ui/view_page_analysis_data_table_page.cpp
  ui/view_page_project_pls_graphics_plot_page.cpp
  ui/view_page_project_mcr_graphics_plot_page.cpp
  ui/view_page_pls_model_page.cpp
  ui/view_page_project_pls_predict.cpp
  ui/view_page_project_markup_version_page.cpp
  ui/view_page_mcr_model_page.cpp
  ui/view_page_project_markup.cpp
  ui/view_page_project_pca_graphics_plot_page.cpp
  ui/view_page_module_selection.cpp
  ui/view_page_profile.cpp
  ui/view_page_library.cpp
  ui/view_page_pca_model.cpp
  ui/view_page_identification_plot_list.cpp
  ui/view_page_statistic.cpp
)

set(
  UIS ${UIS}
  ui/view_root_layout_pages_menu.ui
  ui/view_page_create_identification_plot.ui
  ui/view_page_project_markup.ui
  ui/view_page_module_selection.ui
  ui/view_page_profile.ui
  ui/view_page_library.ui
  ui/view_page_identification_plot_list.ui
  ui/view_page_statistic.ui
  ui/view_page_pca_model.ui
)

set(
  HEADERS ${HEADERS}
  ui/chromatograms_fragments/chromatogramms_table_widget.h
  ui/chromatograms_fragments/fragments_table_widget.h
)

set(
  SOURCES ${SOURCES}
  ui/chromatograms_fragments/chromatogramms_table_widget.cpp
  ui/chromatograms_fragments/fragments_table_widget.cpp
)

#sources / ui / dialog / templates
set(
  HEADERS ${HEADERS}
  ui/dialogs/templates/save_edit.h
  ui/dialogs/templates/info.h
  ui/dialogs/templates/confirm.h
  ui/dialogs/templates/dialog.h
  ui/dialogs/templates/tree_confirmation.h
)

set(
  SOURCES ${SOURCES}
  ui/dialogs/templates/save_edit.cpp
  ui/dialogs/templates/info.cpp
  ui/dialogs/templates/confirm.cpp
  ui/dialogs/templates/dialog.cpp
  ui/dialogs/templates/tree_confirmation.cpp
)

#sources / ui / dialog / models

set(
  HEADERS ${HEADERS}
  ui/dialogs/models/import_xls_proxy_model.h
)

set(
  SOURCES ${SOURCES}
  ui/dialogs/models/import_xls_proxy_model.cpp
)

#sources / ui / dialog

set(
  UIS ${UIS}
  ui/dialogs/templates/save_edit.ui
  ui/dialogs/templates/info.ui
  ui/dialogs/templates/confirm.ui
  ui/dialogs/templates/dialog.ui
  ui/dialogs/templates/tree_confirmation.ui
)

# sources / ui / dialog
set(
  HEADERS ${HEADERS}
  ui/dialogs/web_dialog.h
  ui/dialogs/web_dialog_add_user_to_project.h
  ui/dialogs/web_dialog_chromatogram_information.h
  ui/dialogs/web_dialog_create.h
  ui/dialogs/web_dialog_create_base.h
  ui/dialogs/web_dialog_create_markup.h
  ui/dialogs/web_dialog_create_project.h
  ui/dialogs/web_dialog_create_project_refactor.h
  ui/dialogs/web_dialog_upload_files.h
  ui/dialogs/web_dialog_version_list.h
  ui/dialogs/web_dialog_analysis_list.h
  ui/dialogs/web_overlay.h
  ui/dialogs/web_dialog_pca_legend_settings.h
  ui/dialogs/web_dialog_pca_items_settings.h
  ui/dialogs/web_dialog_analysis_model_settings.h
  ui/dialogs/web_dialog_pick_axis_components.h
  ui/dialogs/web_dialog_step_back.h
  ui/dialogs/web_dialog_new_reference.h
  ui/dialogs/web_dialog_markers_transfer.h
  ui/dialogs/web_dialog_intermarkers_transfer.h
  ui/dialogs/web_dialog_check_code_word.h
  ui/dialogs/chromatogram_widget_ions_dialog.h
  ui/dialogs/markup_save_as_dialog.h
  ui/dialogs/chromatogram_widget_settings_dialog_refactor.h
  ui/dialogs/chromatogram_widget_settings_paramerets_dialog.h
  ui/dialogs/web_dialog_box_with_radio.h
  ui/dialogs/web_dialog_save_current_model.h
  ui/dialogs/web_dialog_files_group_rename.h
  ui/dialogs/web_dialog_markup_creation.h
  ui/dialogs/web_dialog_create_group_manual_hotteling.h
  ui/dialogs/web_dialog_pls_concentration_settings.h
  ui/dialogs/web_dialog_pls_build_settings_refactor.h
  ui/dialogs/web_dialog_import_files.h
  ui/dialogs/web_dialog_import_xls_file.h
  ui/dialogs/web_dialog_import_file_type.h
  ui/dialogs/web_dialog_xls_file_import_type.h
  ui/dialogs/web_dialog_ions_fragmentation.h
  ui/dialogs/web_dialog_file_removing.h
  ui/dialogs/web_dialog_input_dialog.h
  ui/dialogs/web_dialog_pls_load_plot_pick_axis.h
  ui/dialogs/web_dialog_question.h
  ui/dialogs/web_dialog_concentration_table.h
  ui/dialogs/web_dialog_pls_predict_settings.h
  ui/dialogs/web_dialog_mcr_build_settings.h
  ui/dialogs/web_dialog_ratio_matrix_settings.h
  ui/dialogs/web_dialog_stacked.h
  ui/dialogs/markup_save_table_dialog.h
  ui/dialogs/example_genesis_dialog.h
  ui/dialogs/markup/intermarkers_transfer_dialog.h
  ui/dialogs/markup/web_dialog_add_chromatogramms.h
  ui/dialogs/markup/biomarkers_markup_save_table_dialog.h
  ui/dialogs/project_table_rename_dialog.h
  ui/dialogs/project_table_rename_dialog_refactor.h
  ui/dialogs/web_dialog_pick_axis_components.h
  ui/dialogs/table_rename_dialog.h
  ui/dialogs/hotelling_criterion_parameters_dialog.h
  ui/dialogs/export_tables_and_plots_dialog.h
  ui/dialogs/chromatogram_passport_dialog.h
  ui/dialogs/height_ratio_matrix/height_ratio_info.h
  ui/dialogs/naming_pattern_edit_dialog.h
  ui/dialogs/web_dialog_group_editor.h
  ui/dialogs/web_dialog_calculate_coefficient_table.h
  ui/dialogs/web_dialog_join_tables.h
  ui/dialogs/web_dialog_legend_settings.h
)

set(
  SOURCES ${SOURCES}
  ui/dialogs/web_dialog.cpp
  ui/dialogs/web_dialog_add_user_to_project.cpp
  ui/dialogs/web_dialog_chromatogram_information.cpp
  ui/dialogs/web_dialog_create.cpp
  ui/dialogs/web_dialog_create_base.cpp
  ui/dialogs/web_dialog_create_markup.cpp
  ui/dialogs/web_dialog_create_project.cpp
  ui/dialogs/web_dialog_create_project_refactor.cpp
  ui/dialogs/web_dialog_upload_files.cpp
  ui/dialogs/web_dialog_version_list.cpp
  ui/dialogs/web_dialog_analysis_list.cpp
  ui/dialogs/web_overlay.cpp
  ui/dialogs/web_dialog_pca_legend_settings.cpp
  ui/dialogs/web_dialog_pca_items_settings.cpp
  ui/dialogs/web_dialog_analysis_model_settings.cpp
  ui/dialogs/web_dialog_pick_axis_components.cpp
  ui/dialogs/web_dialog_step_back.cpp
  ui/dialogs/web_dialog_new_reference.cpp
  ui/dialogs/web_dialog_markers_transfer.cpp
  ui/dialogs/web_dialog_intermarkers_transfer.cpp
  ui/dialogs/web_dialog_check_code_word.cpp
  ui/dialogs/chromatogram_widget_ions_dialog.cpp
  ui/dialogs/markup_save_as_dialog.cpp
  ui/dialogs/chromatogram_widget_settings_dialog_refactor.cpp
  ui/dialogs/chromatogram_widget_settings_paramerets_dialog.cpp
  ui/dialogs/web_dialog_box_with_radio.cpp
  ui/dialogs/web_dialog_save_current_model.cpp
  ui/dialogs/web_dialog_files_group_rename.cpp
  ui/dialogs/web_dialog_markup_creation.cpp
  ui/dialogs/web_dialog_create_group_manual_hotteling.cpp
  ui/dialogs/web_dialog_pls_concentration_settings.cpp
  ui/dialogs/web_dialog_pls_build_settings_refactor.cpp
  ui/dialogs/web_dialog_import_files.cpp
  ui/dialogs/web_dialog_import_xls_file.cpp
  ui/dialogs/web_dialog_import_file_type.cpp
  ui/dialogs/web_dialog_xls_file_import_type.cpp
  ui/dialogs/web_dialog_ions_fragmentation.cpp
  ui/dialogs/web_dialog_file_removing.cpp
  ui/dialogs/web_dialog_input_dialog.cpp
  ui/dialogs/web_dialog_pls_load_plot_pick_axis.cpp
  ui/dialogs/web_dialog_join_tables.cpp
  ui/dialogs/web_dialog_question.cpp
  ui/dialogs/web_dialog_concentration_table.cpp
  ui/dialogs/web_dialog_pls_predict_settings.cpp
  ui/dialogs/web_dialog_mcr_build_settings.cpp
  ui/dialogs/web_dialog_ratio_matrix_settings.cpp
  ui/dialogs/web_dialog_stacked.cpp
  ui/dialogs/markup_save_table_dialog.cpp
  ui/dialogs/example_genesis_dialog.cpp
  ui/dialogs/markup/intermarkers_transfer_dialog.cpp
  ui/dialogs/markup/web_dialog_add_chromatogramms.cpp
  ui/dialogs/markup/biomarkers_markup_save_table_dialog.cpp
  ui/dialogs/project_table_rename_dialog.cpp
  ui/dialogs/project_table_rename_dialog_refactor.cpp
  ui/dialogs/table_rename_dialog.cpp
  ui/dialogs/hotelling_criterion_parameters_dialog.cpp
  ui/dialogs/export_tables_and_plots_dialog.cpp
  ui/dialogs/chromatogram_passport_dialog.cpp
  ui/dialogs/height_ratio_matrix/height_ratio_info.cpp
  ui/dialogs/naming_pattern_edit_dialog.cpp
  ui/dialogs/web_dialog_group_editor.cpp
  ui/dialogs/web_dialog_calculate_coefficient_table.cpp
  ui/dialogs/web_dialog_legend_settings.cpp
)

set(
  UIS ${UIS}
  ui/dialogs/web_dialog_import_files.ui
  ui/dialogs/web_dialog_import_xls_file.ui
  ui/dialogs/web_dialog_import_file_type.ui
  ui/dialogs/web_dialog_xls_file_import_type.ui
  ui/dialogs/web_dialog_ions_fragmentation.ui
  ui/dialogs/web_dialog_file_removing.ui
  ui/dialogs/web_dialog_pls_predict_settings.ui
  ui/dialogs/web_dialog_pls_load_plot_pick_axis.ui
  ui/dialogs/web_dialog_change_title_and_comment.ui
  ui/dialogs/web_dialog_question.ui
  ui/dialogs/web_dialog_mcr_build_settings.ui
  ui/dialogs/web_dialog_create_project_refactor.ui
  ui/dialogs/web_dialog_check_code_word.ui
  ui/dialogs/markup_save_table_dialog.ui
  ui/dialogs/example_genesis_dialog.ui
  ui/dialogs/chromatogram_widget_settings_dialog_refactor.ui
  ui/dialogs/chromatogram_widget_settings_paramerets_dialog.ui
  ui/dialogs/hotelling_criterion_parameters_dialog.ui
  ui/dialogs/markup/intermarkers_transfer_dialog.ui
  ui/dialogs/markup/web_dialog_add_chromatogramms.ui
  ui/dialogs/markup/biomarkers_markup_save_table_dialog.ui
  ui/dialogs/project_table_rename_dialog.ui
  ui/dialogs/project_table_rename_dialog_refactor.ui
  ui/dialogs/web_dialog_pca_items_settings.ui
  ui/dialogs/web_dialog_pls_build_settings_refactor.ui
  ui/dialogs/chromatogram_passport_dialog.ui
  ui/dialogs/height_ratio_matrix/height_ratio_info.ui
  ui/dialogs/naming_pattern_edit_dialog.ui
  ui/dialogs/web_dialog_legend_settings.ui
)

# sources / ui / controls
set(
  HEADERS ${HEADERS}
  ui/controls/dnd_files.h
  ui/controls/table_status_bar.h
  ui/controls/text_edit.h
  ui/controls/text_edit_emails.h
  ui/controls/spinbox_action.h
  ui/controls/file_plate_widget.h
  ui/controls/dnd_zone_frame.h
  ui/controls/range_widget.h
  ui/controls/tab_switcher.h
  ui/controls/customcheckbox.h
  ui/controls/multi_select_combobox.h
  ui/controls/data_table_caption_button_controls.h
  ui/controls/double_column_delegate.h
  ui/controls/spinbox_item_delegate.h
  ui/controls/combobox_item_delegate.h
  ui/controls/dialog_line_edit_ex.h
  ui/controls/dialog_text_edit_ex.h
  ui/controls/main_menu_splitter.h
  ui/controls/circle_button.h
  ui/controls/dnd_files_widget.h
  ui/controls/dnd_files_container.h
  ui/controls/completer_model.h
  ui/controls/formula_text_edit.h
  ui/controls/substring_highlighter.h
  ui/controls/pen_style_combo_box.h
  ui/controls/line_ending_combo_box.h
)

set(
  SOURCES ${SOURCES}
  ui/controls/dnd_files.cpp
  ui/controls/table_status_bar.cpp
  ui/controls/text_edit.cpp
  ui/controls/customcheckbox.cpp
  ui/controls/text_edit_emails.cpp
  ui/controls/spinbox_action.cpp
  ui/controls/file_plate_widget.cpp
  ui/controls/dnd_zone_frame.cpp
  ui/controls/range_widget.cpp
  ui/controls/tab_switcher.cpp
  ui/controls/multi_select_combobox.cpp
  ui/controls/data_table_caption_button_controls.cpp
  ui/controls/double_column_delegate.cpp
  ui/controls/dialog_line_edit_ex.cpp
  ui/controls/dialog_text_edit_ex.cpp
  ui/controls/main_menu_splitter.cpp
  ui/controls/circle_button.cpp
  ui/controls/dnd_files_widget.cpp
  ui/controls/dnd_files_container.cpp
  ui/controls/completer_model.cpp
  ui/controls/formula_text_edit.cpp
  ui/controls/substring_highlighter.cpp
  ui/controls/spinbox_item_delegate.cpp
  ui/controls/combobox_item_delegate.cpp
  ui/controls/pen_style_combo_box.cpp
  ui/controls/line_ending_combo_box.cpp
)
set(
  UIS ${UIS}
  ui/controls/file_plate_widget.ui
  ui/controls/range_widget.ui
  ui/controls/data_table_caption_button_controls.ui
  ui/controls/dialog_line_edit_ex.ui
  ui/controls/dialog_text_edit_ex.ui
  ui/controls/data_table_caption_button_controls.ui
  ui/controls/dnd_files_widget.ui
  ui/controls/dnd_files_container.ui
)
# sources / ui / itemviews
set(
  HEADERS ${HEADERS}
  ui/itemviews/tableview_headerview.h
  ui/itemviews/tableview_model_actions_column_delegate.h
  ui/itemviews/tableview_headerview_filter_popup.h
  ui/itemviews/tableview_headerview_filter_list.h
  ui/itemviews/tableview_headerview_filter_popup_editor.h
  ui/itemviews/checkable_headerview.h
  ui/itemviews/checkable_marker_headerview.h
  ui/itemviews/common_table_view.h
  ui/itemviews/peaks_column_item_delegate.h
  ui/itemviews/datatable_item_delegate.h
  ui/itemviews/comboboxitemdelegate.h
  ui/itemviews/files_table_actions_column_delegate.h
  ui/itemviews/table_identity_model_actions_delegate.h
  ui/itemviews/data_checkable_headerview.h
  ui/itemviews/event_table_view.h
)

set(
  SOURCES ${SOURCES}
  ui/itemviews/tableview_headerview.cpp
  ui/itemviews/tableview_model_actions_column_delegate.cpp
  ui/itemviews/tableview_headerview_filter_popup.cpp
  ui/itemviews/tableview_headerview_filter_list.cpp
  ui/itemviews/tableview_headerview_filter_popup_editor.cpp
  ui/itemviews/checkable_headerview.cpp
  ui/itemviews/checkable_marker_headerview.cpp
  ui/itemviews/common_table_view.cpp
  ui/itemviews/peaks_column_item_delegate.cpp
  ui/itemviews/datatable_item_delegate.cpp
  ui/itemviews/comboboxitemdelegate.cpp
  ui/itemviews/files_table_actions_column_delegate.cpp
  ui/itemviews/table_identity_model_actions_delegate.cpp
  ui/itemviews/data_checkable_headerview.cpp
  ui/itemviews/event_table_view.cpp
)

# sources / ui / contexts
set(
  SOURCES ${SOURCES}
  ui/contexts/edit_delete_menu_context.cpp
  ui/contexts/analysis_model_settings_context.cpp
  ui/contexts/new_model_settings_context.cpp
  ui/contexts/exist_model_settings_context.cpp
  ui/contexts/data_table_context.cpp
  ui/contexts/build_model_context.cpp
  ui/contexts/slope_offset_context.cpp
  ui/contexts/mcr_difference_menu_context.cpp
  ui/contexts/mcr_origin_restored_menu_context.cpp
  ui/contexts/pls_setup_view_context.cpp
  ui/contexts/plot_setup_view_context.cpp
)

set(
  HEADERS ${HEADERS}
  ui/contexts/edit_delete_menu_context.h
  ui/contexts/analysis_model_settings_context.h
  ui/contexts/new_model_settings_context.h
  ui/contexts/exist_model_settings_context.h
  ui/contexts/data_table_context.h
  ui/contexts/build_model_context.h
  ui/contexts/slope_offset_context.h
  ui/contexts/mcr_difference_menu_context.h
  ui/contexts/mcr_origin_restored_menu_context.h
  ui/contexts/plot_setup_view_context.h
  ui/contexts/pls_setup_view_context.h
)

# sources / ui / chromatogram pages
set(
  SOURCES ${SOURCES}
  ui/chromotogram_page/legend_widget.cpp
)

set(
  HEADERS ${HEADERS}
  ui/chromotogram_page/legend_widget.h
)
# sources / ui / plots
set(
  SOURCES ${SOURCES}
  ui/plots/chromatogram_plot.cpp
  ui/plots/pca_plots.cpp
  ui/plots/load_plot.cpp
  ui/plots/counts_plot.cpp
  ui/plots/counts_plot_manual_hotteling.cpp
  ui/plots/pls_plot_tiped.cpp
  ui/plots/mcr_difference_plot_tiped.cpp
  ui/plots/mcr_origin_restored_plot_tiped.cpp
  ui/plots/efa_plot.cpp
  ui/plots/slope_offset_plot.cpp
  ui/plots/pls_coefficient_plot.cpp
  ui/plots/graphics_interactive_model_plot.cpp
  ui/plots/spectrum_plot.cpp
  ui/plots/analysis_plot_refactor.cpp
  ui/plots/triplot/custom_axis_painter_private.cpp
  ui/plots/triplot/custom_axis.cpp
  ui/plots/triplot/custom_grid.cpp
)

set(
  HEADERS ${HEADERS}
  ui/plots/chromatogram_plot.h
  ui/plots/pca_plots.h
  ui/plots/load_plot.h
  ui/plots/counts_plot.h
  ui/plots/counts_plot_manual_hotteling.h
  ui/plots/pls_plot_tiped.h
  ui/plots/mcr_difference_plot_tiped.h
  ui/plots/mcr_origin_restored_plot_tiped.h
  ui/plots/efa_plot.h
  ui/plots/slope_offset_plot.h
  ui/plots/pls_coefficient_plot.h
  ui/plots/graphics_interactive_model_plot.h
  ui/plots/spectrum_plot.h
  ui/plots/analysis_plot_refactor.h
  ui/plots/triplot/custom_axis_painter_private.h
  ui/plots/triplot/custom_axis.h
  ui/plots/triplot/custom_grid.h
)
# sources / ui / plot items
set(
  SOURCES ${SOURCES}
  ui/plots/gp_items/parameters_info_plate.cpp
  ui/plots/gp_items/sample_info_plate.cpp
  ui/plots/gp_items/gp_curve_tracer_item.cpp
  ui/plots/gp_items/chromatogram_baseline.cpp
  ui/plots/gp_items/chromatogram_curve.cpp
  ui/plots/gp_items/chromatogram_peak.cpp
  ui/plots/gp_items/chromatogram_interval.cpp
  ui/plots/gp_items/gp_highlight_curve.cpp
  ui/plots/gp_items/chromatogram_key_selector.cpp
  ui/plots/gp_items/bars_labels_item.cpp
  ui/plots/gp_items/gpshape.cpp
  ui/plots/gp_items/gpshape_item.cpp
  ui/plots/gp_items/gpshape_with_label_item.cpp
  ui/plots/gp_items/gpshape_with_label_item_custom_hotteling.cpp
  ui/plots/gp_items/gp_rich_text_item.cpp
)

set(
  HEADERS ${HEADERS}
  ui/plots/gp_items/parameters_info_plate.h
  ui/plots/gp_items/sample_info_plate.h
  ui/plots/gp_items/gp_curve_tracer_item.h
  ui/plots/gp_items/chromatogram_baseline.h
  ui/plots/gp_items/chromatogram_curve.h
  ui/plots/gp_items/chromatogram_peak.h
  ui/plots/gp_items/chromatogram_interval.h
  ui/plots/gp_items/gp_highlight_curve.h
  ui/plots/gp_items/chromatogram_key_selector.h
  ui/plots/gp_items/bars_labels_item.h
  ui/plots/gp_items/gpshape.h
  ui/plots/gp_items/gpshape_item.h
  ui/plots/gp_items/gpshape_with_label_item.h
  ui/plots/gp_items/gpshape_with_label_item_custom_hotteling.h
  ui/plots/gp_items/gp_rich_text_item.h
)

set(
  DOCS ${DOCS}
  ui/dialogs/ui_dialog_standard.md
)

# resources
set(RESOURCES resource.qrc)

# icon
if (APPLE)
  set(MACOSX_BUNDLE_ICON_FILE genesis.icns)
  set(PLATFORM_ICON "genesis.icns")
  set_source_files_properties(${PLATFORM_ICON} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources")
else ()
endif ()

if (WIN32)
  SET(WINDOWS_RESOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/app.rc)
endif ()

if(PUBLIC)
  if (NOT CONSOLE_ENABLED)
    add_compile_definitions(LOGGER_ENABLE)
  endif()
endif()

qt_wrap_ui( UI_HEADERS ${UIS} )

# target
add_executable(
  Genesis

  # bundle
  ${PLATFORM_BUNDLE}

  ${PLATFORM_ICON}

  ${SOURCES}
  ${HEADERS}
  ${RESOURCES}
  ${UI_HEADERS}
  ${DOCS}
  ${WINDOWS_RESOURCE_PATH}
  )

# dependencies / genesis libs / crypt
# add_subdirectory(${ROOT_PATH}/genesis_crypt ${ROOT_PATH}/bin)

# dependencies / genesis libs / style
# add_subdirectory(${ROOT_PATH}/genesis_style)

# dependencies / extern / thirdparty
#

# dependencies / extern / core
# add_subdirectory(${ROOT_PATH}/extern/common_core/settings)
# add_subdirectory(${ROOT_PATH}/extern/common_core/standard_models)

# dependencies / extern / core / conditional
if (NOT DEFINED DISABLE_SETTINGS_WRITABLE_LOCATION)
  add_subdirectory(${ROOT_PATH}/extern/common_thirdparty/miniz)
  add_subdirectory(extern/common_core/utils/json_utils)
endif ()

# dependencies / extern / gui
# add_subdirectory(${ROOT_PATH}/extern/common_gui/standard_widgets)

# ssl
if (APPLE)
  if (NOT DEFINED DISABLE_SETTINGS_WRITABLE_LOCATION)
  set(
    SSL_LIBRARIES
    /usr/local/opt/openssl@1.1/lib/libcrypto.a
    /usr/local/opt/openssl@1.1/lib/libssl.a
  )
  endif ()
endif ()
if (WIN32)
  if (NOT DEFINED DISABLE_SETTINGS_WRITABLE_LOCATION)
    set(
      SSL_LIBRARIES
  #    ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/extern/common_thirdparty/openssl-1.1.1l/x64/lib/libcrypto.lib
  #    ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/extern/common_thirdparty/openssl-1.1.1l/x64/lib/libssl.lib

      ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/extern/common_thirdparty/openssl_3/x64/lib/libcrypto.lib
      ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/extern/common_thirdparty/openssl_3/x64/lib/libssl.lib
    )
  endif ()
endif ()

# ssl lib paths

# lib paths
set(CMAKE_EXE_LINKER_FLAGS_DEBUG
  -L${PROJECT_SOURCE_DIR}/${ROOT_PATH}/lib/debug
  -L${PROJECT_SOURCE_DIR}/${ROOT_PATH}/extern/common_thirdparty/lib/debug
  -L${PROJECT_SOURCE_DIR}/${ROOT_PATH}/extern/common_core/lib/debug
  -L${PROJECT_SOURCE_DIR}/${ROOT_PATH}/extern/common_gui/lib/debug
)
set(CMAKE_EXE_LINKER_FLAGS_RELEASE
  -L${PROJECT_SOURCE_DIR}/${ROOT_PATH}/lib/release
  -L${PROJECT_SOURCE_DIR}/${ROOT_PATH}/extern/common_thirdparty/lib/release
  -L${PROJECT_SOURCE_DIR}/${ROOT_PATH}/extern/common_core/lib/release
  -L${PROJECT_SOURCE_DIR}/${ROOT_PATH}/extern/common_gui/lib/release
)

if (${QT_VERSION_MAJOR} EQUAL 6)
  set(QT_COMPATIBILITY_LIBS Qt${QT_VERSION_MAJOR}::Core5Compat)
endif ()

if (NOT PLATFORM_WASM)
  set(QT_DBUS_LIBS Qt${QT_VERSION_MAJOR}::DBus)
endif ()

# libs
if (NOT DEFINED DISABLE_SETTINGS_WRITABLE_LOCATION)
target_link_libraries(
  Genesis

  # qt
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Gui
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::OpenGL
  Qt${QT_VERSION_MAJOR}::PrintSupport
  Qt${QT_VERSION_MAJOR}::Network

  ${QT_COMPATIBILITY_LIBS}
  ${QT_DBUS_LIBS}

  # ssl
  ${SSL_LIBRARIES}

  # genesis
  genesis_crypt
  genesis_style

  # thirdparty
  miniz

  # core
  settings
  standard_models
  json_utils

  # gui
  standard_widgets

#   RenderDoc
#  RenderDoc
)
else()
  target_link_libraries(
    Genesis

    # qt
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Network

    ${QT_COMPATIBILITY_LIBS}
    ${QT_DBUS_LIBS}

    # ssl
    ${SSL_LIBRARIES}

    # genesis
#    genesis_crypt
    genesis_style

    # thirdparty
  #  miniz

    # core
    settings
    standard_models
  #  json_utils

    # gui
    standard_widgets

  #   RenderDoc
  #  RenderDoc
  )
endif()
#target_link_libraries(Genesis RenderDoc)

# version
set (GENESIS_VERSION 0.1)

# bundle
if (APPLE)
  set_target_properties(Genesis PROPERTIES
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
    OUTPUT_NAME "Genesis"
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME "Genesis"
    MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/Info.plist
    MACOSX_BUNDLE_BUNDLE_VERSION ${GENESIS_VERSION}
    MACOSX_BUNDLE_LONG_VERSION_STRING "Genesis v${GENESIS_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${GENESIS_VERSION}"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.my.app"
    MACOSX_BUNDLE_COPYRIGHT "(C) 2021 CET MIPT"
    MACOSX_RPATH TRUE
    MACOSX_FRAMEWORK_IDENTIFIER com.genesis.bundle.id
    XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/../Frameworks/"
    # RESOURCE "${GENESIS_ICON}"
    XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME TRUE
    XCODE_ATTRIBUTE_EXECUTABLE_NAME "Genesis"
    XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"

    # BUILD_WITH_INSTALL_RPATH 1
    # INSTALL_NAME_DIR "@rpath"

    XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
  )
endif ()

# deploy target
if (APPLE)
  set (PLATFORM_DEPLOY_TARGET "$<TARGET_FILE_DIR:Genesis>/../..")
else ()
  set (PLATFORM_DEPLOY_TARGET "$<TARGET_FILE:Genesis>")
  set (PLATFORM_DEPLOY_ARGS   "--no-compiler-runtime")
endif ()

# deploy
if (NOT NO_DEPLOY)
if (APPLE)
  add_custom_command(TARGET Genesis POST_BUILD
    COMMAND "${PLATFORM_DEPLOYQT_EXECUTABLE}" "${PLATFORM_DEPLOY_TARGET}"
    COMMENT "Running macdeployqt..."
  )
endif ()
  if (WIN32)
    add_custom_target(windeployqt ALL ${PLATFORM_DEPLOYQT_EXECUTABLE} --dir $<TARGET_FILE_DIR:Genesis>
    $<TARGET_FILE:Genesis>
    DEPENDS Genesis
    COMMENT "Preparing Qt runtime dependencies")

    if(NOT SKIP_OPENSSL_DEPLOY)
      if(EXISTS ${openssl_OpenSSL_Crypto_BIN_DIRS_RELEASE})
        message(WARNING " OPENSSL FOUND in=${openssl_OpenSSL_Crypto_BIN_DIRS_RELEASE}")
        add_custom_command(TARGET Genesis POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
              "${openssl_OpenSSL_Crypto_BIN_DIRS_RELEASE}/libcrypto-3-x64.dll"
              $<TARGET_FILE_DIR:Genesis>)

        add_custom_command(TARGET Genesis POST_BUILD
              COMMAND ${CMAKE_COMMAND} -E copy_if_different
              "${openssl_OpenSSL_Crypto_BIN_DIRS_RELEASE}/libssl-3-x64.dll"
              $<TARGET_FILE_DIR:Genesis>)
      else()
        message(WARNING " OPENSSL NOT FOUND, try to use binaries from repo")
        add_custom_command(TARGET Genesis POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
              "${PROJECT_SOURCE_DIR}/${ROOT_PATH}/extern/common_thirdparty/openssl_3/x64/bin/libcrypto-3-x64.dll"
              $<TARGET_FILE_DIR:Genesis>)

        add_custom_command(TARGET Genesis POST_BUILD
              COMMAND ${CMAKE_COMMAND} -E copy_if_different
              "${PROJECT_SOURCE_DIR}/${ROOT_PATH}/extern/common_thirdparty/openssl_3/x64/bin/libssl-3-x64.dll"
              $<TARGET_FILE_DIR:Genesis>)
      endif()
    else()
      message(WARNING "SKIP OPENSSL DEPLOY")
    endif()
  endif ()
endif ()

if (PLATFORM_WASM)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s EXPORTED_FUNCTIONS=[_main,_htons,_ntohs,__get_timezone,__get_daylight,__get_tzname,_setThrew] -s EXPORT_ALL=1")
endif ()


# Print all variables

# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()
