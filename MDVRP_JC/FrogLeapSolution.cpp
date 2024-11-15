#include "stdafx.h"
#include "FrogLeapSolution.h"
#include "FeasibleSolution.h"
#include "DecodedFrogLeapSolution.h"
#include "FrogLeapController.h"
#include "Vehicle.h"
#include "Graph.h"
#include "FrogObjectCol.h"
#include "FrogObject.h"
#include "Pair.h"
#include "IndexList.h"
#include "Cluster.h"
#include "ClarkWrightHandler.h"
#include "SolutionData.h"
#include <stdio.h>      /* printf */
#include <math.h>       /* floor */
#include <iostream>
#include <time.h>
#include <random>


FrogLeapSolution::FrogLeapSolution(SolutionGenerationType v_sgt, SourceType v_sourceType, int ncustomers, int n_depots_v, int id):FrogObject(id)
{
	this->sgt = v_sgt;
	this->st = v_sourceType;
	
	this->n_depots = n_depots_v;

	this->size = ncustomers;
	this->nElementsToSort = this->n_depots;
		
	this->values = new float[this->size];		

	customerSelectionList = NULL;

	this->clusterCol = NULL;
	this->cw_handler_col = NULL;
}

FrogLeapSolution::~FrogLeapSolution()
{
	
	delete clusterCol;
	delete cw_handler_col;
	delete[] values;
}

float FrogLeapSolution::getFLValue(int i)
{
	return this->values[i];
}

void FrogLeapSolution::setFLValue(int i, float v_float)
{
	this->values[i] = v_float;
}

void FrogLeapSolution::setSize(int size_v)
{
	this->size = size_v;
}

int FrogLeapSolution::getSize()
{
	return this->size;
}

bool FrogLeapSolution::genRandomSolution(FrogLeapController * controller)
{
	float u;
	int a = this->size;	
	FrogObjectCol * feasibleDepotList = NULL;
	
	// reset all remaining capacities of depots and return them in a new list
	feasibleDepotList = initializeFeasibleDepotList(controller);
	bool result = true;
	int i = 0;
	
	while ((i < this->size) && (result == true))
	{
		u = assignRandomFeasibleDepot(feasibleDepotList, controller, i);

		// code to eliminate fixed random assignment of customers to depots
		//u = this->normalRandomAssigment(controller);

		//this->values[i] = u;

		if(u > 0)
		{
			this->values[i] = u;
		}
		else
		{
			result = false;			
		}

		i++;
	};

	feasibleDepotList->unReferenceFrogObjectCol();
	delete feasibleDepotList;
	feasibleDepotList = NULL;

	return result;
}

bool FrogLeapSolution::genRandomSolution2(FrogLeapController * controller)
{
	float u;
	int a = this->size;
			
	bool result = true;
	int i = 0;

	// reset all remaining capacities of depots and return them in a new list
	controller->resetDepotRemainingCapacities();

	while ((i < this->size) && (result == true))
	{
		u = assignRandomFeasibleDepot2(controller, i);

		// code to eliminate fixed random assignment of customers to depots
		//u = this->normalRandomAssigment(controller);

		//this->values[i] = u;

		if (u > 0)
		{
			this->values[i] = u;
		}
		else
		{
			result = false;
		}

		i++;
	};

	return result;
}

// Selects a random depot between those with capacity enough
bool FrogLeapSolution::genRandomSolution3(FrogLeapController * controller)
{
	float u;
	int a = this->size;

	bool result = true;
	int i = 0;

	controller->resetDepotRemainingCapacities();

	FrogObjectCol * localDepotCol = controller->createDepotListOrderedByCapacity();

	while ((i < this->size) && (result == true))
	{
		u = assignRandomFeasibleDepot3(controller, localDepotCol, i);

		// code to eliminate fixed random assignment of customers to depots
		//u = this->normalRandomAssigment(controller);

		//this->values[i] = u;

		if (u >= 0)
		{
			this->values[i] = u;
		}
		else
		{
			result = false;
		}

		i++;
	};

	localDepotCol->unReferenceFrogObjectCol();
	delete localDepotCol;
	return result;
}

bool FrogLeapSolution::genRandomSolution4(FrogLeapController * controller)
{
	float u;
	int a = this->size;

	bool result = true;
	int i = 0;

	// put the same value of capacity in attributes i and j of Pair object in depotArray
	controller->resetDepotRemainingCapacities();	
	FrogObjectCol * localDepotCol = controller->createDepotListOrderedByCapacity();

	// create a feasibleSolution object (an array) with value i in position i
	// feasible is assigned to attribute randomCustomerSelectionList
	this->initCustomerSelection(controller);

	controller->resetCustomersAsNotAssigned();

	while ((i < this->size) && (result == true))
	{
		// here we need to change to select first the customer with bigger capacity
		int rand_i = this->selectRandomCustomerIndex(i, controller);

		u = controller->assignRandomFeasibleDepot4(localDepotCol, rand_i);

		// code to eliminate fixed random assignment of customers to depots
		//u = this->normalRandomAssigment(controller);

		//this->values[i] = u;

		if (u >= 0)
		{
			this->values[rand_i] = u;
		}
		else
		{
			result = false; 
		}

		i++;
	};

	localDepotCol->unReferenceFrogObjectCol();
	delete localDepotCol;
	
	this->destroyRandomCustomerSelectionList();

	return result;
}

//choose the closest item: available depot or the depot of the closest assigned customer
// we use localCustomerColOrderedByDemand (capacity criteria)
bool FrogLeapSolution::genRandomSolution5(FrogLeapController * controller)
{
	float u;
	int a = this->size;

	bool result = true;	

	// put the same value of capacity in attributes i and j of Pair object in depotArray
	controller->resetDepotRemainingCapacities();
	FrogObjectCol * localDepotCol = controller->createDepotListOrderedByCapacity();

	// create a list ordered by demand in descendant order
	controller->resetCustomerRemainingDemands();
	FrogObjectCol * localCustomerColOrderedByDemand = controller->createCustomerListOrderedByDemandDesc();

	//localCustomerColOrderedByDemand->printFrogObjCol();

	// create a feasibleSolution object (an array) with value i in position i
	// feasible is assigned to attribute randomCustomerSelectionList
	//this->customerSelectionList = new FeasibleSolution(controller->getNumberOfCustomers());
	this->initCustomerSelection(controller);

	controller->resetCustomersAsNotAssigned();

	int i = 0;
	while ((i < this->size) && (result == true))
	{
		// we select first the customer with bigger demand
		Pair * customerPairOrderedByDemand_i = (Pair *) localCustomerColOrderedByDemand->getFrogObject(i);

		int customerIndex = controller->getCustomerListIndexByInternal(customerPairOrderedByDemand_i->getId());

		//if(customerIndex == 6)
		//{
			//printf("ojo acá");
			//localDepotCol->printFrogObjCol();
		//}

		//choose the closest item: available depot or the depot of the closest assigned customer
		u = controller->assignRandomFeasibleDepot4(localDepotCol, customerIndex);

		// code to eliminate fixed random assignment of customers to depots
		//u = this->normalRandomAssigment(controller);

		//this->values[i] = u;

		if (u >= 0)
		{
			this->values[customerIndex] = u;
		}
		else
		{
			result = false;
		}

		i++;
	};

	
	//this->writeFrogLeapSolution(controller);

	localDepotCol->unReferenceFrogObjectCol();
	delete localDepotCol;

	this->destroyRandomCustomerSelectionList();

	return result;
}

// Assigns each customer using the "match criteria"
bool FrogLeapSolution::genRandomSolution6(FrogLeapController * controller)
{
	float u = -1;
	//int a = this->size;

	bool result = true;
	int i = 0;

	// put the same value of capacity in attributes i and j of Pair object in depotArray
	controller->resetDepotRemainingCapacities();
	FrogObjectCol * depotListOrderedByCapacity = controller->createDepotListDescOrderedByCapacity();
	
	// put the same value of demand in attributes i and j of Pair object in customerArray
	controller->resetCustomerRemainingDemands();

	controller->resetCustomersAsNotAssigned();

	//FrogObjectCol * localCustomerColOrderedByDemandDesc = controller->createCustomerListOrderedByDemandDesc();

	// create a feasibleSolution object (an array) with value i in position i
	// feasible is assigned to attribute randomCustomerSelectionList
	//this->customerSelectionList = new FeasibleSolution(controller->getNumberOfCustomers());
	this->initCustomerSelection(controller);
	
	int nDepots = controller->getNumberOfDepots();	

	while ((i < nDepots) && (result == true))
	{		
		Pair * depotPairOrderedByCapacity_i = (Pair *)depotListOrderedByCapacity->getFrogObject(i);

		//assign customers with "match" criteria
		assignRandomFeasibleDepot5(controller, depotPairOrderedByCapacity_i);

		i++;
	};

	assignRemainingCustomersToClosestCluster(controller);

	//localDepotCol->unReferenceFrogObjectCol();
	//delete localDepotCol;

	this->destroyRandomCustomerSelectionList();

	return result;
}

// assigns using the match and closer depot criteria
bool FrogLeapSolution::genRandomSolution7(FrogLeapController * controller)
{
	float u = 0;
	//int a = this->size;

	bool result = true;	

	// put the same value of capacity in attributes i and j of Pair object in depotArray
	controller->resetDepotRemainingCapacities();
	FrogObjectCol * depotListOrderedByCapacity = controller->createDepotListOrderedByCapacity();


	// put the same value of capacity in attributes i and j of Pair object in customerArray
	controller->resetCustomerRemainingDemands();
	controller->resetCustomersAsNotAssigned();

	//FrogObjectCol * localCustomerColOrderedByDemandDesc = controller->createCustomerListOrderedByDemandDesc();

	// create a feasibleSolution object (an array) with value i in position i
	// feasible is assigned to attribute randomCustomerSelectionList
	//this->customerSelectionList = new FeasibleSolution(controller->getNumberOfCustomers());
	FeasibleSolution * depotFeasibleSolution = this->initDepotSelection(controller);	

	int nDepots = controller->getNumberOfDepots();

	
	int i = 0;
	while ((i < nDepots) && (result == true))
	{
		int randomDepotPair_i = this->selectRandomDepotIndex(i, depotFeasibleSolution, controller);

		Pair * depotPairRandomSelected = controller->getDepotPairByIndex(randomDepotPair_i);

		//assign customers with "match" criteria and closer cluster criteria
		assignRandomFeasibleDepot6(controller, depotPairRandomSelected, depotListOrderedByCapacity);

		i++;
	};

	assignRemainingCustomersToClosestCluster(controller);

	//localDepotCol->unReferenceFrogObjectCol();
	//delete localDepotCol;

	this->destroyRandomCustomerSelectionList();

	return result;
}

// assigns using the match and closer depot criteria
bool FrogLeapSolution::genRandomSolution7_5(FrogLeapController* controller)
{
	float u = -1;
	//int a = this->size;

	bool result = true;

	// put the same value of capacity in attributes i and j of Pair object in depotArray
	controller->resetDepotRemainingCapacities();
	FrogObjectCol* depotListOrderedByCapacity = controller->createDepotListOrderedByCapacity();

	// put the same value of capacity in attributes i and j of Pair object in customerArray
	controller->resetCustomerRemainingDemands();
	controller->resetCustomersAsNotAssigned();

	//FrogObjectCol * localCustomerColOrderedByDemandDesc = controller->createCustomerListOrderedByDemandDesc();

	int nDepots = controller->getNumberOfDepots();

	// create a feasibleSolution object (an array) with value i in position i
	// feasible is assigned to attribute randomCustomerSelectionList
	//this->customerSelectionList = new FeasibleSolution(controller->getNumberOfCustomers());
	//FeasibleSolution* depotFeasibleSolution = this->initDepotSelection(controller);
	FeasibleSolution* depotFeasibleSolution = controller->initFeasibleSolution(nDepots);
	
	bool isSolutionFeasible = controller->isCurrentSolutionFeasible();
	bool unassignedCustomerExists = controller->unassignedCustomerPairExists();

	int i = -1;
	int randomDepotPair_i = -1;
	int currentUnassignedCustomers = -1;
		
	while ((isSolutionFeasible == true) && (unassignedCustomerExists == true))
	{
		currentUnassignedCustomers = controller->numberOfUnassignedCustomers();
		if (currentUnassignedCustomers == 26) {
			//printf("parar aca \n");
		}

		//randomDepotPair_i = controller->selectRandomIndex(i, depotFeasibleSolution);
		//Pair * depotPairRandomSelected = controller->getDepotPairByIndex(randomDepotPair_i);		
		Pair* depotPairRandomSelected = controller->getDepotPairByIndex(i);

		//assign customers with "full match" criteria 
		assignRandomFeasibleDepot7(controller, depotPairRandomSelected, depotListOrderedByCapacity);

		i++;
		if (i == nDepots) {
			i = 0;
		};

		isSolutionFeasible = controller->isCurrentSolutionFeasible();
		unassignedCustomerExists = controller->unassignedCustomerPairExists();		
	}

	//this->destroyRandomCustomerSelectionList();

	delete depotFeasibleSolution;

	result = isSolutionFeasible;
	return result;
}

// CLUSTER: assigns using the match and closer depot criteria using clusters
bool FrogLeapSolution::genRandomSolution8_0(FrogLeapController* controller)
{
	float u = -1;
	//int a = this->size;

	bool result = true;

	// put the same value of capacity in attributes i and j of Pair object in depotArray
	controller->resetDepotRemainingCapacities();
	FrogObjectCol* depotListOrderedByCapacity = controller->createDepotListOrderedByCapacity();

	// put the same value of capacity in attributes i and j of Pair object in customerArray
	controller->resetCustomerRemainingDemands();
	controller->resetCustomersAsNotAssigned();

	//FrogObjectCol * localCustomerColOrderedByDemandDesc = controller->createCustomerListOrderedByDemandDesc();

	int nDepots = controller->getNumberOfDepots();

	// create a feasibleSolution object (an array) with value i in position i
	// feasible is assigned to attribute randomCustomerSelectionList
	//this->customerSelectionList = new FeasibleSolution(controller->getNumberOfCustomers());
	//FeasibleSolution* depotFeasibleSolution = this->initDepotSelection(controller);
	FeasibleSolution * depotFeasibleSolution = controller->initFeasibleSolution(nDepots);

	bool isSolutionFeasible = controller->isCurrentSolutionFeasible();
	bool unassignedCustomerExists = controller->unassignedCustomerPairExists();

	this->initClusterCollection(controller);

	int i = 0;
	int randomDepotPair_i;
	int currentUnassignedCustomers;

	while ((isSolutionFeasible == true) && (unassignedCustomerExists == true))
	{
		currentUnassignedCustomers = controller->numberOfUnassignedCustomers();
		FrogObjectCol* unassignedCustomers = controller->getListOfUnassignedCustomers();

		if (currentUnassignedCustomers == 1) {
			//printf("parar aca \n");
		}

		randomDepotPair_i = controller->selectRandomIndex(i, depotFeasibleSolution);
		//Pair * depotPairRandomSelected = controller->getDepotPairByIndex(randomDepotPair_i);		
		
		Cluster * randomClusterSelected = this->getClusterByIndex(randomDepotPair_i);

		//assign customers with "full match" criteria 
		assignRandomFeasibleDepot8(controller, randomClusterSelected, depotListOrderedByCapacity);

		i++;
		if (i == nDepots) {
			i = 0;
		};

		isSolutionFeasible = controller->isCurrentSolutionFeasible();
		unassignedCustomerExists = controller->unassignedCustomerPairExists();

		unassignedCustomers->unReferenceFrogObjectCol();
		delete unassignedCustomers;
	}

	//this->destroyRandomCustomerSelectionList();

	delete depotFeasibleSolution;

	result = isSolutionFeasible;
	return result;
}

