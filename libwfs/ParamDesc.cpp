#include "ParamDesc.h"

namespace SmartMet
{
    namespace Plugin
    {
        namespace WFS
        {
            namespace ParamDesc
            {

                const char* bbox =
                    "Bounding box of area for which to return data.";

                const char* begin_time =
                    "Parameter begin specifies the begin of time interval in ISO-format"
                    " (for example 2012-02-27T00:00:00Z).";

                const char* crs =
                    "Coordinate system to use in response (for example crs:EPSG::4326";

                const char* data_set_id =
                    "Specifies ID of INSPIRE data set to return.";

                const char* end_time =
                    "End of time interval in ISO-format (for example 2012-02-27T00:00:00Z).";

                const char* fmisids =
                    "FMI observation station identifiers (0 or more values).";

                const char* geoids =
                    "GEOIDs of the locations for which to return data (0 or more values).";

                const char* hours =
                    "Hour values for which to report data (0 or more integer values).";

                const char* icao_code =
                    "Four-character alphanumeric code designating each airport around"
                    " the world. (for example EFHK).";

                const char* id =
                    "The GetFeatureById stored query is mandatory by WFS 2.0 spec.\n"
                    "In the FMI case it's mostly useless however, because most of the\n"
                    "features only have temporary IDs that are generated on-the-fly\n"
                    "during the response generation, and thus cannot be used as permanent\n"
                    "identifiers for those features";

                const char* keyword =
                    "";

                const char* keyword_overwritable =
                    "";

                const char* latlons =
                    "Location coordinates for which to return data"
                    " (0 or more latitude/longitude pairs).";

                const char* locale =
                    "Locale to use (for example fi_FI.UTF8)";

                const char* lpnns =
                    "LPNN code of the location for which to return data (0 or more values)";

                const char* max_distance =
                    "Maximal distance of sites from specified place for which to"
                    " provide data (mandatory real value)";

                const char* missing_text =
                    "Text to output when the value is missing";

                const char* num_steps =
                    "Number of timesteps in result set.";

                const char* origin_time =
                    "Override of origin time (for use in tests only,"
                    " leave empty for production use)";

                const char* param =
                    "List of parameters to return";

                const char* places =
                    "The location for which to provide data (0 or more values)";

                const char* quality_info =
                    "Specifies whether to requst quality info";

                const char* time_step =
                    "The time step of data in minutes. Notice that timestep is"
                    " calculated from start of the ongoing hour or day.";

                const char* times =
                    "Time values for which to return data. (0 or more values).";

                const char* tz =
                    "Time zone of the time instant of the data point in the form Area/Location"
                    " (for example America/Costa_Rica). The default value is UTC."
                    " Special values (case insensitive): utc, local";

                const char* wmos =
                    "WMO code of the location for which to return data (0 or more values)";

            }
        }
    }
}
