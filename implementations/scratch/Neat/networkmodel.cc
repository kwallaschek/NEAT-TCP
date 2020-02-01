/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 University of Washington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

//
// This program configures a grid (default 5x5) of nodes on an
// 802.11b physical layer, with
// 802.11b NICs in adhoc mode, and by default, sends one packet of 1000
// (application) bytes to node 1.
//
// The default layout is like this, on a 2-D grid.
//
// n20  n21  n22  n23  n24
// n15  n16  n17  n18  n19
// n10  n11  n12  n13  n14
// n5   n6   n7   n8   n9
// n0   n1   n2   n3   n4
//
// the layout is affected by the parameters given to GridPositionAllocator;
// by default, GridWidth is 5 and numNodes is 25..
//
// There are a number of command-line options available to control
// the default behavior.  The list of available command-line options
// can be listed with the following command:
// ./waf --run "wifi-simple-adhoc-grid --help"
//
// Note that all ns-3 attributes (not just the ones exposed in the below
// script) can be changed at command line; see the ns-3 documentation.
//
// For instance, for this configuration, the physical layer will
// stop successfully receiving packets when distance increases beyond
// the default of 500m.
// To see this effect, try running:
//
// ./waf --run "wifi-simple-adhoc --distance=500"
// ./waf --run "wifi-simple-adhoc --distance=1000"
// ./waf --run "wifi-simple-adhoc --distance=1500"
//
// The source node and sink node can be changed like this:
//
// ./waf --run "wifi-simple-adhoc --sourceNode=20 --sinkNode=10"
//
// This script can also be helpful to put the Wifi layer into verbose
// logging mode; this command will turn on all wifi logging:
//
// ./waf --run "wifi-simple-adhoc-grid --verbose=1"
//
// By default, trace file writing is off-- to enable it, try:
// ./waf --run "wifi-simple-adhoc-grid --tracing=1"
//
// When you are done tracing, you will notice many pcap trace files
// in your directory.  If you have tcpdump installed, you can try this:
//
// tcpdump -r wifi-simple-adhoc-grid-0-0.pcap -nn -tt
//

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/energy-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/applications-module.h"
#include "ns3/tcp-header.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-classifier.h"
#include "ns3/flow-probe.h"
#include "ns3/histogram.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/ipv4-flow-probe.h"
#include "../Neat/network.h"



#include <iostream>
#include <fstream>
#include <vector>
#include <string>

NS_LOG_COMPONENT_DEFINE ("AdhocGridTcp");

using namespace ns3;

class SimpleSource : public Application
{
public:

  SimpleSource ();
  virtual ~SimpleSource();

  /**
   * Register this type.
   * \return The TypeId.
   */
  static TypeId GetTypeId (void);

  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
};

SimpleSource::SimpleSource ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_nPackets (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0)
{
}

SimpleSource::~SimpleSource()
{
  m_socket = 0;
}

/* static */
TypeId
SimpleSource::GetTypeId (void)
{
  static TypeId tid = TypeId ("SimpleSource")
    .SetParent<Application> ()
    .SetGroupName ("Stats")
    .AddConstructor<SimpleSource> ()
    ;
  return tid;
}

void
SimpleSource::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
SimpleSource::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void
SimpleSource::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void
SimpleSource::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}

void
SimpleSource::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &SimpleSource::SendPacket, this);
    }
}


Ptr<PacketSink> sink;
Ptr<PacketSink> sink1;
Ptr<PacketSink> sink2;
Ptr<PacketSink> sink3;
Ptr<PacketSink> sink4;
Ptr<PacketSink> sink5;
Ptr<PacketSink> sink6;
Ptr<PacketSink> sink7;
Ptr<PacketSink> sink8;
Ptr<PacketSink> sink9;
Ptr<PacketSink> sink10;
Ptr<PacketSink> sink11;


int nodeNums;
double totalThroughput;
double meanThroughput;
void
CalculateTotalThroughput ()
{
  double sum = 0.0;
  if (nodeNums == 6){
    sum = sink->GetTotalRx () + sink1->GetTotalRx () + sink2->GetTotalRx () + sink3->GetTotalRx () + sink4->GetTotalRx () 
                + sink5->GetTotalRx () + sink6->GetTotalRx () + sink7->GetTotalRx () + sink8->GetTotalRx () + sink9->GetTotalRx () 
                + sink10->GetTotalRx () + sink11->GetTotalRx ();
  }
  else if (nodeNums == 5){
    sum = sink->GetTotalRx () + sink1->GetTotalRx () + sink2->GetTotalRx () + sink3->GetTotalRx () + sink4->GetTotalRx () 
                + sink5->GetTotalRx () + sink6->GetTotalRx () + sink7->GetTotalRx () + sink8->GetTotalRx () + sink9->GetTotalRx ();
  }
  else if (nodeNums == 3){
    sum = sink->GetTotalRx () + sink1->GetTotalRx () + sink2->GetTotalRx () + sink3->GetTotalRx () + sink4->GetTotalRx () 
                 + sink9->GetTotalRx ();
  }
  Time now = Simulator::Now ();
  totalThroughput = (sum*8)/((now.GetSeconds ()-22)*1024);
  meanThroughput = ((sum*8)/((now.GetSeconds ()-22)*1024))/(nodeNums*2);
}