// CLUSTER: assigns using the match and closer depot criteria using clusters
bool FrogLeapSolution::genRandomSolution9_0(FrogLeapController* controller)
{
	float u = -1;
	//int a = this->size;

	bool result = true;

	// put the same value of capacity in attributes i and j of Pair object in depotArray
	controller->resetDepotRemainingCapacities();
	FrogObjectCol* depotListOrderedByCapacity = controller->createDepotListOrderedByCapacity();

	// put the same value of capacity in attributes i and j of Pair object in customerArray
	controller->resetCustomerRemainingDemands();
	controller->resetCustomersAsNotAssigned();

	//FrogObjectCol * localCustomerColOrderedByDemandDesc = controller->createCustomerListOrderedByDemandDesc();

	int nDepots = controller->getNumberOfDepots();

	// create a feasibleSolution object (an array) with value i in position i
	// feasible is assigned to attribute randomCustomerSelectionList
	//this->customerSelectionList = new FeasibleSolution(controller->getNumberOfCustomers());
	//FeasibleSolution* depotFeasibleSolution = this->initDepotSelection(controller);
	FeasibleSolution* depotFeasibleSolution = controller->initFeasibleSolution(nDepots);

	bool isSolutionFeasible = controller->isCurrentSolutionFeasible();
	bool unassignedCustomerExists = controller->unassignedCustomerPairExists();

	this->initClusterCollection(controller);
	this->updateClusterCollectionWithDistanceType(DistanceType::depot);

	int i = 0;
	int randomDepotPair_i;
	int currentUnassignedCustomers;

	while ((isSolutionFeasible == true) && (unassignedCustomerExists == true))
	{
		currentUnassignedCustomers = controller->numberOfUnassignedCustomers();
		FrogObjectCol* unassignedCustomers = controller->getListOfUnassignedCustomers();

		if (currentUnassignedCustomers == 1) {
			//printf("parar aca \n");
		}

		randomDepotPair_i = controller->selectRandomIndex(i, depotFeasibleSolution);
		//Pair * depotPairRandomSelected = controller->getDepotPairByIndex(randomDepotPair_i);		

		Cluster* randomClusterSelected = this->getClusterByIndex(randomDepotPair_i);

		//assign customers with "full match" criteria 
		assignRandomFeasibleDepot9(controller, randomClusterSelected, depotListOrderedByCapacity);

		i++;
		if (i == nDepots) {
			i = 0;
		};

		isSolutionFeasible = controller->isCurrentSolutionFeasible();
		unassignedCustomerExists = controller->unassignedCustomerPairExists();

		unassignedCustomers->unReferenceFrogObjectCol();
		delete unassignedCustomers;
	}

	//this->destroyRandomCustomerSelectionList();

	delete depotFeasibleSolution;

	result = isSolutionFeasible;
	return result;
}

// CLUSTER: assigns using the match and closer depot criteria using clusters
bool FrogLeapSolution::genRandomSolution10_0(FrogLeapController* controller)
{
	float u = -1;
	//int a = this->size;

	bool result = true;

	// put the same value of capacity in attributes i and j of Pair object in depotArray
	controller->resetDepotRemainingCapacities();
	FrogObjectCol* depotListOrderedByCapacity = controller->createDepotListOrderedByCapacity();

	// put the same value of capacity in attributes i and j of Pair object in customerArray
	controller->resetCustomerRemainingDemands();
	controller->resetCustomersAsNotAssigned();

	//FrogObjectCol * localCustomerColOrderedByDemandDesc = controller->createCustomerListOrderedByDemandDesc();

	int nDepots = controller->getNumberOfDepots();

	// create a feasibleSolution object (an array) with value i in position i
	// feasible is assigned to attribute randomCustomerSelectionList
	//this->customerSelectionList = new FeasibleSolution(controller->getNumberOfCustomers());
	//FeasibleSolution* depotFeasibleSolution = this->initDepotSelection(controller);
	FeasibleSolution* depotFeasibleSolution = controller->initFeasibleSolution(nDepots);

	bool isSolutionFeasible = controller->isCurrentSolutionFeasible();
	bool unassignedCustomerExists = controller->unassignedCustomerPairExists();

	this->initClusterCollection(controller);
	this->updateClusterCollectionWithDistanceType(DistanceType::depot);

	int i = 0;
	int randomDepotPair_i;
	int currentUnassignedCustomers;

	while ((isSolutionFeasible == true) && (unassignedCustomerExists == true))
	{
		currentUnassignedCustomers = controller->numberOfUnassignedCustomers();
		FrogObjectCol* unassignedCustomers = controller->getListOfUnassignedCustomers();

		if (currentUnassignedCustomers == 1) {
			//printf("parar aca \n");
		}

		randomDepotPair_i = controller->selectRandomIndex(i, depotFeasibleSolution);
		//Pair * depotPairRandomSelected = controller->getDepotPairByIndex(randomDepotPair_i);		

		Cluster* randomClusterSelected = this->getClusterByIndex(randomDepotPair_i);

		//assign customers with "full match" criteria 
		assignRandomFeasibleDepot10(controller, randomClusterSelected, depotListOrderedByCapacity);

		i++;
		if (i == nDepots) {
			i = 0;
		};

		isSolutionFeasible = controller->isCurrentSolutionFeasible();
		unassignedCustomerExists = controller->unassignedCustomerPairExists();

		unassignedCustomers->unReferenceFrogObjectCol();
		delete unassignedCustomers;
	}

	//this->destroyRandomCustomerSelectionList();

	delete depotFeasibleSolution;

	result = isSolutionFeasible;
	return result;
}

// CLUSTER: assigns using the match and closer depot criteria using clusters
bool FrogLeapSolution::genRandomSolution11_0(FrogLeapController* controller)
{
	float u = -1;
	//int a = this->size;

	bool result = true;

	// put the same value of capacity in attributes i and j of Pair object in depotArray
	controller->resetDepotRemainingCapacities();
	FrogObjectCol* depotListOrderedByCapacity = controller->createDepotListOrderedByCapacity();

	// put the same value of capacity in attributes i and j of Pair object in customerArray
	controller->resetCustomerRemainingDemands();
	controller->resetCustomersAsNotAssigned();

	//FrogObjectCol * localCustomerColOrderedByDemandDesc = controller->createCustomerListOrderedByDemandDesc();

	int nDepots = controller->getNumberOfDepots();

	// create a feasibleSolution object (an array) with value i in position i
	// feasible is assigned to attribute randomCustomerSelectionList
	//this->customerSelectionList = new FeasibleSolution(controller->getNumberOfCustomers());
	//FeasibleSolution* depotFeasibleSolution = this->initDepotSelection(controller);
	FeasibleSolution* depotFeasibleSolution = controller->initFeasibleSolution(nDepots);

	bool isSolutionFeasible = controller->isCurrentSolutionFeasible();
	bool unassignedCustomerExists = controller->unassignedCustomerPairExists();

	this->initClusterCollection(controller);
	this->updateClusterCollectionWithDistanceType(DistanceType::nearest);

	int i = 0;
	int randomDepotPair_i;
	int currentUnassignedCustomers;

	while ((isSolutionFeasible == true) && (unassignedCustomerExists == true))
	{
		currentUnassignedCustomers = controller->numberOfUnassignedCustomers();
		FrogObjectCol* unassignedCustomers = controller->getListOfUnassignedCustomers();

		if (currentUnassignedCustomers == 1) {
			//printf("parar aca \n");
		}

		randomDepotPair_i = controller->selectRandomIndex(i, depotFeasibleSolution);
		//Pair * depotPairRandomSelected = controller->getDepotPairByIndex(randomDepotPair_i);		

		Cluster* randomClusterSelected = this->getClusterByIndex(randomDepotPair_i);

		//assign customers with "full match" criteria 
		assignRandomFeasibleDepot11(controller, randomClusterSelected, depotListOrderedByCapacity);

		i++;
		if (i == nDepots) {
			i = 0;
		};

		isSolutionFeasible = controller->isCurrentSolutionFeasible();
		unassignedCustomerExists = controller->unassignedCustomerPairExists();

		unassignedCustomers->unReferenceFrogObjectCol();
		delete unassignedCustomers;
	}

	//this->destroyRandomCustomerSelectionList();

	delete depotFeasibleSolution;

	result = isSolutionFeasible;
	return result;
}


// CLUSTER: assigns using the match and closer depot criteria using clusters
bool FrogLeapSolution::genRandomSolution12_0(FrogLeapController* controller)
{
	float u = -1;
	//int a = this->size;

	bool result = true;

	// put the same value of capacity in attributes i and j of Pair object in depotArray
	controller->resetDepotRemainingCapacities();
	FrogObjectCol* depotListOrderedByCapacity = controller->createDepotListOrderedByCapacity();

	// put the same value of capacity in attributes i and j of Pair object in customerArray
	controller->resetCustomerRemainingDemands();
	controller->resetCustomersAsNotAssigned();

	//FrogObjectCol * localCustomerColOrderedByDemandDesc = controller->createCustomerListOrderedByDemandDesc();

	int nDepots = controller->getNumberOfDepots();

	// create a feasibleSolution object (an array) with value i in position i
	// feasible is assigned to attribute randomCustomerSelectionList
	//this->customerSelectionList = new FeasibleSolution(controller->getNumberOfCustomers());
	//FeasibleSolution* depotFeasibleSolution = this->initDepotSelection(controller);
	FeasibleSolution* depotFeasibleSolution = controller->initFeasibleSolution(nDepots);

	bool isSolutionFeasible = controller->isCurrentSolutionFeasible();
	bool unassignedCustomerExists = controller->unassignedCustomerPairExists();

	this->initClusterCollection(controller);
	this->updateClusterCollectionWithDistanceType(DistanceType::furthest);

	int i = 0;
	int randomDepotPair_i;
	int currentUnassignedCustomers;

	while ((isSolutionFeasible == true) && (unassignedCustomerExists == true))
	{
		currentUnassignedCustomers = controller->numberOfUnassignedCustomers();
		FrogObjectCol* unassignedCustomers = controller->getListOfUnassignedCustomers();

		if (currentUnassignedCustomers == 1) {
			//printf("parar aca \n");
		}

		randomDepotPair_i = controller->selectRandomIndex(i, depotFeasibleSolution);
		//Pair * depotPairRandomSelected = controller->getDepotPairByIndex(randomDepotPair_i);		

		Cluster* randomClusterSelected = this->getClusterByIndex(randomDepotPair_i);

		//assign customers with "full match" criteria 
		assignRandomFeasibleDepot12(controller, randomClusterSelected, depotListOrderedByCapacity);

		i++;
		if (i == nDepots) {
			i = 0;
		};

		isSolutionFeasible = controller->isCurrentSolutionFeasible();
		unassignedCustomerExists = controller->unassignedCustomerPairExists();

		unassignedCustomers->unReferenceFrogObjectCol();
		delete unassignedCustomers;
	}

	//this->destroyRandomCustomerSelectionList();

	delete depotFeasibleSolution;

	result = isSolutionFeasible;
	return result;
}

// CLUSTER: assigns using the match and closer depot criteria using clusters
bool FrogLeapSolution::genRandomSolution13_0(FrogLeapController* controller)
{
	float u = -1;
	//int a = this->size;

	bool result = true;

	// put the same value of capacity in attributes i and j of Pair object in depotArray
	controller->resetDepotRemainingCapacities();
	FrogObjectCol* depotListOrderedByCapacity = controller->createDepotListOrderedByCapacity();

	// put the same value of capacity in attributes i and j of Pair object in customerArray
	controller->resetCustomerRemainingDemands();
	controller->resetCustomersAsNotAssigned();

	//FrogObjectCol * localCustomerColOrderedByDemandDesc = controller->createCustomerListOrderedByDemandDesc();

	int nDepots = controller->getNumberOfDepots();

	// create a feasibleSolution object (an array) with value i in position i
	// feasible is assigned to attribute randomCustomerSelectionList
	//this->customerSelectionList = new FeasibleSolution(controller->getNumberOfCustomers());
	//FeasibleSolution* depotFeasibleSolution = this->initDepotSelection(controller);
	FeasibleSolution* depotFeasibleSolution = controller->initFeasibleSolution(nDepots);

	bool isSolutionFeasible = controller->isCurrentSolutionFeasible();
	bool unassignedCustomerExists = controller->unassignedCustomerPairExists();

	this->initClusterCollection(controller);
	this->updateClusterCollectionWithDistanceType(DistanceType::furthest);

	int i = 0;
	int randomDepotPair_i;
	int currentUnassignedCustomers;

	while ((isSolutionFeasible == true) && (unassignedCustomerExists == true))
	{
		currentUnassignedCustomers = controller->numberOfUnassignedCustomers();
		FrogObjectCol* unassignedCustomers = controller->getListOfUnassignedCustomers();

		if (currentUnassignedCustomers == 1) {
			//printf("parar aca \n");
		}

		randomDepotPair_i = controller->selectRandomIndex(i, depotFeasibleSolution);
		//Pair * depotPairRandomSelected = controller->getDepotPairByIndex(randomDepotPair_i);		

		Cluster* randomClusterSelected = this->getClusterByIndex(randomDepotPair_i);

		//assign customers with "full match" criteria 
		assignRandomFeasibleDepot13(controller, randomClusterSelected, depotListOrderedByCapacity);

		i++;
		if (i == nDepots) {
			i = 0;
		};

		isSolutionFeasible = controller->isCurrentSolutionFeasible();
		unassignedCustomerExists = controller->unassignedCustomerPairExists();

		unassignedCustomers->unReferenceFrogObjectCol();
		delete unassignedCustomers;
	}

	//this->destroyRandomCustomerSelectionList();

	delete depotFeasibleSolution;

	result = isSolutionFeasible;
	return result;
}

