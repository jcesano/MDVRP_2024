#include "stdafx.h"
#include "Vehicle.h"
#include "FrogObjectCol.h"
#include "Pair.h"
#include "Graph.h"
#include "DistanceTable.h"
#include "FrogObject.h"
#include "FeasibleSolution.h"
#include "FrogLeapController.h"
#include <limits>

Vehicle::Vehicle(int id):FrogObject(id)
{
	customers = NULL;
	this->ptrController = NULL;
	size = 0;
	this->pathCost = std::numeric_limits<float>::max();
	this->capacity = VEHICLE_CAPACITY;
	this->setValue(this->capacity);
	this->isFeasible = true;  // used for testing purpose in printing information
	localSearchApplied = false;
	this->vehicleCustomerArray = NULL;
}

Vehicle::Vehicle(int id, FrogLeapController * controller) :FrogObject(id)
{
	customers = NULL;
	this->ptrController = controller;
	size = 0;
	this->pathCost = std::numeric_limits<float>::max();
	this->capacity = controller->getVehicleCapacity();
	this->setValue(this->capacity);
	this->remaining_capacity = this->capacity;
	this->isFeasible = true;  // used for testing purpose in printing information
	localSearchApplied = false;	
	this->vehicleCustomerArray = NULL;
}

Vehicle::~Vehicle()
{
	if(this->customers != NULL)
	{
		delete this->customers;
		this->customers = NULL;
	}
	
	if(this->vehicleCustomerArray != NULL)
	{
		delete[] vehicleCustomerArray;
		vehicleCustomerArray = NULL;
	}

	this->ptrController = NULL;
}

void Vehicle::addCustomerPair(Pair * customerPair)
{
	if (this->customers == NULL)
	{
		this->customers = new FrogObjectCol();		
	}

	this->customers->addFrogObjectOrdered(customerPair);
}

void Vehicle::addCustomerPairDoubleOrdered(Pair * customerPair)
{
	if (this->customers == NULL)
	{
		this->customers = new FrogObjectCol();
	}

	this->customers->addFrogObjectDoubleOrdered(customerPair);
}

void Vehicle::addLastCustomerPair(Pair * customerPair)
{
	if (this->customers == NULL)
	{
		this->customers = new FrogObjectCol();
	}

	this->customers->addLastFrogObject(customerPair);
}

void Vehicle::setDepotIndex(int depot_v)
{
	this->depotIndex = depot_v;
}

int Vehicle::getDepotIndex()
{
	return this->depotIndex;
}

void Vehicle::setDepotInternalId(int v_depotId)
{
	this->depotInternalId = v_depotId;
}

int Vehicle::getDepotInternalId()
{
	return this->depotInternalId;
}

float Vehicle::evalPath(FrogLeapController * controller)
{
	float vehiclePathResult = 0;

	int depotIndex = this->getDepotIndex(), depotId;
	Pair * originPair, * destinationPair;
	int originIndex, originId, destinationIndex, destinationId;
	DistanceTable * dt = this->ptrController->getDistanceTable();

	originId = this->ptrController->getDepotInternalId(depotIndex);

	if(this->customers->getSize() > 0)
	{
		for(int i = 0; i < this->customers->getSize(); i++)
		{
			destinationPair = (Pair *) this->customers->getFrogObject(i);
			//destinationIndex = controller->getDepotListIndexByInternal(destinationPair->getId());
			//destinationId = this->ptrController->getCustomerInternalId(destinationIndex);
			destinationId = destinationPair->getId();

			vehiclePathResult = vehiclePathResult + dt->getEdge(originId, destinationId);

			originId = destinationId;
		}

		// add the last edgde from the last customer to the depot
		destinationId = this->ptrController->getDepotInternalId(depotIndex);
		vehiclePathResult = vehiclePathResult + dt ->getEdge(originId, destinationId);
	}

	this->setPathCost(vehiclePathResult);

	return vehiclePathResult;
}

void Vehicle::setController(FrogLeapController * controller)
{
	this->ptrController = controller;
}

FrogLeapController * Vehicle::getController()
{
	return this->ptrController;
}

void Vehicle::setPathCost(float cost)
{
	this->pathCost = cost;
}

