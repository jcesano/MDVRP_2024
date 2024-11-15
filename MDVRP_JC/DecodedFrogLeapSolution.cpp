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


	if(this->assignedCustomers != NULL)
	{
		this->deleteArrayOfFrogObjectCols(this->assignedCustomers, nDepots);
		delete[] assignedCustomers;
	}

	this->assignedCustomers = NULL;

	this->ptrController = NULL;		

	//printf("Destroying of DecodedFrogLeapSolution: FINISHED \n");
}

void DecodedFrogLeapSolution::deleteArrayOfFrogObjectCols(FrogObjectCol ** arrayPtr, int v_size)
{
	int size = v_size;

	for (int i = 0; i < size; i++)
	{
		//if(this->unReferenceItemsBeforeDelete == true)
		//{
			arrayPtr[i]->unReferenceFrogObjectCol();
		//}

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

void DecodedFrogLeapSolution::setDepotCustomersAsNotAssignedToVehicles(int depotIndex, FrogLeapController * controller)
{
	int customerSize = this->assignedCustomers[depotIndex]->getSize();

	for(int i = 0; i < customerSize; i++)
	{
		Pair* customerPair = (Pair*)this->assignedCustomers[depotIndex]->getFrogObject(i);
		customerPair->setCustomerAssignedToVehicle(-1);
	}
}

void DecodedFrogLeapSolution::setCustomersAsNotAssignedToVehicles(FrogLeapController * controller)
{
	for(int i=0;i<this->numDepots;i++)
	{
		setDepotCustomersAsNotAssignedToVehicles(i, controller);
	}
}

void DecodedFrogLeapSolution::mixed_assignDecodedCustomersToVehicles(FrogLeapController* controller)
{
	// Here reset all customers as not assigned to any vehicle
	setCustomersAsNotAssignedToVehicles(controller);
	//////


	for (int i = 0; i < this->numDepots; i++)
	{
		mixed_assignDecodedCustomersToDepotVehicles(i, controller);
	}

}

void DecodedFrogLeapSolution::scn_assignDecodedCustomersToVehicles(FrogLeapController* controller)
{
	// Here reset all customers as not assigned to any vehicle
	setCustomersAsNotAssignedToVehicles(controller);
	//////


	for (int i = 0; i < this->numDepots; i++)
	{
		scn_assignDecodedCustomersToDepotVehicles(i, controller);
	}
}

void DecodedFrogLeapSolution::assignDecodedCustomersToVehicles(FrogLeapController * controller)
{
	// Here reset all customers as not assigned to any vehicle
	setCustomersAsNotAssignedToVehicles(controller);
	//////


	for (int i = 0; i < this->numDepots; i++)
	{
		assignDecodedCustomersToDepotVehicles(i, controller);
	}
}

void DecodedFrogLeapSolution::cw2_assignDecodedCustomersToVehicles(FrogLeapController* controller, FrogLeapSolution* fls)
{
	for (int i = 0; i < this->numDepots; i++)
	{
		cw2_assignDecodedCustomersToDepotVehicles(i, controller, fls);
	}
}

void DecodedFrogLeapSolution::cw_assignDecodedCustomersToVehicles(FrogLeapController* controller, FrogLeapSolution * fls)
{
	for (int i = 0; i < this->numDepots; i++)
	{
		cw_assignDecodedCustomersToDepotVehicles(i, controller, fls);
	}
}

//void DecodedFrogLeapSolution::assignDecodedCustomersToDepotVehicles(int depotIndex, FrogLeapController* controller)
//{
//	int size = this->assignedCustomers[depotIndex]->getSize();
//	Vehicle* veh = NULL;
//	Pair* customerPair;
//
//	for (int i = 0; i < size; i++)
//	{
//		customerPair = (Pair*)this->assignedCustomers[depotIndex]->getFrogObject(i);
//
//		// if customer is not already assigned to a vehicle
//		if (customerPair->isAlreadyAssignedToVehicle() == false)
//		{
//			//assign vehicle to customer	
//			//get the element with minimum remaining capacity enough		
//			int customerDemand = customerPair->get_i_IntValue();
//
//			veh = (Vehicle*)this->vehicles[depotIndex]->getFirstUpperValueFrogObject(customerDemand);
//
//			int vehicleId;
//
//			if (veh == NULL)
//			{
//				vehicleId = controller->getGlobalVehicleId();
//
//				veh = new Vehicle(vehicleId, this->ptrController);
//
//				veh->decRemainingCapacity(customerDemand);
//				veh->incDemand(customerDemand);
//				//int depotIndex = vehicleId / numberOfDepots;
//				veh->setDepotIndex(depotIndex);
//
//				this->vehicles[depotIndex]->addFrogObjectOrdered(veh);
//			}
//			else
//			{
//				veh->decRemainingCapacity(customerDemand);
//				veh->incDemand(customerDemand);
//				//this->vehicles[depotIndex]->reorderFrogObject(veh);
//				this->vehicles[depotIndex]->reorderFrogObject(veh);
//			}
//
//			customerPair->setCustomerAssignedToVehicle(veh->getId());
//			veh->addLastCustomerPair(customerPair);
//
//		}
//	}
//}

Vehicle * DecodedFrogLeapSolution::createVehicleAssignedToDepot(int depotIndex, FrogLeapController* controller)
{
	int vehicleId = controller->getGlobalVehicleId();

	Vehicle * veh = new Vehicle(vehicleId, controller);
	veh->setDepotIndex(depotIndex);
	return veh;
}

void DecodedFrogLeapSolution::addCustomerToVehicleRoute(Pair * nextClosestCustomer, Vehicle * veh)
{
	int customerDemand = nextClosestCustomer->getCustomerDemand();
	veh->decRemainingCapacity(customerDemand);
	veh->incDemand(customerDemand);
	veh->addLastCustomerPair(nextClosestCustomer);

	nextClosestCustomer->setCustomerAssignedToVehicle(veh->getId());	
}

void DecodedFrogLeapSolution::assignCustomersToVehicle(Vehicle * veh, int depotIndex, FrogLeapController * controller)
{
	int currentRemainingCapacity = controller->getVehicleCapacity();

	//bool vehicleCompleted = false;

	// we start ordering customers from depot
	Pair * nextClosestCustomer = getClosestCustomerWithCapacityIndexToDepot(depotIndex, controller->getVehicleCapacity(), controller);

	while(nextClosestCustomer != NULL /* && vehicleCompleted == false*/)
	{
		addCustomerToVehicleRoute(nextClosestCustomer, veh);

		nextClosestCustomer = this->getClosestCustomerIndexWithCapacityToCustomer(nextClosestCustomer->getId(), depotIndex, controller, veh->getRemainingCapacity());
	}

	return;
}

Vehicle * DecodedFrogLeapSolution::createVehicleWithCustomersAssigned(int depotIndex, FrogLeapController* controller)
{
	// create a new Vehicle assigned to depot
	Vehicle * veh = this->createVehicleAssignedToDepot(depotIndex, controller);

	this->assignCustomersToVehicle(veh, depotIndex, controller);

	return veh;
}

bool DecodedFrogLeapSolution::unassignedCustomerToVehicleExists(int depotIndex, FrogLeapController * controller)
{
	bool result = false;

	int size = this->assignedCustomers[depotIndex]->getSize();

	// If there is not any customer in the depotIndex cluster, return false
	if(size == 0)
	{
		return false;
	}

	int i = 0;
	while(result == false && i < size)
	{
		Pair* currentCustomer = (Pair *)this->assignedCustomers[depotIndex]->getFrogObject(i);

		if(!currentCustomer->isAlreadyAssignedToVehicle())
		{
			result = true;
		}

		i++;
	}

	return result;
}

FrogObjectCol * DecodedFrogLeapSolution::copyVehicles(int depotIndex) 
{
	FrogObjectCol* vehicles_list_copy = new FrogObjectCol();
	Vehicle* v_copy = NULL, *current_vehicle;
	int size = this->vehicles[depotIndex]->getSize();

	for(int i = 0; i < size;i++)
	{
		current_vehicle = (Vehicle*)this->vehicles[depotIndex]->getFrogObject(i);
		v_copy = current_vehicle->copy();

		vehicles_list_copy->addLastFrogObject(v_copy);
	}

	return vehicles_list_copy;
}


void DecodedFrogLeapSolution::resetCustomersToDepotVehicles(int depotIndex, FrogLeapController* controller)
{
	int size = this->vehicles[depotIndex]->getSize();

	for(int i=0; i<size;i++)
	{
		Vehicle* current_v = (Vehicle *)this->vehicles[depotIndex]->getFrogObject(0);
		this->vehicles[depotIndex]->removeFrogObjectByPosition(0);
		current_v->unreferenceAndDeleteCustomerList();
		delete current_v;
	}

	delete this->vehicles[depotIndex];
	this->vehicles[depotIndex] = new FrogObjectCol();
}

void DecodedFrogLeapSolution::unreferenceAndDeleteVehicles(FrogObjectCol * & vehicles)
{
	int size = vehicles->getSize();

	for (int i = 0; i < size; i++)
	{
		Vehicle* current_v = (Vehicle*)vehicles->getFrogObject(0);
		vehicles->removeFrogObjectByPosition(0);
		current_v->unreferenceAndDeleteCustomerList();
		delete current_v;
	}

	delete vehicles;	
}

void DecodedFrogLeapSolution::mixed_assignDecodedCustomersToDepotVehicles(int depotIndex, FrogLeapController* controller)
{
	scn_assignDecodedCustomersToDepotVehicles(depotIndex, controller);

	FrogObjectCol * scn = this->copyVehicles(depotIndex);

	this->resetCustomersToDepotVehicles(depotIndex, controller);

	//testing code
	int depotLabelId = ((Pair *)controller->getDepotPairByIndex(depotIndex))->getLabelId();
	// end testing code
	
	float scn_value = this->evalVehiclePaths(scn);

	this->assignDecodedCustomersToDepotVehicles(depotIndex, controller);	

	FrogObjectCol* nn = this->copyVehicles(depotIndex);

	this->resetCustomersToDepotVehicles(depotIndex, controller);

	float nn_value = this->evalVehiclePaths(nn);

	if(scn_value < nn_value)
	{
		delete this->vehicles[depotIndex];
		this->vehicles[depotIndex] = scn;
		this->unreferenceAndDeleteVehicles(nn);
	}
	else
	{
		delete this->vehicles[depotIndex];
		this->vehicles[depotIndex] = nn;
		this->unreferenceAndDeleteVehicles(scn);
	}
 }

void DecodedFrogLeapSolution::scn_assignDecodedCustomersToDepotVehicles(int depotIndex, FrogLeapController* controller)
{
	int size = this->assignedCustomers[depotIndex]->getSize();
	Vehicle* veh = NULL;
	Pair* customerPair;

	for (int i = 0; i < size; i++)
	{
		customerPair = (Pair*)this->assignedCustomers[depotIndex]->getFrogObject(i);

		//assign vehicle to customer	
		//get the element with minimum remaining capacity enough		
		int customerDemand = customerPair->get_i_IntValue();

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
			//this->vehicles[depotIndex]->reorderFrogObject(veh);
		}

		veh->addLastCustomerPair(customerPair);
	}
}


void DecodedFrogLeapSolution::assignDecodedCustomersToDepotVehicles(int depotIndex, FrogLeapController * controller)
{
	int size = this->assignedCustomers[depotIndex]->getSize();
	Vehicle * veh = NULL;
	Pair * customerPair;
	bool unassignedToVehCustExists = this->unassignedCustomerToVehicleExists(depotIndex, controller);;
	
	//for(int i = 0; i < size; i++)
	while (unassignedToVehCustExists == true)
	{
		
		// assign all possible customers to the current vehicle veh
		veh = createVehicleWithCustomersAssigned(depotIndex, controller);

		this->vehicles[depotIndex]->addLastFrogObject(veh);
		
		//If there are still customers in cluster without vehicle, continue in the loop
		unassignedToVehCustExists = this->unassignedCustomerToVehicleExists(depotIndex, controller);

		//	customerPair = (Pair *) this->assignedCustomers[depotIndex]->getFrogObject(0);
		//	//this->assignedCustomers[depotIndex]->removeFirstItem();

		//	// if customer is not already assigned to a vehicle, then we assign more customers to it
		//	if(customerPair->isAlreadyAssignedToVehicle() == false)
		//	{
		//		//assign vehicle to customer	
		//		//get the element with minimum remaining capacity enough		
		//		int customerDemand = customerPair->get_i_IntValue();

		//		veh = (Vehicle*)this->vehicles[depotIndex]->getFirstUpperValueFrogObject(customerDemand);

		//		int vehicleId;

		//		if (veh == NULL)
		//		{
		//			vehicleId = controller->getGlobalVehicleId();

		//			veh = new Vehicle(vehicleId, this->ptrController);

		//			veh->decRemainingCapacity(customerDemand);
		//			veh->incDemand(customerDemand);
		//			//int depotIndex = vehicleId / numberOfDepots;
		//			veh->setDepotIndex(depotIndex);

		//			this->vehicles[depotIndex]->addFrogObjectOrdered(veh);
		//		}
		//		else
		//		{
		//			veh->decRemainingCapacity(customerDemand);
		//			veh->incDemand(customerDemand);
		//			//this->vehicles[depotIndex]->reorderFrogObject(veh);
		//			this->vehicles[depotIndex]->reorderFrogObject(veh);
		//		}

		//		customerPair->setCustomerAssignedToVehicle(veh->getId());
		//		veh->addLastCustomerPair(customerPair);
		//		this->assignedCustomers[depotIndex]->removeFirstItem();
		//	}
	}
}

void DecodedFrogLeapSolution::unassignDecodedCustomersFromDepotVehicles(int depotIndex, FrogLeapController * controller, FrogLeapSolution * fls)
{
	int size = this->vehicles[depotIndex]->getSize();
	Vehicle* veh = NULL;
			
	for(int i = 0; i < size; i++)
	{
		// get and remove the first vehicle from the vehicle set of the depot
		Vehicle* veh = (Vehicle*)this->vehicles[depotIndex]->getFrogObject(0);
		this->vehicles[depotIndex]->removeFrogObject(0);

		veh->unreferenceAndDeleteCustomerList();
		delete veh;
		veh = NULL;
	}
}

void DecodedFrogLeapSolution::cw2_assignDecodedCustomersToDepotVehicles(int depotIndex, FrogLeapController* controller, FrogLeapSolution* fls)
{
	//int size = this->assignedCustomers[depotIndex]->getSize();
	FrogObjectCol* clusterCustomers = fls->getClusterByIndex(depotIndex)->getCustomerCol();
	int nCustomers = clusterCustomers->getSize();
	Vehicle* veh = NULL;
	int vehicleId;

	int cw_handler_col_size = fls->getCWHandlerCol()->getSize();

	ClarkWrightHandler* depot_cw = (ClarkWrightHandler*)fls->getCWHandlerCol()->getFrogObject(depotIndex);

	int routes_size = depot_cw->getFinalRoutesSize();

	// decode routes
	for (int j = 0; j < routes_size; j++)
	{
		FrogObjectCol* current_route = depot_cw->getRoute(j);

		// getting the Depot of route located in the first and last position of the route
		Pair * start_depot = (Pair *)current_route->getFrogObject(0);
				
		int current_route_size = current_route->getSize();
		Pair * final_depot = (Pair*)current_route->getFrogObject(current_route_size-1);
		
		// create the new vehicle for the current route
		vehicleId = controller->getGlobalVehicleId();
		veh = new Vehicle(vehicleId, this->ptrController);
		veh->setDepotIndex(depotIndex);
		this->vehicles[depotIndex]->addLastFrogObject(veh);

		// adding the depot as the starting node for vehicle
		veh->addLastCustomerPair(start_depot);


		// adding customers of the current_route. This is the decoding of customers
		for (int k = 1; k < current_route_size-1; k++)
		{
			Pair* customerPair = (Pair*)current_route->getFrogObject(k);
			int customerDemand = customerPair->get_i_IntValue();
			
			int veh_remaining_capacity = veh->getRemainingCapacity();

			if(veh_remaining_capacity >= customerDemand)
			{
				// assign the customer to vehicle
				veh->decRemainingCapacity(customerDemand);
				veh->incDemand(customerDemand);
				veh->addLastCustomerPair(customerPair);
			}
			else
			{
				// we add the depot as the last node for the route of vehicle and create a new vehicle to continue with the current route
				veh->addLastCustomerPair(final_depot);

				// we create a new vehicle to continue with the rest of customers of current_route		
				vehicleId = controller->getGlobalVehicleId();
				veh = new Vehicle(vehicleId, this->ptrController);
				veh->setDepotIndex(depotIndex);
				this->vehicles[depotIndex]->addLastFrogObject(veh);

				// adding the depot as the starting node for vehicle
				veh->addLastCustomerPair(start_depot);

				// assign the customer to vehicle
				veh->decRemainingCapacity(customerDemand);
				veh->incDemand(customerDemand);
				veh->addLastCustomerPair(customerPair);
			}			
		}

		// we add the depot as the last node for the route of vehicle and create a new vehicle to continue with the current route
		veh->addLastCustomerPair(final_depot);
	}
}


void DecodedFrogLeapSolution::cw_assignDecodedCustomersToDepotVehicles(int depotIndex, FrogLeapController* controller, FrogLeapSolution * fls)
{
	//int size = this->assignedCustomers[depotIndex]->getSize();
	FrogObjectCol* clusterCustomers = fls->getClusterByIndex(depotIndex)->getCustomerCol();
	int nCustomers = clusterCustomers->getSize();
	Vehicle* veh = NULL;
	
	int cw_handler_col_size = fls->getCWHandlerCol()->getSize();
	
	ClarkWrightHandler * depot_cw = (ClarkWrightHandler*)fls->getCWHandlerCol()->getFrogObject(depotIndex);

	int routes_size = depot_cw->getFinalRoutesSize();
		
	// decode routes
	for(int j = 0; j < routes_size; j++)
	{
		FrogObjectCol* current_route = depot_cw->getRoute(j);
			 
		// remove the first and last item because each of then corresponds to the depot
		current_route->removeFirstItem();
		current_route->removeLastItem();

		int current_route_size = current_route->getSize();

		// decode customers
		for(int k=0; k < current_route_size; k++)
		{
			Pair * customerPair = (Pair *)current_route->getFrogObject(k);
			int customerDemand = customerPair->get_i_IntValue();
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

				this->vehicles[depotIndex]->addLastFrogObject(veh);
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

	this->assignedCustomers[depotIndex]->addLastFrogObject(customerPair);
}

Pair * DecodedFrogLeapSolution::getClosestCustomerWithCapacityIndexToDepot(int depotIndex, int capacity, FrogLeapController* controller)
{
	// get the closest customerPair (not assigned to vehicle) to depot with depotIndex
	int closestCustomerIndexToDepot = controller->getClosestCustomerLocalIndexWithCapacityToDepot(depotIndex, 0, this->assignedCustomers[depotIndex]->getSize(), this->assignedCustomers[depotIndex], controller->getVehicleCapacity());
	if(closestCustomerIndexToDepot != -1)
	{
		return (Pair*)this->assignedCustomers[depotIndex]->getFrogObject(closestCustomerIndexToDepot);
	}
	
	return NULL;
}

Pair * DecodedFrogLeapSolution::getClosestCustomerIndexToDepot(int depotIndex, FrogLeapController * controller) 
{
	// get the closest customerPair to depot with depotIndex
	int closestCustomerIndexToDepot = controller->getClosestCustomerLocalIndexToDepot(depotIndex, 0, this->assignedCustomers[depotIndex]->getSize(), this->assignedCustomers[depotIndex]);
	return (Pair *)this->assignedCustomers[depotIndex]->getFrogObject(closestCustomerIndexToDepot);	
}

Pair * DecodedFrogLeapSolution::getClosestCustomerIndexWithCapacityToCustomer(int customerInternalId, int depotIndex, FrogLeapController* controller, int veh_rem_capacity)
{
	// get the closest customerPair to customer with depotindex
	int closestCustomerIndexToCustomer = controller->getClosestCustomerLocalIndexWithCapacityToCustomer(customerInternalId, 0, this->assignedCustomers[depotIndex]->getSize(), this->assignedCustomers[depotIndex], veh_rem_capacity);

	if(closestCustomerIndexToCustomer != -1)
	{
		return (Pair*)this->assignedCustomers[depotIndex]->getFrogObject(closestCustomerIndexToCustomer);
	}

	return NULL;	
}


Pair * DecodedFrogLeapSolution::getClosestCustomerIndexToCustomer(int customerInternalId, int depotIndex, FrogLeapController * controller)
{
	// get the closest customerPair to customer with depotindex
	int closestCustomerIndexToCustomer = controller->getClosestCustomerLocalIndexToCustomer(customerInternalId, 0, this->assignedCustomers[depotIndex]->getSize(), this->assignedCustomers[depotIndex]);

	return (Pair *)this->assignedCustomers[depotIndex]->getFrogObject(closestCustomerIndexToCustomer);
}


//FrogObjectCol * DecodedFrogLeapSolution::extractCustomersWithClosestNextCriteriaAndCapacity(int depotIndex, FrogLeapController * controller)
//{
//	FrogObjectCol * orderedCol = new FrogObjectCol();
//
//	// get the closest customerPair to depot with depotIndex	
//	Pair * customerPair = this->getClosestCustomerLocalIndexWithCapacityToDepot(depotIndex, controller, controller->getVehicleCapacity());
//
//
//}

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


float DecodedFrogLeapSolution::cw2_evalSolution()
{
	Vehicle* vehPtr = NULL;
	float result = 0;

	for (int i = 0; i < this->numDepots; i++)
	{
		Pair* depotPair_i = (Pair*)this->ptrController->getDepotPairByIndex(i);
		float eval = cw2_evalDepotSolution(i);
		result = result + eval;
	}

	//for (int j = 0; j < this->vehicles->getSize(); j++)
	//{
	//	vehPtr = (Vehicle *)this->vehicles->getFrogObject(j);
	//	result = result + vehPtr->evalPath(this->ptrController);
	//}	

	return result;
}


float DecodedFrogLeapSolution::cw_evalSolution()
{
	Vehicle* vehPtr = NULL;
	float result = 0;

	for (int i = 0; i < this->numDepots; i++)
	{
		Pair* depotPair_i = (Pair*)this->ptrController->getDepotPairByIndex(i);
		float eval = cw_evalDepotSolution(i);
		result = result + eval;
	}

	//for (int j = 0; j < this->vehicles->getSize(); j++)
	//{
	//	vehPtr = (Vehicle *)this->vehicles->getFrogObject(j);
	//	result = result + vehPtr->evalPath(this->ptrController);
	//}	

	return result;
}


float DecodedFrogLeapSolution::evalSolution()
{
	Vehicle * vehPtr = NULL;
	float result = 0;

	for(int i = 0; i < this->numDepots; i++)
	{
		Pair * depotPair_i = (Pair *) this->ptrController->getDepotPairByIndex(i);
		float eval = evalDepotSolution(i);
		result = result + eval;
	}

	//for (int j = 0; j < this->vehicles->getSize(); j++)
	//{
	//	vehPtr = (Vehicle *)this->vehicles->getFrogObject(j);
	//	result = result + vehPtr->evalPath(this->ptrController);
	//}	

	return result;
}

float DecodedFrogLeapSolution::cw2_evalDepotSolution(int depotIndex)
{
	Vehicle* vehPtr;
	float result = 0;

	int nsize = this->vehicles[depotIndex]->getSize();
	for (int j = 0; j < nsize; j++)
	{
		vehPtr = (Vehicle*)this->vehicles[depotIndex]->getFrogObject(j);
		result = result + vehPtr->cw2_evalPath(this->ptrController);
	}

	return result;
}


float DecodedFrogLeapSolution::cw_evalDepotSolution(int depotIndex)
{
	Vehicle* vehPtr;
	float result = 0;

	int nsize = this->vehicles[depotIndex]->getSize();
	for (int j = 0; j < nsize; j++)
	{
		vehPtr = (Vehicle*)this->vehicles[depotIndex]->getFrogObject(j);
		result = result + vehPtr->cw_evalPath(this->ptrController);
	}

	return result;
}

float DecodedFrogLeapSolution::evalDepotSolution(int depotIndex)
{
	Vehicle* vehPtr;
	float result = 0;

	int nsize = this->vehicles[depotIndex]->getSize();
	for (int j = 0; j < nsize; j++)
	{
		vehPtr = (Vehicle*)this->vehicles[depotIndex]->getFrogObject(j);
		result = result + vehPtr->evalPath(this->ptrController);
	}

	return result;
}

float DecodedFrogLeapSolution::evalVehiclePaths(FrogObjectCol * vehicles)
{
	Vehicle* vehPtr;
	float result = 0; 
	int size = vehicles->getSize();

	for (int i = 0; i < size; i++)
	{
		vehPtr = (Vehicle*)vehicles->getFrogObject(i);
		result = result + vehPtr->evalPath(this->ptrController);
	}

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

void DecodedFrogLeapSolution::cw_printDecodedSolutionByDepot(int i, SolutionData * sd)
{
	Vehicle* vehPtr;

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
		vehPtr->cw_printFrogObj(this->ptrController, sd);
	}	
}

void DecodedFrogLeapSolution::printDecodedSolutionByDepot(int i, SolutionData * sd)
{
	Vehicle* vehPtr;

	//SolutionData* sd = new SolutionData();

	//sd->printSolutionDataHeader();

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


void DecodedFrogLeapSolution::cw_printFrogObjWithSolutionData()
{
	SolutionData * sd = new SolutionData();

	sd->printSolutionDataHeader();

	for (int i = 0; i < this->numDepots; i++)
	{
		this->cw_printDecodedSolutionByDepot(i, sd);
	}

	delete sd;
}


void DecodedFrogLeapSolution::printFrogObjWithSolutionData()
{
	
	SolutionData* sd = new SolutionData();

	sd->printSolutionDataHeader();

	
	for(int i=0; i<this->numDepots; i++)
	{
		this->printDecodedSolutionByDepot(i, sd);
	}
	
	delete sd;
	//Vehicle* vehPtr;

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

	//SolutionData* sd = new SolutionData();

	//sd->printSolutionDataHeader();

	//for (int i = 0; i < this->numDepots; i++)
	//{
	//	int internalId = this->ptrController->getDepotInternalId(i);
	//	sd->setDepotIndex(i);
	//	sd->setDepotInternalId(internalId);
	//	sd->setDepotLabelId(this->ptrController->getLabelId(internalId));
	//	sd->setDepotCap(this->ptrController->getDepotCapacityByIndex(i));
	//	sd->setDepotRemCap(this->ptrController->getDepotRemainingCapacityByIndex(i));
	//	/*int internalId = this->ptrController->getDepotInternalId(i);*/
	//	/*int labelId = this->ptrController->getLabelId(internalId);*/
	//	/*int depotCapacity = this->ptrController->getDepotCapacityByIndex(i);
	//	int depotRemaniningCapacity = this->ptrController->getDepotRemainingCapacityByIndex(i);*/
	//	//fprintf(pFile, "Deposito index: %d; internalId: %d ; LabelId = ; Capacity = %d, RemainingCapacity = %d \n", i, internalId, labelId, depotCapacity, depotRemaniningCapacity);

	//	int numVehicles_i = this->vehicles[i]->getSize();

	//	sd->setNumVehicles(numVehicles_i);

	//	//fprintf(pFile, "Cantidad de vehiculos: %d \n", numVehicles_i);

	//	for (int j = 0; j < numVehicles_i; j++)
	//	{
	//		vehPtr = (Vehicle*)this->vehicles[i]->getFrogObject(j);
	//		vehPtr->printFrogObj(this->ptrController, sd);
	//	}
	//}

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
