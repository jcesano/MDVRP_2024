#include "stdafx.h"
#include "DecodedFrogLeapSolution.h"
#include "FrogObjectCol.h"
#include "Pair.h"
#include "Vehicle.h"
#include <stdio.h>      /* printf */
#include <math.h>       /* floor */
#include "Graph.h"
#include "FrogLeapController.h"
#include "FrogLeapSolution.h"
#include "Cluster.h"
#include "ClarkWrightHandler.h"
#include "SolutionData.h"


DecodedFrogLeapSolution::DecodedFrogLeapSolution(int n_depots):FrogObject()
{
	//this->vehicles = new FrogObjectCol();
	this->vehicles = new FrogObjectCol * [n_depots];
	this->assignedCustomers = new FrogObjectCol *[n_depots];

	this->unReferenceItemsBeforeDelete = false;

	for (int i = 0; i < n_depots; i++) {
		vehicles[i] = new FrogObjectCol();
		assignedCustomers[i] = new FrogObjectCol();
	};	

	this->ptrController = NULL;
	this->isFeasibleSolution = true;	
	this->numDepots = n_depots;
}

DecodedFrogLeapSolution::DecodedFrogLeapSolution(int n_depots, FrogLeapController * controller) :FrogObject()
{
	//this->vehicles = new FrogObjectCol();
	this->vehicles = new FrogObjectCol *[n_depots];
	this->assignedCustomers = new FrogObjectCol *[n_depots];
	this->unReferenceItemsBeforeDelete = false;
	
	for (int i = 0; i < n_depots; i++) {
		vehicles[i] = new FrogObjectCol();
		assignedCustomers[i] = new FrogObjectCol();
	};

	this->ptrController = controller;
	this->isFeasibleSolution = true;	
	this->numDepots = n_depots;
}


DecodedFrogLeapSolution::~DecodedFrogLeapSolution()
{
	//printf("Destroying of DecodedFrogLeapSolution: Started \n");

	int nDepots = this->ptrController->getNumberOfDepots();

	if(this->vehicles != NULL)
	{
		//for (int i = 0; i < nDepots; i++)
		//{
		//	delete this->vehicles[i];
		//}
		
		this->deleteArrayOfFrogObjectCols(this->vehicles, nDepots);		
		delete [] vehicles;
	}
	
	this->vehicles = NULL;
	//printf("Destroying of DecodedFrogLeapSolution: vehicles destroyed  \n");

	this->ptrController = NULL;		

	//printf("Destroying of DecodedFrogLeapSolution: FINISHED \n");
}

void DecodedFrogLeapSolution::deleteArrayOfFrogObjectCols(FrogObjectCol ** arrayPtr, int v_size)
{
	int size = v_size;

	for (int i = 0; i < size; i++)
	{
		if(this->unReferenceItemsBeforeDelete == true)
		{
			arrayPtr[i]->unReferenceFrogObjectCol();
		}

		delete arrayPtr[i];
		arrayPtr[i] = NULL;
	}	
}

void DecodedFrogLeapSolution::addVehicle(int depotIndex, Vehicle * v_vehicle)
{
	//this->vehicles->addFrogObject(v_vehicle);
	this->vehicles[depotIndex]->addFrogObject(v_vehicle);	
}

/*
void DecodedFrogLeapSolution::addVehicle(Vehicle * vehicle)
{
	vehicles->addFrogObject(vehicle);
}


Vehicle * DecodedFrogLeapSolution::getVehicle(int pos)
{
	return NULL;

	//return (Vehicle *) this->vehicles->getFrogObject(pos);
}
*/

int DecodedFrogLeapSolution::decodeFixedFloatFrogLeapValue(float fvalue, int numberOfDepots)
{
	int result = floor(fvalue);

	if (result == numberOfDepots)
	{
		result--;
	};

	return result;
}

int DecodedFrogLeapSolution::decodeFrogLeapValue(float fvalue, int numberOfDepots)
{
	int result = floor(fvalue);

	if (result == numberOfDepots)
	{
		result--;
	};

	return result;
}