// CLUSTER: assigns using the match and closer depot criteria using clusters
bool FrogLeapSolution::genRandomSolution14_0(FrogLeapController* controller)
{
	float u = -1;
	//int a = this->size;

	bool result = true;

	// put the same value of capacity in attributes i and j of Pair object in depotArray
	controller->resetDepotRemainingCapacities();
	FrogObjectCol* depotListOrderedByCapacity = controller->createDepotListOrderedByCapacity();

	// put the same value of capacity in attributes i and j of Pair object in customerArray
	controller->resetCustomerRemainingDemands();
	controller->resetCustomersAsNotAssigned();

	//FrogObjectCol * localCustomerColOrderedByDemandDesc = controller->createCustomerListOrderedByDemandDesc();

	int nDepots = controller->getNumberOfDepots();

	// create a feasibleSolution object (an array) with value i in position i
	// feasible is assigned to attribute randomCustomerSelectionList
	//this->customerSelectionList = new FeasibleSolution(controller->getNumberOfCustomers());
	//FeasibleSolution* depotFeasibleSolution = this->initDepotSelection(controller);
	FeasibleSolution* depotFeasibleSolution = controller->initFeasibleSolution(nDepots);

	bool isSolutionFeasible = controller->isCurrentSolutionFeasible();
	bool unassignedCustomerExists = controller->unassignedCustomerPairExists();

	this->initClusterCollection(controller);
	this->updateClusterCollectionWithDistanceType(DistanceType::depot);

	int i = 0;
	int randomDepotPair_i;
	int currentUnassignedCustomers;

	while ((isSolutionFeasible == true) && (unassignedCustomerExists == true))
	{
		currentUnassignedCustomers = controller->numberOfUnassignedCustomers();
		FrogObjectCol* unassignedCustomers = controller->getListOfUnassignedCustomers();

		if (currentUnassignedCustomers == 1) {
			//printf("parar aca \n");
		}

		randomDepotPair_i = controller->selectRandomIndex(i, depotFeasibleSolution);
		//Pair * depotPairRandomSelected = controller->getDepotPairByIndex(randomDepotPair_i);		

		Cluster* randomClusterSelected = this->getClusterByIndex(randomDepotPair_i);

		//assign customers with "full match" criteria 
		assignRandomFeasibleDepot14(controller, randomClusterSelected, depotListOrderedByCapacity);

		i++;
		if (i == nDepots) {
			i = 0;
		};

		isSolutionFeasible = controller->isCurrentSolutionFeasible();
		unassignedCustomerExists = controller->unassignedCustomerPairExists();

		unassignedCustomers->unReferenceFrogObjectCol();
		delete unassignedCustomers;
	}

	//this->destroyRandomCustomerSelectionList();

	delete depotFeasibleSolution;

	result = isSolutionFeasible;
	return result;
}

// CLUSTER: assigns using the match and closer depot criteria using clusters
bool FrogLeapSolution::genRandomSolution15_0(FrogLeapController* controller)
{
	float u = -1;
	//int a = this->size;

	bool result = true;

	// put the same value of capacity in attributes i and j of Pair object in depotArray
	controller->resetDepotRemainingCapacities();
	FrogObjectCol* depotListOrderedByCapacity = controller->createDepotListOrderedByCapacity();

	// put the same value of capacity in attributes i and j of Pair object in customerArray
	controller->resetCustomerRemainingDemands();
	controller->resetCustomersAsNotAssigned();

	//FrogObjectCol * localCustomerColOrderedByDemandDesc = controller->createCustomerListOrderedByDemandDesc();

	int nDepots = controller->getNumberOfDepots();

	// create a feasibleSolution object (an array) with value i in position i
	// feasible is assigned to attribute randomCustomerSelectionList
	//this->customerSelectionList = new FeasibleSolution(controller->getNumberOfCustomers());
	//FeasibleSolution* depotFeasibleSolution = this->initDepotSelection(controller);
	FeasibleSolution* depotFeasibleSolution = controller->initFeasibleSolution(nDepots);

	bool isSolutionFeasible = controller->isCurrentSolutionFeasible();
	bool unassignedCustomerExists = controller->unassignedCustomerPairExists();

	this->initClusterCollection(controller);
	this->updateClusterCollectionWithDistanceType(DistanceType::depot);

	int i = 0;
	int randomDepotPair_i;
	int currentUnassignedCustomers;

	while ((isSolutionFeasible == true) && (unassignedCustomerExists == true))
	{
		currentUnassignedCustomers = controller->numberOfUnassignedCustomers();
		FrogObjectCol* unassignedCustomers = controller->getListOfUnassignedCustomers();

		if (currentUnassignedCustomers == 1) {
			//printf("parar aca \n");
		}

		randomDepotPair_i = controller->selectRandomIndex(i, depotFeasibleSolution);
		//Pair * depotPairRandomSelected = controller->getDepotPairByIndex(randomDepotPair_i);		

		Cluster* randomClusterSelected = this->getClusterByIndex(randomDepotPair_i);

		//assign customers with "full match" criteria 
		assignRandomFeasibleDepot15(controller, randomClusterSelected, depotListOrderedByCapacity);

		i++;
		if (i == nDepots) {
			i = 0;
		};

		isSolutionFeasible = controller->isCurrentSolutionFeasible();
		unassignedCustomerExists = controller->unassignedCustomerPairExists();

		unassignedCustomers->unReferenceFrogObjectCol();
		delete unassignedCustomers;
	}

	//this->destroyRandomCustomerSelectionList();

	delete depotFeasibleSolution;

	result = isSolutionFeasible;
	return result;
}

// CLUSTER: assigns using the match and closer depot criteria using clusters
bool FrogLeapSolution::genRandomSolution16_0(FrogLeapController* controller)
{
	float u = -1;
	//int a = this->size;

	bool result = true;

	// put the same value of capacity in attributes i and j of Pair object in depotArray
	controller->resetDepotRemainingCapacities();
	FrogObjectCol* depotListOrderedByCapacity = controller->createDepotListOrderedByCapacity();

	// put the same value of capacity in attributes i and j of Pair object in customerArray
	controller->resetCustomerRemainingDemands();
	controller->resetCustomersAsNotAssigned();

	//FrogObjectCol * localCustomerColOrderedByDemandDesc = controller->createCustomerListOrderedByDemandDesc();

	int nDepots = controller->getNumberOfDepots();

	// create a feasibleSolution object (an array) with value i in position i
	// feasible is assigned to attribute randomCustomerSelectionList
	//this->customerSelectionList = new FeasibleSolution(controller->getNumberOfCustomers());
	//FeasibleSolution* depotFeasibleSolution = this->initDepotSelection(controller);
	FeasibleSolution* depotFeasibleSolution = controller->initFeasibleSolution(nDepots);

	bool isSolutionFeasible = controller->isCurrentSolutionFeasible();
	bool unassignedCustomerExists = controller->unassignedCustomerPairExists();

	this->initClusterCollection(controller);
	this->updateClusterCollectionWithDistanceType(DistanceType::furthest);

	int i = 0;
	int randomDepotPair_i;
	int currentUnassignedCustomers;

	while ((isSolutionFeasible == true) && (unassignedCustomerExists == true))
	{
		currentUnassignedCustomers = controller->numberOfUnassignedCustomers();
		FrogObjectCol* unassignedCustomers = controller->getListOfUnassignedCustomers();

		if (currentUnassignedCustomers == 1) {
			//printf("parar aca \n");
		}

		randomDepotPair_i = controller->selectRandomIndex(i, depotFeasibleSolution);
		//Pair * depotPairRandomSelected = controller->getDepotPairByIndex(randomDepotPair_i);		

		Cluster* randomClusterSelected = this->getClusterByIndex(randomDepotPair_i);

		//assign customers with "full match" criteria 
		assignRandomFeasibleDepot16(controller, randomClusterSelected, depotListOrderedByCapacity);

		i++;
		if (i == nDepots) {
			i = 0;
		};

		isSolutionFeasible = controller->isCurrentSolutionFeasible();
		unassignedCustomerExists = controller->unassignedCustomerPairExists();

		unassignedCustomers->unReferenceFrogObjectCol();
		delete unassignedCustomers;
	}

	//this->destroyRandomCustomerSelectionList();

	delete depotFeasibleSolution;

	result = isSolutionFeasible;
	return result;
}

// CLUSTER: assigns using the match and closer depot criteria using clusters
bool FrogLeapSolution::genRandomSolution17_0(FrogLeapController* controller)
{
	float u = -1;
	//int a = this->size;

	bool result = true;

	// put the same value of capacity in attributes i and j of Pair object in depotArray
	controller->resetDepotRemainingCapacities();
	FrogObjectCol* depotListOrderedByCapacity = controller->createDepotListOrderedByCapacity();

	// put the same value of capacity in attributes i and j of Pair object in customerArray
	controller->resetCustomerRemainingDemands();
	controller->resetCustomersAsNotAssigned();

	//FrogObjectCol * localCustomerColOrderedByDemandDesc = controller->createCustomerListOrderedByDemandDesc();

	int nDepots = controller->getNumberOfDepots();

	// create a feasibleSolution object (an array) with value i in position i
	// feasible is assigned to attribute randomCustomerSelectionList
	//this->customerSelectionList = new FeasibleSolution(controller->getNumberOfCustomers());
	//FeasibleSolution* depotFeasibleSolution = this->initDepotSelection(controller);
	FeasibleSolution* depotFeasibleSolution = controller->initFeasibleSolution(nDepots);

	bool isSolutionFeasible = controller->isCurrentSolutionFeasible();
	bool unassignedCustomerExists = controller->unassignedCustomerPairExists();

	this->initClusterCollection(controller);
	this->updateClusterCollectionWithDistanceType(DistanceType::nearest);

	int i = 0;
	int randomDepotPair_i;
	int currentUnassignedCustomers;

	while ((isSolutionFeasible == true) && (unassignedCustomerExists == true))
	{
		currentUnassignedCustomers = controller->numberOfUnassignedCustomers();
		FrogObjectCol* unassignedCustomers = controller->getListOfUnassignedCustomers();

		if (currentUnassignedCustomers == 1) {
			//printf("parar aca \n");
		}

		randomDepotPair_i = controller->selectRandomIndex(i, depotFeasibleSolution);
		//Pair * depotPairRandomSelected = controller->getDepotPairByIndex(randomDepotPair_i);		

		Cluster* randomClusterSelected = this->getClusterByIndex(randomDepotPair_i);

		//assign customers with "full match" criteria 
		assignRandomFeasibleDepot17(controller, randomClusterSelected, depotListOrderedByCapacity);

		i++;
		if (i == nDepots) {
			i = 0;
		};

		isSolutionFeasible = controller->isCurrentSolutionFeasible();
		unassignedCustomerExists = controller->unassignedCustomerPairExists();

		unassignedCustomers->unReferenceFrogObjectCol();
		delete unassignedCustomers;
	}

	//this->destroyRandomCustomerSelectionList();

	delete depotFeasibleSolution;

	result = isSolutionFeasible;
	return result;
}

// CLUSTER: assigns using the match and closer depot criteria using clusters
bool FrogLeapSolution::genRandomSolution18_0(FrogLeapController* controller)
{
	float u = -1;
	//int a = this->size;

	bool result = true;

	// put the same value of capacity in attributes i and j of Pair object in depotArray
	controller->resetDepotRemainingCapacities();
	FrogObjectCol* depotListOrderedByCapacity = controller->createDepotListOrderedByCapacity();

	// put the same value of capacity in attributes i and j of Pair object in customerArray
	controller->resetCustomerRemainingDemands();
	controller->resetCustomersAsNotAssigned();

	//FrogObjectCol * localCustomerColOrderedByDemandDesc = controller->createCustomerListOrderedByDemandDesc();

	int nDepots = controller->getNumberOfDepots();

	// create a feasibleSolution object (an array) with value i in position i
	// feasible is assigned to attribute randomCustomerSelectionList
	//this->customerSelectionList = new FeasibleSolution(controller->getNumberOfCustomers());
	//FeasibleSolution* depotFeasibleSolution = this->initDepotSelection(controller);
	FeasibleSolution* depotFeasibleSolution = controller->initFeasibleSolution(nDepots);

	bool isSolutionFeasible = controller->isCurrentSolutionFeasible();
	bool unassignedCustomerExists = controller->unassignedCustomerPairExists();

	this->initClusterCollection(controller);
	this->updateClusterCollectionWithDistanceType(DistanceType::nearest);

	int i = 0;
	int randomDepotPair_i;
	int currentUnassignedCustomers;

	while ((isSolutionFeasible == true) && (unassignedCustomerExists == true))
	{
		currentUnassignedCustomers = controller->numberOfUnassignedCustomers();
		FrogObjectCol* unassignedCustomers = controller->getListOfUnassignedCustomers();

		if (currentUnassignedCustomers == 1) {
			//printf("parar aca \n");
		}

		randomDepotPair_i = controller->selectRandomIndex(i, depotFeasibleSolution);
		//Pair * depotPairRandomSelected = controller->getDepotPairByIndex(randomDepotPair_i);		

		Cluster* randomClusterSelected = this->getClusterByIndex(randomDepotPair_i);

		//assign customers with "full match" criteria 
		assignRandomFeasibleDepot18(controller, randomClusterSelected, depotListOrderedByCapacity);

		i++;
		if (i == nDepots) {
			i = 0;
		};

		isSolutionFeasible = controller->isCurrentSolutionFeasible();
		unassignedCustomerExists = controller->unassignedCustomerPairExists();

		unassignedCustomers->unReferenceFrogObjectCol();
		delete unassignedCustomers;
	}

	//this->destroyRandomCustomerSelectionList();

	delete depotFeasibleSolution;

	result = isSolutionFeasible;
	return result;
}

// CLUSTER: assigns using the match and closer depot criteria using clusters
bool FrogLeapSolution::genRandomSolution19_0(FrogLeapController* controller)
{
	float u = -1;
	//int a = this->size;

	bool result = true;

	// put the same value of capacity in attributes i and j of Pair object in depotArray
	controller->resetDepotRemainingCapacities();
	FrogObjectCol* depotListOrderedByCapacity = controller->createDepotListOrderedByCapacity();

	// put the same value of capacity in attributes i and j of Pair object in customerArray
	controller->resetCustomerRemainingDemands();
	controller->resetCustomersAsNotAssigned();

	//FrogObjectCol * localCustomerColOrderedByDemandDesc = controller->createCustomerListOrderedByDemandDesc();

	int nDepots = controller->getNumberOfDepots();

	// create a feasibleSolution object (an array) with value i in position i
	// feasible is assigned to attribute randomCustomerSelectionList
	//this->customerSelectionList = new FeasibleSolution(controller->getNumberOfCustomers());
	//FeasibleSolution* depotFeasibleSolution = this->initDepotSelection(controller);
	FeasibleSolution* depotFeasibleSolution = controller->initFeasibleSolution(nDepots);

	bool isSolutionFeasible = controller->isCurrentSolutionFeasible();
	bool unassignedCustomerExists = controller->unassignedCustomerPairExists();

	this->initClusterCollection(controller);
	this->updateClusterCollectionWithDistanceType(DistanceType::nearest);

	int i = 0;
	int randomDepotPair_i;
	int currentUnassignedCustomers;

	while ((isSolutionFeasible == true) && (unassignedCustomerExists == true))
	{
		currentUnassignedCustomers = controller->numberOfUnassignedCustomers();
		FrogObjectCol* unassignedCustomers = controller->getListOfUnassignedCustomers();

		if (currentUnassignedCustomers == 1) {
			//printf("parar aca \n");
		}

		randomDepotPair_i = controller->selectRandomIndex(i, depotFeasibleSolution);
		//Pair * depotPairRandomSelected = controller->getDepotPairByIndex(randomDepotPair_i);		

		Cluster* randomClusterSelected = this->getClusterByIndex(randomDepotPair_i);

		//assign customers with "full match" criteria 
		assignRandomFeasibleDepot19(controller, randomClusterSelected, depotListOrderedByCapacity);

		i++;
		if (i == nDepots) {
			i = 0;
		};

		isSolutionFeasible = controller->isCurrentSolutionFeasible();
		unassignedCustomerExists = controller->unassignedCustomerPairExists();

		unassignedCustomers->unReferenceFrogObjectCol();
		delete unassignedCustomers;
	}

	//this->destroyRandomCustomerSelectionList();

	delete depotFeasibleSolution;

	result = isSolutionFeasible;
	return result;
}

