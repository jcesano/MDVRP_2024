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
}

FrogLeapSolution::~FrogLeapSolution()
{
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
	float u;
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
	float u;
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

		matchCustomerCol = controller->assignDepotToCustomerPairsUntilDemandComplete(currentDepotPair, matchCustomerCol);
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

	// get the index of the first customer with lower demand to be attended by current depot
	FrogObjectCol * matchCustomerCol = controller->createMatchCustomerList(currentDepotPair);

	int totalCustomerDemand = controller->getTotalDemandOrCapacity(matchCustomerCol);

	int depotRemainingCapacity = currentDepotPair->get_j_IntValue();

	if (totalCustomerDemand <= depotRemainingCapacity)
	{
		//controller->assignDepotToCustomerPairs(currentDepotPair, matchCustomerCol);
		controller->assignCustomersToCluster(currentDepotPair, matchCustomerCol, depotListOrderedByCapacity, this);

		//this->assignMatchCustomerList(matchCustomerCol, controller);
	}
	else
	{
		matchCustomerCol = controller->orderCustomerPairListByNthClosestDepotDesc(n, matchCustomerCol);

		matchCustomerCol = controller->selectCustomerPairsUntilDemandComplete(currentDepotPair, matchCustomerCol);
		controller->assignCustomersToCluster(currentDepotPair, matchCustomerCol, depotListOrderedByCapacity, this);

		//this->assignMatchCustomerList(matchCustomerCol, controller);
	}

	matchCustomerCol->unReferenceFrogObjectCol();
	delete matchCustomerCol;
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
	controller->resetDepotRemainingCapacities();

	return this->decodeFrogLeapWholeSolutionWithClosestNextCriteria(controller, true);
}

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

// uses the sweep algorithm to assign customer order for each depot
// Here we also assign vehicles to customer according to the angular order of customers
DecodedFrogLeapSolution * FrogLeapSolution::decodeFrogLeapWholeSolutionWithAngularCriteria(FrogLeapController * controller)
{

	DecodedFrogLeapSolution * decodedSolution = new DecodedFrogLeapSolution(this->n_depots, controller);

	controller->resetDepotRemainingCapacities();
	
	decodedSolution->setUnReferenceBeforeDelete(true);

	int i = 0, rand_i;
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
	fprintf(pFile, "Values are the following: \n");

	for (int i = 0; i < this->getSize() - 1; i++)
	{
		fprintf(pFile, "%.4f \n ", this->values[i]);
	}

	//printing the last element (replacing the comma by the point)
	fprintf(pFile, "%.4f. \n", this->values[this->getSize() - 1]);

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