bool DecodedFrogLeapSolution::decodeFrogLeapItem(FrogLeapController * controller, float fvalue, int customerIndex, int numberOfDepots)
{
	bool result = true;
	int vehicleId;
	int depotIndex = this->decodeFrogLeapValue(fvalue, numberOfDepots);

	int customerDemand = this->ptrController->getCustomerDemandByIndex(customerIndex);
	int remainingDepotCapacity = this->ptrController->getDepotRemainingCapacityByIndex(depotIndex);
	int customerId = this->ptrController->getCustomerInternalId(customerIndex);

	if(customerDemand > controller->getVehicleCapacity() || customerDemand > remainingDepotCapacity)
	{
		this->setIsFeasibleSolution(false);
		this->setNotAddedCustomer(customerId);
		result = false;
		return result;
	}

	this->ptrController->decRemainingDepotCapacity(depotIndex, customerDemand);

	//assign vehicle to customer	
	//get the element with minimum remaining capacity enough
	Vehicle * veh = (Vehicle *)this->vehicles[depotIndex]->getFirstUpperValueFrogObject(customerDemand); 
	//Vehicle * veh = (Vehicle *)this->getFirstUpperValueVehicle(customerDemand, depotIndex);
	
	if(veh == NULL)
	{
		vehicleId = controller->getGlobalVehicleId();

 		veh = new Vehicle(vehicleId, this->ptrController);
		
		veh->decRemainingCapacity(customerDemand);				

		//int depotIndex = vehicleId / numberOfDepots;
		veh->setDepotIndex(depotIndex);
				
		this->vehicles[depotIndex]->addFrogObjectOrdered(veh);		
	}
	else
	{
		veh->decRemainingCapacity(customerDemand);

		//this->vehicles[depotIndex]->reorderFrogObject(veh);
		this->vehicles[depotIndex]->reorderFrogObject(veh);		
	}

	Pair * customerPair = new Pair(PairType::IntVsFloat);
	customerPair->set_i_IntValue(customerIndex);
	customerPair->set_j_FloatValue(fvalue);
	customerPair->setValue(fvalue);
	customerPair->setId(customerIndex);

	veh->addCustomerPair(customerPair);	

	customerPair = NULL;
	veh = NULL;
	return result;
}


bool DecodedFrogLeapSolution::decodeFrogLeapItemToListWithClosestNextCriteria(FrogLeapController * controller, float fvalue, int customerIndex, int numberOfDepots)
{
	bool result = true;
	int vehicleId = -1;
	int depotIndex = this->decodeFrogLeapValue(fvalue, numberOfDepots);

	// assign customer to depot and adjust remaining capacity of the depot
	int customerDemand = this->ptrController->getCustomerDemandByIndex(customerIndex);
	int remainingDepotCapacity = this->ptrController->getDepotRemainingCapacityByIndex(depotIndex);
	int customerId = this->ptrController->getCustomerInternalId(customerIndex);

	if (customerDemand > controller->getVehicleCapacity() || customerDemand > remainingDepotCapacity)
	{
		this->setIsFeasibleSolution(false);
		this->setNotAddedCustomer(customerId);
		result = false;
		return result;
	}

	this->ptrController->decRemainingDepotCapacity(depotIndex, customerDemand);

	// we calculate angular values of customer pair and assign it to an ordered list of the depot
	Pair * customerPair = controller->getCustomerPairByIndex(customerIndex);
	int customerInternalId = controller->getCustomerInternalId(customerIndex);

	Pair * depotPair = controller->getDepotPairByIndex(depotIndex);
	int depotInternalId = controller->getDepotInternalId(depotIndex);

	// calculate next closest next customer
	

	// assign it to an ordered list of the depot
	this->assignedCustomers[depotIndex]->addFrogObjectDoubleOrdered(customerPair);

	return result;
}

bool DecodedFrogLeapSolution::decodeFrogLeapItemToListWithAngularCriteria(FrogLeapController * controller, float fvalue, int customerIndex, int numberOfDepots)
{
	bool result = true;
	int vehicleId = -1;
	int depotIndex = this->decodeFrogLeapValue(fvalue, numberOfDepots);

	// assign customer to depot and adjust remaining capacity of the depot
	int customerDemand = this->ptrController->getCustomerDemandByIndex(customerIndex);
	int remainingDepotCapacity = this->ptrController->getDepotRemainingCapacityByIndex(depotIndex);
	int customerId = this->ptrController->getCustomerInternalId(customerIndex);

	if (customerDemand > controller->getVehicleCapacity() || customerDemand > remainingDepotCapacity)
	{
		this->setIsFeasibleSolution(false);
		this->setNotAddedCustomer(customerId);
		result = false;
		return result;
	}

	this->ptrController->decRemainingDepotCapacity(depotIndex, customerDemand);

	// we calculate angular values of customer pair and assign it to an ordered list of the depot
	Pair * customerPair = controller->getCustomerPairByIndex(customerIndex);
	int customerInternalId = controller->getCustomerInternalId(customerIndex);

	Pair * depotPair = controller->getDepotPairByIndex(depotIndex);
	int depotInternalId = controller->getDepotInternalId(depotIndex);

	// calculate angular values of customer pair
	controller->setAngularValues(customerPair, customerInternalId, depotInternalId);

	// assign it to an ordered list of the depot
	this->assignedCustomers[depotIndex]->addFrogObjectDoubleOrdered(customerPair);

	return result;
}