// CLUSTER: assigns using the match and closer depot criteria using clusters
bool FrogLeapSolution::genRandomSolution20_0(FrogLeapController* controller)
{
	float u = -1;
	//int a = this->size;

	bool result = true;

	// put the same value of capacity in attributes i and j of Pair object in depotArray
	controller->resetDepotRemainingCapacities();
	FrogObjectCol* depotListOrderedByCapacity = controller->createDepotListOrderedByCapacity();

	// put the same value of capacity in attributes i and j of Pair object in customerArray
	controller->resetCustomerRemainingDemands();
	controller->resetCustomersAsNotAssigned();

	//FrogObjectCol * localCustomerColOrderedByDemandDesc = controller->createCustomerListOrderedByDemandDesc();

	int nDepots = controller->getNumberOfDepots();

	// create a feasibleSolution object (an array) with value i in position i
	// feasible is assigned to attribute randomCustomerSelectionList
	//this->customerSelectionList = new FeasibleSolution(controller->getNumberOfCustomers());
	//FeasibleSolution* depotFeasibleSolution = this->initDepotSelection(controller);
	FeasibleSolution* depotFeasibleSolution = controller->initFeasibleSolution(nDepots);

	bool isSolutionFeasible = controller->isCurrentSolutionFeasible();
	bool unassignedCustomerExists = controller->unassignedCustomerPairExists();

	this->initClusterCollection(controller);
	this->updateClusterCollectionWithDistanceType(DistanceType::depot);

	int i = 0;
	int randomDepotPair_i;
	int currentUnassignedCustomers;

	while ((isSolutionFeasible == true) && (unassignedCustomerExists == true))
	{
		currentUnassignedCustomers = controller->numberOfUnassignedCustomers();
		FrogObjectCol* unassignedCustomers = controller->getListOfUnassignedCustomers();

		if (currentUnassignedCustomers == 1) {
			//printf("parar aca \n");
		}

		randomDepotPair_i = controller->selectRandomIndex(i, depotFeasibleSolution);
		//Pair * depotPairRandomSelected = controller->getDepotPairByIndex(randomDepotPair_i);		

		Cluster* randomClusterSelected = this->getClusterByIndex(randomDepotPair_i);

		//assign customers with "full match" criteria 
		assignRandomFeasibleDepot20(controller, randomClusterSelected, depotListOrderedByCapacity);

		i++;
		if (i == nDepots) {
			i = 0;
		};

		isSolutionFeasible = controller->isCurrentSolutionFeasible();
		unassignedCustomerExists = controller->unassignedCustomerPairExists();

		unassignedCustomers->unReferenceFrogObjectCol();
		delete unassignedCustomers;
	}

	//this->destroyRandomCustomerSelectionList();

	delete depotFeasibleSolution;

	result = isSolutionFeasible;
	return result;
}

// CLUSTER: assigns using the match and closer depot criteria using clusters
bool FrogLeapSolution::genRandomSolution21_0(FrogLeapController* controller)
{
	float u = -1;
	//int a = this->size;

	bool result = true;

	// put the same value of capacity in attributes i and j of Pair object in depotArray
	controller->resetDepotRemainingCapacities();
	FrogObjectCol* depotListOrderedByCapacity = controller->createDepotListOrderedByCapacity();

	// put the same value of capacity in attributes i and j of Pair object in customerArray
	controller->resetCustomerRemainingDemands();
	controller->resetCustomersAsNotAssigned();

	//FrogObjectCol * localCustomerColOrderedByDemandDesc = controller->createCustomerListOrderedByDemandDesc();

	int nDepots = controller->getNumberOfDepots();

	// create a feasibleSolution object (an array) with value i in position i
	// feasible is assigned to attribute randomCustomerSelectionList
	//this->customerSelectionList = new FeasibleSolution(controller->getNumberOfCustomers());
	//FeasibleSolution* depotFeasibleSolution = this->initDepotSelection(controller);
	FeasibleSolution* depotFeasibleSolution = controller->initFeasibleSolution(nDepots);

	bool isSolutionFeasible = controller->isCurrentSolutionFeasible();
	bool unassignedCustomerExists = controller->unassignedCustomerPairExists();

	this->initClusterCollection(controller);
	this->updateClusterCollectionWithDistanceType(DistanceType::depot);

	int i = 0;
	int randomDepotPair_i;
	int currentUnassignedCustomers;

	while ((isSolutionFeasible == true) && (unassignedCustomerExists == true))
	{
		currentUnassignedCustomers = controller->numberOfUnassignedCustomers();
		FrogObjectCol* unassignedCustomers = controller->getListOfUnassignedCustomers();

		if (currentUnassignedCustomers == 1) {
			//printf("parar aca \n");
		}

		randomDepotPair_i = controller->selectRandomIndex(i, depotFeasibleSolution);
		//Pair * depotPairRandomSelected = controller->getDepotPairByIndex(randomDepotPair_i);		

		Cluster* randomClusterSelected = this->getClusterByIndex(randomDepotPair_i);

		//assign customers with "full match" criteria 
		assignRandomFeasibleDepot20(controller, randomClusterSelected, depotListOrderedByCapacity);

		i++;
		if (i == nDepots) {
			i = 0;
			this->updateClusterCollectionWithDistanceType(DistanceType::nearest);
		};

		isSolutionFeasible = controller->isCurrentSolutionFeasible();
		unassignedCustomerExists = controller->unassignedCustomerPairExists();

		unassignedCustomers->unReferenceFrogObjectCol();
		delete unassignedCustomers;
	}

	//this->destroyRandomCustomerSelectionList();

	delete depotFeasibleSolution;

	result = isSolutionFeasible;
	return result;
}

Cluster* FrogLeapSolution::getClusterByIndex(int i)
{
	return (Cluster*)this->clusterCol->getFrogObject(i);
}

// assigns using the match and closer depot criteria
//bool FrogLeapSolution::genRandomSolution8(FrogLeapController* controller)
//{
//	float u;
//	//int a = this->size;
//
//	bool result = true;
//
//	// put the same value of capacity in attributes i and j of Pair object in depotArray
//	controller->resetDepotRemainingCapacities();
//	FrogObjectCol* depotListOrderedByCapacity = controller->createDepotListOrderedByCapacity();
//
//
//	// put the same value of capacity in attributes i and j of Pair object in customerArray
//	controller->resetCustomerRemainingDemands();
//	controller->resetCustomersAsNotAssigned();
//
//	//FrogObjectCol * localCustomerColOrderedByDemandDesc = controller->createCustomerListOrderedByDemandDesc();
//
//	// create a feasibleSolution object (an array) with value i in position i
//	// feasible is assigned to attribute randomCustomerSelectionList
//	//this->customerSelectionList = new FeasibleSolution(controller->getNumberOfCustomers());
//	FeasibleSolution* depotFeasibleSolution = this->initDepotSelection(controller);
//
//	int nDepots = controller->getNumberOfDepots();
//	int nCustomers = controller->getNumberOfCustomers();
//	int maxDepotIndexToMatch = -1;
//
//	while (controller->unassignedCustomerPairExists()) 
//	{
//		for (int i = 0; i < nCustomers; i++)
//		{
//			Pair * custPair = controller->getCustomerPairByIndex(i);
//			Pair * matchedDepotPair = controller->getMatchedDepotPair(custPair);
//
//			if(matchedDepotPair != NULL)
//			{
//				int depotIndex = controller->getDepotIndexByLabelId(matchedDepotPair->getId());
//
//				if(controller->getCustomerDemandByIndex(i) <= controller->getDepotRemainingCapacityByIndex(depotIndex))
//				{
//					controller->setCustomerPairAsAssigned(i, depotIndex);
//				}
//				else 
//				{
//					//controller->incMatchedDepotIndex(i);
//				}
//			}
//		}
//		
//		//maxDepotIndexToMatch = controller->getMaxDepotIndexToMatch();
//	}
//
//	if(maxDepotIndexToMatch == controller->getNumberOfDepots())
//	{
//		result = false;
//	}
//
//	//assignRemainingCustomersToClosestCluster(controller);
//
//	//localDepotCol->unReferenceFrogObjectCol();
//	//delete localDepotCol;
//
//	//this->destroyRandomCustomerSelectionList();
//
//	return result;
//}

bool FrogLeapSolution::genRandomSolutionFromTestCase(FrogLeapController * controller)
{
	float u;
	int a = this->size;

	bool result = true;
	int i = 0;

	//controller->resetDepotRemainingCapacities();

	FrogObjectCol * testCustomerPairs = controller->getTestCustomerSectionList();

	while ((i < this->size) && (result == true))
	{
		u = assignRandomFeasibleDepot3(controller, testCustomerPairs, i);

		// code to eliminate fixed random assignment of customers to depots
		//u = this->normalRandomAssigment(controller);

		//this->values[i] = u;

		if (u >= 0)
		{
			this->values[i] = u;
		}
		else
		{
			result = false;
		}

		i++;
	};

	testCustomerPairs->unReferenceFrogObjectCol();
	delete testCustomerPairs;
	return result;
}

float FrogLeapSolution::normalRandomAssigment(FrogLeapController * controller)
{	
	float depotsNum = controller->getNumberOfDepots();
	float u = controller->genRandomFloatingNumber(0, depotsNum);

	if ( u >= depotsNum)
	{
		u = u - 1;		
	}

	return u;
}

float FrogLeapSolution::assignRandomFeasibleDepot(FrogObjectCol * feasibleDepotList, FrogLeapController * controller, int customerIndex)
{	
	float u = -1;	
	
	FrogObjectCol * deletedFrogObjects = new FrogObjectCol();

	// obtain the customer demand to check the set of candidate depots
	int customerDemand = controller->getCustomerDemandByIndex(customerIndex);

	// eliminate all depots that have remainingCapacity lower than demand of the customer
	feasibleDepotList->removeAllLowerValueFrogObject(customerDemand, deletedFrogObjects);

	if (feasibleDepotList->getSize() > 0)
	{
		// select a random depot between the set of candidate depots
		int size = feasibleDepotList->getSize();
		u = controller->genRandomFloatingNumber(0, size);
		int position = floor(u);
		if(position == size)
		{
			position--;
		}
		
		Pair * depotPair = (Pair *)feasibleDepotList->getFrogObject(position);

		//update remaining capacity of depot pair
		int newCapacity = depotPair->get_j_IntValue() - customerDemand;
		depotPair->set_j_IntValue(newCapacity);
		depotPair->setValue(newCapacity);

		int depotInternalId = depotPair->getId();
		int depotArrayIndex = controller->getDepotListIndexByInternal(depotInternalId);

		// This is to give a priority to establish the order in which the depot give the service to customers
		u = depotArrayIndex + controller->genRandomFloatingNumber(0, 1);
		if (u >= depotArrayIndex + 1)
		{
			u = depotArrayIndex;
		}
	}

	if(deletedFrogObjects->getSize() > 0)
	{
		feasibleDepotList->addAllFrogObjects(deletedFrogObjects);
	}
	
	deletedFrogObjects->unReferenceFrogObjectCol();
	delete deletedFrogObjects;
	deletedFrogObjects = NULL;
	return  u;
}

float FrogLeapSolution::assignRandomFeasibleDepot2(FrogLeapController * controller, int customerIndex)
{	
	float u = -1, result = -1;
	
	int customerDemand = 0;

	// get the number of existing depots
	int numberOfDepots = controller->getNumberOfDepots();

	// get a random number among depots
	u = controller->genRandomFloatingNumber(0, numberOfDepots);
	
	if (u >= numberOfDepots) 
	{
		u = u - 1;
	}

	int position = floor(u);

	int depotRemainingCap = 0;

	int itCounter = 0;
	bool customerAllocated = false;
	Pair * depotPair = NULL;

	customerDemand = controller->getCustomerDemandByIndex(customerIndex);

	while(itCounter < numberOfDepots && customerAllocated == false)
	{
		depotRemainingCap = controller->getDepotRemainingCapacityByIndex(position);

		if (customerDemand <= depotRemainingCap)
		{
			customerAllocated = true;

			depotPair = (Pair *)controller->getDepotPairByIndex(position);

			//update remaining capacity of depot pair
			int newCapacity = depotPair->get_j_IntValue() - customerDemand;
			depotPair->set_j_IntValue(newCapacity);
			depotPair->setValue(newCapacity);

			result = position + controller->genRandomFloatingNumber(0, 1);

			if (result >= position + 1)
			{
				result = position;
			}
		}
		else
		{
			position++;
			if(position == numberOfDepots)
			{
				position = 0;
			}
		}

		itCounter++;
	}

	depotPair = NULL;
	return  result;
}

//Selects a random depot between those with capacity enough
float FrogLeapSolution::assignRandomFeasibleDepot3(FrogLeapController * controller, FrogObjectCol * localDepotCol, int customerIndex)
{
	float u = -1, result = -1;
	Pair * depotPairSelected = NULL;
	int lowBoundIndex = -1;

	// get the customer demand
	int customerDemand = controller->getCustomerDemandByIndex(customerIndex);

	// get the index of the first depot with capacity enough to attend customer demand
	localDepotCol->getFirstHigherValueFrogObjectIndex(customerDemand, lowBoundIndex);

	// if there is not any depot then return -1
	if(lowBoundIndex == -1)
	{
		return result;
	}

	// choose between the available depots with suffiecient capacity to attend the customer demand
	int numberOfDepots = controller->getNumberOfDepots();

	do
	{
		u = controller->genRandomFloatingNumber(lowBoundIndex, numberOfDepots);
	} while (u >= numberOfDepots);

	int positionSelected = floor(u);

	// get the depotPair selected in the list of available depots
	depotPairSelected = (Pair *)localDepotCol->getFrogObject(positionSelected);
		
	//update remaining capacity of depot pair
	int depotRemainingCap = depotPairSelected->get_j_IntValue();		
	int newCapacity = depotRemainingCap - customerDemand;
	depotPairSelected->set_j_IntValue(newCapacity);
	depotPairSelected->setValue(newCapacity);

	localDepotCol->reorderFrogObject(depotPairSelected);

	// get the depot index in the controller of the selected depot
	int depotId = depotPairSelected->getId();
	int depotIndex = controller->getDepotListIndexByInternal(depotId);

	// assign a random number to the depot selected
	float randnum;

	do
	{
		randnum = controller->genRandomFloatingNumber(0, 1);
	} while (randnum >= 1);

	result = depotIndex + randnum;

	return  result;
}

