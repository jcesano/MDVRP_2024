#ifndef FROGLEAPCONTROLLER_H_   /* Include guard */
#define FROGLEAPCONTROLLER_H_

class DecodedFrogLeapSolution;
class FrogObject;
class FrogObjectCol;
class TspLibEuc2D;
class TestCaseObj;
class DistanceTable;
class FloatDistanceTable;
class Graph;
class FrogLeapSolution;
class FeasibleSolution;
class Pair;
class IndexList;
class Cluster;

enum class SourceType { Graph, Tsp2DEuc, ClarkWright, Sweep};

enum class SolutionGenerationType { FrogLeaping, FixedFrogLeaping};

const int VEHICLE_CAPACITY = 500;
const long int TOPE_ITERATIONS = 10000;


class FrogLeapController
{
	private:

		long int successAttempts;
		long int failAttempts;
		long int localGeneratedSolutions;

		long int totalImprovements;
		long int localSearchImprovements;
		long int globalImprovements;
		long int vehicle_capacity;

		long long int globalVehicleId;

		int minValueIteration;

		float minCostValue;

		time_t timeSeedUsed;

		DecodedFrogLeapSolution * ptrBestSolution;

		TspLibEuc2D * tspLibEud2DPtr;

		TestCaseObj * testCaseObjPtr;

		DistanceTable * distTablePtr;
		FloatDistanceTable * floatDistTablePtr;

		Graph * graphPtr;

		SourceType source_t;

		SolutionGenerationType sgt;

		FrogObjectCol * customerList;

		FrogObjectCol * depotList;

		FrogObjectCol * vehiclePairList;		

		Pair * * vehiclePairArray;

		Pair * * customerArray;

		Pair * * depotArray;

		int origin;

		int nVehiclesPerDepot;

		FILE * pFile;
		
		char * outPutFileName;	

		bool reassignCustomers;

	public:

		FrogLeapController();

		~FrogLeapController();

		void deleteArray(Pair ** arrayPtr, int v_size);		

		void setSuccessAttempts(int vsuccessAttempts);
		int getSuccessAttempts();
		void incSuccessAttempts();

		void setFailAttempts(int vfailAttempts);
		int getFailAttempts();
		void incFailAttempts();

		int getTotalImprovements();
		void incTotalImprovements();

		int getLocalSearchImprovements();
		void incLocalSearchImprovements();

		int getGlobalSearchImprovements();
		void incGlobalSearchImprovements();

		int getLocalGeneratedSolutions();
		void incLocalGeneratedSolutions();
		
		long int getNumberOfIterations();

		float getMinCostValue();

		void setMinCostValue(float cost);

		void setBestDecodedFrogLeapSolution(DecodedFrogLeapSolution * ptrSolution);

		DecodedFrogLeapSolution * getBestDecodedFrogLeapSolution();

		void setRandomSeed();

		void applyLocalSearch();

		void printCtrl();

		void writeCtrl();

		void loadTSPEUC2D_Data(const char * fileName);

		DecodedFrogLeapSolution * loadTestCaseData(const char * fileName);

		void readTSPSection(FILE * filePtr, char * ctrlSectionTag, char * ctrlSeparatorChar, int * off_set);

		void loadTSPSection(char * buf, char * sectionTag);

		void loadAssignations(FILE * filePtr, TestCaseObj * testCaseObjPtr);

		DecodedFrogLeapSolution * loadAssignations2(FILE * filePtr, TestCaseObj * testCaseObjPtr);

		FrogLeapSolution * loadAssignations3(FILE * filePtr, TestCaseObj * testCaseObjPtr);

		void loadCoordinates(FILE * filePtr, TspLibEuc2D * tspLibEuc2DPtr);
		
		void loadDemand(FILE * filePtr, TspLibEuc2D * tspLibEuc2DPtr);

		void loadDepots(FILE * filePtr, TspLibEuc2D * tspLibEuc2DPtr);

		DistanceTable * loadDistanceTable();

		void setDistanceTable(DistanceTable * t);

		DistanceTable * getDistanceTable();

		void setFloatDistanceTable(FloatDistanceTable * t);

		FloatDistanceTable * getFloatDistanceTable();

		int getNumberOfDepots();

		int getNumberOfCustomers();

		Pair * getDepotPairByIndex(int position);

		Pair* getMatchedDepotPair(Pair* customerPair);

		Pair * getCustomerPairByIndex(int position);

		Pair * getPairByInternalId(int position);

		void setAngularValues(Pair * veh_customerPair, int customerInternalId, int depotInternalId);		

		void setGraph(Graph * gPtr);

		Graph * getGraph();

		void setSourceType(SourceType sourc);

		SourceType getSourceType();

		void setSolutionGenerationType(SolutionGenerationType v_sgt);