double runTimer;
void
TrackSimulatorProgress () 
{
  int barWidth = 70;
  Time now = Simulator::Now ();
  float progress = now.GetSeconds ()/runTimer ;
  std::cout.flush();
    std::cout << "[";

    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
  std::cout << '\r';
  
    std::cout.flush();

  Simulator::Schedule (Seconds (1), &TrackSimulatorProgress);
}


std::vector<double> Initialize (int nodeNum, int runNum, int runTime, bool neat, NEAT::Network *ntw)
{
  RngSeedManager::SetRun (runNum);
  runTimer = runTime;
  nodeNums = nodeNum;
  std::string phyMode ("DsssRate1Mbps");
  double distance = 500;  // m  
  uint32_t numNodes = nodeNum*nodeNum;
  bool verbose = false;
  bool tracing = false;
  
 // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  // turn on RTS/CTS for frames over 100 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("100"));
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));
  
// Queue
  Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (26800));
  Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (26800));
  Config::SetDefault ("ns3::TcpSocketBase::Sack", BooleanValue (true));

  NodeContainer c;
  c.Create (numNodes);

  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;
  if (verbose)
    {
     // wifi.EnableLogComponents ();  // Turn on all Wifi logging
    }
  
  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  // set it to zero; otherwise, gain will be added
  wifiPhy.Set ("RxGain", DoubleValue (-10) );
  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add an upper mac and disable rate control
  NqosWifiMacHelper wifiMac;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));
  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, c);

  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (distance),
                                 "DeltaY", DoubleValue (distance),
                                 "GridWidth", UintegerValue (5),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (c);


  // Enable OLSR
  OlsrHelper olsr;
  Ipv4StaticRoutingHelper staticRouting;

  Ipv4ListRoutingHelper list;
  list.Add (staticRouting, 0);
  list.Add (olsr, 10);

  InternetStackHelper internet;
  internet.SetRoutingHelper (list); // has effect on the next Install ()
  internet.Install (c);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);