//choose the closest item: available depot or the depot of the closest assigned customer
// localDepotCol is ordered in ascendant order
float FrogLeapSolution::assignRandomFeasibleDepot4(FrogLeapController * controller, FrogObjectCol * & localDepotCol, int customerIndex)
{
	float u = -1, result = -1;
	Pair * depotPairSelected = NULL;
	int lowBoundIndex = -1;

	// get the customer demand
	int customerDemand = controller->getCustomerDemandByIndex(customerIndex);

	// get the index of the first depot with capacity enough to attend customer demand
	localDepotCol->getFirstHigherValueFrogObjectIndex(customerDemand, lowBoundIndex);

	// if there is not any depot then return -1
	if (lowBoundIndex == -1)
	{
		return result;
	}

	// choose between the available depots with suffiecient capacity to attend the customer demand
	int numberOfDepots = controller->getNumberOfDepots();

	float distanceToDepot;
	int positionSelected = controller->getClosestLocalDepotIndexToCustomer(customerIndex, lowBoundIndex, localDepotCol->getSize(), localDepotCol, distanceToDepot);
	
	float distanceToDepotCustomer;
	
	// get the depot index of the closest already assigned customer. Capacity of depot is checked
	int closestCustomerDepotIndex = controller->getDepotIndexOfClosestAssignedCustomer(customerIndex, localDepotCol, lowBoundIndex, localDepotCol->getSize(), distanceToDepotCustomer);

	if(closestCustomerDepotIndex != -1)
	{
		if(distanceToDepotCustomer < distanceToDepot)
		{
			depotPairSelected = controller->getDepotPairByIndex(closestCustomerDepotIndex);			
		}
		else
		{
			// get the depotPair selected in the list of available depots
			depotPairSelected = (Pair *)localDepotCol->getFrogObject(positionSelected);			
		}
	}
	else
	{
		// get the depotPair selected in the list of available depots
		depotPairSelected = (Pair *)localDepotCol->getFrogObject(positionSelected);
	}

	//update remaining capacity of depot pair
	int depotRemainingCap = depotPairSelected->get_j_IntValue();
	int newCapacity = depotRemainingCap - customerDemand;
	depotPairSelected->set_j_IntValue(newCapacity);
	depotPairSelected->setValue(newCapacity);

	localDepotCol->reorderFrogObject(depotPairSelected);

	// get the depot index in the controller of the selected depot
	int depotId = depotPairSelected->getId();
	int depotIndex = controller->getDepotListIndexByInternal(depotId);

	controller->setCustomerPairAsAssigned(customerIndex, depotIndex);

	// assign a random number to the depot selected
	float randnum;

	do
	{
		randnum = controller->genRandomFloatingNumber(0, 1);
	} while (randnum >= 1);

	result = depotIndex + randnum;

	return  result;
}

//choose the closest match item: from a given depot, we choose the closest customer that satisfies that the closest depot is also the same given depot
// at the end, with the unassigned customer we choose randomly any of them and assign the closest cluster
void FrogLeapSolution::assignRandomFeasibleDepot5(FrogLeapController * controller, Pair * currentDepotPair)
{
	float u = -1, result = -1;
	Pair * depotPairSelected = NULL;
	int lowCustomerBoundIndex = -1;
	int n = 2;

	// get the depot current capacity
	//int depotCurrentCapacity = currentDepotPair->get_j_IntValue();

	// get the index of the first customer with lower demand to be attended by current depot
	FrogObjectCol * matchCustomerCol = controller->createMatchCustomerList(currentDepotPair);

	int totalCustomerDemand = controller->getTotalDemandOrCapacity(matchCustomerCol);

	int depotRemainingCapacity = currentDepotPair->get_j_IntValue();

	if(totalCustomerDemand <= depotRemainingCapacity)
	{
		controller->assignDepotToCustomerPairs(currentDepotPair, matchCustomerCol);
		this->assignMatchCustomerListToFLValues(matchCustomerCol, controller);
	}
	else 
	{
		matchCustomerCol = controller->orderCustomerPairListByNthClosestDepotDesc(n, matchCustomerCol);

		matchCustomerCol = controller->assignDepotToCustomerPairsUntilCapacityIsComplete(currentDepotPair, matchCustomerCol, this);
		this->assignMatchCustomerListToFLValues(matchCustomerCol, controller);
	}

	matchCustomerCol->unReferenceFrogObjectCol();
	delete matchCustomerCol;
}

//from a given depot, we choose the closest customer that satisfies that the closest depot is also the same given depot
//after we take each customer of the matchcol list and assign the closest cluster.
void FrogLeapSolution::assignRandomFeasibleDepot6(FrogLeapController * controller, Pair * currentDepotPair, FrogObjectCol * depotListOrderedByCapacity)
{
	float u = -1, result = -1;
	Pair * depotPairSelected = NULL;
	int lowCustomerBoundIndex = -1;
	int n = 2;

	//FrogObjectCol * localDepotCol = controller->createDepotListOrderedByCapacity();

	// get the depot current capacity
	//int depotCurrentCapacity = currentDepotPair->get_j_IntValue();

	// get the index of the first customer with lower distance from current depot to be attended by current depot
	FrogObjectCol * matchCustomerCol = controller->createMatchCustomerList(currentDepotPair);

	int totalCustomerDemand = controller->getTotalDemandOrCapacity(matchCustomerCol);

	int depotRemainingCapacity = currentDepotPair->get_j_IntValue();

	if (totalCustomerDemand <= depotRemainingCapacity)
	{
		//controller->assignDepotToCustomerPairs(currentDepotPair, matchCustomerCol);
		controller->assignCustomersToDepot(currentDepotPair, matchCustomerCol, depotListOrderedByCapacity, this);

		//this->assignMatchCustomerList(matchCustomerCol, controller);
	}
	else
	{
		matchCustomerCol = controller->orderCustomerPairListByNthClosestDepotDesc(n, matchCustomerCol);

		matchCustomerCol = controller->selectCustomerPairsUntilDemandComplete(currentDepotPair, matchCustomerCol);
		controller->assignCustomersToDepot(currentDepotPair, matchCustomerCol, depotListOrderedByCapacity, this);

		//this->assignMatchCustomerList(matchCustomerCol, controller);
	}

	matchCustomerCol->unReferenceFrogObjectCol();
	delete matchCustomerCol;
}

//from a given depot, we choose the closest set of customers that satisfies that the closest available (with remaining capacity enough) depot
// is also the same given depot.
//Later we take each customer of the matchcol list and assign the previous depot.
void FrogLeapSolution::assignRandomFeasibleDepot7(FrogLeapController* controller, Pair* currentDepotPair, FrogObjectCol* depotListOrderedByCapacity)
{
	float u = -1, result = -1;
	Pair* depotPairSelected = NULL;
	int lowCustomerBoundIndex = -1;
	int n = 2;

	// get the index of the first customer with lower distance from current depot to be attended by current depot
	FrogObjectCol* matchCustomerCol = controller->createFullMatchCustomerList(currentDepotPair);

	// all customers that fit in depot are assigned. The rest of customers are left without any assignation
	FrogObjectCol * assignedCustomers = controller->assignDepotToCustomerPairsUntilCapacityIsComplete(currentDepotPair, matchCustomerCol, this);

	//FrogObjectCol* assignedCustomers = controller->assignDepotRndToCustomerPairsUntilCapacityIsComplete(currentDepotPair, matchCustomerCol, this);

	matchCustomerCol->unReferenceFrogObjectCol();
	delete matchCustomerCol;

	assignedCustomers->unReferenceFrogObjectCol();
	delete assignedCustomers;
}


// CLUSTER
//from a given depot, we choose the closest set of customers that satisfies that the closest available (with remaining capacity enough) depot
// is also the same given depot.
//Later we take each customer of the matchcol list and assign the previous depot.
void FrogLeapSolution::assignRandomFeasibleDepot8(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity)
{
	float u = -1, result = -1;
	Pair* depotPairSelected = NULL;
	int lowCustomerBoundIndex = -1;
	int n = 2;

	// get the index of the first customer with lower distance from current depot to be attended by current depot
	//FrogObjectCol* matchCustomerCol = controller->createFullMatchCustomerList(currentDepotPair);
	FrogObjectCol* matchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);

	// all customers that fit in depot are assigned. The rest of customers are left without any assignation
	FrogObjectCol* assignedCustomers = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, matchCustomerCol, this);

	//FrogObjectCol* assignedCustomers = controller->assignDepotRndToCustomerPairsUntilCapacityIsComplete(currentDepotPair, matchCustomerCol, this);

	matchCustomerCol->unReferenceFrogObjectCol();
	delete matchCustomerCol;

	assignedCustomers->unReferenceFrogObjectCol();
	delete assignedCustomers;
}

// CLUSTER: full match with depot and then full match with nearest
//from a given depot, we choose the closest set of customers that satisfies that the closest available (with remaining capacity enough) depot
// is also the same given depot.
//Later we take each customer of the matchcol list and assign the previous depot.
void FrogLeapSolution::assignRandomFeasibleDepot9(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity)
{
	float u = -1, result = -1;
	Pair* depotPairSelected = NULL;
	int lowCustomerBoundIndex = -1;
	int n = 2;

	controller->setRassignCustomerSettings(false);

	// get the index of the first customer with lower distance from current depot to be attended by current depot
	//FrogObjectCol* matchCustomerCol = controller->createFullMatchCustomerList(currentDepotPair);
	FrogObjectCol* matchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);

	// all customers that fit in depot are assigned. The rest of customers are left without any assignation
	FrogObjectCol* assignedCustomers = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, matchCustomerCol, this);

	//FrogObjectCol* assignedCustomers = controller->assignDepotRndToCustomerPairsUntilCapacityIsComplete(currentDepotPair, matchCustomerCol, this);

	if((matchCustomerCol->getSize() - assignedCustomers->getSize()) > 0)
	{
		this->updateClusterCollectionWithDistanceType(DistanceType::nearest);
		FrogObjectCol * secondMatchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);
		FrogObjectCol* assignedCustomers2 = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, secondMatchCustomerCol, this);

		secondMatchCustomerCol->unReferenceFrogObjectCol();
		delete secondMatchCustomerCol;

		assignedCustomers2->unReferenceFrogObjectCol();
		delete assignedCustomers2;
	}
	

	matchCustomerCol->unReferenceFrogObjectCol();
	delete matchCustomerCol;

	assignedCustomers->unReferenceFrogObjectCol();
	delete assignedCustomers;
}

// CLUSTER: full match with depot and then full match with nearest
//from a given depot, we choose the closest set of customers that satisfies that the closest available (with remaining capacity enough) depot
// is also the same given depot.
//Later we take each customer of the matchcol list and assign the previous depot.
void FrogLeapSolution::assignRandomFeasibleDepot10(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity)
{
	float u = -1, result = -1;
	Pair* depotPairSelected = NULL;
	int lowCustomerBoundIndex = -1;
	int n = 2;

	controller->setRassignCustomerSettings(false);

	// get the index of the first customer with lower distance from current depot to be attended by current depot
	//FrogObjectCol* matchCustomerCol = controller->createFullMatchCustomerList(currentDepotPair);
	FrogObjectCol* matchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);

	// all customers that fit in depot are assigned. The rest of customers are left without any assignation
	FrogObjectCol* assignedCustomers = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, matchCustomerCol, this);

	//FrogObjectCol* assignedCustomers = controller->assignDepotRndToCustomerPairsUntilCapacityIsComplete(currentDepotPair, matchCustomerCol, this);

	if ((matchCustomerCol->getSize() - assignedCustomers->getSize()) > 0)
	{
		this->updateClusterCollectionWithDistanceType(DistanceType::furthest);
		FrogObjectCol* secondMatchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);
		FrogObjectCol* assignedCustomers2 = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, secondMatchCustomerCol, this);

		secondMatchCustomerCol->unReferenceFrogObjectCol();
		delete secondMatchCustomerCol;

		assignedCustomers2->unReferenceFrogObjectCol();
		delete assignedCustomers2;
	}


	matchCustomerCol->unReferenceFrogObjectCol();
	delete matchCustomerCol;

	assignedCustomers->unReferenceFrogObjectCol();
	delete assignedCustomers;
}

// CLUSTER: full match with depot and then full match with nearest
//from a given depot, we choose the closest set of customers that satisfies that the closest available (with remaining capacity enough) depot
// is also the same given depot.
//Later we take each customer of the matchcol list and assign the previous depot.
void FrogLeapSolution::assignRandomFeasibleDepot11(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity)
{
	float u = -1, result = -1;
	Pair* depotPairSelected = NULL;
	int lowCustomerBoundIndex = -1;
	int n = 2;

	controller->setRassignCustomerSettings(false);

	// get the index of the first customer with lower distance from current depot to be attended by current depot
	//FrogObjectCol* matchCustomerCol = controller->createFullMatchCustomerList(currentDepotPair);
	FrogObjectCol* matchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);

	// all customers that fit in depot are assigned. The rest of customers are left without any assignation
	FrogObjectCol* assignedCustomers = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, matchCustomerCol, this);

	//FrogObjectCol* assignedCustomers = controller->assignDepotRndToCustomerPairsUntilCapacityIsComplete(currentDepotPair, matchCustomerCol, this);

	if ((matchCustomerCol->getSize() - assignedCustomers->getSize()) > 0)
	{
		this->updateClusterCollectionWithDistanceType(DistanceType::furthest);
		FrogObjectCol* secondMatchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);
		FrogObjectCol* assignedCustomers2 = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, secondMatchCustomerCol, this);

		secondMatchCustomerCol->unReferenceFrogObjectCol();
		delete secondMatchCustomerCol;

		assignedCustomers2->unReferenceFrogObjectCol();
		delete assignedCustomers2;
	}


	matchCustomerCol->unReferenceFrogObjectCol();
	delete matchCustomerCol;

	assignedCustomers->unReferenceFrogObjectCol();
	delete assignedCustomers;
}

// CLUSTER: full match with depot and then full match with nearest
//from a given depot, we choose the closest set of customers that satisfies that the closest available (with remaining capacity enough) depot
// is also the same given depot.
//Later we take each customer of the matchcol list and assign the previous depot.
void FrogLeapSolution::assignRandomFeasibleDepot12(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity)
{
	float u = -1, result = -1;
	Pair* depotPairSelected = NULL;
	int lowCustomerBoundIndex = -1;
	int n = 2;

	controller->setRassignCustomerSettings(false);

	// get the index of the first customer with lower distance from current depot to be attended by current depot
	//FrogObjectCol* matchCustomerCol = controller->createFullMatchCustomerList(currentDepotPair);
	FrogObjectCol* matchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);

	// all customers that fit in depot are assigned. The rest of customers are left without any assignation
	FrogObjectCol* assignedCustomers = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, matchCustomerCol, this);

	//FrogObjectCol* assignedCustomers = controller->assignDepotRndToCustomerPairsUntilCapacityIsComplete(currentDepotPair, matchCustomerCol, this);

	if ((matchCustomerCol->getSize() - assignedCustomers->getSize()) > 0)
	{
		this->updateClusterCollectionWithDistanceType(DistanceType::nearest);
		FrogObjectCol* secondMatchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);
		FrogObjectCol* assignedCustomers2 = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, secondMatchCustomerCol, this);

		secondMatchCustomerCol->unReferenceFrogObjectCol();
		delete secondMatchCustomerCol;

		assignedCustomers2->unReferenceFrogObjectCol();
		delete assignedCustomers2;
	}


	matchCustomerCol->unReferenceFrogObjectCol();
	delete matchCustomerCol;

	assignedCustomers->unReferenceFrogObjectCol();
	delete assignedCustomers;
}

