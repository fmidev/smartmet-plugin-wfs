#pragma once

#include "StoredQueryHandlerFactoryDef.h"

// The stored query handler factories are looked up by name with dlsym(): the
// constructor_name setting of a stored query configuration names one of these
// objects. They must therefore have external linkage.

extern SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef wfs_coverage_query_handler_factory;
extern SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef wfs_dummy_handler_factory;
extern SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef wfs_flash_handler_factory;
extern SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef wfs_forecast_handler_factory;
extern SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef wfs_get_data_set_by_id_handler_factory;
extern SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef wfs_get_feature_by_id_handler_factory;
extern SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef wfs_grid_forecast_handler_factory;
extern SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef wfs_isoline_query_handler_factory;
extern SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef wfs_obs_handler_factory;
extern SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef wfs_stored_air_nuclide_handler_factory;
extern SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef wfs_stored_atom_handler_base_factory;
extern SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef wfs_stored_aviation_observation_handler_factory;
extern SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef wfs_stored_file_handler_factory;
extern SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef wfs_stored_geoserver_handler_factory;
extern SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef wfs_stored_grid_handler_factory;
extern SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef wfs_stored_mast_handler_factory;
extern SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef wfs_stored_qe_download_handler_factory;
extern SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef wfs_stored_sounding_handler_factory;
