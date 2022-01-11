#ifndef SENSOR_TOPOLOGY_MANAGER_H
#define SENSOR_TOPOLOGY_MANAGER_H

#include "ns3/core-module.h"
#include "ns3/node-container.h"
#include "hapnode.h"
#include <random>
#include "end-device-manager.h"
#include "ns3/mobility-helper.h"
#include "global-variables.h"
#include <vector>
#include "ns3/geographic-positions.h"

using namespace ns3;
using namespace std;

struct CoordDifference {
    public:
    double lat;
    double lon;
    double angle; // radian please
};

class RingCoord
{
    public:
    double X;
    double Y;
    double Z;
    double distance;
    double angle;
};

class SensorTopologyManager {
    private:
    public:

    MobilityHelper mobility;
    SensorLocation* swain_coordinate_reference;
    list <SensorLocation*> slList;
    static double distanceBetweenCoords;

    static double min_lat;
    static double max_lat;
    static double min_lon;
    static double max_lon;

    void CreateSensors (NodeContainer& endDevices);
    void CreateSensorsFromCityPoint (std::string name, double x, double y, double z, NodeContainer& endDevices);
    static CoordDifference* CalculateDistanceBetweenTwoCoords (SensorLocation begin, SensorLocation end);
    static double CalculateDistanceBetweenNodes (Ptr<HAPNode> gw, Ptr<HAPNode> ed);
};

#endif