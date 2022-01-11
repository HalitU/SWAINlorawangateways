#include "gateway-manager.h"


NodeContainer GatewayManager::CreateGateways (MobilityHelper mobility) 
{
    // Get the gateway location file
    std::ifstream in (GlobalVariables::experimentGatewayCoordinateFile);
    std::string line;

    if (!in){
      NS_LOG_UNCOND ("File can't be opened!");
      exit(1);
    }

    // Assign a mobility model to the node
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");    

    // Read it line by line and create the gateways
    NodeContainer gateways;
    std::string gw_id;
    double lat;
    double lon;    
    double altitude;
    while (getline(in, line))
    {
        istringstream ss (line);
        ss >> gw_id >> lat >> lon >> altitude;

        Ptr<HAPNode> newNode = CreateObject<HAPNode> ();
        newNode->node_name = gw_id;
        gateways.Add (newNode);        

        mobility.Install (newNode);

        // Convert geographical coordinates into vector
        Vector crr_loc = GeographicPositions::GeographicToCartesianCoordinates (lat, lon, altitude, GeographicPositions::EarthSpheroidType::WGS84);

        Ptr<MobilityModel> node_mobility = newNode->GetObject<MobilityModel> ();
        Vector position = node_mobility->GetPosition ();
        position.x = crr_loc.x;
        position.y = crr_loc.y;
        position.z = crr_loc.z;
        node_mobility->SetPosition (position); 
    }

    return gateways;
};


void GatewayManager::InstallGatewayNetDevices (
    LoraPhyHelper phyHelper, LorawanMacHelper macHelper, LoraHelper helper, NodeContainer gateways) {
    // Create a netdevice for each gateway
    phyHelper.SetDeviceType (LoraPhyHelper::GW);
    macHelper.SetDeviceType (LorawanMacHelper::GW);
    helper.Install (phyHelper, macHelper, gateways);  
};