		SolutionGenerationType getSolutionGenerationType();

		void setAsCustomer(int customerId, int demand);
		
		void setAsCustomer(int customerId, int demand, int labelId);

		void setAsDepot(int depotId, int capacity);
		
		void setAsDepot(int depotId, int capacity, int labelId);
		
		void setUpCustomerList();

		void setUpDepotList();

		void setUpCustomerAndDepotLists();

		void setUpVehicleCapacity();

		void setUpVehicleCapacity(int vehicleCapacity);

		long int getVehicleCapacity();

		void setUpVehiclesPerDepot();

		void assignVehiclesToDepots(int depotId, int depotDemand);

		void setUpVehiclePairList();

		int getNumberOfVehicles();

		int getCustomerInternalId(int position);

		int getCustomerLabelId(int position);
		
		int getCustomerDemandByIndex(int position);

		int getDepotInternalId(int position);

		int getDepotCapacityByIndex(int position);

		int getDepotRemainingCapacityByIndex(int position);

		void decRemainingDepotCapacity(int position, int capacity_to_dec);

		void setDepotRemainingCapacityByIndex(int position, int remaining_capacity);

		FrogLeapSolution * genRandomFrogLeapSolution(FrogLeapController * controller);

		void resetCapacityOrDemand(Pair * depotPair);

		long int getTope();

		FrogObjectCol * createDepotListOrderedByCapacity();

		FrogObjectCol * createDepotListDescOrderedByCapacity();

		FrogObjectCol * createDepotListOrderedByRemainingCapacity();

		FrogObjectCol * createCustomerListOrderedByDemandDesc();

		FrogObjectCol * createCustomerListOrderedByDemand();

		void resetDepotRemainingCapacities();

		void resetCustomerRemainingDemands();

		void loadCustomerAndDepotList();

		int getLabelId(int internalId);

		int getDepotListIndexByInternal(int depotInternalId);

		int getCustomerListIndexByInternal(int customerInternalId);

		int getDepotIndexByLabelId(int depotLabelId);

		Pair * getNodeCoordPairById(int pairId);

		Pair* getNodeDemandPairById(int pairId);

		int getCustomerIndexByLabelId(int depotLabelId);

		FrogObjectCol * getTestCustomerSectionList();

		long long int getGlobalVehicleId();		

		int getCloserIndexToDepot(int depotIndex, int lowBoundIndex, int topBoundIndex, FrogObjectCol * localDepotCol);

		int getClosestLocalDepotIndexToCustomer(int customerIndex, int lowBoundIndex, int topBoundIndex, FrogObjectCol * localDepotCol, float & distance);

		int getClosestCustomerLocalIndexWithCapacityToDepot(int depotIndex, int lowBoundIndex, int topBoundIndex, FrogObjectCol* localCustomerIndexesCol, int capacity);

		int getClosestCustomerLocalIndexToDepot(int depotIndex, int lowBoundIndex, int topBoundIndex, FrogObjectCol * localNodeCol);

		int getClosestCustomerLocalIndexWithCapacityToCustomer(int customerIndex, int lowBoundIndex, int topBoundIndex, FrogObjectCol* localCustomerIndexesCol, int veh_rem_capacity);

		int getClosestCustomerLocalIndexToCustomer(int customerIndex, int lowBoundIndex, int topBoundIndex, FrogObjectCol * localCustomerIndexesCol);

		int getNextClosestFittingCustomerIndexFrom(Pair* item_pair, int remaining_capacity, int depotIndex, int lowBoundIndex, int topBoundIndex, FrogObjectCol* localCustomerIndexesCol, FrogLeapController* controller);

		bool existInLocalDepotList(int assignedDepotIndex, FrogObjectCol * localDepotCol, int low, int top);

		int getDepotIndexOfClosestAssignedCustomer(int customerIndex, FrogObjectCol * localDepotCol, int low, int top, float & distanceToCustomer);

		bool unassignedCustomerPairExists();

		int numberOfUnassignedCustomers();

		void setCustomerPairAsAssigned(int customerIndex, int depotIndex);

		void resetCustomersAsNotAssigned();

		float genRandomFloatingNumber(float a, float b);

		int genRandomIntNumber(int a, int b);

		int getX_Coord(int nodeInternalId);

		int getY_Coord(int nodeInternalId);
		
		FrogObjectCol * createMatchCustomerList(Pair * currentDepotPair);

		FrogObjectCol* createFullMatchCustomerList(Pair* currentDepotPair);

		FrogObjectCol* createMatchCustomerList_Cluster(Cluster* currentCluster, FrogLeapSolution* fls);

		FrogObjectCol * createCustomerListOrderedByDistanceFromDepot(Pair * currentDepotPair);

		FrogObjectCol * createDepotListOrderedByDistanceFromCustomer(Pair * currentCustomerPair);