bool DecodedFrogLeapSolution::decodeFrogLeapItemWithAngularCriteria(FrogLeapController * controller, float fvalue, int customerIndex, int numberOfDepots)
{
	bool result = true;
	int vehicleId;
	int depotIndex = this->decodeFrogLeapValue(fvalue, numberOfDepots);

	int customerDemand = this->ptrController->getCustomerDemandByIndex(customerIndex);
	int remainingDepotCapacity = this->ptrController->getDepotRemainingCapacityByIndex(depotIndex);
	int customerId = this->ptrController->getCustomerInternalId(customerIndex);

	if(depotIndex == 6)
	{
		//printf("ojo 3");
	}
	
	if (customerDemand > controller->getVehicleCapacity() || customerDemand > remainingDepotCapacity)
	{
		this->setIsFeasibleSolution(false);
		this->setNotAddedCustomer(customerId);
		result = false;
		return result;
	}

	this->ptrController->decRemainingDepotCapacity(depotIndex, customerDemand);

	//assign vehicle to customer	
	//get the element with minimum remaining capacity enough
	Vehicle * veh = (Vehicle *)this->vehicles[depotIndex]->getFirstUpperValueFrogObject(customerDemand);
	//Vehicle * veh = (Vehicle *)this->getFirstUpperValueVehicle(customerDemand, depotIndex);

	if (veh == NULL)
	{
		vehicleId = controller->getGlobalVehicleId();

		veh = new Vehicle(vehicleId, this->ptrController);

		veh->decRemainingCapacity(customerDemand);

		//int depotIndex = vehicleId / numberOfDepots;
		veh->setDepotIndex(depotIndex);

		this->vehicles[depotIndex]->addFrogObjectOrdered(veh);
	}
	else
	{
		veh->decRemainingCapacity(customerDemand);

		//this->vehicles[depotIndex]->reorderFrogObject(veh);
		this->vehicles[depotIndex]->reorderFrogObject(veh);
	}

	Pair * veh_customerPair = new Pair(PairType::IntVsFloat);
	veh_customerPair->set_i_IntValue(customerIndex);
	veh_customerPair->set_j_FloatValue(fvalue);

	// we calculate angular values
	Pair * customerPair = controller->getCustomerPairByIndex(customerIndex);
	int customerInternalId = controller->getCustomerInternalId(customerIndex);
	
	Pair * depotPair = controller->getDepotPairByIndex(depotIndex);
	int depotInternalId = controller->getDepotInternalId(depotIndex);

	controller->setAngularValues(veh_customerPair, customerInternalId, depotInternalId);	
	
	veh_customerPair->setId(customerIndex);

	veh->addCustomerPairDoubleOrdered(veh_customerPair);

	veh_customerPair = NULL;
	veh = NULL;
	return result;
}

void DecodedFrogLeapSolution::assignDecodedCustomersToVehicles(FrogLeapController * controller)
{
	for (int i = 0; i < this->numDepots; i++)
	{
		assignDecodedCustomersToDepotVehicles(i, controller);
	}
}

void DecodedFrogLeapSolution::cw_assignDecodedCustomersToVehicles(FrogLeapController* controller, FrogLeapSolution * fls)
{
	for (int i = 0; i < this->numDepots; i++)
	{
		cw_assignDecodedCustomersToDepotVehicles(i, controller, fls);
	}
}

void DecodedFrogLeapSolution::assignDecodedCustomersToDepotVehicles(int depotIndex, FrogLeapController * controller)
{
	int size = this->assignedCustomers[depotIndex]->getSize();
	Vehicle * veh = NULL;
	Pair * customerPair;

	for(int i = 0; i < size; i++)
	{
		customerPair = (Pair *) this->assignedCustomers[depotIndex]->getFrogObject(i);

		//assign vehicle to customer	
		//get the element with minimum remaining capacity enough		
		int customerDemand = customerPair->get_i_IntValue();

		veh = (Vehicle *)this->vehicles[depotIndex]->getFirstUpperValueFrogObject(customerDemand);		

		int vehicleId;

		if (veh == NULL)
		{
			vehicleId = controller->getGlobalVehicleId();

			veh = new Vehicle(vehicleId, this->ptrController);

			veh->decRemainingCapacity(customerDemand);
			veh->incDemand(customerDemand);
			//int depotIndex = vehicleId / numberOfDepots;
			veh->setDepotIndex(depotIndex);

			this->vehicles[depotIndex]->addFrogObjectOrdered(veh);
		}
		else
		{
			veh->decRemainingCapacity(customerDemand);
			veh->incDemand(customerDemand);
			//this->vehicles[depotIndex]->reorderFrogObject(veh);
			this->vehicles[depotIndex]->reorderFrogObject(veh);
		}

		veh->addLastCustomerPair(customerPair);
	}
}

