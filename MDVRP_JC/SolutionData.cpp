#include "stdafx.h"
#include "SolutionData.h"

void SolutionData::setDepotIndex(int v_depotIndex)
{
	this->depotIndex = v_depotIndex;
}

void SolutionData::setDepotLabelId(int v_depotId)
{
	this->depotLabelId = v_depotId;
}

void SolutionData::setDepotInternalId(int v_depotInternalId)
{
	this->depotInternalId = v_depotInternalId;
}

void SolutionData::setDepotCap(int v_depotCap)
{
	this->depotCap = v_depotCap;
}

void SolutionData::setDepotRemCap(int v_depotRemCap)
{
	this->depotRemainingCap = v_depotRemCap;
}

void SolutionData::setNumVehicles(int v_numVehicles)
{
	this->numVehicles = v_numVehicles;
}

void SolutionData::setRouteCost(float v_routeCost)
{
	this->routeCost = v_routeCost;
}

void SolutionData::setRouteLength(int v_routeLength)
{
	this->routeLength;
}

int SolutionData::getDepotIndex()
{
	return this->depotIndex;
}

int SolutionData::getDepotLabelId()
{
	return this->depotLabelId;
}

int SolutionData::getDepotInternalId()
{
	return this->depotInternalId;
}

int SolutionData::getDepotCap()
{
	return this->depotCap;
}

int SolutionData::getDepotRemCap()
{
	return this->depotRemainingCap;
}

int SolutionData::getNumVehicles()
{
	return this->numVehicles;
}

float SolutionData::getRouteCost()
{
	return this->routeCost;
}

int SolutionData::getRouteLength()
{
	return this->routeCost;
}

void SolutionData::writeSolutionDataHeader(FILE* pFile)
{
	fprintf(pFile, "DepotIndex; DepotInternalId; DepotLabelId; DepotCapacity; DepotRemainingCap; NumVehicles; RouteCost; Route; RouteDemand \n");
}

void SolutionData::printSolutionDataHeader()
{
	printf("DepotIndex; DepotInternalId; DepotLabelId; DepotCapacity; DepotRemainingCap; NumVehicles; RouteCost; Route; RouteDemand; RouteLength \n");
}

void SolutionData::writeSolutionData(FILE* pFile)
{
	//fprintf(pFile, "DepotIndex; DepotInternalId; DepotLabelId; DepotCapacity; DepotRemainingCap; RouteCost; Route");
	//printf("%d; %d; %d; %d; %d; %d; %.2f; ", this->getDepotIndex(), this->getDepotInternalId(), this->getDepotLabelId(), this->getDepotCap(), this->getDepotRemCap(), this->getNumVehicles(), this->getRouteCost()); //the last data is the complete route put from outside of this procedure
	fprintf(pFile, "%d; %d; %d; %d; %d; %d; %.2f; ", this->getDepotIndex(), this->getDepotInternalId(), this->getDepotLabelId(), this->getDepotCap(), this->getDepotRemCap(), this->getNumVehicles(), this->getRouteCost()); //the last data is the complete route put from outside of this procedure
}

void SolutionData::printSolutionData()
{
	//fprintf(pFile, "DepotIndex; DepotInternalId; DepotLabelId; DepotCapacity; DepotRemainingCap; RouteCost; Route");
	//printf("%d; %d; %d; %d; %d; %d; %.2f; ", this->getDepotIndex(), this->getDepotInternalId(), this->getDepotLabelId(), this->getDepotCap(), this->getDepotRemCap(), this->getNumVehicles(), this->getRouteCost()); //the last data is the complete route put from outside of this procedure
	printf("%d; %d; %d; %d; %d; %d; %.2f; ", this->getDepotIndex(), this->getDepotInternalId(), this->getDepotLabelId(), this->getDepotCap(), this->getDepotRemCap(), this->getNumVehicles(), this->getRouteCost()); //the last data is the complete route put from outside of this procedure
}

void SolutionData::writeClusterSolutionData(FILE * pFile)
{
	//fprintf(pFile, "DepotIndex; DepotInternalId; DepotLabelId; DepotCapacity; DepotRemainingCap; AssignedCustomers; TotalCustomerDemand \n");
	fprintf(pFile, "%d; %d; %d; %d; %d; ", this->getDepotIndex(), this->getDepotInternalId(), this->getDepotLabelId(), this->getDepotCap(), this->getDepotRemCap()); //the last data is the complete route put from outside of this procedure
}

void SolutionData::printClusterSolutionData()
{
	//fprintf(pFile, "DepotIndex; DepotInternalId; DepotLabelId; DepotCapacity; DepotRemainingCap; AssignedCustomers; TotalCustomerDemand \n");
	printf("%d; %d; %d; %d; %d; ", this->getDepotIndex(), this->getDepotInternalId(), this->getDepotLabelId(), this->getDepotCap(), this->getDepotRemCap()); //the last data is the complete route put from outside of this procedure
}

// abstract methods
void SolutionData::printFrogObj()
{
}

bool SolutionData::isTheSame(FrogObject* fs)
{
	return true;
}