// CLUSTER: full match with depot and then full match with nearest
//from a given depot, we choose the closest set of customers that satisfies that the closest available (with remaining capacity enough) depot
// is also the same given depot.
//Later we take each customer of the matchcol list and assign the previous depot.
void FrogLeapSolution::assignRandomFeasibleDepot13(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity)
{
	float u = -1, result = -1;
	Pair* depotPairSelected = NULL;
	int lowCustomerBoundIndex = -1;
	int n = 2;

	controller->setRassignCustomerSettings(true);

	// get the index of the first customer with lower distance from current depot to be attended by current depot
	//FrogObjectCol* matchCustomerCol = controller->createFullMatchCustomerList(currentDepotPair);
	FrogObjectCol* matchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);

	// all customers that fit in depot are assigned. The rest of customers are left without any assignation
	FrogObjectCol* assignedCustomers = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, matchCustomerCol, this);

	//FrogObjectCol* assignedCustomers = controller->assignDepotRndToCustomerPairsUntilCapacityIsComplete(currentDepotPair, matchCustomerCol, this);

	if ((matchCustomerCol->getSize() - assignedCustomers->getSize()) > 0)
	{
		this->updateClusterCollectionWithDistanceType(DistanceType::nearest);
		FrogObjectCol* secondMatchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);
		FrogObjectCol* assignedCustomers2 = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, secondMatchCustomerCol, this);

		secondMatchCustomerCol->unReferenceFrogObjectCol();
		delete secondMatchCustomerCol;

		assignedCustomers2->unReferenceFrogObjectCol();
		delete assignedCustomers2;
	}


	matchCustomerCol->unReferenceFrogObjectCol();
	delete matchCustomerCol;

	assignedCustomers->unReferenceFrogObjectCol();
	delete assignedCustomers;
}

// CLUSTER: full match with depot and then full match with nearest
//from a given depot, we choose the closest set of customers that satisfies that the closest available (with remaining capacity enough) depot
// is also the same given depot.
//Later we take each customer of the matchcol list and assign the previous depot.
void FrogLeapSolution::assignRandomFeasibleDepot14(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity)
{
	float u = -1, result = -1;
	Pair* depotPairSelected = NULL;
	int lowCustomerBoundIndex = -1;
	int n = 2;

	controller->setRassignCustomerSettings(true);

	// get the index of the first customer with lower distance from current depot to be attended by current depot
	//FrogObjectCol* matchCustomerCol = controller->createFullMatchCustomerList(currentDepotPair);
	FrogObjectCol* matchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);

	//matchCustomerCol = controller->reorderCustomerListByDemandPerDistance(matchCustomerCol, currentCluster);

	//printf("Showing matchCustomerCol ordered by demand \n");
	//matchCustomerCol->printFrogObjCol();
	//printf("\n");
	
	int depotRemainingCapacity = currentCluster->getDepotRemainingCapacity();

	// reorder customer by the following value = customer_demand / distance
	
	// all customers that fit in depot are assigned. The rest of customers are left without any assignation
	FrogObjectCol* assignedCustomers = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, matchCustomerCol, this);

	int demandOfAssignedCustomers = controller->getDemandFromCustomerList(assignedCustomers);
	
	depotRemainingCapacity = currentCluster->getDepotRemainingCapacity();

	int depotCapacity = currentCluster->getDepotCapacity();

	if(demandOfAssignedCustomers > depotCapacity || depotRemainingCapacity < 0)
	{
		printf("ERROR DE CAPACIDAD \n");		
		// Terminate the program
		exit(EXIT_FAILURE);
	}

	//ESTA LINEA NO SE DEBE DESCOMENTAR FrogObjectCol* assignedCustomers = controller->assignDepotRndToCustomerPairsUntilCapacityIsComplete(currentDepotPair, matchCustomerCol, this);
	//printf("Showing DepotPair information \n");
	//currentCluster->getDepotPair()->printFrogObj();
	//printf("\n");

	//printf("Showing demand of assignedCustomerList: %d \n", demandOfAssignedCustomers);
	//
	//printf("showing matchcolCustomerCol \n");
	//matchCustomerCol->printFrogObjCol();
	//printf("\n");

	//printf("showing assignedCustomers \n");
	//assignedCustomers->printFrogObjCol();
	//printf("\n");

	
	if ((matchCustomerCol->getSize() - assignedCustomers->getSize()) > 0)
	{
		this->updateClusterCollectionWithDistanceType(DistanceType::nearest);
		
		depotRemainingCapacity = currentCluster->getDepotRemainingCapacity();
		FrogObjectCol* secondMatchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);

		//secondMatchCustomerCol = controller->reorderCustomerListByDemandPerDistance(secondMatchCustomerCol, currentCluster);

		//printf("Showing secondMatchCustomerCol \n");
		//secondMatchCustomerCol->printFrogObjCol();
		//printf("\n");

		depotRemainingCapacity = currentCluster->getDepotRemainingCapacity();
		FrogObjectCol* assignedCustomers2 = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, secondMatchCustomerCol, this);
		
		demandOfAssignedCustomers = controller->getDemandFromCustomerList(assignedCustomers2);
		depotRemainingCapacity = currentCluster->getDepotRemainingCapacity();
		depotCapacity = currentCluster->getDepotCapacity();

		if (demandOfAssignedCustomers > depotCapacity || depotRemainingCapacity < 0)
		{
			printf("ERROR DE CAPACIDAD \n");
			// Terminate the program
			exit(EXIT_FAILURE);
		}

		
		//if ((secondMatchCustomerCol->getSize() - assignedCustomers2->getSize()) > 0)
		//{
		//	this->updateClusterCollectionWithDistanceType(DistanceType::depot);

		//	depotRemainingCapacity = currentCluster->getDepotRemainingCapacity();
		//	FrogObjectCol* thirdMatchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);

		//	depotRemainingCapacity = currentCluster->getDepotRemainingCapacity();
		//	FrogObjectCol* assignedCustomers3 = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, secondMatchCustomerCol, this);

		//	demandOfAssignedCustomers = controller->getDemandFromCustomerList(assignedCustomers2);
		//	depotRemainingCapacity = currentCluster->getDepotRemainingCapacity();
		//	depotCapacity = currentCluster->getDepotCapacity();

		//	if (demandOfAssignedCustomers > depotCapacity || depotRemainingCapacity < 0)
		//	{
		//		printf("ERROR DE CAPACIDAD \n");
		//		// Terminate the program
		//		exit(EXIT_FAILURE);
		//	}

		//	thirdMatchCustomerCol->unReferenceFrogObjectCol();
		//	delete thirdMatchCustomerCol;

		//	assignedCustomers3->unReferenceFrogObjectCol();
		//	delete assignedCustomers3;
		//}
		
		secondMatchCustomerCol->unReferenceFrogObjectCol();
		delete secondMatchCustomerCol;

		assignedCustomers2->unReferenceFrogObjectCol();
		delete assignedCustomers2;
	}

	matchCustomerCol->unReferenceFrogObjectCol();
	delete matchCustomerCol;

	assignedCustomers->unReferenceFrogObjectCol();
	delete assignedCustomers;
}

// CLUSTER: full match with depot and then full match with nearest
//from a given depot, we choose the closest set of customers that satisfies that the closest available (with remaining capacity enough) depot
// is also the same given depot.
//Later we take each customer of the matchcol list and assign the previous depot.
void FrogLeapSolution::assignRandomFeasibleDepot15(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity)
{
	float u = -1, result = -1;
	Pair* depotPairSelected = NULL;
	int lowCustomerBoundIndex = -1;
	int n = 2;

	controller->setRassignCustomerSettings(true);

	// get the index of the first customer with lower distance from current depot to be attended by current depot
	//FrogObjectCol* matchCustomerCol = controller->createFullMatchCustomerList(currentDepotPair);
	FrogObjectCol* matchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);

	// all customers that fit in depot are assigned. The rest of customers are left without any assignation
	FrogObjectCol* assignedCustomers = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, matchCustomerCol, this);

	//FrogObjectCol* assignedCustomers = controller->assignDepotRndToCustomerPairsUntilCapacityIsComplete(currentDepotPair, matchCustomerCol, this);

	if ((matchCustomerCol->getSize() - assignedCustomers->getSize()) > 0)
	{
		this->updateClusterCollectionWithDistanceType(DistanceType::depot);
		FrogObjectCol* secondMatchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);
		FrogObjectCol* assignedCustomers2 = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, secondMatchCustomerCol, this);

		secondMatchCustomerCol->unReferenceFrogObjectCol();
		delete secondMatchCustomerCol;

		assignedCustomers2->unReferenceFrogObjectCol();
		delete assignedCustomers2;
	}


	matchCustomerCol->unReferenceFrogObjectCol();
	delete matchCustomerCol;

	assignedCustomers->unReferenceFrogObjectCol();
	delete assignedCustomers;
}

void FrogLeapSolution::assignRandomFeasibleDepot16(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity)
{
	float u = -1, result = -1;
	Pair* depotPairSelected = NULL;
	int lowCustomerBoundIndex = -1;
	int n = 2;

	controller->setRassignCustomerSettings(true);

	// get the index of the first customer with lower distance from current depot to be attended by current depot
	//FrogObjectCol* matchCustomerCol = controller->createFullMatchCustomerList(currentDepotPair);
	FrogObjectCol* matchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);

	// all customers that fit in depot are assigned. The rest of customers are left without any assignation
	FrogObjectCol* assignedCustomers = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, matchCustomerCol, this);

	//FrogObjectCol* assignedCustomers = controller->assignDepotRndToCustomerPairsUntilCapacityIsComplete(currentDepotPair, matchCustomerCol, this);

	if ((matchCustomerCol->getSize() - assignedCustomers->getSize()) > 0)
	{
		this->updateClusterCollectionWithDistanceType(DistanceType::furthest);
		FrogObjectCol* secondMatchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);
		FrogObjectCol* assignedCustomers2 = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, secondMatchCustomerCol, this);

		secondMatchCustomerCol->unReferenceFrogObjectCol();
		delete secondMatchCustomerCol;

		assignedCustomers2->unReferenceFrogObjectCol();
		delete assignedCustomers2;
	}


	matchCustomerCol->unReferenceFrogObjectCol();
	delete matchCustomerCol;

	assignedCustomers->unReferenceFrogObjectCol();
	delete assignedCustomers;
}

void FrogLeapSolution::assignRandomFeasibleDepot17(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity)
{
	float u = -1, result = -1;
	Pair* depotPairSelected = NULL;
	int lowCustomerBoundIndex = -1;
	int n = 2;

	controller->setRassignCustomerSettings(true);

	// get the index of the first customer with lower distance from current depot to be attended by current depot
	//FrogObjectCol* matchCustomerCol = controller->createFullMatchCustomerList(currentDepotPair);
	FrogObjectCol* matchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);

	// all customers that fit in depot are assigned. The rest of customers are left without any assignation
	FrogObjectCol* assignedCustomers = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, matchCustomerCol, this);

	//FrogObjectCol* assignedCustomers = controller->assignDepotRndToCustomerPairsUntilCapacityIsComplete(currentDepotPair, matchCustomerCol, this);

	if ((matchCustomerCol->getSize() - assignedCustomers->getSize()) > 0)
	{
		this->updateClusterCollectionWithDistanceType(DistanceType::nearest);
		FrogObjectCol* secondMatchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);
		FrogObjectCol* assignedCustomers2 = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, secondMatchCustomerCol, this);

		secondMatchCustomerCol->unReferenceFrogObjectCol();
		delete secondMatchCustomerCol;

		assignedCustomers2->unReferenceFrogObjectCol();
		delete assignedCustomers2;
	}


	matchCustomerCol->unReferenceFrogObjectCol();
	delete matchCustomerCol;

	assignedCustomers->unReferenceFrogObjectCol();
	delete assignedCustomers;
}

void FrogLeapSolution::assignRandomFeasibleDepot18(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity)
{
	float u = -1, result = -1;
	Pair* depotPairSelected = NULL;
	int lowCustomerBoundIndex = -1;
	int n = 2;

	controller->setRassignCustomerSettings(true);

	// get the index of the first customer with lower distance from current depot to be attended by current depot
	//FrogObjectCol* matchCustomerCol = controller->createFullMatchCustomerList(currentDepotPair);
	FrogObjectCol* matchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);

	// all customers that fit in depot are assigned. The rest of customers are left without any assignation
	FrogObjectCol* assignedCustomers = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, matchCustomerCol, this);

	//FrogObjectCol* assignedCustomers = controller->assignDepotRndToCustomerPairsUntilCapacityIsComplete(currentDepotPair, matchCustomerCol, this);

	if ((matchCustomerCol->getSize() - assignedCustomers->getSize()) > 0)
	{
		this->updateClusterCollectionWithDistanceType(DistanceType::depot);
		FrogObjectCol* secondMatchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);
		FrogObjectCol* assignedCustomers2 = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, secondMatchCustomerCol, this);

		secondMatchCustomerCol->unReferenceFrogObjectCol();
		delete secondMatchCustomerCol;

		assignedCustomers2->unReferenceFrogObjectCol();
		delete assignedCustomers2;
	}


	matchCustomerCol->unReferenceFrogObjectCol();
	delete matchCustomerCol;

	assignedCustomers->unReferenceFrogObjectCol();
	delete assignedCustomers;
}

void FrogLeapSolution::assignRandomFeasibleDepot19(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity)
{
	float u = -1, result = -1;
	Pair* depotPairSelected = NULL;
	int lowCustomerBoundIndex = -1;
	int n = 2;

	controller->setRassignCustomerSettings(true);

	// get the index of the first customer with lower distance from current depot to be attended by current depot
	//FrogObjectCol* matchCustomerCol = controller->createFullMatchCustomerList(currentDepotPair);
	FrogObjectCol* matchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);

	// all customers that fit in depot are assigned. The rest of customers are left without any assignation
	FrogObjectCol* assignedCustomers = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, matchCustomerCol, this);

	//FrogObjectCol* assignedCustomers = controller->assignDepotRndToCustomerPairsUntilCapacityIsComplete(currentDepotPair, matchCustomerCol, this);

	if ((matchCustomerCol->getSize() - assignedCustomers->getSize()) > 0)
	{
		this->updateClusterCollectionWithDistanceType(DistanceType::furthest);
		FrogObjectCol* secondMatchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);
		FrogObjectCol* assignedCustomers2 = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, secondMatchCustomerCol, this);

		secondMatchCustomerCol->unReferenceFrogObjectCol();
		delete secondMatchCustomerCol;

		assignedCustomers2->unReferenceFrogObjectCol();
		delete assignedCustomers2;
	}


	matchCustomerCol->unReferenceFrogObjectCol();
	delete matchCustomerCol;

	assignedCustomers->unReferenceFrogObjectCol();
	delete assignedCustomers;
}