void DecodedFrogLeapSolution::cw_assignDecodedCustomersToDepotVehicles(int depotIndex, FrogLeapController* controller, FrogLeapSolution * fls)
{
	//int size = this->assignedCustomers[depotIndex]->getSize();
	FrogObjectCol* clusterCustomers = fls->getClusterByIndex(depotIndex)->getCustomerCol();
	int nCustomers = clusterCustomers->getSize();
	Vehicle* veh = NULL;
	
	int cw_handler_col_size = fls->getCWHandlerCol()->getSize();
	
	// decode cw_handler_col
	for (int i = 0; i < cw_handler_col_size; i++)
	{
		ClarkWrightHandler * current_cw = (ClarkWrightHandler*)fls->getCWHandlerCol()->getFrogObject(i);
		
		int routes_size = current_cw->getRoutesSize();
		
		// decode routes
		for(int j = 0; j < routes_size; j++)
		{
			FrogObjectCol* current_route = current_cw->getRoute(j);
			 
			// remove the first and last item because each of then corresponds to the depot
			current_route->removeFirstItem();
			current_route->removeLastItem();

			int current_route_size = current_route->getSize();

			// decode customers
			for(int k=0; k < current_route_size; k++)
			{
				Pair* customerPair = (Pair *)current_route->getFrogObject(k);
				float customerDemand = customerPair->get_i_IntValue();
				veh = (Vehicle*)this->vehicles[depotIndex]->getFirstUpperValueFrogObject(customerDemand);
				int vehicleId;
				if (veh == NULL)
				{
					vehicleId = controller->getGlobalVehicleId();

					veh = new Vehicle(vehicleId, this->ptrController);

					veh->decRemainingCapacity(customerDemand);
					veh->incDemand(customerDemand);

					//int depotIndex = vehicleId / numberOfDepots;
					veh->setDepotIndex(depotIndex);

					this->vehicles[depotIndex]->addFrogObjectOrdered(veh);
				}
				else
				{
					veh->decRemainingCapacity(customerDemand);
					veh->incDemand(customerDemand);

					//this->vehicles[depotIndex]->reorderFrogObject(veh);
					this->vehicles[depotIndex]->reorderFrogObject(veh);
				}

				veh->addLastCustomerPair(customerPair);
			}
		}

		//customerPair = (Pair*)this->assignedCustomers[depotIndex]->getFrogObject(i);
		//customerPair = (Pair *)clusterCustomers->getFrogObject(i);
		//assign vehicle to customer	
		//get the element with minimum remaining capacity enough		
		//float customerDemand = customerPair->get_i_IntValue();

		//veh = (Vehicle*)this->vehicles[depotIndex]->getFirstUpperValueFrogObject(customerDemand);

		//int vehicleId;

		//if (veh == NULL)
		//{
		//	vehicleId = controller->getGlobalVehicleId();

		//	veh = new Vehicle(vehicleId, this->ptrController);

		//	veh->decRemainingCapacity(customerDemand);

		//	//int depotIndex = vehicleId / numberOfDepots;
		//	veh->setDepotIndex(depotIndex);

		//	this->vehicles[depotIndex]->addFrogObjectOrdered(veh);
		//}
		//else
		//{
		//	veh->decRemainingCapacity(customerDemand);

		//	//this->vehicles[depotIndex]->reorderFrogObject(veh);
		//	this->vehicles[depotIndex]->reorderFrogObject(veh);
		//}

		//veh->addLastCustomerPair(customerPair);
	}
}

void DecodedFrogLeapSolution::assignCustomersToDepotLists(FrogLeapController * controller, FrogLeapSolution * fls)
{
	int size = fls->getSize();//this corresponds to the number of customers

	for (int i = 0; i < size; i++)
	{
		assignCustomerToDepotList(controller, fls, i);
	}
}

void DecodedFrogLeapSolution::assignCustomerToDepotList(FrogLeapController * controller, FrogLeapSolution * fls, int customerIndex)
{	
	int depotIndex = this->decodeFrogLeapValue(fls->getFLValue(customerIndex), controller->getNumberOfDepots());

	Pair * customerPair = controller->getCustomerPairByIndex(customerIndex);

	this->assignedCustomers[depotIndex]->addFrogObject(customerPair);
}