		FrogObjectCol* createCustomerListOrderedByDistanceFromCluster(Cluster* currentCluster, FrogLeapSolution * fls);

		FrogObjectCol * getListOfUnassignedCustomers();

		FrogObjectCol * reorderCustomerListByDemandPerDistance(FrogObjectCol* matchCustomerCol, Cluster* cluster);

		bool getReassignCustomersSettings();

		void setRassignCustomerSettings(bool reassignCustomers);

		void assignCustomerToClusterByCustomerIndex(Cluster* currentCluster, int i);

		void unassignCustomerFromCluster(Pair* customerPair, Cluster* currentCluster);

		bool isAMatch(Pair * currentCustomerPair, Pair * currentDepotPair);
				
		bool isAFullMatch(Pair* currentCustomerPair, Pair* currentDepotPair);

		void upDateRemainingCapacity(Pair* depotPair, Pair* customerPair);

		Pair * getFirstDepotPairWithRemainingCapacity(FrogObjectCol * depotListOrderded, int customerDemand);

		Cluster* getFirstClusterWithRemainingCapacity(FrogObjectCol* clusterListOrdered, Pair * customerPair, FrogLeapController* controller);

		int getTotalDemandOrCapacity(FrogObjectCol * pairCol);

		void assignDepotToCustomerPairs(Pair * depotPair, FrogObjectCol * customerCol);

		float assignRandomToDepotSelected(int depotIndex);

		bool isCustomerAssignedToCluster(Pair * customerPair, Cluster * cluster);		

		FrogObjectCol* assignDepotToCustomerPairsUntilCapacityIsComplete(Pair* depotPair, FrogObjectCol* customerCol, FrogLeapSolution* fs);

		FrogObjectCol* assignCustomersToClusterUntilCapacityIsComplete(Cluster* cluster, FrogObjectCol* matchCustomerCol, FrogLeapSolution* fs);

		FrogObjectCol* assignDepotRndToCustomerPairsUntilCapacityIsComplete(Pair* depotPair, FrogObjectCol* customerCol, FrogLeapSolution* fs);

		FrogObjectCol * selectCustomerPairsUntilDemandComplete(Pair * depotPair, FrogObjectCol * customerCol);

		void assignCustomersToDepot(Pair * depotPair, FrogObjectCol * & customerCol, FrogObjectCol * depotListOrderedByCapacity, FrogLeapSolution * fs);
						
		void assignCustomersToDepot2(Pair* depotPair, FrogObjectCol*& customerCol, FrogObjectCol* depotListOrderedByCapacity, FrogLeapSolution* fs);

		FrogObjectCol * orderCustomerPairListByNthClosestDepotDesc(int n, FrogObjectCol * customerPairCol);

		void assignDepotToCustomer(Pair * depotPair, Pair * customerPair);

		void unassignCustomerFromItsCluster(Pair* customerPair, FrogLeapSolution * fls);

		float addRandomNumberToInt(int index);

		bool isCustomerPairAssigned(Pair * customerPair);

		float assignRandomFeasibleDepot4(FrogObjectCol * & localDepotCol, int customerIndex);

		bool isCurrentSolutionFeasibleForCustomer(int customerIndex);

		bool isCurrentSolutionFeasible();

		bool isCustomerPairAssigned(int customerIndex);

		void assignDepotToCustomerAndUpdateRemainingCapacity(Pair * depotPair, Pair * , FrogLeapSolution * fs);

		time_t getSeedUsed();

		int getDemandFromCustomerList(FrogObjectCol* assignedCustomers);
		
		// functions for writing information in an output file
		void openOutPutFile();

		void closeOutPutFile();

		FILE * openFile (char* fileName);

		void closeFile(FILE* filePtr);

		FILE * getPFile();

		char * getTestCaseName();

		char * getTestCaseComment();

		int getTestCaseDimension();

		char * getTestCaseType();

		int getTestCaseCapacity();

		FeasibleSolution * initFeasibleSolution(int size);

		int selectRandomIndex(int i, FeasibleSolution* fs);

		int getMinValueIteration();

		void setMinValueIteration(int min_i);

		// functions for testing the Clark and Wright Algorithm
		void cw_loadDistanceTable();
		void cw_loadDistanceTableValues(DistanceTable * dt);
		void cw_loadCustomersAndDepots();
		void cw_loadVehicleCapacity();

		void cw_printCtrlSolutionData(FrogLeapController* controller);

		void printCtrlSolutionData(FrogLeapController* controller);

		void writeSeed();
		void writeFrogLeapSolution(FrogLeapSolution * fls);		
		void writeIterationInfo(long long int i, float currentValue);
		void writeRandomInfo(float a, float b, float finalRandom);
		void writeExecutionInfo();		
};
#endif
