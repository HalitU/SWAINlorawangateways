import numpy as np
from module.clustering.circle_clustering import CircleClustering
import geopy.distance
import math

def apply_threshold_determine_final_gateways (sklearn_array, clusters, outlier_threshold, clustering, removed_sensor_coords
    , distance_threshold, distance_error_margin):

    print ("Finding outliers which are below the threshold...")
    # Determine the outlier indices, which shows which clusters are below the threshold
    outlier_sensors = []
    outlier_clusters = []
    for key, cluster in clusters.items ():
        if len (cluster) <= outlier_threshold:
            outlier_sensors += cluster
            outlier_clusters.append (key)

    print ("Cluster sensor outliers which include sensors less than or equal to threshold: ", len (outlier_sensors), " " , outlier_sensors)

    # Get the real location of the gateway points for the non-outlier clusters
    gateway_points = {}
    for key, cluster in clusters.items ():
        if len (cluster) > outlier_threshold:
            current_cluster_points = [sensor for sensor in sklearn_array if sensor.name in cluster]
            gateway_point = clustering.intersectionArea (current_cluster_points)
            # print (current_cluster_points)
            # print (gateway_point)
            assert (len (gateway_point) > 0)
            gateway_points [key] = gateway_point

    # Assert the distance between gateway locations and the sensors in the corresponding clusters
    # this distance also includes the euclidean height
    for cluster_key, cluster in gateway_points.items ():
        g_x, g_y, g_z = gateway_points [cluster_key]
        sensors = clusters [cluster_key]
        # Calculate the distance between points and the gateway
        for s_name in sensors:
            sensor = [sensor for sensor in sklearn_array if sensor.name == s_name]
            assert (len (sensor) == 1) # Single sensor with such name should exist
            sensor = sensor [0]
            s_x, s_y, s_z = sensor.lat, sensor.lon, sensor.height
            dist = geopy.distance.distance ((g_x, g_y), (s_x, s_y)).m
            euclidean_dist = math.sqrt (dist**2 + (g_z - s_z)**2)
            assert (distance_threshold + distance_error_margin >= euclidean_dist)

    # Get the removed sensors
    for rmv_sensor in [sensor for sensor in sklearn_array if sensor.name in outlier_sensors]:
        removed_sensor_coords.append (rmv_sensor)

    # Remove the sensor outliers from the original sensor array
    sklearn_array = [sensor for sensor in sklearn_array if sensor.name not in outlier_sensors]

    # Remove the unnecessary clusters
    for key in outlier_clusters:
        del clusters [key]    

    return sklearn_array, clusters, gateway_points, len (outlier_sensors), removed_sensor_coords