Pair * DecodedFrogLeapSolution::getClosestCustomerIndexToDepot(int depotIndex, FrogLeapController * controller) 
{
	// get the closest customerPair to depot with depotIndex
	int closestCustomerIndexToDepot = controller->getClosestCustomerLocalIndexToDepot(depotIndex, 0, this->assignedCustomers[depotIndex]->getSize(), this->assignedCustomers[depotIndex]);
	return (Pair *)this->assignedCustomers[depotIndex]->getFrogObject(closestCustomerIndexToDepot);	
}

Pair * DecodedFrogLeapSolution::getClosestCustomerIndexToCustomer(int customerInternalId, int depotIndex, FrogLeapController * controller)
{
	// get the closest customerPair to customer with depotindex
	int closestCustomerIndexToCustomer = controller->getClosestCustomerLocalIndexToCustomer(customerInternalId, 0, this->assignedCustomers[depotIndex]->getSize(), this->assignedCustomers[depotIndex]);

	return (Pair *)this->assignedCustomers[depotIndex]->getFrogObject(closestCustomerIndexToCustomer);
}

void DecodedFrogLeapSolution::orderDepotCustomersWithClosestNextCriteria(int depotIndex, FrogLeapController * controller)
{
	FrogObjectCol * customerDistanceOrderedCol = new FrogObjectCol();	
	int customerInternalId;
	
	// get the closest customerPair to depot with depotIndex	
	Pair * customerPair = this->getClosestCustomerIndexToDepot(depotIndex, controller);

	//remove customerPair from original disordered customer list and assign it to a new ordered list
	this->assignedCustomers[depotIndex]->removeFrogObject(customerPair);
	customerDistanceOrderedCol->addLastFrogObject(customerPair);

	while (this->assignedCustomers[depotIndex]->getSize() > 0)
	{
		customerInternalId = customerPair->getId();
		customerPair = this->getClosestCustomerIndexToCustomer(customerInternalId, depotIndex, controller);

		//remove customerPair from original disordered customer list and assign it to a new ordered list
		this->assignedCustomers[depotIndex]->removeFrogObject(customerPair);
		customerDistanceOrderedCol->addLastFrogObject(customerPair);
	}	

	this->assignedCustomers[depotIndex]->unReferenceFrogObjectCol();
	delete this->assignedCustomers[depotIndex];
	this->assignedCustomers[depotIndex] = customerDistanceOrderedCol;
}

void DecodedFrogLeapSolution::orderCustomersWithClosestNextCriteria(FrogLeapController * controller)
{
	int nDepots = controller->getNumberOfDepots();
	for (int i = 0; i < nDepots; i++)
	{
		this->orderDepotCustomersWithClosestNextCriteria(i, controller);
	}
}

//bool DecodedFrogLeapSolution::decodeFrogLeapAssignCustomerToDepotWithAngularValues(FrogLeapController * controller, float fvalue, int customerIndex, int numberOfDepots)
//{
	//bool result = true;
	//int vehicleId;
	//int depotIndex = this->decodeFrogLeapValue(fvalue, numberOfDepots);

	//int customerDemand = this->ptrController->getCustomerDemandByIndex(customerIndex);
	//int remainingDepotCapacity = this->ptrController->getDepotRemainingCapacityByIndex(depotIndex);
	//int customerId = this->ptrController->getCustomerId(customerIndex);

	//if (customerDemand > controller->getVehicleCapacity() || customerDemand > remainingDepotCapacity)
	//{
	//	this->setIsFeasibleSolution(false);
	//	this->setNotAddedCustomer(customerId);
	//	result = false;
	//	return result;
	//}

	//this->ptrController->decRemainingDepotCapacity(depotIndex, customerDemand);

	//// calculate the angular value of the customer
	//Pair customerAssign = (Pair *) this->calculateAngularValue(customerIndex, depotIndex, controller)



	////assign vehicle to customer	
	////get the element with maximum remaining capacity
	//Vehicle * veh = (Vehicle *)this->vehicles[depotIndex]->getFirstUpperValueFrogObject(customerDemand);
	////Vehicle * veh = (Vehicle *)this->getFirstUpperValueVehicle(customerDemand, depotIndex);

	//if (veh == NULL)
	//{
	//	vehicleId = controller->getGlobalVehicleId();

	//	veh = new Vehicle(vehicleId, this->ptrController);

	//	veh->decRemainingCapacity(customerDemand);

	//	//int depotIndex = vehicleId / numberOfDepots;
	//	veh->setDepotIndex(depotIndex);

	//	this->vehicles[depotIndex]->addFrogObjectOrdered(veh);
	//}
	//else
	//{
	//	veh->decRemainingCapacity(customerDemand);

	//	//this->vehicles[depotIndex]->reorderFrogObject(veh);
	//	this->vehicles[depotIndex]->reorderFrogObject(veh);
	//}

	//Pair * customerPair = new Pair(PairType::IntVsFloat);
	//customerPair->set_i_IntValue(customerIndex);
	//customerPair->set_j_FloatValue(fvalue);
	//customerPair->setValue(fvalue);
	//customerPair->setId(customerIndex);

	//veh->addCustomerPair(customerPair);

	//customerPair = NULL;
	//veh = NULL;
	//return result;
