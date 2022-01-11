# https://stackoverflow.com/questions/3207219/how-do-i-list-all-files-of-a-directory
import glob
import numpy as np

import pandas as pd

# We need to separate each experiment into different sections according to their ED count and speed
results = {}

# Get all of the results
main_directory = "experiments/swain-simulation-results/"
experiment_result_list = glob.glob(main_directory + "*/per_sensor_results.txt")

# Read the appropriate keys from the relationship file
rels = {}
rel = open ("clustering-example/output/gateway_sensor_relations.csv", "rt")
rel_lines = rel.read ().splitlines ()
for rel_line in rel_lines [1:]:
    current_rel = rel_line.split (',')
    rel_sensor_key = current_rel [1]
    rel_distance_threshold = float (current_rel [2].split ('.')[0])
    rel_outlier_threshold = int (current_rel [3].split ('.')[0])
    rel_remove_intersections = int (current_rel [4].split ('.')[0])
    rel_score_threshold = float (current_rel [5])

    assert rel_score_threshold == 0.001 or rel_score_threshold == 0.3 or rel_score_threshold == 0.47, "Problem with score: " + rel_score_threshold

    if (rel_sensor_key, rel_distance_threshold, rel_outlier_threshold, rel_remove_intersections, rel_score_threshold) not in rels:
        rels [rel_sensor_key, rel_distance_threshold, rel_outlier_threshold, rel_remove_intersections, rel_score_threshold] = []

print ("Reading experiment files...")

# Read the experiment results
for ex_file in experiment_result_list:
    # Read input file
    fin = open (ex_file, "rt")

    # Get the lines
    all_lines = fin.read().splitlines()

    # Read the first line for header
    header = all_lines [0]

    # Read the sensor data lines
    for line in all_lines [1:]:
        # Get the cluster parameters and the end-device name
        # key = distance_threshold	outlier_threshold	remove_intersections	sensor_name
        current_results = line.split ('\t')
        
        sensor_name = current_results[0]
        distance_threshold = float (current_results[1])
        outlier_threshold = int (current_results[2])
        remove_intersections = int (current_results[3])
        score_threshold = float (current_results[4])
        
        # Assertion is done for the sake of python float problems
        assert score_threshold == 0.001 or score_threshold == 0.3 or score_threshold == 0.47, "Problem with score: " + score_threshold

        # First check if the key exists in the original relationship
        if (sensor_name, distance_threshold, outlier_threshold, remove_intersections, score_threshold) in rels:
            # Add the result into the dictionary or create list then add if 
            # the key does not exists
            if (sensor_name, distance_threshold, outlier_threshold, remove_intersections, score_threshold) in results:
                results [sensor_name, distance_threshold, outlier_threshold, remove_intersections, score_threshold].append (line)
            else:
                results [sensor_name, distance_threshold, outlier_threshold, remove_intersections, score_threshold] = []
                results [sensor_name, distance_threshold, outlier_threshold, remove_intersections, score_threshold].append (line)
        else:
            print (sensor_name, distance_threshold, outlier_threshold, remove_intersections, score_threshold)
            print ("No such sensor param combination exists in the relationship file.")

# Print the results into a new file
sensor_results = open (main_directory + 'sensor_results.csv', "w")

# Write single header to avg results file
sensor_results.write (','.join (head for head in header.split ('\t')) + '\n')

for (sensor_name, distance_threshold, outlier_threshold, remove_intersections, score_threshold), value in results.items():

    # Fix the values
    fixed_list = []
    for ix in range (len (value)):
        val_str = value [ix].split ('\t')
        fixed_vals = []
        for val in val_str:
            try:
                fixed_vals.append (float (val))
            except:
                fixed_vals.append (val)
        fixed_list.append (fixed_vals)



    name = np.array (fixed_list)[:,0:1] [0].item ()
    dframe = pd.DataFrame(fixed_list)  
    means = [mean for mean in dframe.mean ()]
    result = []
    for ix, mean in enumerate (means):
        if ix == 0:
            result.append (name)
        result.append (mean)

    # Calculate average and write it to a separate file
    current_average_string = ','.join ([str(num) for num in result])

    sensor_results.write (current_average_string + '\n')

sensor_results.close ()

#################################################
### FIX dot to comma in state files
print ("Done.")