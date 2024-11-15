#pragma once
#include "FrogObject.h"

class SolutionData : public FrogObject
{
	int depotIndex;
	int depotLabelId;
	int depotInternalId;
	int depotCap;
	int depotRemainingCap;
	int numVehicles;
	float routeCost;
	int routeLength;

public:

	void setDepotIndex(int v_depotIndex);
	void setDepotLabelId(int v_depotId);
	void setDepotInternalId(int v_depotInternalId);
	void setDepotCap(int v_depotCap);
	void setDepotRemCap(int v_depotRemCap);
	void setNumVehicles(int v_numVehicles);
	void setRouteCost(float v_routeCost);
	void setRouteLength(int v_routeLength);

	int getDepotIndex();
	int getDepotLabelId();
	int getDepotInternalId();
	int getDepotCap();
	int getDepotRemCap();
	int getNumVehicles();
	float getRouteCost();
	int getRouteLength();

	void writeSolutionDataHeader(FILE* pFile);
	void printSolutionDataHeader();
	void writeSolutionData(FILE* pFile);
	void printSolutionData();
	void writeClusterSolutionData(FILE* pFile);
	void printClusterSolutionData();



	// abstract methods
	void printFrogObj();

	bool isTheSame(FrogObject* fs);
};