//}

float DecodedFrogLeapSolution::evalSolution()
{
	Vehicle * vehPtr = NULL;
	float result = 0;

	for(int i = 0; i < this->numDepots; i++)
	{
		for (int j = 0; j < this->vehicles[i]->getSize(); j++)
		{
			vehPtr = (Vehicle *)this->vehicles[i]->getFrogObject(j);
			result = result + vehPtr->evalPath(this->ptrController);
		}
	}

	//for (int j = 0; j < this->vehicles->getSize(); j++)
	//{
	//	vehPtr = (Vehicle *)this->vehicles->getFrogObject(j);
	//	result = result + vehPtr->evalPath(this->ptrController);
	//}	

	return result;
}

void DecodedFrogLeapSolution::writeFrogObj()
{
	FILE* pFile = this->ptrController->getPFile();

	Vehicle* vehPtr;

	fprintf(pFile, "\n Showing DecodedFrogLeapSolution data results: ");

	if (this->isFeasibleSolution == true)
	{
		fprintf(pFile, "Feasible \n");
	}
	else
	{
		fprintf(pFile, "NOT FEASIBLE \n");
	}

	fprintf(pFile, "Vehiculos por deposito\n");
	for (int i = 0; i < this->numDepots; i++)
	{
		int internalId = this->ptrController->getDepotInternalId(i);
		int labelId = this->ptrController->getLabelId(internalId);
		int depotCapacity = this->ptrController->getDepotCapacityByIndex(i);
		int depotRemaniningCapacity = this->ptrController->getDepotRemainingCapacityByIndex(i);		
		fprintf(pFile, "Deposito index: %d; internalId: %d ; LabelId = %d; Capacity = %d, RemainingCapacity = %d \n", i, internalId, labelId, depotCapacity, depotRemaniningCapacity);

		int numVehicles_i = this->vehicles[i]->getSize();		

		fprintf(pFile, "Cantidad de vehiculos: %d \n", numVehicles_i);

		for (int j = 0; j < numVehicles_i; j++)
		{
			vehPtr = (Vehicle*)this->vehicles[i]->getFrogObject(j);
			vehPtr->writeFrogObj(this->ptrController, nullptr);
		}
	}

	fprintf(pFile, "DecodedFrogLeapSolution FINISHED \n");
}


void DecodedFrogLeapSolution::writeFrogObjWithSolutionData()
{
	FILE* pFile = this->ptrController->getPFile();

	Vehicle* vehPtr;

	//fprintf(pFile, "\n Showing DecodedFrogLeapSolution data results: ");

	//if (this->isFeasibleSolution == true)
	//{
	//	fprintf(pFile, "Feasible \n");
	//}
	//else
	//{
	//	fprintf(pFile, "NOT FEASIBLE \n");
	//}

	//fprintf(pFile, "Vehiculos por deposito\n");

	SolutionData* sd = new SolutionData();

	sd->writeSolutionDataHeader(pFile);

	for (int i = 0; i < this->numDepots; i++)
	{
		int internalId = this->ptrController->getDepotInternalId(i);
		sd->setDepotIndex(i);
		sd->setDepotInternalId(internalId);
		sd->setDepotLabelId(this->ptrController->getLabelId(internalId));		
		sd->setDepotCap(this->ptrController->getDepotCapacityByIndex(i));
		sd->setDepotRemCap(this->ptrController->getDepotRemainingCapacityByIndex(i));
		/*int internalId = this->ptrController->getDepotInternalId(i);*/
		/*int labelId = this->ptrController->getLabelId(internalId);*/
		/*int depotCapacity = this->ptrController->getDepotCapacityByIndex(i);
		int depotRemaniningCapacity = this->ptrController->getDepotRemainingCapacityByIndex(i);*/
		//fprintf(pFile, "Deposito index: %d; internalId: %d ; LabelId = ; Capacity = %d, RemainingCapacity = %d \n", i, internalId, labelId, depotCapacity, depotRemaniningCapacity);

		int numVehicles_i = this->vehicles[i]->getSize();

		sd->setNumVehicles(numVehicles_i);

		//fprintf(pFile, "Cantidad de vehiculos: %d \n", numVehicles_i);

		for (int j = 0; j < numVehicles_i; j++)
		{
			vehPtr = (Vehicle*)this->vehicles[i]->getFrogObject(j);
			vehPtr->writeFrogObj(this->ptrController, sd);
		}
	}

	//fprintf(pFile, "DecodedFrogLeapSolution FINISHED \n");
}