// CLUSTER: full match with depot and then full match with nearest
//from a given depot, we choose the closest set of customers that satisfies that the closest available (with remaining capacity enough) depot
// is also the same given depot.
//Later we take each customer of the matchcol list and assign the previous depot.
void FrogLeapSolution::assignRandomFeasibleDepot20(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity)
{
	float u = -1, result = -1;
	Pair* depotPairSelected = NULL;
	int lowCustomerBoundIndex = -1;
	int n = 2;

	controller->setRassignCustomerSettings(true);

	// get the index of the first customer with lower distance from current depot to be attended by current depot
	//FrogObjectCol* matchCustomerCol = controller->createFullMatchCustomerList(currentDepotPair);
	FrogObjectCol* matchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);

	// all customers that fit in depot are assigned. The rest of customers are left without any assignation
	FrogObjectCol* assignedCustomers = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, matchCustomerCol, this);

	//FrogObjectCol* assignedCustomers = controller->assignDepotRndToCustomerPairsUntilCapacityIsComplete(currentDepotPair, matchCustomerCol, this);

	if ((matchCustomerCol->getSize() - assignedCustomers->getSize()) > 0)
	{
		this->updateClusterCollectionWithDistanceType(DistanceType::nearest);
		FrogObjectCol* secondMatchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);
		FrogObjectCol* assignedCustomers2 = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, secondMatchCustomerCol, this);

		
		if((secondMatchCustomerCol->getSize() - assignedCustomers2->getSize()) > 0)
		{
			this->updateClusterCollectionWithDistanceType(DistanceType::nearest);
			FrogObjectCol* thirdMatchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);
			FrogObjectCol* assignedCustomers3 = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, thirdMatchCustomerCol, this);

			thirdMatchCustomerCol->unReferenceFrogObjectCol();
			delete thirdMatchCustomerCol;

			assignedCustomers3->unReferenceFrogObjectCol();
			delete assignedCustomers3;
		}
		
		secondMatchCustomerCol->unReferenceFrogObjectCol();
		delete secondMatchCustomerCol;

		assignedCustomers2->unReferenceFrogObjectCol();
		delete assignedCustomers2;
	}


	matchCustomerCol->unReferenceFrogObjectCol();
	delete matchCustomerCol;

	assignedCustomers->unReferenceFrogObjectCol();
	delete assignedCustomers;
}

// CLUSTER: full match with depot and then full match with nearest
//from a given depot, we choose the closest set of customers that satisfies that the closest available (with remaining capacity enough) depot
// is also the same given depot.
//Later we take each customer of the matchcol list and assign the previous depot.
void FrogLeapSolution::assignRandomFeasibleDepot21(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity)
{
	float u = -1, result = -1;
	Pair* depotPairSelected = NULL;
	int lowCustomerBoundIndex = -1;
	int n = 2;

	controller->setRassignCustomerSettings(true);

	// get the index of the first customer with lower distance from current depot to be attended by current depot
	//FrogObjectCol* matchCustomerCol = controller->createFullMatchCustomerList(currentDepotPair);
	FrogObjectCol* matchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);

	// all customers that fit in depot are assigned. The rest of customers are left without any assignation
	FrogObjectCol* assignedCustomers = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, matchCustomerCol, this);

	//FrogObjectCol* assignedCustomers = controller->assignDepotRndToCustomerPairsUntilCapacityIsComplete(currentDepotPair, matchCustomerCol, this);

	if ((matchCustomerCol->getSize() - assignedCustomers->getSize()) > 0)
	{
		this->updateClusterCollectionWithDistanceType(DistanceType::nearest);
		FrogObjectCol* secondMatchCustomerCol = controller->createMatchCustomerList_Cluster(currentCluster, this);
		FrogObjectCol* assignedCustomers2 = controller->assignCustomersToClusterUntilCapacityIsComplete(currentCluster, secondMatchCustomerCol, this);

		secondMatchCustomerCol->unReferenceFrogObjectCol();
		delete secondMatchCustomerCol;

		assignedCustomers2->unReferenceFrogObjectCol();
		delete assignedCustomers2;
	}


	matchCustomerCol->unReferenceFrogObjectCol();
	delete matchCustomerCol;

	assignedCustomers->unReferenceFrogObjectCol();
	delete assignedCustomers;
}

void FrogLeapSolution::assignRemainingCustomersToClosestCluster(FrogLeapController * controller)
{
	int size = controller->getNumberOfCustomers();

	FrogObjectCol * localDepotCol = controller->createDepotListOrderedByRemainingCapacity();

	IndexList * notAssignedCustomerIndexes = new IndexList();
	
	for(int i = 0; i < size; i++)
	{
		Pair * currentCustomerPair = (Pair *)controller->getCustomerPairByIndex(i);
		if(!controller->isCustomerPairAssigned(currentCustomerPair))
		{			
			notAssignedCustomerIndexes->addIndex(i);
			//this->values[i] = controller->assignRandomFeasibleDepot4(localDepotCol, i);
		}
	}

	int listSize = notAssignedCustomerIndexes->getSize();

	for(int j = 0; j < listSize; j++)
	{
		int currentSize = notAssignedCustomerIndexes->getSize();
		int rand_itemIndex = controller->genRandomIntNumber(0, currentSize);

		if (notAssignedCustomerIndexes->getSize() == 114)
		{
			printf("hola");
		}

		int randCustomerIndex = notAssignedCustomerIndexes->removeIndexByPosition(rand_itemIndex);
		this->values[randCustomerIndex] = controller->assignRandomFeasibleDepot4(localDepotCol, randCustomerIndex);
	}

}

FrogObjectCol * FrogLeapSolution::initializeFeasibleDepotList(FrogLeapController *controller)
{
	Pair * depotPair = NULL;
	FrogObjectCol * feasibleDepotList = new FrogObjectCol();
	int depotNum = controller->getNumberOfDepots();

	controller->resetDepotRemainingCapacities();

	for (int i = 0; i < depotNum; i++)
	{
		depotPair = controller->getDepotPairByIndex(i);
		feasibleDepotList->addLastFrogObject(depotPair);
		depotPair = NULL;
	}

	return feasibleDepotList;
}

DecodedFrogLeapSolution * FrogLeapSolution::decodeSolution(FrogLeapController * controller)
{
	controller->resetDepotRemainingCapacities();
	return this->decodeFrogLeapSolution(controller, true);
}

DecodedFrogLeapSolution * FrogLeapSolution::decodeWholeSolutionWithClosestNextCriteria(FrogLeapController * controller)
{
	// al pedo
	//controller->resetDepotRemainingCapacities();

	return this->decodeFrogLeapWholeSolutionWithClosestNextCriteria(controller, true);
}

DecodedFrogLeapSolution* FrogLeapSolution::decodeWholeSolutionWithSimplifiedClosestNextCriteria(FrogLeapController* controller)
{
	// al pedo
	//controller->resetDepotRemainingCapacities();

	return this->decodeFrogLeapWholeSolutionWithSimplifiedClosestNextCriteria(controller, true);
}

DecodedFrogLeapSolution* FrogLeapSolution::decodeWholeSolutionWithMixedCriteria(FrogLeapController* controller)
{
	return this->decodeFrogLeapWholeSolutionWithMixedCriteria(controller, true);
}

void FrogLeapSolution::printClusterContentHeader()
{
	//printf("DepotInternalId; CustomerInternalId; DepotLabelId; CustomerLabelId \n");
	printf("DepotLabelId; CustomerLabelId \n");
}

void FrogLeapSolution::printClusterColInfo()
{
	int n_clusters = this->clusterCol->getSize();
	Cluster* currentCluster;

	
	printf("SHOWING CLUSTER COLLECTION INFORMATION \n");

	this->printClusterContentHeader();

	for(int i=0; i<n_clusters; i++)
	{
		currentCluster = (Cluster*)this->clusterCol->getFrogObject(i);
		currentCluster->printFrogObj();
	}

	printf("END OF SHOWING CLUSTER COLLECTION INFORMATION \n");
}

DecodedFrogLeapSolution* FrogLeapSolution::decodeWholeSolutionWithClarkWrightCriteria2(FrogLeapController* controller)
{
	int nSize = this->clusterCol->getSize();
	this->cw_handler_col = new FrogObjectCol();

	//this->printClusterColInfo();		

	DecodedFrogLeapSolution* decodedSolution = new DecodedFrogLeapSolution(this->n_depots, controller);

	for (int i = 0; i < nSize; i++)
	{
		ClarkWrightHandler* current_cw_handler = new ClarkWrightHandler(controller, this->getClusterByIndex(i));
		current_cw_handler->cw_execute();
		this->cw_handler_col->addLastFrogObject(current_cw_handler);
	}

	//controller->resetDepotRemainingCapacities();
	decodedSolution->setUnReferenceBeforeDelete(true);
	//decodedSolution->assignCustomersToDepotLists(controller, this);
	//decodedSolution->orderCustomersWithClosestNextCriteria(controller);
	//decodedSolution->assignDecodedCustomersToVehicles(controller);
	decodedSolution->cw2_assignDecodedCustomersToVehicles(controller, this);

	return decodedSolution;
}


DecodedFrogLeapSolution * FrogLeapSolution::decodeWholeSolutionWithClarkWrightCriteria(FrogLeapController * controller)
{
	int nSize = this->clusterCol->getSize();
	this->cw_handler_col = new FrogObjectCol();

	//this->printClusterColInfo();		
	
	DecodedFrogLeapSolution* decodedSolution = new DecodedFrogLeapSolution(this->n_depots, controller);

	for(int i = 0; i < nSize; i++)
	{
		ClarkWrightHandler * current_cw_handler = new ClarkWrightHandler(controller, this->getClusterByIndex(i));
		current_cw_handler->cw_execute();
		this->cw_handler_col->addLastFrogObject(current_cw_handler);
	}
	
	//controller->resetDepotRemainingCapacities();
	decodedSolution->setUnReferenceBeforeDelete(true);
	//decodedSolution->assignCustomersToDepotLists(controller, this);
	//decodedSolution->orderCustomersWithClosestNextCriteria(controller);
	//decodedSolution->assignDecodedCustomersToVehicles(controller);
	decodedSolution->cw_assignDecodedCustomersToVehicles(controller, this);
		
	return decodedSolution;
}

int FrogLeapSolution::getCWHandlerColSize()
{
	return this->cw_handler_col->getSize();
}

FrogObjectCol * FrogLeapSolution::getCWHandlerCol()
{
	return this->cw_handler_col;
}

//ClarkWrightHandler * FrogLeapSolution::getCWHandler(int cw_index)
//{
//	return (ClarkWrightHandler *) this->cw_handler_col->getFrogObject(cw_index);
//}

// this uses the sweep algorithm to determine the routes for each depot
DecodedFrogLeapSolution * FrogLeapSolution::decodeWholeSolutionWithAngularCriteria(FrogLeapController * controller)
{
	controller->resetDepotRemainingCapacities();
	return this->decodeFrogLeapWholeSolutionWithAngularCriteria(controller);
}

// this uses the sweep algorithm to determine the routes for each depot
DecodedFrogLeapSolution * FrogLeapSolution::decodeSolutionWithAngularCriteria(FrogLeapController * controller)
{
	controller->resetDepotRemainingCapacities();
	return this->decodeFrogLeapSolutionWithAngularCriteria(controller, true);
}

//if generated instance of DecodedFrogLeapSolution is NULL then solution is not valid due to a vehicle capacity violation
// This algorithm uses a float distance table
DecodedFrogLeapSolution * FrogLeapSolution::decodeFrogLeapSolution(FrogLeapController * controller, bool adjustVehicleRoutes)
{
	DecodedFrogLeapSolution * decodedSolution = new DecodedFrogLeapSolution(this->n_depots, controller);
	
	int i = 0, rand_i;
	bool feasible = true;

	this->initCustomerSelection(controller);

	do 
	{
		rand_i = this->selectRandomCustomerIndex(i, controller);
		feasible = decodedSolution->decodeFrogLeapItem(controller, this->getFLValue(rand_i), rand_i, this->n_depots);
		i++;
	} while (i < this->getSize() && feasible == true);	
		
	this->destroyRandomCustomerSelectionList();
	
	if(adjustVehicleRoutes)
	{
		decodedSolution->adjustVehicleRoutes(controller);

	}	

	return decodedSolution;
}


//if generated instance of DecodedFrogLeapSolution is NULL then solution is not valid due to a vehicle capacity violation
// This algorithm uses a float distance table
DecodedFrogLeapSolution * FrogLeapSolution::decodeFrogLeapWholeSolutionWithClosestNextCriteria(FrogLeapController * controller, bool adjustVehicleRoutes)
{
	DecodedFrogLeapSolution * decodedSolution = new DecodedFrogLeapSolution(this->n_depots, controller);

	decodedSolution->setUnReferenceBeforeDelete(true);

	decodedSolution->assignCustomersToDepotLists(controller, this);

	decodedSolution->orderCustomersWithClosestNextCriteria(controller);

	decodedSolution->assignDecodedCustomersToVehicles(controller);

	return decodedSolution;
}

DecodedFrogLeapSolution* FrogLeapSolution::decodeFrogLeapWholeSolutionWithSimplifiedClosestNextCriteria(FrogLeapController* controller, bool adjustVehicleRoutes)
{
	DecodedFrogLeapSolution* decodedSolution = new DecodedFrogLeapSolution(this->n_depots, controller);

	decodedSolution->setUnReferenceBeforeDelete(true);

	decodedSolution->assignCustomersToDepotLists(controller, this);

	decodedSolution->orderCustomersWithClosestNextCriteria(controller);

	decodedSolution->scn_assignDecodedCustomersToVehicles(controller);

	return decodedSolution;
}

DecodedFrogLeapSolution* FrogLeapSolution::decodeFrogLeapWholeSolutionWithMixedCriteria(FrogLeapController* controller, bool adjustVehicleRoutes)
{
	DecodedFrogLeapSolution* decodedSolution = new DecodedFrogLeapSolution(this->n_depots, controller);

	decodedSolution->setUnReferenceBeforeDelete(true);

	decodedSolution->assignCustomersToDepotLists(controller, this);

	decodedSolution->orderCustomersWithClosestNextCriteria(controller);

	decodedSolution->mixed_assignDecodedCustomersToVehicles(controller);

	return decodedSolution;
}

// uses the sweep algorithm to assign customer order for each depot
// Here we also assign vehicles to customer according to the angular order of customers
DecodedFrogLeapSolution * FrogLeapSolution::decodeFrogLeapWholeSolutionWithAngularCriteria(FrogLeapController * controller)
{

	DecodedFrogLeapSolution * decodedSolution = new DecodedFrogLeapSolution(this->n_depots, controller);

	controller->resetDepotRemainingCapacities();
	
	decodedSolution->setUnReferenceBeforeDelete(true);

	int i = 0, rand_i = -1;
	bool feasible = true;

	do
	{
		//rand_i = this->selectRandomCustomerIndex(i, controller);
		feasible = decodedSolution->decodeFrogLeapItemToListWithAngularCriteria(controller, this->getFLValue(i), i, this->n_depots);
		i++;
	} while (i < this->getSize() && feasible == true);

	decodedSolution->assignDecodedCustomersToVehicles(controller);
	
	//this->destroyRandomCustomerSelectionList();
	return decodedSolution;
}

// uses the sweep algorithm to assign customer order for each depot
// Here we also assign vehicles to customer according to the angular order of customers
DecodedFrogLeapSolution * FrogLeapSolution::decodeFrogLeapSolutionWithAngularCriteria(FrogLeapController * controller, bool adjustVehicleRoutes)
{
	
	DecodedFrogLeapSolution * decodedSolution = new DecodedFrogLeapSolution(this->n_depots, controller);

	int i = 0, rand_i;
	bool feasible = true;

	//this->initCustomerSelection(controller);

	do
	{
		//rand_i = this->selectRandomCustomerIndex(i, controller);
		feasible = decodedSolution->decodeFrogLeapItemWithAngularCriteria(controller, this->getFLValue(i), i, this->n_depots);
		i++;
	} while (i < this->getSize() && feasible == true);

	//decodedSolution->assignDecodedCustomersToVehicles(controller);

	return decodedSolution;
}