float Vehicle::getPathCost()
{
	return this->pathCost;
}

long int Vehicle::getCapacity()
{
	return this->capacity;
}

long int Vehicle::getRemainingCapacity()
{
	return this->remaining_capacity;
}

void Vehicle::decRemainingCapacity(int capacity_to_dec)
{
	this->remaining_capacity = this->remaining_capacity - capacity_to_dec;
	this->setValue(this->remaining_capacity);
}

int Vehicle::incDemand(int aditionalDemand)
{
	return this->demand += aditionalDemand;
}

int Vehicle::getDemand()
{
	return this->demand;
}

void Vehicle::setAsFeasible()
{
	this->isFeasible = true;
}

void Vehicle::setAsUnFeasible()
{
	this->isFeasible = false;
}

bool Vehicle::getIsFeasible()
{
	return this->isFeasible;
}

void Vehicle::setupLocalSearch()
{
	//NOT FINISHED
	int n_customers = this->customers->getSize();

	this->vehicleCustomerArray = new int [n_customers];

	for (int i = 0; i < n_customers; i++)
	{
		//obtaining the customerInternalId (internal id) in the graph, from customer index (position) in customerList
		vehicleCustomerArray[i] = this->ObtainCustomerInternalIdFromIndex(i);
	}

	//obtaining the depotId in the graph, from depot index (position)
	this->setDepotInternalId(this->ObtainDepotInternalIdFromIndex());	
}

int Vehicle::ObtainDepotInternalIdFromIndex()
{
	// Aux variable
	Pair * tmp = NULL;
	int depotInternalId, depotIndex;
	
	depotIndex = this->getDepotIndex(); //obtaining Pair(CustomerIndex, flValue)	
	depotInternalId = this->ptrController->getDepotInternalId(depotIndex); //obtaining the customerId in the graph

	return depotInternalId;
}

int Vehicle::ObtainCustomerInternalIdFromIndex(int position)
{
	// Aux variable
	Pair * tmp = NULL;
	int customerInternalId, customerIndex;

	tmp = (Pair *) this->customers->getFrogObject(position); //obtaining Pair(CustomerIndex, flValue)
	customerIndex = tmp->get_i_IntValue();
	customerInternalId = this->ptrController->getCustomerInternalId(customerIndex); //obtaining the customerId

	return customerInternalId;
}

//return the new cost found or the previous value instead
float Vehicle::applyLocalSearch(FrogLeapController * controller)
{
	bool exitLocalSearch = false;
	bool improvementFound = false;	

	if(this->localSearchApplied == false)
	{
		setupLocalSearch();		
		this->localSearchApplied = true;
	}

	if(this->customers->getSize() == 0)
	{
		return 0;
	}

	// until we do not find any better solution we continue applying local search to each improvement solution found.
	while(!exitLocalSearch)
	{
		// we look for 100 local solutions. If we find an improvement then generateLocalSolutionsAndEvaluate = true to keep in the loop
		// and execute a new local search for new solutions from the last solution found
		improvementFound = generateLocalSolutionsAndEvaluate(controller);
		if (improvementFound == false)
		{
			exitLocalSearch = true;
		}		
	}

	return this->pathCost;
}

bool Vehicle::generateLocalSolutionsAndEvaluate(FrogLeapController * controller)
{
	FeasibleSolution * fs = new FeasibleSolution(this->customers->getSize(), this->vehicleCustomerArray);

	// generate a one distance permutation solutions, evaluate each one and stop when there is an improvement 
	// and update the best solution in the vehicle or stop when 100 different solutions where evaluated without any udpate.
	bool improvement = fs->searchOneSwapFeasibleSolutionsAndEval(this, controller);

	delete fs;

	return improvement;
}

void Vehicle::updateBestSolution(FeasibleSolution * fs, float cost)
{
	for(int i=0; i < fs->getSize(); i++)
	{
		this->vehicleCustomerArray[i] = fs->getSolFactValue(i);
	}

	this->pathCost = cost;
}

