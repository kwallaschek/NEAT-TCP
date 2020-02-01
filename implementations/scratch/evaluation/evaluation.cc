#include "networkmodel.h"
#include "../Neat/network.h"
#include "../Neat/nnode.h"




#include <iostream>
#include <fstream>
#include <vector>
#include <string>

NS_LOG_COMPONENT_DEFINE ("evaluation");

int main(int argc, char *argv[]){

	std::ofstream outFile("replace with your filename_results");

	bool neat = true;
	/** Initialize NEAT-TCP2**/
	/*NEAT::NNode* node1 = new NEAT::NNode(NEAT::nodetype::SENSOR, 1, NEAT::nodeplace::INPUT);
	NEAT::NNode* node2 = new NEAT::NNode(NEAT::nodetype::SENSOR, 2, NEAT::nodeplace::INPUT);
	NEAT::NNode* node3 = new NEAT::NNode(NEAT::nodetype::SENSOR, 3, NEAT::nodeplace::INPUT);
	NEAT::NNode* node4 = new NEAT::NNode(NEAT::nodetype::NEURON, 4, NEAT::nodeplace::OUTPUT);
	NEAT::NNode* node5 = new NEAT::NNode(NEAT::nodetype::NEURON, 5, NEAT::nodeplace::HIDDEN);
	NEAT::NNode* node20 = new NEAT::NNode(NEAT::nodetype::NEURON, 20, NEAT::nodeplace::HIDDEN);
	NEAT::NNode* node170 = new NEAT::NNode(NEAT::nodetype::NEURON, 170, NEAT::nodeplace::HIDDEN);
	
	node4->add_incoming(node1, -1.63406);
	node4->add_incoming(node3, -0.850649);
	node5->add_incoming(node1, 3.18706);
	node4->add_incoming(node5, 3.86763);
	node5->add_incoming(node2, 0.105857);
	node4->add_incoming(node4, 5.81376, true);
	node20->add_incoming(node5, -1.22305);
	node4->add_incoming(node20, 1.41814);
	node20->add_incoming(node20, 1.41345, true);
	node20->add_incoming(node4, -3.01396, true);
	node170->add_incoming(node4, 3.11488, true);
	node4->add_incoming(node170, -0.991602);
	node5->add_incoming(node170, 0.886755);
	node170->add_incoming(node3, -0.326281);
	node170->add_incoming(node2, -0.307835);
	node20->add_incoming(node170, -1.72663);
	node170->add_incoming(node170, -3.08321);
	node170->add_incoming(node1, 0.496156);


	std::vector<NEAT::NNode*> allNodes;
	std::vector<NEAT::NNode*> inputNodes;
	std::vector<NEAT::NNode*> outputNodes;

	allNodes.push_back(node1);
	allNodes.push_back(node2);
	allNodes.push_back(node3);
	allNodes.push_back(node4);
	allNodes.push_back(node5);
	allNodes.push_back(node20);
	allNodes.push_back(node170);


	inputNodes.push_back(node1);
	inputNodes.push_back(node2);
	inputNodes.push_back(node3);

	outputNodes.push_back(node4);
	*/

	/** Initialize NEAT-TCP1**/
	NEAT::NNode* node1 = new NEAT::NNode(NEAT::nodetype::SENSOR, 1, NEAT::nodeplace::INPUT);
	NEAT::NNode* node2 = new NEAT::NNode(NEAT::nodetype::SENSOR, 2, NEAT::nodeplace::INPUT);
	NEAT::NNode* node3 = new NEAT::NNode(NEAT::nodetype::SENSOR, 3, NEAT::nodeplace::INPUT);
	NEAT::NNode* node4 = new NEAT::NNode(NEAT::nodetype::NEURON, 4, NEAT::nodeplace::OUTPUT);
	NEAT::NNode* node6 = new NEAT::NNode(NEAT::nodetype::NEURON, 6, NEAT::nodeplace::HIDDEN);

	
	// Add Links to Nodes
	node4->add_incoming(node1, -0.728733);
	node4->add_incoming(node2, -1.76637);
	node6->add_incoming(node3, 0.500052);
	node4->add_incoming(node6, 1.43347);
	node4->add_incoming(node4, 1.53149, true);
	node6->add_incoming(node1, 0.947995);

	std::vector<NEAT::NNode*> allNodes;
	std::vector<NEAT::NNode*> inputNodes;
	std::vector<NEAT::NNode*> outputNodes;

	// Fill Arrays with the corresponding Nodes
	allNodes.push_back(node1);
	allNodes.push_back(node2);
	allNodes.push_back(node3);
	allNodes.push_back(node4);
	allNodes.push_back(node6);

	inputNodes.push_back(node1);
	inputNodes.push_back(node2);
	inputNodes.push_back(node3);

	outputNodes.push_back(node4);

	// Build Network
	NEAT::Network* ntw = new NEAT::Network(inputNodes, outputNodes, allNodes, 1);

	std::vector<double> p;
	int numSimulations = 1200;
	int simulation = 0;
	/** 3x3 Grid **/
	outFile<<"3x3 Grid:\n";
	outFile<<"RunTime Run Thrput MeanThrput Jain delaySumMs lostPkts rcvdPkts txdPkts";
	// 40s runtime
	for(int runs = 0; runs < 100; runs++){
		simulation++;
		std::cout<<"Start Simulation "<< simulation << "/"<<numSimulations<<std::endl;
 		p = Initialize (3, runs, 40, neat, ntw);
 		outFile<< "\n40 " << runs << " ";
 		for (const auto &e : p) outFile << e << " ";
	} 
	// 60s runtime
	for(int runs = 0; runs < 100; runs++){
		simulation++;
		std::cout<<"Start Simulation "<< simulation << "/"<<numSimulations<<std::endl;
 		p = Initialize (3, runs, 60, neat, ntw);
 		outFile<< "\n60 " << runs << " ";
 		for (const auto &e : p) outFile << e << " ";
	} 
	
	// 100s runtime
	for(int runs = 0; runs < 100; runs++){
		simulation++;
		std::cout<<"Start Simulation "<< simulation << "/"<<numSimulations<<std::endl;
 		p = Initialize (3, runs, 100, neat, ntw);
		outFile<< "\n100 " << runs << " ";
 		for (const auto &e : p) outFile << e << " ";
	} 
	// 300s runtime
	for(int runs = 0; runs < 100; runs++){
		simulation++;
		std::cout<<"Start Simulation "<< simulation << "/"<<numSimulations<<std::endl;
 		p = Initialize (3, runs, 300, neat, ntw);
 		outFile<< "\n300 " << runs << " ";
 		for (const auto &e : p) outFile << e << " ";
	} 
	
	/** 5x5 Grid **/
	outFile<<"\n5x5 Grid:\n";
	outFile<<"RunTime Run Thrput MeanThrput Jain delaySumMs lostPkts rcvdPkts txdPkts";
	// 40s runtime
	for(int runs = 0; runs < 100; runs++){
		simulation++;
		std::cout<<"Start Simulation "<< simulation << "/"<<numSimulations<<std::endl;
 		p = Initialize (5, runs, 40, neat, ntw);
		outFile<< "\n40 " << runs << " ";
 		for (const auto &e : p) outFile << e << " ";
	} 
	
	// 60s runtime
	for(int runs = 0; runs < 100; runs++){
		simulation++;
		std::cout<<"Start Simulation "<< simulation << "/"<<numSimulations<<std::endl;
 		p = Initialize (5, runs, 60, neat, ntw);
		outFile<< "\n60 " << runs << " ";
 		for (const auto &e : p) outFile << e << " ";
	} 
	
	// 100s runtime
	for(int runs = 0; runs < 100; runs++){
		simulation++;
		std::cout<<"Start Simulation "<< simulation << "/"<<numSimulations<<std::endl;
 		p = Initialize (5, runs, 100, neat, ntw);
 		outFile<< "\n100 " << runs << " ";
 		for (const auto &e : p) outFile << e << " ";
	} 
	// 300s runtime
	for(int runs = 0; runs < 100; runs++){
		simulation++;
		std::cout<<"Start Simulation "<< simulation << "/"<<numSimulations<<std::endl;
 		p = Initialize (5, runs, 300, neat, ntw);
 		outFile<< "\n300 " << runs << " ";
 		for (const auto &e : p) outFile << e << " ";
	} 
	
	/** 6x6 Grid **/
	outFile<<"\n6x6 Grid:\n";
	outFile<<"RunTime Run Thrput MeanThrput Jain delaySumMs lostPkts rcvdPkts txdPkts";
	// 40s runtime
	for(int runs = 0; runs < 100; runs++){
		simulation++;
		std::cout<<"Start Simulation "<< simulation << "/"<<numSimulations<<std::endl;
 		p = Initialize (6, runs, 40, neat, ntw);
		outFile<< "\n40 " << runs << " ";
 		for (const auto &e : p) outFile << e << " ";
	} 
	// 60s runtime
	for(int runs = 0; runs < 100; runs++){
		simulation++;
		std::cout<<"Start Simulation "<< simulation << "/"<<numSimulations<<std::endl;
 		p = Initialize (6, runs, 60, neat, ntw);
		outFile<< "\n60 " << runs << " ";
 		for (const auto &e : p) outFile << e << " ";
	} 
	// 100s runtime
	for(int runs = 0; runs < 100; runs++){
		simulation++;
		std::cout<<"Start Simulation "<< simulation << "/"<<numSimulations<<std::endl;
 		p = Initialize (6, runs, 100, neat, ntw);
		outFile<< "\n100 " << runs << " ";
 		for (const auto &e : p) outFile << e << " ";
	} 
	// 300s runtime
	for(int runs = 0; runs < 100; runs++){
		simulation++;
		std::cout<<"Start Simulation "<< simulation << "/"<<numSimulations<<std::endl;
 		p = Initialize (6, runs, 300, neat, ntw);
 		outFile<< "\n300 " << runs << " ";
 		for (const auto &e : p) outFile << e << " ";
	} 
}