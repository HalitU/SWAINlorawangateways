#include "global-variables.h"

double GlobalVariables::PathLossExponent = 2.2; // 2.2, 1.8, 2.2, 3.5 for good antenna height TODO: fixxxxxxxx 3.76, 3.3 for corner 28 km case; paper mentions 2.65, 3.1 is nice
double GlobalVariables::ReferenceDistance = 100; // 100, 100, 1000, 100 1 in meters
double GlobalVariables::ReferenceLoss = 78; // 78, 80, 91.22, 87  7.7 13.225 mentioned in the paper

uint32_t GlobalVariables::nNetworkservers = 1;

double GlobalVariables::simulationTime = 86400; // TODO: 86400;// 15000;//72000.0; 86400 seconds is one day
// int appPeriodSeconds = 600;

string GlobalVariables::animFile = "lorawan-animation.xml";

uint32_t GlobalVariables::dataGenerationPeriod = 180;//3600;//320; // in seconds
uint32_t GlobalVariables::dataMaxSize = 6000;
uint32_t GlobalVariables::dataGenerationRate = 48;//52;
uint32_t GlobalVariables::dataCollectionTime = 3;    

double GlobalVariables::distanceBetweenCoords = 111000; // in meters

// Gateway
double GlobalVariables::HAPSRange = 28284.2712475; // basically 20k * sqrt (2)
uint32_t GlobalVariables::EDIdleDurationBeforeNextRangeControl = 15;    

// Clustering parameters
double GlobalVariables::distance_threshold;
int GlobalVariables::outlier_threshold;
int GlobalVariables::remove_intersections;
double GlobalVariables::score_threshold;

// Common circle variables
bool GlobalVariables::backoff = false; // ALWAYS SET IT HERE BECAUSE THESE ARE GLOBAL VALUES!!!
double GlobalVariables::fullCircle = 2 * M_PI;
double GlobalVariables::degToRad = M_PI / 180.0;

// Duty cycle parameters
int GlobalVariables::txDutyDuration = 5; // seconds 5
int GlobalVariables::txDutyCycleBackoff = 135; // seconds 135

int GlobalVariables::expectedNumberOfMessages = 10;

double GlobalVariables::EDsupplyVoltage = 5.5;
double GlobalVariables::EDactiveCurrent = 0.1;
double GlobalVariables::EDawakeningCurrent = 0.15;
double GlobalVariables::EDcollectionCurrent = 0.12;
double GlobalVariables::EDshuttingdownCurrent = 0.08;
double GlobalVariables::EDsleepCurrent = 0.05;

Time GlobalVariables::EDactiveDuration = Seconds (10);
Time GlobalVariables::EDawakeningDuration = Seconds (11);
Time GlobalVariables::EDcollectionDuration = Seconds (3);
Time GlobalVariables::EDshuttingdownDuration = Seconds (4);
Time GlobalVariables::EDsleepingDuration = Seconds (5);

// Cloud buildings
int GlobalVariables::minCloudCount = 1; // 5000;
int GlobalVariables::maxCloudCount = 1; // 10000;
int GlobalVariables::cloudInnerCircleRadius = 1;
int GlobalVariables::cloudOuterCircleRadius = 1;// 1000;
int GlobalVariables::minCloudXSize = 1;// 100;
int GlobalVariables::maxCloudXSize = 1;// 500;
int GlobalVariables::minCloudYSize = 1;// 100;
int GlobalVariables::maxCloudYSize = 1;// 500;
int GlobalVariables::cloudZSize = 1;// 10;
int GlobalVariables::cloudAltitude = 7600;

//
bool GlobalVariables::DEBUG = false;
bool GlobalVariables::SM_DEBUG = false;

// network address
uint8_t GlobalVariables::nwkId = 54;
uint32_t GlobalVariables::nwkAddr = 1864;    

