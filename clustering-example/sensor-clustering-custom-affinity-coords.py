# ward minimizes the variance of the clusters being merged.
# average uses the average of the distances of each observation of the two sets.
# complete or maximum linkage uses the maximum distances between all observations of the two sets.
# single uses the minimum of the distances between all observations of the two sets.

# https://dev.to/meetsohail/change-the-python3-default-version-in-ubuntu-1ekb
# https://scikit-learn.org/stable/install.html
# https://scikit-learn.org/stable/modules/generated/sklearn.cluster.AgglomerativeClustering.html

import sys
import sklearn

from module.helpers import visualization, output, sensors, rivers, gateways
from module.clustering import intersections, outliers
from module.clustering.circle_clustering import CircleClustering

import geopy.distance

import time
start_time = time.time()
##############################################
# Process sys args for their default values
##############################################
# Maximum distance threshold between two sensors in a cluster
# 5000 range =>  8660.2540378 max distance between two points
distance_threshold = float (sys.argv [1])

# If the number of sensors in a cluster is less than or equal to this
# these sensors will be removed before restarting the clustering
outlier_threshold = int (sys.argv [2])

remove_intersections = int (sys.argv [3])

river_available = int (sys.argv [4])

distance_error_margin = int (sys.argv [5])

sensor_file_name = sys.argv [6]

sensor_score_threshold = float (sys.argv [9])

if int (sys.argv [7]) == 1:
    sensor_cost_available = True
    sensor_cost_file = sys.argv [8]
else:
    sensor_cost_available = False

# River coordinates
if river_available == 1:
    river = rivers.read_river_data ()
else:
    river = []

# Clustering helper
clustering = CircleClustering ()
clustering.distance_threshold = distance_threshold
clustering.rivers = river
clustering.distance_error_margin = distance_error_margin

# In order to ease the computation times
# we decide on some minimum cut thresholds between lat and lon
# differences. So that we wont have to use geopy for every single
# calculation.
if distance_threshold < 3000:
    clustering.lat_margin = 0.04 # 0.1 # 0.04
    clustering.lon_margin = 0.05 # 0.2 # 0.05
if distance_threshold >= 3000:
    clustering.lat_margin = 0.04 # 0.3 # 0.04
    clustering.lon_margin = 0.05 # 0.4 # 0.05
if distance_threshold >= 4000:
    clustering.lat_margin = 0.07 # 0.5 # 0.07
    clustering.lon_margin = 0.1 # 0.6 # 0.1
if distance_threshold >= 7500:
    clustering.lat_margin = 0.1 # 0.7 # 0.1
    clustering.lon_margin = 0.15 # 0.8 # 0.15
if distance_threshold >= 11000:
    print ("Not supported distance.")
    sys.exit (0)

lat_dist = geopy.distance.distance ([40.0, 27.0], [40.0 + clustering.lat_margin, 27.0]).m
lon_dist = geopy.distance.distance ([40.0, 27.0], [40.0, 27.0 + clustering.lon_margin]).m
print ("Using min lat-lon thresholds: ", clustering.lat_margin, " ", clustering.lon_margin)
print ("Quick distance checks according to these thresholds (m): ", lat_dist, " ", lon_dist)

#####################################################
# Initial clustering to determine outliers
# distance_threshold is the maximum distance allowed between
# any two points, this is practically the range x square_root of 3
print("Sklearn version: ", sklearn.__version__)

sklearn_array = sensors.read_sensor_data (sensor_file_name, sensor_cost_available, sensor_cost_file, sensor_score_threshold)
initial_enddevice_count = len (sklearn_array)
print ("Number of sensors found: ", initial_enddevice_count)
assert (initial_enddevice_count > 0)

labels, clusters = clustering.clustering_algorithm_set_cover (sklearn_array)

# assert (False)
# labels, clusters = clustering.clustering_algorithm (sklearn_array)

print ("Number of Clusters: ", len (clusters))
print ("# of labels: ", len (labels), ", Labels: ", labels)

# Debugging
print ("clusters: ", clusters)

removed_sensor_coords = []

sklearn_array, clusters, gateway_points, threshold_outlier_count, removed_sensor_coords = \
    outliers.apply_threshold_determine_final_gateways (sklearn_array, clusters, outlier_threshold, clustering, removed_sensor_coords, distance_threshold, distance_error_margin)

if len(gateway_points) == 0:
    print ("No appropriate gateways found.")
    # Update the validation file
    c_validation_f = open("clustering-example/clustering_validation.txt", "w")
    c_validation_f.write ('0')
    c_validation_f.close ()
    sys.exit (0)
else:
    # Update the validation file
    c_validation_f = open("clustering-example/clustering_validation.txt", "w")
    c_validation_f.write ('1')
    c_validation_f.close ()    

##################################################
# Determine which of the sensors should be removed
# from the clusters, considering if they are in the
# range of more than one centroid!
##################################################
interfering_sensors, no_element_clusters = intersections.remove_intersection_sensors (sklearn_array, gateway_points, clusters, distance_threshold, distance_error_margin)
if remove_intersections:
    # Update the removed sensor list
    for rmv_sensor in [sensor for sensor in sklearn_array if sensor.name in interfering_sensors]:
        removed_sensor_coords.append (rmv_sensor)

    # Update original array
    sklearn_array = [sensor for sensor in sklearn_array if sensor.name not in interfering_sensors]

    # Remove the clusters with no elements left in them
    for no_element_key in no_element_clusters:
        del clusters [no_element_key]
        del gateway_points [no_element_key]

    print ("Sensors in multiple clusters are removed.")
else:
    print ("Sensors in multiple clusters are NOT removed.")

##################################################
# Everything should be finished by here
# Print the locations into files
##################################################
output.output_locations (distance_threshold, outlier_threshold, remove_intersections, sensor_score_threshold, sklearn_array, gateway_points, clusters)

##################################################
# Visualize the expected environment with and without the removed sensors
##################################################
visualization.process_visualization (sklearn_array, gateway_points, river)

if len (removed_sensor_coords) > 0:
    visualization.process_visualization_with_removed (sklearn_array, gateway_points, river, removed_sensor_coords)

##################################################
# Statistics
##################################################
# Values
gateway_count = len (gateway_points)
remaining_sensor_count = len (sklearn_array)
multiple_cluster_sensor_count = len (interfering_sensors)
river_point_count = len (river)
if river_available == 1:
    rivers_within_gateway_count = clustering.rivers_within_gateway_count (gateway_points, river)
else:
    rivers_within_gateway_count = 0
# Output
f = open("clustering-example/output/statistics.txt", "w")
statistics_header = "Signal Range Threshold" + "\t" + "Sensor Count Threshold" + "\t" + "Gateway Count" + "\t" \
    + "Total Sensor" + "\t" + "Remaining Sensors" + "\t" \
    + "Threshold Outlier Sensors" + "\t" + "Sensors in Multiple Clusters" + "\t" + "Number of river points" + "\t" \
    + "Number of River points in range of a gateway" + "\n"
statistics_output = str (int(distance_threshold)) + "\t" + str (outlier_threshold) + "\t" + str (gateway_count) + "\t" \
    + str (initial_enddevice_count) + "\t"  + str (remaining_sensor_count) + "\t" \
    + str (threshold_outlier_count) + "\t" + str (multiple_cluster_sensor_count) + "\t" \
    + str (river_point_count) + "\t" + str (rivers_within_gateway_count)
# Writing
f.write (statistics_header)
f.write (statistics_output)
f.close ()
print("Clustering execution time: --- %s seconds ---" % (time.time() - start_time))
# END