void DecodedFrogLeapSolution::printFrogObjWithSolutionData()
{
	Vehicle* vehPtr;

	//fprintf(pFile, "\n Showing DecodedFrogLeapSolution data results: ");

	//if (this->isFeasibleSolution == true)
	//{
	//	fprintf(pFile, "Feasible \n");
	//}
	//else
	//{
	//	fprintf(pFile, "NOT FEASIBLE \n");
	//}

	//fprintf(pFile, "Vehiculos por deposito\n");

	SolutionData* sd = new SolutionData();

	sd->printSolutionDataHeader();

	for (int i = 0; i < this->numDepots; i++)
	{
		int internalId = this->ptrController->getDepotInternalId(i);
		sd->setDepotIndex(i);
		sd->setDepotInternalId(internalId);
		sd->setDepotLabelId(this->ptrController->getLabelId(internalId));
		sd->setDepotCap(this->ptrController->getDepotCapacityByIndex(i));
		sd->setDepotRemCap(this->ptrController->getDepotRemainingCapacityByIndex(i));
		/*int internalId = this->ptrController->getDepotInternalId(i);*/
		/*int labelId = this->ptrController->getLabelId(internalId);*/
		/*int depotCapacity = this->ptrController->getDepotCapacityByIndex(i);
		int depotRemaniningCapacity = this->ptrController->getDepotRemainingCapacityByIndex(i);*/
		//fprintf(pFile, "Deposito index: %d; internalId: %d ; LabelId = ; Capacity = %d, RemainingCapacity = %d \n", i, internalId, labelId, depotCapacity, depotRemaniningCapacity);

		int numVehicles_i = this->vehicles[i]->getSize();

		sd->setNumVehicles(numVehicles_i);

		//fprintf(pFile, "Cantidad de vehiculos: %d \n", numVehicles_i);

		for (int j = 0; j < numVehicles_i; j++)
		{
			vehPtr = (Vehicle*)this->vehicles[i]->getFrogObject(j);
			vehPtr->printFrogObj(this->ptrController, sd);
		}
	}

	//fprintf(pFile, "DecodedFrogLeapSolution FINISHED \n");
}
// abstract methods
void DecodedFrogLeapSolution::printFrogObj()
{
	Vehicle * vehPtr;

	printf("\n Showing DecodedFrogLeapSolution data results: ");

	if(this->isFeasibleSolution == true)
	{
		printf("Feasible \n");
	}
	else
	{
		printf("NOT FEASIBLE \n");
	}
	
	printf("Vehiculos por deposito\n");
	for(int i=0; i < this->numDepots; i++)
	{
		int internalId = this->ptrController->getDepotInternalId(i);
		int depotCapacity = this->ptrController->getDepotCapacityByIndex(i);
		int depotRemaniningCapacity = this->ptrController->getDepotRemainingCapacityByIndex(i);
		printf("Deposito index: %d; internalId: %d ; Capacity = %d, RemainingCapacity = %d \n", i, internalId, depotCapacity, depotRemaniningCapacity);

		int numVehicles_i = this->vehicles[i]->getSize();
		
		printf("Cantidad de vehiculos: %d \n", numVehicles_i);

		for (int j = 0; j < numVehicles_i; j++)
		{
			vehPtr = (Vehicle *)this->vehicles[i]->getFrogObject(j);			
			vehPtr->printFrogObj();
		}
	}

	printf("DecodedFrogLeapSolution FINISHED \n");
}

void DecodedFrogLeapSolution::setUnReferenceBeforeDelete(bool v_bol)
{
	this->unReferenceItemsBeforeDelete = v_bol;
}

bool DecodedFrogLeapSolution::getUnReferenceBeforeDelete()
{
	return this->unReferenceItemsBeforeDelete;
}

void DecodedFrogLeapSolution::writeDecodedFrogLeapSolution(FrogLeapController * controller)
{

	Vehicle * vehPtr;
	FILE * pFile = controller->getPFile();

	fprintf(pFile, "Seed used: ", controller->getSeedUsed());
	fprintf(pFile, "NAME : %s \n", controller->getTestCaseName());
	fprintf(pFile, "COMMENT : %d \n", controller->getTestCaseComment());
	fprintf(pFile, "DIMENSION : %d \n", controller->getTestCaseDimension());

	fprintf(pFile, "TYPE : %s \n", controller->getTestCaseType());
	fprintf(pFile, "CAPACITY : %d \n", controller->getTestCaseCapacity());
	fprintf(pFile, "ASSIGNATION \n");

	for (int i = 0; i < controller->getNumberOfDepots(); i++)
	{
		int numVehicles_i = this->vehicles[i]->getSize();

		for (int j = 0; j < numVehicles_i; j++)
		{
			vehPtr = (Vehicle *)this->vehicles[i]->getFrogObject(j);
			vehPtr->writeFrogObj(controller, NULL);
		}
	}

	fprintf(pFile, "DecodedFrogLeapSolution FINISHED \n");


}

