#ifndef NETWORKMODEL_H
#define NETWORKMODEL_H


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/stats-module.h"
#include "ns3/wifi-module.h"
#include "../Neat/network.h"

#include <iostream>
using namespace ns3;


std::vector<double> Initialize (int nodeNum, int runNum, int runTime, bool neat, NEAT::Network *ntw);


#endif