// abstract methods
void Vehicle::printFrogObj()
{
	printf("\n SHOWING VEHICLE INFORMATION \n");
	if (this->getIsFeasible() == true)
	{
		printf(" Vehicle Id (feasible): %d with number of customers assigned = %d and path cost %.3f \n", this->getId(), this->customers->getSize(), this->getPathCost());
	}
	else 
	{
		printf(" Vehicle Id (NOT feasible): %d with number of customers assigned = %d \n", this->getId(), this->customers->getSize());
	}
	
	if (this->customers->getSize() > 0)
	{
		if(this->localSearchApplied == true)
		{
			printLocalSolution();
		}
		else
		{
			printGlobalSolution();
		}
	}
}

void Vehicle::writeFrogObj(FrogLeapController * controller)
{
	setupLocalSearch();

	writeSolution(controller);
}

void Vehicle::writeSolution(FrogLeapController * controller)
{
	FILE * pFile = controller->getPFile();

	int depotIndex = this->getDepotIndex(), depotIddestinationId;
	Pair * originPair, *destinationPair;
	int originIndex, originId, destinationIndex, destinationId, originLabelId, destinationLabelId;

	originId = this->ptrController->getDepotInternalId(depotIndex);
	originLabelId = this->ptrController->getLabel(originId);

	fprintf(pFile, "%d %.2f ", originLabelId, this->getPathCost());

	for (int i = 0; i < this->customers->getSize(); i++)
	{
		destinationId = this->vehicleCustomerArray[i];
		destinationLabelId = this->ptrController->getLabel(destinationId);

		fprintf(pFile, "%d ", destinationLabelId);	

		originId = destinationId;
		originLabelId = this->ptrController->getLabel(originId);
	}

	fprintf(pFile, "\n");
}

void Vehicle::writeFrogObjWithCoordinates(FrogLeapController * controller)
{
	setupLocalSearch();

	writeSolutionWithCoordinates(controller);
}

void Vehicle::writeSolutionWithCoordinates(FrogLeapController * controller)
{
	FILE * pFile = controller->getPFile();

	int depotIndex = this->getDepotIndex(), depotIddestinationId;
	Pair * originPair, *destinationPair;
	int originIndex, originInternalId, destinationIndex, destinationInternalId, originLabelId, destinationLabelId;

	// Write coordinate info of the node	
	originInternalId = this->ptrController->getDepotInternalId(depotIndex);
	writeNodeCoordinateInfo(controller, originInternalId, -1, -1);

	for (int i = 0; i < this->customers->getSize(); i++)
	{
		//obtaining the nodeInternalId (internal id) in the graph, from customer index (position) in customerList
		destinationInternalId = this->vehicleCustomerArray[i];
		writeNodeCoordinateInfo(controller, destinationInternalId, i, originInternalId);

		originInternalId = destinationInternalId;
	}
}

void Vehicle::writeNodeCoordinateInfo(FrogLeapController * controller, int nodeInternalId, int orderInPath, int prevInternalId)
{
	FILE * pFile = controller->getPFile();

	int nodeLabelId = controller->getLabel(nodeInternalId);
	int prevLabelId = -1;
	
	if (prevInternalId != -1)
		prevLabelId = controller->getLabel(prevInternalId);

	int x_coord = controller->getX_Coord(nodeInternalId);
	int y_coord = controller->getY_Coord(nodeInternalId);
	int vehicleId = this->getId();
	int depotInternalId = this->getDepotInternalId();
	int depotLabelId = controller->getLabel(depotInternalId);
	bool isDepot = (nodeLabelId == depotLabelId);
	float distanceFromDepot = controller->getDistanceTable()->getEdge(nodeInternalId, depotInternalId);
	float distanceFromPrev = 0;

	if (prevInternalId != -1)
		distanceFromPrev = controller->getDistanceTable()->getEdge(nodeInternalId, prevInternalId);
	
	fprintf(pFile, "%d, %d, %d, %d, %d, %d, %d, %d, %.2f, %.2f \n", nodeLabelId, x_coord, y_coord, vehicleId, depotLabelId, isDepot, orderInPath, prevLabelId, distanceFromPrev, distanceFromDepot);
}

