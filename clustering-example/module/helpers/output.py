import sys
import pandas as pd

def output_locations (distance_threshold, outlier_threshold, remove_intersections, score_threshold, enddevice_points, gateway_points, clusters):
    ##################################################
    # Everything should be finished by here
    # Print the locations into files
    ##################################################
    # Output the sensor locations
    sensor_file = open("clustering-example/output/sensors.txt", "w")
    for sensor in enddevice_points:
        sensor_file.write (sensor.name + '\t' + str (sensor.lat) + '\t' + str (sensor.lon) + '\t' + str (sensor.height) + '\n')
    sensor_file.close ()

    ##################################################
    # Output the gateway locations
    gateway_file = open("clustering-example/output/gateways.txt", "w")
    for gw_key, centroid in gateway_points.items ():
    # for key, centroid in cluster_gateway_locations.items ():
        gateway_file.write (str (gw_key) + '\t' + str (centroid [0]) + '\t' + str (centroid [1]) + '\t' + str (centroid [2]) + '\n')
    gateway_file.close ()

    # CSV Format for the first file
    output_gateway_locations_csv_format (distance_threshold, outlier_threshold, remove_intersections, score_threshold, gateway_points);

    # CSV output for the gateway-cluster relationship
    output_gateway_sensor_relationship (distance_threshold, outlier_threshold, remove_intersections, score_threshold, gateway_points, clusters, enddevice_points);


def output_gateway_locations_csv_format (distance_threshold, outlier_threshold, remove_intersections, score_threshold, gateway_points):
    # Create a new pandas dataframe
    temp_dataset = [[gw_key, distance_threshold, outlier_threshold, remove_intersections, score_threshold, lat, lon, altitude] for gw_key, (lat, lon, altitude) in gateway_points.items ()]
    df_new = pd.DataFrame(temp_dataset, columns=['gw_key', 'distance_threshold', 'outlier_threshold', 'remove_intersections', 'score_threshold', 'gw_latitude', 'gw_longitude', 'gw_altitude'])

    # First read existing csv file if it exist
    try:
        df = pd.read_csv("clustering-example/output/gateway_locations.csv", delimiter=',')
        print ("Cumulative csv output file is found. Results will be written/replaced on it.")
        # Remove the current key from the existing data-frame
        df.drop(df[(df.distance_threshold == distance_threshold) & (df.outlier_threshold == outlier_threshold) & (df.remove_intersections == remove_intersections) & (df.score_threshold == score_threshold)].index, inplace=True)
        df = df.append (df_new)
    except OSError as e:
        print ("Cumulative csv output file is not found. A new one will be created.")
        df = df_new

    # Print data-frame to csv file
    df.to_csv ("clustering-example/output/gateway_locations.csv", header=True, index_label=False, index=False)

def output_gateway_sensor_relationship (distance_threshold, outlier_threshold, remove_intersections, score_threshold, gateway_points, clusters, enddevice_points):
    # Create a new pandas dataframe for relation
    relation_dataset = [[get_gateway_of_sensor (sensor, clusters), sensor.name, sensor.lat, sensor.lon, sensor.height] for sensor in  enddevice_points]
    temp_dataset = [[gw_key, name, distance_threshold, outlier_threshold, remove_intersections, score_threshold, lat, lon, height] for (gw_key, name, lat, lon, height) in relation_dataset]
    df_new = pd.DataFrame(temp_dataset, columns=['gw_key', 'sensor_key', 'distance_threshold', 'outlier_threshold', 'remove_intersections', 'score_threshold', 'sensor_latitude', 'sensor_longitude', 'sensor_altitude'])

    # First read existing csv file if it exist
    try:
        df = pd.read_csv("clustering-example/output/gateway_sensor_relations.csv", delimiter=',')
        print ("Cumulative csv output file is found. Results will be written/replaced on it.")
        # Remove the current key from the existing data-frame
        df.drop(df[(df.distance_threshold == distance_threshold) & (df.outlier_threshold == outlier_threshold) & (df.remove_intersections == remove_intersections) & (df.score_threshold == score_threshold)].index, inplace=True)
        df = df.append (df_new)
    except OSError as e:
        print ("Cumulative csv output file is not found. A new one will be created.")
        df = df_new

    # Print data-frame to csv file
    df.to_csv ("clustering-example/output/gateway_sensor_relations.csv", header=True, index_label=False, index=False)

def get_gateway_of_sensor (sensor, clusters):
    for gw_key, c_s_s in clusters.items ():
        for c_s in c_s_s:
            if sensor.name == c_s:
                return gw_key
    assert (False)