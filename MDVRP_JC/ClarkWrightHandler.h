#ifndef CLARKWRIGHTHANDLER_H_   /* Include guard */
#define CLARKWRIGHTHANDLER_H_

#include "FrogObject.h"
#include "FrogObjectCol.h"
#include "Pair.h"
#include "FrogLeapController.h"
#include "FrogLeapSolution.h"
#include "Cluster.h"
#include "DistanceTable.h"

enum class LocationType {No_Exists, At_Begin, At_Last, At_Middle};

class ClarkWrightHandler :  public FrogObject
{
	DistanceTable* distanceTable_ptr;
	FrogObjectCol* savingsList;
	FrogObjectCol* * routes;
	FrogObjectCol* * final_routes;
	int final_routes_size;
	Cluster* cluster;	
	long int vehicleCapacity;


private:
	void create_SavingsList();

	void create_Routes();

	float eval_final_route(int i);

	void print_final_route(int i);
	
	void print_final_routes();

	void select_final_routes();

	void create_Null_Routes();

	void printRoutesStats();

	int getNumberOfNullRoutes();

	int getNumberOfNotNullRoutes();

	FrogObjectCol * create_Route(int i);

	void merge_Routes();

	int findDepotInRoutes(LocationType& result);
	
	int findInRoutes(int customerClusterInternalId, LocationType& result_i);

	enum class LocationType findDepotInRoute(FrogObjectCol* route);

	enum class LocationType findInRoute(FrogObjectCol* route, int customerClusterIndex);

	bool internalIdsExistInDifferentRoutes(int index_route_i, int index_route_j, LocationType result_i, LocationType result_j);	

	bool internalIdsExistInSameRoute(int route_index_i, int route_index_j);

	bool noIndexExists(int index_route_i, int route_j);

	void merge_diff_routes(int routeIndex_i, int routeIndex_j, LocationType result_i, LocationType result_j, int internalId_route_i, int internalId_route_j);

	int routeItem_LocationInRoute(int route_index, int internalId_route_i);

	bool internalIdsDoNotExist(LocationType result_i, LocationType result_j);

	void add_New_Route(int routeIndex_i, int routeIndex_j, LocationType result_i, LocationType result_j, int internalId_route_i, int internalId_route_j);

	int findEmptyRouteIndex();

	bool onlyInternalId_i_ExistsAtBegin(LocationType result_i, LocationType result_j);

	bool onlyInternalId_i_ExistsAtEnd(LocationType result_i, LocationType result_j);

	bool onlyInternalId_j_ExistsAtBegin(LocationType result_i, LocationType result_j);

	bool onlyInternalId_j_ExistsAtEnd(LocationType result_i, LocationType result_j);

	void add_Node_AtPosition_into_route(int routeIndex, LocationType position_to_add, int internalId_node_to_add);

	int getRouteDemand(FrogObjectCol* route);

	void deleteFinalRoutes();

public:
	ClarkWrightHandler(FrogLeapController* controller, Cluster* v_cluster);

	~ClarkWrightHandler();

	void cw_execute();

	FrogObjectCol* getRoute(int route_index);

	int getFinalRoutesSize();

	// abstract methods
	void printFrogObj();

	bool isTheSame(FrogObject* fs);
};

#endif