void Vehicle::printLocalSolution()
{
	int depotIndex = this->getDepotIndex(), depotIddestinationId;
	Pair * originPair, *destinationPair;
	int originIndex, originId, destinationIndex, destinationId, originLabelId, destinationLabelId;

	originId = this->ptrController->getDepotInternalId(depotIndex);
	originLabelId = this->ptrController->getLabel(originId);

	printf("\n SHOWING LOCAL SOLUTION \n");
	printf("		List of customers Ids (depot and customers): ");

	for (int i = 0; i < this->customers->getSize(); i++)
	{
		destinationId = this->vehicleCustomerArray[i];
		destinationLabelId = this->ptrController->getLabel(destinationId);

		printf("(%d - %d) = %.3f  ", originLabelId, destinationLabelId, this->ptrController->getDistanceTable()->getEdge(originId, destinationId));

		originId = destinationId;
		originLabelId = this->ptrController->getLabel(originId);
	}

	// add the last edgde from the last customer to the depot
	destinationId = this->ptrController->getDepotInternalId(depotIndex);
	destinationLabelId = this->ptrController->getLabel(destinationId);
	printf("(%d - %d) = %.3f  \n", originLabelId, destinationLabelId, this->ptrController->getDistanceTable()->getEdge(originId, destinationId));	
}

void Vehicle::printGlobalSolution()
{
	int depotIndex = this->getDepotIndex(), depotId;
	Pair * originPair, *destinationPair;
	int originIndex, originId, destinationIndex, destinationId, originLabelId, destinationLabelId;

	originId = this->ptrController->getDepotInternalId(depotIndex);
	originLabelId = this->ptrController->getLabel(originId);

	printf("\n SHOWING GLOBAL SOLUTION \n");
	printf("		List of customers Ids (depot and customers): ");

	for (int i = 0; i < this->customers->getSize(); i++)
	{
		Pair * customerPair = (Pair *) this->customers->getFrogObject(i);
		int customerIndex = customerPair->get_i_IntValue();
		destinationId = this->ptrController->getCustomerInternalId(customerIndex);
		destinationLabelId = this->ptrController->getLabel(destinationId);
		printf("(%d - %d) = %.3f  ", originLabelId, destinationLabelId, this->ptrController->getDistanceTable()->getEdge(originId, destinationId));

		originId = destinationId;
		originLabelId = this->ptrController->getLabel(originId);
	}

	if (this->getIsFeasible() == true)
	{
		// add the last edgde from the last customer to the depot
		destinationId = this->ptrController->getDepotInternalId(depotIndex);
		destinationLabelId = this->ptrController->getLabel(destinationId);
		printf("(%d - %d) = %.3f  \n", originLabelId, destinationLabelId, this->ptrController->getDistanceTable()->getEdge(originId, destinationId));
	}
}

void Vehicle::adjustCustomerRoute(FrogLeapController * controller)
{
	FrogObjectCol * customerDistanceOrderedCol = new FrogObjectCol();
	int depotIndex = this->getDepotIndex();
	int customerIndex;

	Pair * customerPair = this->getClosestIndexCustomerPairToDepot(depotIndex, controller);

	this->customers->removeFrogObject(customerPair);

	customerDistanceOrderedCol->addLastFrogObject(customerPair);

	while (this->customers->getSize() > 0)
	{
		customerIndex = customerPair->getId();
		customerPair = this->getClosestIndexCustomerPairToCustomer(customerIndex, controller);
		
		this->customers->removeFrogObject(customerPair);
		customerDistanceOrderedCol->addLastFrogObject(customerPair);
	}

	delete this->customers;
	this->customers = customerDistanceOrderedCol;
}

Pair * Vehicle::getClosestIndexCustomerPairToDepot(int depotIndex, FrogLeapController * controller)
{
	int closestIndex = controller->getClosestCustomerLocalIndexToDepot(depotIndex, 0, this->customers->getSize(), this->customers);
	
	return (Pair *)this->customers->getFrogObject(closestIndex);
}

Pair * Vehicle::getClosestIndexCustomerPairToCustomer(int customerIndex, FrogLeapController * controller)
{
	int closestIndex = controller->getClosestCustomerLocalIndexToCustomer(customerIndex, 0, this->customers->getSize(), this->customers);

	return (Pair *)this->customers->getFrogObject(closestIndex);
}

bool Vehicle::isTheSame(FrogObject * fs)
{
	return (this == fs);
}
