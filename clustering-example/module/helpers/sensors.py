import sys
import math

class SensorLocation:
    def __init__ (self, name, lat, lon, height):
        self.name = name
        self.lat = lat
        self.lon = lon
        self.height = height
        self.cost = math.inf
        self.score = -math.inf

def read_sensor_data (sensor_file_name, sensor_cost_available, sensor_cost_file, sensor_score_threshold):
    # Process the tab seperated sensor location file and convert them into X, Y, Z
    sensor_locations_file = open(sensor_file_name, 'r')
    locations = sensor_locations_file.readlines()
    locations_array = [] # will be used to keep the information of all sensors
    sec_delim = '\''
    # min_delim = 'Â°' # TODO: something might be wrong with the .tsv file in windows
    min_delim = '°'

    # Strips the newline character
    skip_header = True
    for location in locations:
        if skip_header:
            skip_header = False
        else:
            raw_data = location.strip().split ('\t')

            # Sensor name
            sensor_name = raw_data [0]

            # Latitude
            sec_vector = raw_data [1][0:len (raw_data [1]) - 1].split (sec_delim)
            lat_seconds = float (sec_vector [-1])
            sec_vector = sec_vector [0].split (min_delim)
            lat_minutes = float (sec_vector [-1])
            lat_degrees = float (sec_vector [0])
            lat_decimal_degrees = lat_degrees + lat_minutes / 60.0 + lat_seconds / 3600.0;

            # Longitude
            sec_vector = raw_data [2][0:len (raw_data [2]) - 1].split (sec_delim)
            lon_seconds = float (sec_vector [-1])
            sec_vector = sec_vector [0].split (min_delim)
            lon_minutes = float (sec_vector [-1])
            lon_degrees = float (sec_vector [0])
            lon_decimal_degrees = lon_degrees + lon_minutes / 60.0 + lon_seconds / 3600.0;    

            # Height
            height = float (raw_data [3])

            # Create the sensor object and add it to the array
            locations_array.append (SensorLocation (sensor_name, lat_decimal_degrees, lon_decimal_degrees, height))

    # Get the cost values and append them to the existing sensors
    if sensor_cost_available:
        sensor_cost_file = open(sensor_cost_file, 'r')
        sensor_costs = sensor_cost_file.readlines()
        skip_header = True
        for sensor_cost in sensor_costs:
            if skip_header:
                skip_header = False
            else:
                # Read the costs line by line
                raw_data = sensor_cost.strip().split (',')
                
                # Get the cost
                sensor_name = raw_data [1]
                current_cost = float (raw_data [3])
                current_score = float (raw_data [4])

                # Update the locations_array
                for location in locations_array:
                    if location.name == sensor_name:
                        location.cost = current_cost
                        location.score = current_score

        # Remove the sensors are above the given cost threshold
        locations_array = [location for location in locations_array if location.score >= sensor_score_threshold]

    return locations_array
