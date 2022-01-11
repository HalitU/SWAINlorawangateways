import geopy.distance

def remove_intersection_sensors (sklearn_array, gateway_points, clusters, distance_threshold, distance_error_margin):
    interfering_sensors = []
    no_element_clusters = []
    # for each sensor
    for sensor in sklearn_array:
        #
        s_x, s_y, s_z = sensor.lat, sensor.lon, sensor.height
        #
        n_close_centroids = 0
        # check the distance to each centroid
        for gw_key, (c_x, c_y, c_z) in gateway_points.items ():
            dist = geopy.distance.distance ((s_x, s_y), (c_x, c_y)).m
            if dist <= (distance_threshold + distance_error_margin):
                n_close_centroids += 1
        
        # Make sure that all sensors are within the range of a gateway
        if n_close_centroids > 1:
            interfering_sensors.append (sensor.name)

        # At this point, all sensors should have at least be within the range of a gateway
        assert (n_close_centroids > 0)

    # Check if there are gateways who have left with no sensors!
    # TODO: We should remove the ones with no elements left inside them
    print (gateway_points)
    print (interfering_sensors)
    for gw_key, centroid in gateway_points.items ():
        cluster_sensors = clusters [gw_key]
        should_removed = includes_all_elements (interfering_sensors, cluster_sensors)
        if should_removed:
            no_element_clusters.append (gw_key)

    print ("# of sensors in multiple clusters: ", len (interfering_sensors), " list: ", interfering_sensors)
    print ("# of clusters with no elements left inside: ", len (no_element_clusters))

    return interfering_sensors, no_element_clusters

def includes_all_elements (source_arr, check_arr):
    for element in check_arr:
        if element not in source_arr:
            return False
    return True