void DecodedFrogLeapSolution::writeDecodedFrogLeapSolutionWithCoordinates(FrogLeapController * controller)
{

	Vehicle * vehPtr;
	FILE * pFile = controller->getPFile();

	fprintf(pFile, "NODE_ID, X_COORD, Y_COORD, VEHICLE_ID, DEPOT_ID, IS_DEPOT, ORDER_IN_PATH, PREV_LABEL_ID, DISTANCE_FROM_PREV, DISTANCE_FROM_DEPOT\n");

	for (int i = 0; i < controller->getNumberOfDepots(); i++)
	{
		int numVehicles_i = this->vehicles[i]->getSize();

		for (int j = 0; j < numVehicles_i; j++)
		{
			vehPtr = (Vehicle *)this->vehicles[i]->getFrogObject(j);
			vehPtr->writeFrogObjWithCoordinates(controller);
		}
	}

	fprintf(pFile, "DecodedFrogLeapSolution FINISHED \n");
}

bool DecodedFrogLeapSolution::isTheSame(FrogObject * fs)
{
	return (this == fs);
}

void DecodedFrogLeapSolution::setController(FrogLeapController * controller)
{
	this->ptrController = controller;
}

FrogLeapController * DecodedFrogLeapSolution::getController()
{
	return this->ptrController;
}

void DecodedFrogLeapSolution::setIsFeasibleSolution(bool v_isFeasible)
{
	this->isFeasibleSolution = v_isFeasible;
}

bool DecodedFrogLeapSolution::getIsFeasibleSolution()
{
	return this->isFeasibleSolution;
}

float DecodedFrogLeapSolution::applyLocalSearch(FrogLeapController * controller)
{
	Vehicle * vehPtr = NULL;

	float newLocalCostFound = 0;

	this->localSearchApplied = true;

	for(int i = 0; i < this->numDepots; i++)
	{
		for (int j = 0; j < this->vehicles[i]->getSize(); j++)
		{
			vehPtr = (Vehicle *)this->vehicles[i]->getFrogObject(j);
			newLocalCostFound += vehPtr->applyLocalSearch(controller);
		}
	}

	//for (int j = 0; j < this->vehicles->getSize(); j++)
	//{
	//	vehPtr = (Vehicle *)this->vehicles->getFrogObject(j);
	//	newLocalCostFound += vehPtr->applyLocalSearch(controller);
	//}

	return newLocalCostFound;
}

int DecodedFrogLeapSolution::getNotAddedCustomer()
{
	return this->notAddedCustomer;
}

void DecodedFrogLeapSolution::setNotAddedCustomer(int customerId)
{
	this->notAddedCustomer = customerId;
}

Vehicle * DecodedFrogLeapSolution::getFirstUpperValueVehicle(int customerDemand, int depotIndex)
{
	Vehicle * cur_veh = NULL, * result_veh = NULL;
	int size = this->vehicles[depotIndex]->getSize();
	int i = 0;
	bool found = false;

	while (i < size && found == false)
	{
		cur_veh = (Vehicle *)this->vehicles[depotIndex]->getFrogObject(i);

		if(cur_veh->getDepotIndex() == depotIndex)		
		{
			result_veh = cur_veh;
			found = true;
		}
		else
		{
			i++;
		}		
	}

	return result_veh;
}

void DecodedFrogLeapSolution::adjustVehicleRoutes(FrogLeapController * controller)
{
	for(int i = 0; i < controller->getNumberOfDepots(); i++)
	{
		this->adjustDepotVehicleRoutes(this->vehicles[i], controller);
	}	
}

void DecodedFrogLeapSolution::adjustDepotVehicleRoutes(FrogObjectCol * vehicleList, FrogLeapController * controller)
{
	Vehicle * currentVehicle = NULL;
	int size = vehicleList->getSize();

	for(int i = 0; i < size; i++)
	{
		currentVehicle = (Vehicle *)vehicleList->getFrogObject(i);

		currentVehicle->adjustCustomerRoute(controller);
	}
}

//Pair * DecodedFrogLeapSolution::calculateAngularValue(int customerIndex, int depotIndex, FrogLeapController * controller)
//{
//	return nullptr;
//}
