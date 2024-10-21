#ifndef CLUSTER_H_   /* Include guard */
#define CLUSTER_H_

//=================================
// included dependencies
#include "FrogObject.h"

class FrogLeapController;
class FrogObject;
class FrogObjectCol;
class Pair;
class DistanceTable;

enum class DistanceType { mean, nearest, furthest, depot};

class Cluster : public FrogObject
{	

	Pair * assignedDepotPair;
	Pair * nearestCluster;

	FrogObjectCol * customerPairCol;

	enum class DistanceType distance_t;

public:
	Cluster(int clusterId);
	
	Cluster(Pair * depotPair, int clusterId);
	
	virtual ~Cluster();

	Pair * getDepotPair();

	void setDepot(int depotIndex, FrogLeapController * controller);	

	void setDistanceType(DistanceType distance_t);

	void addCustomer(int customerIndex, FrogLeapController * controller);

	FrogObjectCol * getCustomerCol();

	void setCustomerCol(FrogObjectCol * v_pairCol);

	float getDistanceToCustomer(int customerIndex, FrogLeapController * controller);

	float getMeanDistanceToCustomer(int customerIndex, FrogLeapController * controller);

	float getNearestDistanceToCustomer(int customerIndex, FrogLeapController * controller);

	float getDepotDistanceToCustomer(int customerIndex, FrogLeapController* controller);

	float getFurthestDistanceToCustomer(int customerIndex, FrogLeapController* controller);

	//float getDistanceToDepot(int depotIndex, FrogLeapController * controller);

	void unassignCustomerFromCluster(Pair * customerPair);

	Pair * createClusterCopy(FrogLeapController * controller);

	int remainingDepotCapacity();
	
	int getDepotCapacity();

	int getDepotRemainingCapacity();

	void exportClusterToVRP(FrogLeapController* controller);

	void writeDepotLabelId(FILE* file, FrogLeapController* controller);

	void writeDepotDemand(FILE* file, FrogLeapController* controller);

	void writeCustomerColNodeDemands(FILE* file, FrogLeapController* controller);

	void writeDepotNodeCoords(FILE* file, FrogLeapController* controller);

	void writeCustomerColNodeCoords(FILE* file, FrogLeapController* controller);

	//void printClusterContentHeader();

	void printClusterContentRecord(int customerIndexInCol);

	// abstract methods
	void printFrogObj();

	bool isTheSame(FrogObject * fs);
};
#endif
