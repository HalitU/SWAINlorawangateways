#ifndef GLOBAL_VARIABLES_H
#define GLOBAL_VARIABLES_H

#include "ns3/lora-helper.h"
#include "ns3/core-module.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/trace-helper.h"
#include <chrono> 
#include <ctime>
#include <iomanip>
#include <stdlib.h>
#include <iostream>
#include <sys/stat.h>
#include <fstream>

using namespace ns3;
using namespace lorawan;
using namespace std;
using namespace std::chrono;

class GlobalVariables {
    private:

    public:

    static double PathLossExponent; // 3.76;
    static double ReferenceDistance; // 1
    static double ReferenceLoss; // 7.7

    static uint32_t nNetworkservers;

    static double simulationTime;// 15000;//72000.0;
    
    static string animFile;

    static uint32_t dataGenerationPeriod;//320; // in seconds
    static uint32_t dataMaxSize;
    static uint32_t dataGenerationRate;//52;
    static uint32_t dataCollectionTime;    

    static double distanceBetweenCoords; // in meters
    
    // Gateway
    static double HAPSRange; // basically 20k * sqrt (2)
    static uint32_t EDIdleDurationBeforeNextRangeControl;

    // Clustering parameters
    static double distance_threshold;
    static int outlier_threshold;
    static int remove_intersections;    
    static double score_threshold;

    // Common circle variables
    static bool backoff; // ALWAYS SET IT HERE BECAUSE THESE ARE GLOBAL VALUES!!!
    static double fullCircle;
    static double degToRad;
    static Ptr<OutputStreamWrapper> experimentOverlappingInterferenceDurationsFile;
    
    // Duty cycle parameters
    static int txDutyDuration; // seconds 5
    static int txDutyCycleBackoff; // seconds 135

    static int expectedNumberOfMessages;
    static double EDsupplyVoltage;
    static double EDactiveCurrent;
    static double EDawakeningCurrent;
    static double EDcollectionCurrent;
    static double EDshuttingdownCurrent;
    static double EDsleepCurrent;

    static Time EDactiveDuration;
    static Time EDawakeningDuration;
    static Time EDcollectionDuration;
    static Time EDshuttingdownDuration;
    static Time EDsleepingDuration;

    // Cloud buildings
    static int minCloudCount; // 5000;
    static int maxCloudCount; // 10000;
    static int cloudInnerCircleRadius;
    static int cloudOuterCircleRadius;// 1000;
    static int minCloudXSize;// 100;
    static int maxCloudXSize;// 500;
    static int minCloudYSize;// 100;
    static int maxCloudYSize;// 500;
    static int cloudZSize;// 10;
    static int cloudAltitude;

    //
    static bool DEBUG;
    static bool SM_DEBUG;

    // network address
    static uint8_t nwkId;
    static uint32_t nwkAddr;

    // Lorawan radio energy parameters
    static double BasicEnergySourceInitialEnergyJ;
    static double BasicEnergySupplyVoltageV;
    static double StandbyCurrentA;
    static double TxCurrentA;
    static double SleepCurrentA;
    static double RxCurrentA;

    // Tracker paths
    // void PrepareTrackerPaths ();

    GlobalVariables (void);

    // TODO: Statistical outputs
    static void CreateEssentialFolders ();

    static string experimentFileName;
    static string experimentFolder;
    static string experimentEnddeviceFolder;
    static string experimentEnddeviceTimedFolder;
    static string experimentEnddeviceDeviceStateFolder;
    static string experimentEnddeviceLorawanStateFolder;
    static string experimentEnddeviceDataRecordFolder;
    static string experimentGatewayCoordinateFile;
    static string experimentSensorCoordinateFile;

    string experimentPath = experimentFolder + "main_summary.txt";
    string improvedExperimentPath = experimentFolder + "improved_main_summary.txt";
    string phypacketgwPath = experimentFolder + "phy_packet_gw.txt";
    string energySpentOutsideLorawanPath = experimentFolder + "energy_spent_per_enddevice_except_lorawan_module.txt";
    string enddeviceColletionTransmissionPath = experimentFolder + "enddevice_collection_transmission_counts.txt";
    string enddeviceDataCollectionSummaryPath = experimentFolder + "enddevice_collection_summary.txt";
    string enddeviceLorawanModuleEnergyConsumptionPath = experimentFolder + "enddevice_lorawan_module_energy_consumption.txt";
    string experimentEnvironmentPath = experimentFolder + "experiment_environment_locs.txt";
    string averageTxInformationPath = experimentFolder + "average_tx_information.txt";
    string perSensorResultsPath = experimentFolder + "per_sensor_results.txt";

    string pythonExperimentEnvVisualizationPath = "python visualize_experiment_environment_colored.py " + experimentEnvironmentPath;

    ofstream experimentFile;
    ofstream improvedExperimentFile;  
    ofstream phypacketgwFile;
    ofstream energySpentOutsideLorawanFile;
    ofstream enddeviceColletionTransmissionFile;
    ofstream enddeviceDataCollectionSummaryFile;
    ofstream enddeviceLorawanModuleEnergyConsumptionFile;
    ofstream experimentEnvironmentFile;
    ofstream averageTxInformationFile;
    ofstream perSensorResultsFile;

    static void ParseArguments (int argc, char *argv[]);
};

#endif