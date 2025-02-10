#include "stdafx.h"
#include "Cluster.h"
#include <limits>
#include <string>
#include <iostream>
#include <fstream>

// included dependencies
#include "FrogObjectCol.h"
#include "FrogLeapController.h"
#include "Pair.h"
#include "DistanceTable.h"
#include <string>
#include <string>
#include <string>


Cluster::Cluster(int id) :FrogObject(id)
{
	this->assignedDepotPair = NULL;
	this->nearestCluster = NULL;
	this->customerPairCol = new FrogObjectCol();
	this->distance_t = DistanceType::mean;
}

Cluster::Cluster(Pair * depotPair, int id) :FrogObject(id)
{
	this->assignedDepotPair = depotPair;
	this->nearestCluster = NULL;
	this->customerPairCol = new FrogObjectCol();
	this->distance_t = DistanceType::mean;
}

Cluster::~Cluster()
{
	this->assignedDepotPair = NULL;
	this->nearestCluster = NULL;
	this->customerPairCol->unReferenceFrogObjectCol();
	delete this->customerPairCol;
}

void Cluster::setDepot(int depotIndex, FrogLeapController * controller)
{
	this->assignedDepotPair = controller->getDepotPairByIndex(depotIndex);
}

void Cluster:: setDistanceType(DistanceType v_distance_t)
{
	this->distance_t = v_distance_t;
}

void Cluster::addCustomer(int customerIndex, FrogLeapController * controller)
{
	Pair * customerPair = controller->getCustomerPairByIndex(customerIndex);

	this->customerPairCol->addFrogObject(customerPair);
}

FrogObjectCol * Cluster::getCustomerCol()
{
	return this->customerPairCol;
}

void Cluster::setCustomerCol(FrogObjectCol * v_pairCol)
{
	this->customerPairCol = v_pairCol;
}

float Cluster::getDistanceToCustomer(int customerIndex, FrogLeapController * controller)
{
	if(this->distance_t == DistanceType::mean)
	{
		return this->getMeanDistanceToCustomer(customerIndex, controller);
	}

	if(this->distance_t == DistanceType::nearest)
	{
		return this->getNearestDistanceToCustomer(customerIndex, controller);
	}

	if (this->distance_t == DistanceType::depot)
	{
		return this->getDepotDistanceToCustomer(customerIndex, controller);
	}

	if(this->distance_t == DistanceType::furthest)
	{
		return this->getFurthestDistanceToCustomer(customerIndex, controller);
	}

}

float Cluster::getMeanDistanceToCustomer(int customerIndex, FrogLeapController * controller)
{
	float currentTotalDistance = std::numeric_limits<float>::max();;

	Pair * customerPair = controller->getCustomerPairByIndex(customerIndex);

	int customerInternalId = customerPair->getId();

	int depotInternalId = this->getDepotPair()->getId();

	DistanceTable * dt = controller->getDistanceTable();

	currentTotalDistance += dt->getEdge(customerInternalId, depotInternalId);

	for (int i = 0; i < this->customerPairCol->getSize(); i++)
	{
		Pair * clusterCustomerPair = (Pair *)this->customerPairCol->getFrogObject(i);
		int clusterCustomerInternalId = clusterCustomerPair->getId();
		currentTotalDistance += dt->getEdge(customerInternalId, clusterCustomerInternalId);
	}

	// k = number of customer + 1 depot
	int k = this->customerPairCol->getSize() + 1;
	return currentTotalDistance / k;
}

float Cluster::getNearestDistanceToCustomer(int customerIndex, FrogLeapController * controller)
{	

	Pair * customerPair = controller->getCustomerPairByIndex(customerIndex);

	int customerInternalId = customerPair->getId();

	int depotInternalId = this->getDepotPair()->getId();

	DistanceTable * dt = controller->getDistanceTable();

	float minDistance = dt->getEdge(customerInternalId, depotInternalId);
	
	int size = this->customerPairCol->getSize();
	for (int i = 0; i < size; i++)
	{
		Pair * clusterCustomerPair = (Pair *)this->customerPairCol->getFrogObject(i);
		int clusterCustomerInternalId = clusterCustomerPair->getId();
		float currentDistance = dt->getEdge(customerInternalId, clusterCustomerInternalId);
		if(currentDistance < minDistance)
		{
			minDistance = currentDistance;			
		}
	}

	return minDistance;
}

float Cluster::getDepotDistanceToCustomer(int customerIndex, FrogLeapController* controller)
{
	Pair* customerPair = controller->getCustomerPairByIndex(customerIndex);

	int customerInternalId = customerPair->getId();

	int depotInternalId = this->getDepotPair()->getId();

	DistanceTable* dt = controller->getDistanceTable();

	float depotDistance = dt->getEdge(customerInternalId, depotInternalId);

	return depotDistance;
}

float Cluster::getFurthestDistanceToCustomer(int customerIndex, FrogLeapController* controller)
{
	Pair* customerPair = controller->getCustomerPairByIndex(customerIndex);

	int customerInternalId = customerPair->getId();

	int depotInternalId = this->getDepotPair()->getId();

	DistanceTable* dt = controller->getDistanceTable();

	float maxDistance = dt->getEdge(customerInternalId, depotInternalId);

	for (int i = 0; i < this->customerPairCol->getSize(); i++)
	{
		Pair* clusterCustomerPair = (Pair*)this->customerPairCol->getFrogObject(i);
		int clusterCustomerInternalId = clusterCustomerPair->getId();
		float currentDistance = dt->getEdge(customerInternalId, clusterCustomerInternalId);
		if (currentDistance > maxDistance)
		{
			maxDistance = currentDistance;
		}
	}

	return maxDistance;
}