/** Flow config for 9 Nodes **/
if (nodeNum == 3){
  // TCP
  uint16_t port = 9 ;
  // Receiver Config
  // Sink Node 0
  Address sinkLocalAddress(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
  ApplicationContainer sinkApp = sinkHelper.Install (c.Get (0));
  sink = StaticCast<PacketSink> (sinkApp.Get (0));

  Address sinkLocalAddress9(InetSocketAddress (Ipv4Address::GetAny (), 10));
  PacketSinkHelper sinkHelper9 ("ns3::TcpSocketFactory", sinkLocalAddress9);
  ApplicationContainer sinkApp9 = sinkHelper9.Install (c.Get (0));
  sink9 = StaticCast<PacketSink> (sinkApp9.Get (0));
  // Sink Node 1
  Address sinkLocalAddress1(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper1 ("ns3::TcpSocketFactory", sinkLocalAddress1);
  ApplicationContainer sinkApp1 = sinkHelper1.Install (c.Get (1));
  sink1 = StaticCast<PacketSink> (sinkApp1.Get (0));
  // Sink Node 2
  Address sinkLocalAddress2(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper2 ("ns3::TcpSocketFactory", sinkLocalAddress2);
  ApplicationContainer sinkApp2 = sinkHelper2.Install (c.Get (2));
  sink2 = StaticCast<PacketSink> (sinkApp2.Get (0));
  // Sink Node 3
  Address sinkLocalAddress3(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper3 ("ns3::TcpSocketFactory", sinkLocalAddress3);
  ApplicationContainer sinkApp3 = sinkHelper3.Install (c.Get (3));
  sink3 = StaticCast<PacketSink> (sinkApp3.Get (0));
  // Sink Node 6
  Address sinkLocalAddress4(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper4 ("ns3::TcpSocketFactory", sinkLocalAddress4);
  ApplicationContainer sinkApp4 = sinkHelper4.Install (c.Get (6));
  sink4 = StaticCast<PacketSink> (sinkApp4.Get (0));
  // Sender Config
  // 6 -> 0
  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (c.Get (6), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app = CreateObject<SimpleSource> ();
  app->Setup (ns3TcpSocket, InetSocketAddress (i.GetAddress (0), port), 1540, 1000000, DataRate ("1MBps"));
  c.Get (6)->AddApplication (app);
  app->SetStartTime (Seconds (1.));
  // 2 -> 0
  Ptr<Socket> ns3TcpSocket1 = Socket::CreateSocket (c.Get (2), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app1 = CreateObject<SimpleSource> ();
  app1->Setup (ns3TcpSocket1, InetSocketAddress (i.GetAddress (0), 10), 1540, 1000000, DataRate ("1MBps"));
  c.Get (2)->AddApplication (app1);
  app1->SetStartTime (Seconds (1.));
  // 7 -> 1
  Ptr<Socket> ns3TcpSocket2 = Socket::CreateSocket (c.Get (7), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app2 = CreateObject<SimpleSource> ();
  app2->Setup (ns3TcpSocket2, InetSocketAddress (i.GetAddress (1), port), 1540, 1000000, DataRate ("1MBps"));
  c.Get (7)->AddApplication (app2);
  app2->SetStartTime (Seconds (1.));
  // 8 -> 2
  Ptr<Socket> ns3TcpSocket3 = Socket::CreateSocket (c.Get (8), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app3 = CreateObject<SimpleSource> ();
  app3->Setup (ns3TcpSocket3, InetSocketAddress (i.GetAddress (2), port), 1540, 1000000, DataRate ("1MBps"));
  c.Get (8)->AddApplication (app3);
  app3->SetStartTime (Seconds (1.));
  // 8 -> 6
  Ptr<Socket> ns3TcpSocket4 = Socket::CreateSocket (c.Get (8), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app4 = CreateObject<SimpleSource> ();
  app4->Setup (ns3TcpSocket4, InetSocketAddress (i.GetAddress (6), port), 1540, 1000000, DataRate ("1MBps"));
  c.Get (8)->AddApplication (app4);
  app4->SetStartTime (Seconds (1.));
  // 5-> 3
  Ptr<Socket> ns3TcpSocket5 = Socket::CreateSocket (c.Get (5), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app5 = CreateObject<SimpleSource> ();
  app5->Setup (ns3TcpSocket5, InetSocketAddress (i.GetAddress (3), port), 1540, 1000000, DataRate ("1MBps"));
  c.Get (5)->AddApplication (app5);
  app5->SetStartTime (Seconds (1.));
  // Start applications
  sinkApp.Start (Seconds (0.0));
  sinkApp1.Start (Seconds (0.0));
  sinkApp2.Start (Seconds (0.0));
  sinkApp3.Start (Seconds (0.0));
  sinkApp4.Start (Seconds (0.0));
  sinkApp9.Start (Seconds (0.0));
  // Set NEAT NN if enabled
  if (neat){
    ns3TcpSocket->SetNeat(ntw);
    ns3TcpSocket1->SetNeat(ntw);
    ns3TcpSocket2->SetNeat(ntw);
    ns3TcpSocket3->SetNeat(ntw);
    ns3TcpSocket4->SetNeat(ntw);
    ns3TcpSocket5->SetNeat(ntw);
  }

}
/** Flow config for 25 nodes **/
 else if (nodeNum == 5){
  // TCP
  uint16_t port = 9 ;
  // Receiver Config
  // Sink Node 0
  Address sinkLocalAddress(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
  ApplicationContainer sinkApp = sinkHelper.Install (c.Get (0));
  sink = StaticCast<PacketSink> (sinkApp.Get (0));

  Address sinkLocalAddress9(InetSocketAddress (Ipv4Address::GetAny (), 10));
  PacketSinkHelper sinkHelper9 ("ns3::TcpSocketFactory", sinkLocalAddress9);
  ApplicationContainer sinkApp9 = sinkHelper9.Install (c.Get (0));
  sink9 = StaticCast<PacketSink> (sinkApp9.Get (0));
  // Sink Node 1
  Address sinkLocalAddress1(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper1 ("ns3::TcpSocketFactory", sinkLocalAddress1);
  ApplicationContainer sinkApp1 = sinkHelper1.Install (c.Get (1));
  sink1 = StaticCast<PacketSink> (sinkApp1.Get (0));
  // Sink Node 2
  Address sinkLocalAddress2(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper2 ("ns3::TcpSocketFactory", sinkLocalAddress2);
  ApplicationContainer sinkApp2 = sinkHelper2.Install (c.Get (2));
  sink2 = StaticCast<PacketSink> (sinkApp2.Get (0));
  // Sink Node 3
  Address sinkLocalAddress3(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper3 ("ns3::TcpSocketFactory", sinkLocalAddress3);
  ApplicationContainer sinkApp3 = sinkHelper3.Install (c.Get (3));
  sink3 = StaticCast<PacketSink> (sinkApp3.Get (0));
  // Sink Node 4
  Address sinkLocalAddress4(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper4 ("ns3::TcpSocketFactory", sinkLocalAddress4);
  ApplicationContainer sinkApp4 = sinkHelper4.Install (c.Get (4));
  sink4 = StaticCast<PacketSink> (sinkApp4.Get (0));
  // Sink Node 5
  Address sinkLocalAddress5(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper5 ("ns3::TcpSocketFactory", sinkLocalAddress5);
  ApplicationContainer sinkApp5 = sinkHelper5.Install (c.Get (5));
  sink5 = StaticCast<PacketSink> (sinkApp5.Get (0));
  // Sink Node 10
  Address sinkLocalAddress6(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper6 ("ns3::TcpSocketFactory", sinkLocalAddress6);
  ApplicationContainer sinkApp6 = sinkHelper6.Install (c.Get (10));
  sink6 = StaticCast<PacketSink> (sinkApp6.Get (0));
  // Sink Node 15
  Address sinkLocalAddress7(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper7 ("ns3::TcpSocketFactory", sinkLocalAddress7);
  ApplicationContainer sinkApp7 = sinkHelper7.Install (c.Get (15));
  sink7 = StaticCast<PacketSink> (sinkApp7.Get (0));
  // Sink Node 20
  Address sinkLocalAddress8(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper8 ("ns3::TcpSocketFactory", sinkLocalAddress8);
  ApplicationContainer sinkApp8 = sinkHelper8.Install (c.Get (20));
  sink8 = StaticCast<PacketSink> (sinkApp8.Get (0));
  // Sender Config
  // 20 -> 0
  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (c.Get (20), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app = CreateObject<SimpleSource> ();
  app->Setup (ns3TcpSocket, InetSocketAddress (i.GetAddress (0), port), 1540, 1000000, DataRate ("1MBps"));
  c.Get (20)->AddApplication (app);
  app->SetStartTime (Seconds (1.));
  // 4 -> 0
  Ptr<Socket> ns3TcpSocket2 = Socket::CreateSocket (c.Get (4), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app2 = CreateObject<SimpleSource> ();
  app2->Setup (ns3TcpSocket2, InetSocketAddress (i.GetAddress (0), 10), 1540, 1000000, DataRate ("1MBps"));
  c.Get (4)->AddApplication (app2);
  app2->SetStartTime (Seconds (1.));
  // 9 -> 5
  Ptr<Socket> ns3TcpSocket3 = Socket::CreateSocket (c.Get (9), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app3 = CreateObject<SimpleSource> ();
  app3->Setup (ns3TcpSocket3, InetSocketAddress (i.GetAddress (5), port), 1540, 1000000, DataRate ("1MBps"));
  c.Get (9)->AddApplication (app3);
  app3->SetStartTime (Seconds (1.));
  // 14 -> 10
  Ptr<Socket> ns3TcpSocket4 = Socket::CreateSocket (c.Get (14), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app4 = CreateObject<SimpleSource> ();
  app4->Setup (ns3TcpSocket4, InetSocketAddress (i.GetAddress (10), port), 1540, 1000000, DataRate ("1MBps"));
  c.Get (14)->AddApplication (app4);
  app4->SetStartTime (Seconds (1.));
  // 19 -> 15
  Ptr<Socket> ns3TcpSocket5 = Socket::CreateSocket (c.Get (19), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app5 = CreateObject<SimpleSource> ();
  app5->Setup (ns3TcpSocket5, InetSocketAddress (i.GetAddress (15), port), 1540, 1000000, DataRate ("1MBps"));
  c.Get (19)->AddApplication (app5);
  app5->SetStartTime (Seconds (1.));
 // 24 -> 20
  Ptr<Socket> ns3TcpSocket6 = Socket::CreateSocket (c.Get (24), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app6 = CreateObject<SimpleSource> ();
  app6->Setup (ns3TcpSocket6, InetSocketAddress (i.GetAddress (20), port), 1540, 1000000, DataRate ("1MBps"));
  c.Get (24)->AddApplication (app6);
  app6->SetStartTime (Seconds (1.));
  // 21 -> 1
  Ptr<Socket> ns3TcpSocket7 = Socket::CreateSocket (c.Get (21), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app7 = CreateObject<SimpleSource> ();
  app7->Setup (ns3TcpSocket7, InetSocketAddress (i.GetAddress (1), port), 1540, 1000000, DataRate ("1MBps"));
  c.Get (21)->AddApplication (app7);
  app7->SetStartTime (Seconds (1.));
  // 22 -> 2
  Ptr<Socket> ns3TcpSocket8 = Socket::CreateSocket (c.Get (22), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app8 = CreateObject<SimpleSource> ();
  app8->Setup (ns3TcpSocket8, InetSocketAddress (i.GetAddress (2), port), 1540, 1000000, DataRate ("1MBps"));
  c.Get (22)->AddApplication (app8);
  app8->SetStartTime (Seconds (1.));
  // 23 -> 3
  Ptr<Socket> ns3TcpSocket9 = Socket::CreateSocket (c.Get (23), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app9 = CreateObject<SimpleSource> ();
  app9->Setup (ns3TcpSocket9, InetSocketAddress (i.GetAddress (3), port), 1540, 1000000, DataRate ("1MBps"));
  c.Get (23)->AddApplication (app9);
  app9->SetStartTime (Seconds (1.));
  // 24 -> 4
  Ptr<Socket> ns3TcpSocket10 = Socket::CreateSocket (c.Get (24), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app10 = CreateObject<SimpleSource> ();
  app10->Setup (ns3TcpSocket10, InetSocketAddress (i.GetAddress (4), port), 1540, 1000000, DataRate ("1MBps"));
  c.Get (24)->AddApplication (app10);
  app10->SetStartTime (Seconds (1.));
  // Start applications
  sinkApp.Start (Seconds (0.0));
  sinkApp1.Start (Seconds (0.0));
  sinkApp2.Start (Seconds (0.0));
  sinkApp3.Start (Seconds (0.0));
  sinkApp4.Start (Seconds (0.0));
  sinkApp5.Start (Seconds (0.0));
  sinkApp6.Start (Seconds (0.0));
  sinkApp7.Start (Seconds (0.0));
  sinkApp8.Start (Seconds (0.0));
  sinkApp9.Start (Seconds (0.0));
  // Set NEAT NN if enabled
  if (neat){
    
    ns3TcpSocket->SetNeat(ntw);
    ns3TcpSocket2->SetNeat(ntw);
    ns3TcpSocket3->SetNeat(ntw);
    ns3TcpSocket4->SetNeat(ntw);
    ns3TcpSocket5->SetNeat(ntw);
    ns3TcpSocket6->SetNeat(ntw);
    ns3TcpSocket7->SetNeat(ntw);
    ns3TcpSocket8->SetNeat(ntw);
    ns3TcpSocket9->SetNeat(ntw);
    ns3TcpSocket10->SetNeat(ntw);
  }
}

/** Flow config for 36 nodes **/
 else if (nodeNum == 6){
  // TCP
  uint16_t port = 9 ;
  // Receiver Config
  // Sink Node 0
  Address sinkLocalAddress(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
  ApplicationContainer sinkApp = sinkHelper.Install (c.Get (0));
  sink = StaticCast<PacketSink> (sinkApp.Get (0));

  Address sinkLocalAddress9(InetSocketAddress (Ipv4Address::GetAny (), 10));
  PacketSinkHelper sinkHelper9 ("ns3::TcpSocketFactory", sinkLocalAddress9);
  ApplicationContainer sinkApp9 = sinkHelper9.Install (c.Get (0));
  sink9 = StaticCast<PacketSink> (sinkApp9.Get (0));
  // Sink Node 1
  Address sinkLocalAddress1(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper1 ("ns3::TcpSocketFactory", sinkLocalAddress1);
  ApplicationContainer sinkApp1 = sinkHelper1.Install (c.Get (1));
  sink1 = StaticCast<PacketSink> (sinkApp1.Get (0));
  // Sink Node 2
  Address sinkLocalAddress2(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper2 ("ns3::TcpSocketFactory", sinkLocalAddress2);
  ApplicationContainer sinkApp2 = sinkHelper2.Install (c.Get (2));
  sink2 = StaticCast<PacketSink> (sinkApp2.Get (0));
  // Sink Node 3
  Address sinkLocalAddress3(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper3 ("ns3::TcpSocketFactory", sinkLocalAddress3);
  ApplicationContainer sinkApp3 = sinkHelper3.Install (c.Get (3));
  sink3 = StaticCast<PacketSink> (sinkApp3.Get (0));
  // Sink Node 4
  Address sinkLocalAddress4(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper4 ("ns3::TcpSocketFactory", sinkLocalAddress4);
  ApplicationContainer sinkApp4 = sinkHelper4.Install (c.Get (4));
  sink4 = StaticCast<PacketSink> (sinkApp4.Get (0));
  // Sink Node 5
  Address sinkLocalAddress5(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper5 ("ns3::TcpSocketFactory", sinkLocalAddress5);
  ApplicationContainer sinkApp5 = sinkHelper5.Install (c.Get (5));
  sink5 = StaticCast<PacketSink> (sinkApp5.Get (0));
  // Sink Node 6
  Address sinkLocalAddress6(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper6 ("ns3::TcpSocketFactory", sinkLocalAddress6);
  ApplicationContainer sinkApp6 = sinkHelper6.Install (c.Get (6));
  sink6 = StaticCast<PacketSink> (sinkApp6.Get (0));
  // Sink Node 12
  Address sinkLocalAddress7(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper7 ("ns3::TcpSocketFactory", sinkLocalAddress7);
  ApplicationContainer sinkApp7 = sinkHelper7.Install (c.Get (12));
  sink7 = StaticCast<PacketSink> (sinkApp7.Get (0));
  // Sink Node 18
  Address sinkLocalAddress8(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper8 ("ns3::TcpSocketFactory", sinkLocalAddress8);
  ApplicationContainer sinkApp8 = sinkHelper8.Install (c.Get (18));
  sink8 = StaticCast<PacketSink> (sinkApp8.Get (0));
  // Sink Node 24
  Address sinkLocalAddress10(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper10 ("ns3::TcpSocketFactory", sinkLocalAddress10);
  ApplicationContainer sinkApp10 = sinkHelper10.Install (c.Get (24));
  sink10 = StaticCast<PacketSink> (sinkApp10.Get (0));
  // Sink Node 30
  Address sinkLocalAddress11(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper11 ("ns3::TcpSocketFactory", sinkLocalAddress11);
  ApplicationContainer sinkApp11 = sinkHelper11.Install (c.Get (30));
  sink11 = StaticCast<PacketSink> (sinkApp11.Get (0));
  // Sender Config
  // 30 -> 0
  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (c.Get (30), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app = CreateObject<SimpleSource> ();
  app->Setup (ns3TcpSocket, InetSocketAddress (i.GetAddress (0), port), 1540, 1000000, DataRate ("1MBps"));
  c.Get (30)->AddApplication (app);
  app->SetStartTime (Seconds (1.));
  // 5 -> 0
  Ptr<Socket> ns3TcpSocket2 = Socket::CreateSocket (c.Get (5), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app2 = CreateObject<SimpleSource> ();
  app2->Setup (ns3TcpSocket2, InetSocketAddress (i.GetAddress (0), 10), 1540, 1000000, DataRate ("1MBps"));
  c.Get (5)->AddApplication (app2);
  app2->SetStartTime (Seconds (1.));
  // 9 -> 5
  Ptr<Socket> ns3TcpSocket3 = Socket::CreateSocket (c.Get (9), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app3 = CreateObject<SimpleSource> ();
  app3->Setup (ns3TcpSocket3, InetSocketAddress (i.GetAddress (5), port), 1540, 1000000, DataRate ("1MBps"));
  c.Get (9)->AddApplication (app3);
  app3->SetStartTime (Seconds (1.));
  // 11 -> 6
  Ptr<Socket> ns3TcpSocket4 = Socket::CreateSocket (c.Get (11), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app4 = CreateObject<SimpleSource> ();
  app4->Setup (ns3TcpSocket4, InetSocketAddress (i.GetAddress (6), port), 1540, 1000000, DataRate ("1MBps"));
  c.Get (11)->AddApplication (app4);
  app4->SetStartTime (Seconds (1.));
  // 17 -> 12
  Ptr<Socket> ns3TcpSocket5 = Socket::CreateSocket (c.Get (17), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app5 = CreateObject<SimpleSource> ();
  app5->Setup (ns3TcpSocket5, InetSocketAddress (i.GetAddress (12), port), 1540, 1000000, DataRate ("1MBps"));
  c.Get (17)->AddApplication (app5);
  app5->SetStartTime (Seconds (1.));
  // 23 -> 18
  Ptr<Socket> ns3TcpSocket6 = Socket::CreateSocket (c.Get (23), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app6 = CreateObject<SimpleSource> ();
  app6->Setup (ns3TcpSocket6, InetSocketAddress (i.GetAddress (18), port), 1540, 1000000, DataRate ("1MBps"));
  c.Get (23)->AddApplication (app6);
  app6->SetStartTime (Seconds (1.));
  // 29 -> 24
  Ptr<Socket> ns3TcpSocket7 = Socket::CreateSocket (c.Get (29), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app7 = CreateObject<SimpleSource> ();
  app7->Setup (ns3TcpSocket7, InetSocketAddress (i.GetAddress (24), port), 1540, 1000000, DataRate ("1MBps"));
  c.Get (29)->AddApplication (app7);
  app7->SetStartTime (Seconds (1.));
  // 35 -> 30
  Ptr<Socket> ns3TcpSocket8 = Socket::CreateSocket (c.Get (35), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app8 = CreateObject<SimpleSource> ();
  app8->Setup (ns3TcpSocket8, InetSocketAddress (i.GetAddress (30), port), 1540, 1000000, DataRate ("1MBps"));
  c.Get (35)->AddApplication (app8);
  app8->SetStartTime (Seconds (1.));
  // 31 -> 1
  Ptr<Socket> ns3TcpSocket9 = Socket::CreateSocket (c.Get (31), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app9 = CreateObject<SimpleSource> ();
  app9->Setup (ns3TcpSocket9, InetSocketAddress (i.GetAddress (1), port), 1540, 1000000, DataRate ("1MBps"));
  c.Get (31)->AddApplication (app9);
  app9->SetStartTime (Seconds (1.));
  // 32 -> 2
  Ptr<Socket> ns3TcpSocket10 = Socket::CreateSocket (c.Get (32), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app10 = CreateObject<SimpleSource> ();
  app10->Setup (ns3TcpSocket10, InetSocketAddress (i.GetAddress (2), port), 1540, 1000000, DataRate ("1MBps"));
  c.Get (32)->AddApplication (app10);
  app10->SetStartTime (Seconds (1.));
  // 33 -> 3
  Ptr<Socket> ns3TcpSocket11 = Socket::CreateSocket (c.Get (33), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app11 = CreateObject<SimpleSource> ();
  app11->Setup (ns3TcpSocket11, InetSocketAddress (i.GetAddress (3), port), 1540, 1000000, DataRate ("1MBps"));
  c.Get (33)->AddApplication (app11);
  app11->SetStartTime (Seconds (1.));
  // 34 -> 4
  Ptr<Socket> ns3TcpSocket12 = Socket::CreateSocket (c.Get (34), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app12 = CreateObject<SimpleSource> ();
  app12->Setup (ns3TcpSocket12, InetSocketAddress (i.GetAddress (4), port), 1540, 100000, DataRate ("1MBps"));
  c.Get (34)->AddApplication (app12);
  app12->SetStartTime (Seconds (1.));
  // 35 -> 5
  Ptr<Socket> ns3TcpSocket13 = Socket::CreateSocket (c.Get (35), TcpSocketFactory::GetTypeId ());
  Ptr<SimpleSource> app13 = CreateObject<SimpleSource> ();
  app13->Setup (ns3TcpSocket13, InetSocketAddress (i.GetAddress (5), port), 1540, 1000000, DataRate ("1MBps"));
  c.Get (35)->AddApplication (app13);
  app13->SetStartTime (Seconds (1.));
  // Start applications
  sinkApp.Start (Seconds (0.0));
  sinkApp1.Start (Seconds (0.0));
  sinkApp2.Start (Seconds (0.0));
  sinkApp3.Start (Seconds (0.0));
  sinkApp4.Start (Seconds (0.0));
  sinkApp5.Start (Seconds (0.0));
  sinkApp6.Start (Seconds (0.0));
  sinkApp7.Start (Seconds (0.0));
  sinkApp8.Start (Seconds (0.0));
  sinkApp9.Start (Seconds (0.0));
  sinkApp10.Start (Seconds (0.0));
  sinkApp11.Start (Seconds (0.0));
  // Set NEAT NN if enabled
  if (neat){
    ns3TcpSocket->SetNeat(ntw);
    ns3TcpSocket2->SetNeat(ntw);
    ns3TcpSocket3->SetNeat(ntw);
    ns3TcpSocket4->SetNeat(ntw);
    ns3TcpSocket5->SetNeat(ntw);
    ns3TcpSocket6->SetNeat(ntw);
    ns3TcpSocket7->SetNeat(ntw);
    ns3TcpSocket8->SetNeat(ntw);
    ns3TcpSocket9->SetNeat(ntw);
    ns3TcpSocket10->SetNeat(ntw);
    ns3TcpSocket11->SetNeat(ntw);
    ns3TcpSocket12->SetNeat(ntw);

  }
}

  Simulator::Schedule (Seconds (runTime-0.1), &CalculateTotalThroughput);
  Simulator::Schedule (Seconds (0.9), &TrackSimulatorProgress);

  // Flow Monitor
  Ptr<FlowMonitor> monitor;
           FlowMonitorHelper flowmon_helper;
 
           monitor = flowmon_helper.InstallAll();

  if (tracing == true)
    {
      AsciiTraceHelper ascii;
      wifiPhy.EnableAsciiAll (ascii.CreateFileStream ("wifi-simple-adhoc-grid.tr"));
      wifiPhy.EnablePcap ("wifi-simple-adhoc-grid", devices);
      // Trace routing tables
      Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("wifi-simple-adhoc-grid.routes", std::ios::out);
      olsr.PrintRoutingTableAllEvery (Seconds (2), routingStream);
    }

  Simulator::Stop (Seconds (runTime));
  Simulator::Run ();
NS_LOG_UNCOND("\nDone");
monitor->SerializeToXmlFile ("3x3fitness/PktAck/results.xml" , true, true );
// Print per flow statistics
    monitor->CheckForLostPackets ();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon_helper.GetClassifier ());

    std::map< FlowId, FlowMonitor::FlowStats > stats = monitor->GetFlowStats ();


double Thrput=0.0;
double transmit_packets=0.0;
double recieve_packets=0.0;
double delaySum = 0.0;
double lostPackets= 0.0;
   
for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter)
      {
 Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);
 //NS_LOG_UNCOND("Flow ID: " << iter->first << " Src Addr " << t.sourceAddress << " Dst Addr " << t.destinationAddress);
 //NS_LOG_UNCOND("Tx Packets = " << iter->second.txPackets);
 transmit_packets=transmit_packets+iter->second.txPackets;
//std::cout<<"Tx Packets = " <<transmit_packets;

   //     NS_LOG_UNCOND("Rx Packets = " << iter->second.rxPackets);
recieve_packets=recieve_packets+iter->second.rxPackets;
delaySum+=iter->second.delaySum.GetSeconds();
lostPackets+=iter->second.lostPackets;
//std::cout<<"\t Rx Packets = " <<recieve_packets;

     //   NS_LOG_UNCOND("Throughput="<<iter->second.rxBytes * 8.0 /(iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds())/ 1024<<"Kbps");
   Thrput+=(iter->second.rxBytes*8/(iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstRxPacket.GetSeconds()))/1024; 
      }
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
  // Calculate Jain's Fairness
  double jainsNumerator = 0.0;
  double jainsDenumerator = 0.0;
  if (nodeNum == 3){
    jainsNumerator = pow(sink->GetTotalRx() + sink1->GetTotalRx() + sink2->GetTotalRx()+ sink3->GetTotalRx()+ sink4->GetTotalRx()
                      + sink9->GetTotalRx(),2);
    jainsDenumerator = 6*(pow(sink->GetTotalRx(),2)+ pow(sink1->GetTotalRx(),2)+ pow(sink2->GetTotalRx(),2)+ pow(sink3->GetTotalRx(),2)
                      + pow(sink4->GetTotalRx(),2)+ pow(sink9->GetTotalRx(),2));
  }
  else if (nodeNum == 4){
    jainsNumerator = pow(sink->GetTotalRx() + sink1->GetTotalRx() + sink2->GetTotalRx()+ sink3->GetTotalRx()+ sink4->GetTotalRx()
                      + sink5->GetTotalRx()+ sink6->GetTotalRx()+ sink9->GetTotalRx(),2);
    jainsDenumerator = 10*(pow(sink->GetTotalRx(),2)+ pow(sink1->GetTotalRx(),2)+ pow(sink2->GetTotalRx(),2)+ pow(sink3->GetTotalRx(),2)
                      + pow(sink4->GetTotalRx(),2)+ pow(sink5->GetTotalRx(),2)+ pow(sink6->GetTotalRx(),2)+ pow(sink9->GetTotalRx(),2));
  }
  else if (nodeNum == 5){
    jainsNumerator = pow(sink->GetTotalRx() + sink1->GetTotalRx() + sink2->GetTotalRx()+ sink3->GetTotalRx()+ sink4->GetTotalRx()
                      + sink5->GetTotalRx()+ sink6->GetTotalRx()+ sink7->GetTotalRx()+ sink8->GetTotalRx()+ sink9->GetTotalRx(),2);
    jainsDenumerator = 10*(pow(sink->GetTotalRx(),2)+ pow(sink1->GetTotalRx(),2)+ pow(sink2->GetTotalRx(),2)+ pow(sink3->GetTotalRx(),2)
                      + pow(sink4->GetTotalRx(),2)+ pow(sink5->GetTotalRx(),2)+ pow(sink6->GetTotalRx(),2)+ pow(sink7->GetTotalRx(),2)
                      + pow(sink8->GetTotalRx(),2)+ pow(sink9->GetTotalRx(),2));
  }
  else if (nodeNum == 6){
    jainsNumerator = pow(sink->GetTotalRx() + sink1->GetTotalRx() + sink2->GetTotalRx()+ sink3->GetTotalRx()+ sink4->GetTotalRx()
                      + sink5->GetTotalRx()+ sink6->GetTotalRx()+ sink7->GetTotalRx()+ sink8->GetTotalRx()+ sink9->GetTotalRx() 
                      + sink10->GetTotalRx() + sink11->GetTotalRx(),2);
    jainsDenumerator = 12*(pow(sink->GetTotalRx(),2)+ pow(sink1->GetTotalRx(),2)+ pow(sink2->GetTotalRx(),2)+ pow(sink3->GetTotalRx(),2)
                      + pow(sink4->GetTotalRx(),2)+ pow(sink5->GetTotalRx(),2)+ pow(sink6->GetTotalRx(),2)+ pow(sink7->GetTotalRx(),2)
                      + pow(sink8->GetTotalRx(),2)+ pow(sink9->GetTotalRx(),2)+pow(sink10->GetTotalRx(),2)+pow(sink11->GetTotalRx(),2));
  }

  double jainsFairness = jainsNumerator/jainsDenumerator;

  // TODO: flowmonitor variables
  std::vector<double> results;
  results.push_back(totalThroughput);
  results.push_back(meanThroughput);
  results.push_back(jainsFairness);
  results.push_back(delaySum);
  results.push_back(lostPackets);
  results.push_back(recieve_packets);
  results.push_back(transmit_packets);
  return results;
}

