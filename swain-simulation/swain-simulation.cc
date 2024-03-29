// ns-3 imports
#include "ns3/core-module.h"

#include "buildings-manager.h"
#include "network-server-manager.h"
#include "address-manager.h"
#include "gateway-manager.h"
#include "end-device-manager.h"
#include "helper-manager.h"
#include "channel-manager.h"
#include "global-variables.h"
#include "sensor-topology-manager.h"
#include "radio-energy-helper.h"
#include "statistics-helper.h"
#include "application-manager.h"

// lorawan imports
#include "ns3/node-container.h"
#include "ns3/command-line.h"

// utilities
#include <stdlib.h>
#include <chrono> 

using namespace ns3;
using namespace lorawan;
using namespace std::chrono; 
using namespace std;

NS_LOG_COMPONENT_DEFINE ("SwainSimulation");

// Main func
int 
main (int argc, char *argv[]) {
    /***********
     * Preparing files
     ***********/
    /* Create the experiment record file */
    // Create and open a text file
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y_%H-%M-%S");
    auto experimentFileName = oss.str();  

    // Static calls for file/directory hierarchy creation
    GlobalVariables::experimentFileName = experimentFileName;
    GlobalVariables::experimentFolder = "experiments/swain-simulation-results/" + GlobalVariables::experimentFileName + "/";
    GlobalVariables::experimentEnddeviceFolder = GlobalVariables::experimentFolder + "enddevice/";
    GlobalVariables::experimentEnddeviceTimedFolder = GlobalVariables::experimentFolder + "enddevice_timed/";
    GlobalVariables::experimentEnddeviceDeviceStateFolder = GlobalVariables::experimentFolder + "enddevice_device_state/";
    GlobalVariables::experimentEnddeviceLorawanStateFolder = GlobalVariables::experimentFolder + "enddevice_lorawan_state/";
    GlobalVariables::experimentEnddeviceDataRecordFolder = GlobalVariables::experimentFolder + "enddevice_data_tx_record/";
    GlobalVariables::CreateEssentialFolders ();
    
    // Parse arguments
    GlobalVariables::ParseArguments (argc, argv);

    // Fed by outside scripts
    GlobalVariables::experimentGatewayCoordinateFile = "clustering-example/output/gateways.txt";
    GlobalVariables::experimentSensorCoordinateFile = "clustering-example/output/sensors.txt";

    // Output file for keeping track of overlapping signal durations
    AsciiTraceHelper customStaticsOutputAsciiHelper;
    Ptr<OutputStreamWrapper> experimentOverlappingInterferenceDurationsFile = 
        customStaticsOutputAsciiHelper.CreateFileStream (GlobalVariables::experimentFolder + "interference_overlapping_durations.txt");
    LoraInterferenceHelper::experimentOverlappingInterferenceDurationsFile = experimentOverlappingInterferenceDurationsFile;  

    GlobalVariables* gv = new GlobalVariables ();

    StatisticsHelper sh;
    sh.gv = gv;

    SensorTopologyManager::distanceBetweenCoords = GlobalVariables::distanceBetweenCoords;

    /************************
     *  Parameters  *
     ************************/
    HelperManager hm;
    hm.InitializeSimulationRandomness ();

    // Clock for simulation timer
    auto start = high_resolution_clock::now();

    /* Creating simulation environment */

    /************************
     *  Channel  *
     ************************/
    ChannelManager cm;
    Ptr<LoraChannel> channel = cm.InitializeChannelWithLoss (GlobalVariables::PathLossExponent, GlobalVariables::ReferenceDistance, GlobalVariables::ReferenceLoss);

    /************************
     *  Create the helpers  *
     ************************/
    hm.InitializeHelpers (channel);

    /***************
     * End-devices *
     **************/
    NodeContainer endDevices;
    SensorTopologyManager stm;
    stm.mobility = hm.mobility;
    stm.CreateSensors (endDevices);

    NS_LOG_UNCOND ("Number of end-devices created is: " << endDevices.GetN());

    /************************
     * Install end-device address and device type *
     ************************/
    AddressManager adm;
    adm.InstallAddressToDevices (hm.phyHelper, hm.macHelper, hm.helper, endDevices);

    NS_LOG_UNCOND ("Address assignment completed...");

    /*********************
     *  Create Gateways  *
     *********************/
    GatewayManager gm;
    NodeContainer gateways = gm.CreateGateways (hm.mobility);  

    NS_LOG_UNCOND ("Gateways created...");

    /************************
     * Install gateway net-devices
     ************************/
    gm.InstallGatewayNetDevices (hm.phyHelper, hm.macHelper, hm.helper, gateways);

    NS_LOG_UNCOND ("Gateway net-devices are installed...");

    /**************************
     *  Create Network Server  *
     ***************************/
    NetworkServerManager nsm;
    nsm.InstallNetworkServer (GlobalVariables::nNetworkservers, hm.mobility, hm.nsHelper, endDevices, gateways, hm.forHelper);

    NS_LOG_UNCOND ("Network server installed...");

    /************************
     *  Buildings  *
     ************************/  
    // bm.CreateBuildingsFromFile (building_path);
    // string building_path = "scratch/lorawan-simulation-main/data/buildings.txt";
    // BuildingsManager bm;
    // bm.DistributeRandomClouds (stm);
    // bm.RegisterBuildingsToNodes (endDevices, gateways);

    // NS_LOG_UNCOND ("Buildings are disributed...");

    /**********************************************
     *  Set up the end device's spreading factor  *
     **********************************************/
    // hm.macHelper.SetSpreadingFactorsUp (endDevices, gateways, channel);

    NS_LOG_UNCOND ("Completed configuration, number of enddevices are: " << endDevices.GetN ());

    /*******************
     * Install radio energy model
     *******************/
    RadioEnergyHelper reh;
    reh.InstallLorawanRadioEnergy (endDevices, adm.endDevicesNetDevices);

    /*********************************************
     *  Install applications on the end devices  *
     *********************************************/    
    ApplicationManager apm;
    apm.CreateEndDeviceApplications (gateways, endDevices, &sh);

    /*********************************************
     *  Callbacks and statistics  *
     *********************************************/  
    hm.SetupEndDeviceStateTrackers (endDevices, gv);

    sh.VisualizeEnvironment (endDevices, gateways);

    ////////////////
    // Simulation //
    ////////////////
    NS_LOG_UNCOND ("Setting the simulation time...");
    Simulator::Stop (Seconds (GlobalVariables::simulationTime));

    NS_LOG_UNCOND ("Running simulation...");
    Simulator::Run ();

    // TODO: Update the energy consumption values for the reset of the simulation duration, since state might have not changed
    NS_LOG_UNCOND ("Simulation duration: " << Simulator::Now ().GetSeconds ());
    // For all sensors check the latest state and update energy consumptions accordingly
    for (NodeContainer::Iterator i = endDevices.Begin (); i != endDevices.End (); ++i)
    {
        // Get the node
        Ptr<HAPNode> node = (dynamic_cast<HAPNode *> (PeekPointer(*i)));
        // Get the final non-processed duration
        double finalDuration = Simulator::Now().GetSeconds() - node->_customDeviceEnergyModel.GetLastEnergyControlTime();
        // Get the latest state
        CustomDeviceState deviceState = node->_customDeviceEnergyModel.GetCurrentState ();
        double consumedEnergy = 0.0;
        // If it is active, update active idle time
        // if it is sleeping, update sleeping
        switch (deviceState)
        {
            case CustomDeviceState::SLEEPING:
                consumedEnergy = 
                      node->_customDeviceEnergyModel.GetSleepCurrent()
                    * node->_customDeviceEnergyModel.GetSupplyVoltage()
                    * finalDuration;
                node->_customDeviceEnergyModel._sleepConsumedEnergy += consumedEnergy;
                break;
            case CustomDeviceState::ACTIVE:
                consumedEnergy = 
                      node->_customDeviceEnergyModel.GetActiveCurrent()
                    * node->_customDeviceEnergyModel.GetSupplyVoltage()
                    * finalDuration;
                node->_customDeviceEnergyModel._activeModeConsumedEnergy += consumedEnergy;
                break;
            default:
                NS_ASSERT_MSG (false, "INVALID STATE!!!!");
                break;
        }
        // Update last energy control time
        node->_customDeviceEnergyModel._lastEnergyControlTime = Simulator::Now().GetSeconds();
        node->_customDeviceEnergyModel.UpdateConsumedEnergy (consumedEnergy);

        node->_customDeviceEnergyModel._totalProcessedEnergyTime += finalDuration;

        // Check the correctness
        NS_LOG_UNCOND ("Time spend for energy calculations: " << node->_customDeviceEnergyModel._totalProcessedEnergyTime);
    }

    // 
    sh.EDCollectionTransmissionSummary (endDevices);
    sh.EnergyCalculations (endDevices);
    sh.VisitedNodes (endDevices);
    sh.gatewayN = gateways.GetN ();
    sh.FillImprovedExperimentFile ();


    // Data tx record for the nodes which were still in range at the end of the simulation
    for (NodeContainer::Iterator i = endDevices.Begin (); i != endDevices.End (); ++i)
    {
        Ptr<HAPNode> node = (dynamic_cast<HAPNode *> (PeekPointer(*i)));
        bool recordProcess = node->update_last_out_range_time (gv->simulationTime);
        if (recordProcess == true) {        
            node->write_latest_data_tx_record ();
        }
    };

    Simulator::Destroy ();
    /*****************
     * SOME OUTPUTS
     */
    LoraPacketTracker &tracker = hm.helper.GetPacketTracker ();

    // Main result file to process
    sh.FillAverageTXInformationFile (gv, gateways, endDevices, tracker);

    // Fill the file where per-sensor outputs are given
    sh.FillSensorOutputFile (gv, gateways, endDevices, tracker);

    sh.PacketTracker (hm.helper, gateways);
    
    std::cout << "Global MAC packets (sent - received): " << 
        tracker.CountMacPacketsGlobally (Seconds (0), Seconds (GlobalVariables::simulationTime) + Hours (1)) << std::endl;
    std::cout << "PrintPhyPacketsPerGw: " << "TOTAL" << "\t" << "RECEIVED" << "\t" << "INTERFERED" << "\t" << "NO_MORE_RECEIVERS"
        << "\t" << "UNDER_SENSITIVITY" << "\t" << "LOST_BECAUSE_TX" << std::endl;
    // Per gateway
    for (NodeContainer::Iterator i = gateways.Begin (); i != gateways.End (); ++i)
    {
        Ptr<HAPNode> node = (dynamic_cast<HAPNode *> (PeekPointer(*i)));
        std::cout << "Node [" + std::to_string (node->GetId ()) << "]: " <<
            tracker.PrintPhyPacketsPerGw (Seconds (0), Seconds (GlobalVariables::simulationTime) + Hours (1), node->GetId ())
            << std::endl;
    };

    ///////////////////////////
    // Print results to file //
    ///////////////////////////
    NS_LOG_INFO ("Computing performance metrics...");

    std::cout << tracker.CountMacPacketsGlobally (Seconds (0), Seconds (GlobalVariables::simulationTime) + Hours (1)) << std::endl;

    /*********
     * Done. *
     *********/
    NS_LOG_UNCOND ("Simulation done.");

    auto stop = high_resolution_clock::now(); 
    auto duration = duration_cast<microseconds>(stop - start);

    // Run python script for visualization
    system(gv->pythonExperimentEnvVisualizationPath.c_str ());

    NS_LOG_UNCOND ("Simulation execution time (microseconds): " << duration.count ());
    NS_LOG_UNCOND ("Simulation execution time (seconds): " << (duration.count () * 1.0 / 1000000));
};