void Cluster::unassignCustomerFromCluster(Pair* customerPair)
{
	this->customerPairCol->removeFrogObject(customerPair);
}

Pair * Cluster::createClusterCopy(FrogLeapController * controller)
{
	Pair * clusterPair = new Pair(PairType::IntVsInt, this->getId());
	clusterPair->set_i_IntValue(controller->getDepotInternalId(this->getId()));

	return clusterPair;
}

int Cluster::remainingDepotCapacity()
{
	
	return this->assignedDepotPair->get_j_IntValue();
}

int Cluster::getDepotCapacity()
{
	return this->assignedDepotPair->get_i_IntValue();
}

int Cluster::getDepotRemainingCapacity()
{
	return this->assignedDepotPair->get_j_IntValue();
}

void Cluster::exportClusterToVRP(FrogLeapController * controller)
{
	int number = this->getDepotPair()->getLabelId();
	char fileName[50];
	//std::sprintf(fileName, "ClusterId_%d.vrp", number);
	sprintf_s(fileName, sizeof(fileName), "ClusterId_%d.vrp", number);
	FILE* file = controller->openFile(fileName);

	char clusterName[50];
	//std::sprintf(clusterName, "ClusterId_%d", number);
	sprintf_s(clusterName, sizeof(clusterName), "ClusterId_%d", number);
	std::string strClusterName(clusterName);
	
	if (file == NULL) {
		std::cerr << "Unable to open file: " << fileName << std::endl;
		return;
	}

	fprintf(file, "NAME : %s \n", clusterName);
	
	fprintf(file, "COMMENT : (Gillet and Johnson) \n");
	fprintf(file,"TYPE : CVRP \n");
	fprintf(file, "DIMENSION : %d \n", this->getCustomerCol()->getSize()+1);

	
	fprintf(file, "EDGE_WEIGHT_TYPE : EUC_2D \n");
	fprintf(file, "CAPACITY : %d \n", controller->getVehicleCapacity());
	fprintf(file, "NODE_COORD_SECTION \n");
	
	writeDepotNodeCoords(file, controller);

	writeCustomerColNodeCoords(file, controller);

	fprintf(file, "DEMAND_SECTION \n");
	
	writeDepotDemand(file, controller);

	writeCustomerColNodeDemands(file, controller);

	fprintf(file, "DEPOT_SECTION \n");
	writeDepotLabelId(file, controller);
	fprintf(file, " -1\n"); // -1 is used to end the depot section
	fprintf(file, "EOF");
	fclose(file);	
}

void Cluster::writeDepotLabelId(FILE* file, FrogLeapController* controller)
{
	Pair* depotNodeCoord = controller->getNodeCoordPairById(this->getDepotPair()->getId());
	depotNodeCoord->writeLabelId(file);
}

void Cluster::writeDepotDemand(FILE* file, FrogLeapController* controller)
{
	Pair * depotNodeDemand = controller->getNodeDemandPairById(this->getDepotPair()->getId());

	depotNodeDemand->writeDemand(file);		
}

void Cluster::writeCustomerColNodeDemands(FILE * file, FrogLeapController * controller)
{
	Pair * customerDemandPair = NULL, * customerPair;

	for (int i = 0; i < this->customerPairCol->getSize(); i++)
	{
		customerPair = (Pair *) this->customerPairCol->getFrogObject(i);
		customerDemandPair = (Pair *) controller->getNodeDemandPairById(customerPair->getId());
		customerDemandPair->writeDemand(file);
	}
}

void Cluster::writeDepotNodeCoords(FILE * file, FrogLeapController * controller)
{
	Pair* depotNodeCoord = controller->getNodeCoordPairById(this->getDepotPair()->getId());

	depotNodeCoord->writeCoords(file);  
}

void Cluster::writeCustomerColNodeCoords(FILE * file, FrogLeapController * controller)
{
	Pair * customerCoordPair = NULL, * customerPair;

	for (int i = 0; i < this->customerPairCol->getSize(); i++)
	{
		customerPair = (Pair*)this->customerPairCol->getFrogObject(i);
		customerCoordPair = (Pair *)controller->getNodeCoordPairById(customerPair->getId());
		customerCoordPair->writeCoords(file);
	}
}

//void Cluster::printClusterContentHeader()
//{
//	printf("DepotInternalId; CustomerInternalId; DepotLabelId; CustomerLabelId \n");
//}

void Cluster::printClusterContentRecord(int customerIndexInCol)
{
	Pair * customerPair = (Pair *)this->getCustomerCol()->getFrogObject(customerIndexInCol);
	Pair* depotPair = (Pair*)this->getDepotPair();

	//printf("%d; %d; %d; %d \n", depotPair->getId(), customerPair->getId(), depotPair->getLabelId(), customerPair->getLabelId());
	printf("%d; %d \n", depotPair->getLabelId(), customerPair->getLabelId());
}

void Cluster::printFrogObj()
{
	printf("SHOWING  CLUSTER CONTENT \n");

	printf("Showing Cluster Value = %.2f \n", this->getValue());
	printf("Cluster Depot Information \n");
	this->getDepotPair()->printFrogObj();

	int n_customers = this->getCustomerCol()->getSize();

	//this->printClusterContentHeader();
	printf("Showing Cluster Customers Information \n");

	if(n_customers == 0)
	{
		printf("Cluster is empty \n");
	}
	else 
	{
		for (int i = 0; i < n_customers; i++)
		{
			this->printClusterContentRecord(i);
		}
	}

	//printf("FINISH OF SHOWING CLUSTER CONTENT \n");
}

bool Cluster::isTheSame(FrogObject * fs)
{
	return (this == fs);
}

Pair * Cluster::getDepotPair()
{
	return this->assignedDepotPair;
}


