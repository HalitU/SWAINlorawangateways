class ExistingGateways:
    def __init__ (self, gw_key, gw_latitude, gw_longitude, gw_altitude):
        self.gw_key = gw_key;
        self.gw_latitude = gw_latitude
        self.gw_longitude = gw_longitude;
        self.gw_altitude = gw_altitude;


def read_existing_gateways (distance_threshold, outlier_threshold, remove_intersections, existing_gateway_sensor_file, existing_gateway_file, sensors):
    existing_gateways = []
    clusters = {}

    gateways_file = open(existing_gateway_sensor_file, 'r')
    gateways = gateways_file.readlines()
    
    sensor_keys = [sensor.name for sensor in sensors]

    skip_header = True
    for gateway_sensor in gateways:
        if skip_header:
            skip_header = False
        else:
            raw_data = gateway_sensor.strip().split (',')

            # Get the gateways with the required parameters and has connection to the chosen sensors
            gw_key, sensor_key, gw_distance_threshold, gw_outlier_threshold, gw_remove_intersections, sensor_latitude,sensor_longitude,sensor_altitude = raw_data

            gw_distance_threshold = float (gw_distance_threshold)
            gw_remove_intersections = float (gw_remove_intersections)
            gw_outlier_threshold = float (gw_outlier_threshold)

            if gw_distance_threshold == distance_threshold and gw_outlier_threshold == outlier_threshold and gw_remove_intersections == remove_intersections \
                and sensor_key in sensor_keys:
                if gw_key not in existing_gateways:
                    existing_gateways.append (gw_key)
                    clusters [gw_key] = []
                clusters [gw_key].append (sensor_key)

    # Get the location of the gateways
    finalized_gateways = {}

    gateway_locations = open(existing_gateway_file, 'r')
    gateway_locations = gateway_locations.readlines()
    
    skip_header = True
    for gateway_location in gateway_locations:
        if skip_header:
            skip_header = False
        else:
            raw_data = gateway_location.strip().split (',')
            gw_key,gw_distance_threshold,gw_remove_intersections,gw_outlier_threshold,gw_latitude,gw_longitude,gw_altitude = raw_data

            gw_distance_threshold = float (gw_distance_threshold)
            gw_remove_intersections = float (gw_remove_intersections)
            gw_outlier_threshold = float (gw_outlier_threshold)

            if gw_distance_threshold == distance_threshold and gw_outlier_threshold == outlier_threshold and gw_remove_intersections == remove_intersections \
                and gw_key in existing_gateways:
                finalized_gateways [gw_key]  = [gw_latitude, gw_longitude, gw_altitude]

    print ("Number of existing gateways found: ", len (finalized_gateways))
    assert (len (finalized_gateways.items ()) > 0)
    return finalized_gateways, clusters