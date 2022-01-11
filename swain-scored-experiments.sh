#!/bin/bash
set -eo pipefail
# Keeping old results
DELETE_EXISTNG_EXPERIMENTS=1

# Clustering Parameters
LOOP_COUNT=1 # Set to 10 for final experiments
RIVER_AVAILABLE=1
DISTANCE_ERROR_MARGIN=100
GW_MAX_RANGE=10000
DISTANCE_THRESHOLD=500

SENSOR_TSV_FILE=clustering-example/data/sensor_locations.tsv
SENSOR_COST_AVAILABLE=1
SENSOR_COST_FILE=clustering-example/data/sensor_placements.csv

# Reset BASH time counter
SECONDS=0

# Clear existing files and recreate the subdirectory
if (($DELETE_EXISTNG_EXPERIMENTS == 1))
then
    # Logging
    echo "-----------------------------------"
    echo "Clearing existing experiment results for consistency"
    echo "-----------------------------------"

    rm -rf experiments/swain-simulation-results && mkdir experiments/swain-simulation-results
    rm -rf clustering-example/output && mkdir clustering-example/output
else
    # Logging
    echo "-----------------------------------"
    echo "Not clearing existing files could mean any existing results will be taken in account"
    echo "-----------------------------------"
fi

# Main loop
# for ((current_step=1; $DISTANCE_THRESHOLD<=$GW_MAX_RANGE; current_step++))
while [ $DISTANCE_THRESHOLD -le $GW_MAX_RANGE ]
do
    # Clustering loop
    for REMOVE_INTERSECTIONS in 0 1
    do
        for OUTLIER_THRESHOLD in 0 1 2 3
        do
            # DO NOT USE EXACT INTEGERS HERE
            for SCORE_THRESHOLD in 0.001 0.3 0.47
            do
                # Logging
                echo "-----------------------------------"
                echo "Running the clustering algorithm for gateway location decision, distance_threshold=$DISTANCE_THRESHOLD, outlier_threshold=$OUTLIER_THRESHOLD, intersections_removed=$REMOVE_INTERSECTIONS, score_threshold=$SCORE_THRESHOLD"
                echo "Currently rivers are always available for Ergene river, not available for Findland data"
                echo "-----------------------------------"

                # Running the clustering script for determining sensor and gateway locations
                python clustering-example/sensor-clustering-custom-affinity-coords.py $DISTANCE_THRESHOLD $OUTLIER_THRESHOLD $REMOVE_INTERSECTIONS $RIVER_AVAILABLE $DISTANCE_ERROR_MARGIN $SENSOR_TSV_FILE $SENSOR_COST_AVAILABLE $SENSOR_COST_FILE $SCORE_THRESHOLD

                # Read clustering validation file
                validation=$(<clustering-example/clustering_validation.txt)
                echo "validation bit is: $validation"

                if (($validation==1))
                then

                    # Logging
                    echo "-----------------------------------"
                    echo "Starting the lorawan experiments"
                    echo "-----------------------------------"

                    # Experiment loop
                    for ((repeat_count=1; repeat_count <= $LOOP_COUNT; repeat_count++))
                    do
                        # Signal range with margin
                        MARGIN_SIGNAL_RANGE=$((DISTANCE_THRESHOLD+DISTANCE_ERROR_MARGIN))

                        # Running the NS-3 simulator
                        echo "Running experiment: iteration=$repeat_count, with range=$DISTANCE_THRESHOLD"
                        ./waf --run "swain-simulation --gatewayRange=$MARGIN_SIGNAL_RANGE --distance_threshold=$DISTANCE_THRESHOLD --outlier_threshold=$OUTLIER_THRESHOLD --remove_intersections=$REMOVE_INTERSECTIONS --score_threshold=$SCORE_THRESHOLD"
                        
                        # Logging
                        ELAPSED="Current elapsed time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
                        echo $ELAPSED
                    done

                    # Logging
                    echo "One gateway loop completed, processed distance: $DISTANCE_THRESHOLD"
                    ELAPSED="Current elapsed time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
                    echo $ELAPSED

                else
                    echo "Validation bit is invalid, meaning no gateways are found, no need to run the NS-3 simulator"
                fi            
            done
        done
    done
    # Update distance
    DISTANCE_THRESHOLD=$(($DISTANCE_THRESHOLD+500))
done

# Logging
echo "-----------------------------------"
echo "Processing separate results into one cumulative file"
echo "-----------------------------------"

# Run a python script for processing results
python process_swain_per_sensor_results.py

echo "Simulations are completed"
ELAPSED="Elapsed: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
echo $ELAPSED
