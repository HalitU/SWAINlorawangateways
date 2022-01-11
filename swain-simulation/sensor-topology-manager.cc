#include "sensor-topology-manager.h"

double SensorTopologyManager::distanceBetweenCoords; // = 111000;// 111000;
double SensorTopologyManager::min_lat;
double SensorTopologyManager::max_lat;
double SensorTopologyManager::min_lon;
double SensorTopologyManager::max_lon;

vector<string> split (std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    vector<std::string> res;

    while ((pos_end = s.find (delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
};


CoordDifference* SensorTopologyManager::CalculateDistanceBetweenTwoCoords (SensorLocation begin, SensorLocation end) {
    CoordDifference* cd = new CoordDifference;

    cd->lon = std::abs (begin.lon - end.lon) * distanceBetweenCoords;
    cd->lat = std::abs (begin.lat - end.lat) * distanceBetweenCoords;

    return cd;
};

double SensorTopologyManager::CalculateDistanceBetweenNodes (Ptr<HAPNode> gw, Ptr<HAPNode> ed)
{
    Ptr<ConstantPositionMobilityModel> gatewayMobilityModel = gw->GetObject<ConstantPositionMobilityModel>();
    Ptr<ConstantPositionMobilityModel> enddeviceMobilityModel = ed->GetObject<ConstantPositionMobilityModel>();

    return gatewayMobilityModel->GetDistanceFrom (enddeviceMobilityModel);
};

void SensorTopologyManager::CreateSensors (NodeContainer& endDevices) {
    // Reading the coordinate file

    // TODO: ergene havzasının sol alt köşe koordinatlarını bul
    // TODO: findout the possibility of using geographic locations in NS-3
    // TODO: converting dsm to dd in coordinate format
    // TODO: read coordinates, convert them into decimal degree format
    // TODO: calculate the distance according to an origin point (south-west corner)
    // TODO: place them into their respective locations
    // TODO: decide on where to place the gateways
    // https://www.latlong.net/degrees-minutes-seconds-to-decimal-degrees
    // for corner => 40.55, 26.20
    // TODO: handling the window scheduling will be different as well

    // Get the sensor location file
    std::ifstream in (GlobalVariables::experimentSensorCoordinateFile);
    std::string line;

    if (!in){
      NS_LOG_UNCOND ("File can't be opened!");
      exit(1);
    }

    std::string name;
    double lat;
    double lon;    
    double altitude;
    while (getline(in, line))
    {
        istringstream ss (line);
        ss >> name >> lat >> lon >> altitude;

        // NS_LOG_UNCOND (name << " " << lat << " " << lon << " " << altitude);

        // Convert geographical coordinates into vector
        Vector crr_loc = GeographicPositions::GeographicToCartesianCoordinates (lat, lon, altitude, GeographicPositions::EarthSpheroidType::WGS84);

        // Create the new Sensor coordinate struct
        SensorLocation* new_sensor = new SensorLocation;
        new_sensor->name = name;
        new_sensor->lat = crr_loc.x;
        new_sensor->lon = crr_loc.y;
        new_sensor->altitude = crr_loc.z;

        slList.push_back (new_sensor);        
    }

    // NS_ASSERT (false);

    // Create the sensors for each coordinate aside from the map reference point
    // in a sphere distribution format
    for (SensorLocation* const& iter : slList) {
        // Get the location of the coordinate according to map reference
        // NS_LOG_UNCOND ("Placing a sensor at [lat lon altitude]: " << iter->lat << " " << iter->lon << " " << iter->altitude);
        // Create the sensors from this location
        CreateSensorsFromCityPoint (iter->name, iter->lat, iter->lon, iter->altitude, endDevices);
    }
};


void SensorTopologyManager::CreateSensorsFromCityPoint (std::string name, double x, double y, double z, NodeContainer& endDevices) {
    EndDeviceManager edm;

    // Set mobility helper model to constant position
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

    // Create it
    Ptr<HAPNode> newNode = edm.CreateSingleEndDevice ();
    
    // Install its mobility model
    mobility.Install (newNode);

    // Update its coordinates
    Ptr<ConstantPositionMobilityModel> mobilityModel = newNode->GetObject<ConstantPositionMobilityModel>();
    Vector currrentPosition = mobilityModel->GetPosition();

    currrentPosition.x = x;
    currrentPosition.y = y;
    currrentPosition.z = z;
    mobilityModel->SetPosition (currrentPosition);

    // Energy
    newNode->expectedNumberOfMessages = GlobalVariables::expectedNumberOfMessages;
    newNode->currentMessageCount = 0;

    newNode->node_name = name;
    newNode->SetNodeType ("ED");
    newNode->_customDeviceEnergyModel._consumedEnergy = 0.0;
    newNode->_customDeviceEnergyModel._sleepConsumedEnergy = 0.0;
    newNode->_customDeviceEnergyModel._awakeningConsumedEnergy = 0.0;
    newNode->_customDeviceEnergyModel._activeModeConsumedEnergy = 0.0;
    newNode->_customDeviceEnergyModel._collectionModeConsumedEnergy = 0.0;
    newNode->_customDeviceEnergyModel._shutDownModeConsumedEnergy = 0.0;
    newNode->_customDeviceEnergyModel._totalTransmitted = 0;
    newNode->_customDeviceEnergyModel._totalCollected = 0;
    newNode->_customDeviceEnergyModel._totalAttempted = 0;
    newNode->_customDeviceEnergyModel._totalPacketAttempted = 0;
    newNode->_customDeviceEnergyModel._totalSentBytes = 0.0;
    newNode->_customDeviceEnergyModel._lastStateUpdateTime = 0.0;
    newNode->_customDeviceEnergyModel._lastEnergyControlTime = 0.0;
    newNode->_customDeviceEnergyModel._someUpdate = 0;
    // newNode->_customDeviceEnergyModel._currentState = CustomDeviceState::SLEEPING;
    // Need to activate all sensors from start because all of them will be within the range
    // and setting to sleeping mode will cost some initial startup time
    newNode->_customDeviceEnergyModel._currentState = CustomDeviceState::ACTIVE;

    // Setup the device energy model for the node
    // A scheduling is required to update this devices state (not the lorawan device state!)
    // lorawan state changes and the external device changes can be used together to
    // calculate the total consumed energy
    newNode->_customDeviceEnergyModel.SetSupplyVoltage(GlobalVariables::EDsupplyVoltage);
    newNode->_customDeviceEnergyModel.SetActiveCurrent (GlobalVariables::EDactiveCurrent);
    newNode->_customDeviceEnergyModel.SetAwakeningCurrent (GlobalVariables::EDawakeningCurrent);
    newNode->_customDeviceEnergyModel.SetCollectingCurrent (GlobalVariables::EDcollectionCurrent);
    newNode->_customDeviceEnergyModel.SetShuttingDownCurrent (GlobalVariables::EDshuttingdownCurrent);
    newNode->_customDeviceEnergyModel.SetSleepCurrent (GlobalVariables::EDsleepCurrent);
    
    newNode->_customDeviceEnergyModel.SetActiveTime (GlobalVariables::EDactiveDuration);
    newNode->_customDeviceEnergyModel.SetAwakeningTime (GlobalVariables::EDawakeningDuration);
    newNode->_customDeviceEnergyModel.SetCollectingTime (GlobalVariables::EDcollectionDuration);
    newNode->_customDeviceEnergyModel.SetShuttingDownTime (GlobalVariables::EDshuttingdownDuration);
    newNode->_customDeviceEnergyModel.SetSleepTime (GlobalVariables::EDsleepingDuration);     

    // Add it to the container
    endDevices.Add (newNode);
};