// Lorawan radio energy parameters
double GlobalVariables::BasicEnergySourceInitialEnergyJ = 10000;
double GlobalVariables::BasicEnergySupplyVoltageV = 5;
double GlobalVariables::StandbyCurrentA = 0.0014;
double GlobalVariables::TxCurrentA = 0.028;
double GlobalVariables::SleepCurrentA = 0.0000015;
double GlobalVariables::RxCurrentA = 0.0112;

Ptr<OutputStreamWrapper> GlobalVariables::experimentOverlappingInterferenceDurationsFile;

string GlobalVariables::experimentFileName;
string GlobalVariables::experimentFolder;
string GlobalVariables::experimentEnddeviceFolder;
string GlobalVariables::experimentEnddeviceTimedFolder;
string GlobalVariables::experimentEnddeviceDeviceStateFolder;
string GlobalVariables::experimentEnddeviceLorawanStateFolder;
string GlobalVariables::experimentEnddeviceDataRecordFolder;
string GlobalVariables::experimentGatewayCoordinateFile;
string GlobalVariables::experimentSensorCoordinateFile;

GlobalVariables::GlobalVariables (void) 
: experimentFile (experimentPath)
, improvedExperimentFile (improvedExperimentPath)
, phypacketgwFile (phypacketgwPath)
, energySpentOutsideLorawanFile (energySpentOutsideLorawanPath)
, enddeviceColletionTransmissionFile (enddeviceColletionTransmissionPath)
, enddeviceDataCollectionSummaryFile (enddeviceDataCollectionSummaryPath)
, enddeviceLorawanModuleEnergyConsumptionFile (enddeviceLorawanModuleEnergyConsumptionPath)
, experimentEnvironmentFile (experimentEnvironmentPath)
, averageTxInformationFile (averageTxInformationPath)
, perSensorResultsFile (perSensorResultsPath)
{
};

void GlobalVariables::CreateEssentialFolders () {
    mkdir (experimentFolder.c_str (), 0777);
    mkdir (experimentEnddeviceFolder.c_str (), 0777);
    mkdir (experimentEnddeviceTimedFolder.c_str (), 0777);
    mkdir (experimentEnddeviceDeviceStateFolder.c_str (), 0777);
    mkdir (experimentEnddeviceLorawanStateFolder.c_str (), 0777);
    mkdir (experimentEnddeviceDataRecordFolder.c_str (), 0777);
};

void GlobalVariables::ParseArguments (int argc, char *argv[])
{
    // Env opts
    uint32_t gatewayRange = 6500;

    // Cluster parameters
    double distance_threshold;
    double outlier_threshold;
    double remove_intersections;
    double score_threshold;

    // Parsing args
    CommandLine cmd;
    cmd.AddValue("gatewayRange", "default=6500", gatewayRange);
    cmd.AddValue("distance_threshold", "default=6500", distance_threshold);
    cmd.AddValue("outlier_threshold", "default=0", outlier_threshold);
    cmd.AddValue("remove_intersections", "default=0", remove_intersections);
    cmd.AddValue("score_threshold", "default=0", score_threshold);
    cmd.Parse (argc, argv);

    // Logs
    NS_LOG_UNCOND ("----------------------------------");
    NS_LOG_UNCOND ("Running with following cmd args...");
    NS_LOG_UNCOND ("Gateway range: " << gatewayRange);
    NS_LOG_UNCOND ("Clustering distance_threshold: " << distance_threshold);
    NS_LOG_UNCOND ("Clustering outlier_threshold: " << outlier_threshold);
    NS_LOG_UNCOND ("Clustering remove_intersections: " << remove_intersections);
    NS_LOG_UNCOND ("Clustering score threshold: " << score_threshold);
    NS_LOG_UNCOND ("----------------------------------");

    GlobalVariables::HAPSRange = gatewayRange;
    GlobalVariables::distance_threshold = distance_threshold;
    GlobalVariables::outlier_threshold = outlier_threshold;
    GlobalVariables::remove_intersections = remove_intersections;
    GlobalVariables::score_threshold = score_threshold;
};