void FrogLeapSolution::setSolutionGenerationType(SolutionGenerationType v_sgt)
{
	this->sgt = v_sgt;
}

SolutionGenerationType FrogLeapSolution::getSolutionGenerationType()
{
	return this->sgt;
}

// inherited methods
void FrogLeapSolution::printFrogObj()
{
	printf("\nPrinting values of frog leaping solution \n");
	printf("Values are the following: \n");

	for(int i = 0; i < this->getSize() - 1;i++)
	{
		printf("%.4f \n ", this->values[i]);
	}

	//printing the last element (replacing the comma by the point)
	printf("%.4f. \n", this->values[this->getSize() - 1]);	
}

bool FrogLeapSolution::isTheSame(FrogObject * fs)
{
	FrogLeapSolution * fls = (FrogLeapSolution *)fs;

	if (this->getSize() != fls->getSize())
	{
		return false;
	}

	for (int i = 0; i < this->getSize(); i++)
	{
		if (this->values[i] != fls->values[i])
			return false;		
	}

	return true;
}

int FrogLeapSolution::selectRandomCustomerIndex(int i, FrogLeapController * controller)
{
	// select a random depot between the set of customers
	int size = this->customerSelectionList->getSize();
	int customerIndex;
	float u;

	do
	{
		u = controller->genRandomFloatingNumber(i, size);
	} while (u >= size);

	int positionSelected = floor(u);

	customerIndex = this->customerSelectionList->getSolFactValue(positionSelected);

	this->customerSelectionList->swapItems(i, positionSelected);

	return customerIndex;
}

int FrogLeapSolution::selectRandomDepotIndex(int i, FeasibleSolution * fs, FrogLeapController * controller)
{
	// select a random depot between the set of customers
	int size = fs->getSize();
	int depotIndex;
	float u;

	do
	{
		u = controller->genRandomFloatingNumber(i, size);
	} while (u >= size);

	int positionSelected = floor(u);

	depotIndex = fs->getSolFactValue(positionSelected);

	fs->swapItems(i, positionSelected);

	return depotIndex;
}

void FrogLeapSolution::assignMatchCustomerListToFLValues(FrogObjectCol * matchCustomerCol, FrogLeapController * controller)
{
	int size = matchCustomerCol->getSize();
	for(int i = 0; i < size; i++)
	{
		Pair * customerPair = (Pair *)matchCustomerCol->getFrogObject(i);
		int customerIndex = controller->getCustomerListIndexByInternal(customerPair->getId());

		this->setFLValue(customerIndex, controller->addRandomNumberToInt(customerPair->getAssignedDepotIndex()));
	}
}

void FrogLeapSolution::writeFrogLeapSolution(FrogLeapController * controller)
{
	FILE * pFile = controller->getPFile();

	fprintf(pFile, "\nPrinting values of frog leaping solution \n");
	printf("\nPrinting values of frog leaping solution \n");
	fprintf(pFile, "Values are the following: \n");
	printf("Values are the following: \n");

	for (int i = 0; i < this->getSize() - 1; i++)
	{
		fprintf(pFile, "%.4f \n ", this->values[i]);
		printf("%.4f \n ", this->values[i]);
	}

	//printing the last element (replacing the comma by the point)
	fprintf(pFile, "%.4f. \n", this->values[this->getSize() - 1]);
	printf("%.4f. \n", this->values[this->getSize() - 1]);
}

void FrogLeapSolution::writeFLSWithSolutionDataHeader(FrogLeapController* controller)
{
	FILE* pFile = controller->getPFile();

	fprintf(pFile, "SHOWING CLUSTER INFORMATION\n");
	fprintf(pFile, "DepotIndex; DepotInternalId; DepotLabelId; DepotCapacity; DepotRemainingCap; AssignedCustomers; TotalCustomerDemands.\n");	
}

void FrogLeapSolution::printFLSWithSolutionDataHeader(FrogLeapController* controller)
{
	printf("SHOWING CLUSTER INFORMATION\n");
	printf("DepotIndex; DepotInternalId; DepotLabelId; DepotCapacity; DepotRemainingCap; AssignedCustomers; TotalCustomerDemands; TotalCustomersAssigned.\n");
}

void FrogLeapSolution::printFrogLeapSolutionWithSolutionData(FrogLeapController * controller)
{
	SolutionData* sd = new SolutionData();

	this->printFLSWithSolutionDataHeader(controller);

	Cluster* currentCluster = NULL;

	int clusterColSize = this->clusterCol->getSize();

	for(int i = 0; i < clusterColSize; i++)
	{
		currentCluster = (Cluster *)this->clusterCol->getFrogObject(i);
		Pair* depotPair = currentCluster->getDepotPair();
		sd->setDepotIndex(controller->getDepotListIndexByInternal(depotPair->getId()));
		sd->setDepotInternalId(depotPair->getId());
		sd->setDepotLabelId(controller->getLabelId(depotPair->getId()));
		sd->setDepotCap(controller->getDepotCapacityByIndex(sd->getDepotIndex()));
		sd->setDepotRemCap(controller->getDepotRemainingCapacityByIndex(sd->getDepotIndex()));
		sd->printClusterSolutionData();
		this->printAssignedCustomers(currentCluster);
	}

	delete sd;

	printf("END OF CLUSTER INFORMATION\n\n\n\n");
}

void FrogLeapSolution::writeFrogLeapSolutionWithSolutionData(FrogLeapController* controller)
{
	FILE* pFile = controller->getPFile();
	SolutionData* sd = new SolutionData();

	this->writeFLSWithSolutionDataHeader(controller);

	Cluster* currentCluster = NULL;

	int clusterColSize = this->clusterCol->getSize();

	for (int i = 0; i < clusterColSize; i++)
	{
		currentCluster = (Cluster*)this->clusterCol->getFrogObject(i);
		Pair* depotPair = currentCluster->getDepotPair();
		sd->setDepotIndex(controller->getDepotListIndexByInternal(depotPair->getId()));
		sd->setDepotInternalId(depotPair->getId());
		sd->setDepotLabelId(controller->getLabelId(depotPair->getId()));
		sd->setDepotCap(controller->getDepotCapacityByIndex(sd->getDepotIndex()));
		sd->setDepotRemCap(controller->getDepotRemainingCapacityByIndex(sd->getDepotIndex()));
		sd->writeClusterSolutionData(controller->getPFile());
		this->writeAssignedCustomers(currentCluster, controller->getPFile());
	}


	fprintf(pFile, "END OF CLUSTER INFORMATION\n");
}

void FrogLeapSolution::writeAssignedCustomers(Cluster * cluster, FILE * pFile)
{	
	int nCustomers = cluster->getCustomerCol()->getSize();
	int totalCustomerDemands = 0;
	Pair * currentCustomer = NULL;

	if (nCustomers > 0) 
	{
		for (int i = 0; i < nCustomers - 1; i++)
		{
			currentCustomer = (Pair*)cluster->getCustomerCol()->getFrogObject(i);
			fprintf(pFile, "%d(%d) - ", currentCustomer->getId(), currentCustomer->get_i_IntValue());
			totalCustomerDemands += currentCustomer->get_i_IntValue();
		}

		currentCustomer = (Pair*)cluster->getCustomerCol()->getFrogObject(nCustomers - 1);
		fprintf(pFile, "%d(%d) - END; %d \n", currentCustomer->getId(), currentCustomer->get_i_IntValue(), totalCustomerDemands);
	}
	else
	{
		fprintf(pFile, "END; %d \n", totalCustomerDemands);
	}

}

void FrogLeapSolution::printAssignedCustomers(Cluster* cluster)
{
	int nCustomers = cluster->getCustomerCol()->getSize();
	int totalCustomerDemands = 0;
	Pair* currentCustomer = NULL;

	if (nCustomers > 0)
	{
		for (int i = 0; i < nCustomers - 1; i++)
		{
			currentCustomer = (Pair*)cluster->getCustomerCol()->getFrogObject(i);
			printf("%d(%d) - ", currentCustomer->getLabelId(), currentCustomer->get_i_IntValue());
			totalCustomerDemands += currentCustomer->get_i_IntValue();
		}

		currentCustomer = (Pair*)cluster->getCustomerCol()->getFrogObject(nCustomers - 1);
		totalCustomerDemands += currentCustomer->get_i_IntValue();
		printf("%d(%d) - END; %d; %d\n", currentCustomer->getLabelId(), currentCustomer->get_i_IntValue(), totalCustomerDemands, nCustomers);
	}
	else
	{
		printf("END; %d; %d \n", totalCustomerDemands, nCustomers);
	}
}

void FrogLeapSolution::initClusterCollection(FrogLeapController* controller)
{
	this->clusterCol = new FrogObjectCol();

	int nDepots = controller->getNumberOfDepots();
	for (int i = 0; i < nDepots; i++)
	{
		Pair * depotPair = controller->getDepotPairByIndex(i);
		Cluster * currentCluster = new Cluster(depotPair, i);
		this->clusterCol->addLastFrogObject(currentCluster);
	}
}

void FrogLeapSolution::updateClusterCollectionWithDistanceType(DistanceType distance_t)
{	
	int nCluster = this->clusterCol->getSize();

	for (int i = 0; i < nCluster; i++)
	{
		Cluster * currentCluster = (Cluster *)this->clusterCol->getFrogObject(i);
		currentCluster->setDistanceType(distance_t);
	}
}

// create a feasibleSolution object (an array) with value i in position i
// feasible is assigned to attribute randomCustomerSelectionList
void FrogLeapSolution::initCustomerSelection(FrogLeapController * controller)
{
	this->customerSelectionList = new FeasibleSolution(controller->getNumberOfCustomers());

	for (int i = 0; i < controller->getNumberOfCustomers(); i++)
	{
		this->customerSelectionList->setSolFactValue(i, i);
	}
}

void FrogLeapSolution::destroyRandomCustomerSelectionList()
{
	delete this->customerSelectionList;
}

bool FrogLeapSolution::isAMatch_Cluster(Pair* currentCustomerPair, Cluster* currentCluster, FrogLeapController* controller)
{
	bool result = false;
	
	FrogObjectCol* clusterListOrdered = this->createClusterListOrderedByDistanceFromCustomer(currentCustomerPair, controller);

	//if(currentCustomerPair->getLabelId() == 15)
	//{
	//	printf("Showing clusterListOrdered of currentCustomerPair with labelId = %d \n", currentCustomerPair->getLabelId());
	//	clusterListOrdered->printFrogObjCol();
	//	printf(" \n");
	//}
	
	Cluster * closestClusterPair = (Cluster*) controller->getFirstClusterWithRemainingCapacity(clusterListOrdered, currentCustomerPair, controller);

	result = (currentCluster->getId() == closestClusterPair->getId());

	clusterListOrdered->unReferenceFrogObjectCol();
	delete clusterListOrdered;
	
	return result;
}

FrogObjectCol* FrogLeapSolution::createClusterListOrderedByDistanceFromCustomer(Pair* currentCustomerPair, FrogLeapController* controller)
{
	FrogObjectCol* clusterDistanceList = new FrogObjectCol();

	int depotSetSize = controller->getNumberOfDepots();
	float currentDistance;

	int currentCustomerInternalId = currentCustomerPair->getId();
	int currentCustomerIndex = controller->getCustomerListIndexByInternal(currentCustomerInternalId);

	for (int i = 0; i < depotSetSize; i++)
	{
		Cluster* currentCluster = this->getClusterByIndex(i);
		Pair* clusterDepot = currentCluster->getDepotPair();
		int clusterDepotIndex = controller->getDepotListIndexByInternal(clusterDepot->getId());
		
		if (controller->getReassignCustomersSettings() == true)
		{
			if (currentCustomerPair->getAssignedDepotIndex() == clusterDepotIndex)
			{
				// this is momentarily just to recalculate the distance from currentCustomer to the rest of members of its cluster.
				controller->unassignCustomerFromCluster(currentCustomerPair, currentCluster);

				currentDistance = currentCluster->getDistanceToCustomer(currentCustomerIndex, controller);
				currentCluster->setValue(currentDistance);

				// assigning again the customer to its original cluster
				controller->assignCustomerToClusterByCustomerIndex(currentCluster, currentCustomerIndex);
			}
			else
			{
				currentDistance = currentCluster->getDistanceToCustomer(currentCustomerIndex, controller);
				currentCluster->setValue(currentDistance);
			}
		}
		else
		{
			currentDistance = currentCluster->getDistanceToCustomer(currentCustomerIndex, controller);
			currentCluster->setValue(currentDistance);
		}

		clusterDistanceList->addFrogObjectOrdered(currentCluster);

		//Pair* clusterPairCopy = currentCluster->createClusterCopy(controller);

		//clusterPairCopy->setValue(currentDistance);

	};

	return clusterDistanceList;
}

// create a feasibleSolution object (an array) with value i in position i
// feasible is assigned to attribute randomCustomerSelectionList
FeasibleSolution * FrogLeapSolution::initDepotSelection(FrogLeapController * controller)
{
	int nDepots = controller->getNumberOfDepots();
	FeasibleSolution * depotSelectionArray = new FeasibleSolution(nDepots);

	for (int i = 0; i < nDepots; i++)
	{
		depotSelectionArray->setSolFactValue(i, i);
	}

	return depotSelectionArray;
}

void  FrogLeapSolution::setAllFLSWithValue(float value) {
	
	for (int i = 0; i < this->size; i++) {
		this->setFLValue(i, value);
	}
}

void  FrogLeapSolution::printFrogLeapSolutionArray(FrogLeapController* controller)
{	
	int nsize = this->getSize();

	this->printFLS_ArrayHeader();
	for(int i = 0; i < nsize; i++)
	{
		this->printFLS_Record(i, controller);
	}
	this->printFLS_ArrayFooter();
}

void FrogLeapSolution::printFLS_ArrayHeader() 
{
	printf("Showing FLS_ARRAY \n");

	printf("DepotIndex; DepotCapacity; DepotRemaCapacity; CustomerIndex; CustomerInternalId; CustomerLabelId; CustomerDemand \n");
}

void FrogLeapSolution::printFLS_ArrayFooter()
{
	printf("END OF FLS_ARRAY \n\n\n\n");
}

void FrogLeapSolution::printFLS_Record(int i, FrogLeapController * controller)
{
	float depotFLValue = this->getFLValue(i);
	int depotIndex = floor(depotFLValue);
	int depotCapacity = controller->getDepotCapacityByIndex(depotIndex);
	int depotRemCap = controller->getDepotRemainingCapacityByIndex(depotIndex);
	int customerInternalId = controller->getCustomerInternalId(i);
	int customerLabelId = controller->getCustomerLabelId(i);
	int customerDemand = controller->getCustomerDemandByIndex(i);
	printf(" %d; %d ; %d; %d; %d ; %d; %d\n", depotIndex, depotCapacity, depotRemCap, i, customerInternalId, customerLabelId, customerDemand);
}


void FrogLeapSolution::exportClusterColToVRP(FrogLeapController * controller)
{
	int clusterColSize = this->clusterCol->getSize();

	for(int i = 0; i < clusterColSize;i++)
	{
		Cluster* currentCluster = (Cluster*)this->clusterCol->getFrogObject(i);
		currentCluster->exportClusterToVRP(controller);
	}
}