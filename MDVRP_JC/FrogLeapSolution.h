#ifndef FROGLEAPSOLUTION_H_   /* Include guard */
#define FROGLEAPSOLUTION_H_

//=================================
// included dependencies
#include "FrogObject.h"

class FeasibleSolution;
class DecodedFrogLeapSolution;
class Graph;
class FrogLeapController;
class FrogObjectCol;
class FrogObject;
class Pair;
class Cluster;
class ClarkWrigthHandler;

enum class SolutionGenerationType;
enum class SourceType;
enum class DistanceType;

class FrogLeapSolution: public FrogObject
{
	private:
	
		float * values;
		int size;			// number of customers

		int n_depots;		// number of depots
		int nElementsToSort;
		//time_t timeSeedUsed;
		SolutionGenerationType sgt;
		SourceType st;		

		FeasibleSolution * customerSelectionList;

		FrogObjectCol * clusterCol;
		FrogObjectCol * cw_handler_col;

	public:
			
		FrogLeapSolution(SolutionGenerationType v_sgt, SourceType v_sourceType, int ncustomers, int n_depots_v, int id);

		~FrogLeapSolution();

		float getFLValue(int i);

		void setFLValue(int i, float v_float);

		void setSize(int size_v);

		int getSize();

		bool genRandomSolution(FrogLeapController * controller);

		bool genRandomSolution2(FrogLeapController * controller);

		bool genRandomSolution3(FrogLeapController * controller);

		bool genRandomSolution4(FrogLeapController * controller);

		bool genRandomSolution5(FrogLeapController * controller);

		bool genRandomSolution6(FrogLeapController * controller);

		bool genRandomSolution7(FrogLeapController * controller);

		bool genRandomSolution7_5(FrogLeapController* controller);

		bool genRandomSolution8_0(FrogLeapController* controller);	

		bool genRandomSolution9_0(FrogLeapController* controller);

		bool genRandomSolution10_0(FrogLeapController* controller);

		bool genRandomSolution11_0(FrogLeapController* controller);

		bool genRandomSolution12_0(FrogLeapController* controller);

		bool genRandomSolution13_0(FrogLeapController* controller);

		bool genRandomSolution14_0(FrogLeapController* controller);

		bool genRandomSolution15_0(FrogLeapController* controller);

		bool genRandomSolution16_0(FrogLeapController* controller);

		bool genRandomSolution17_0(FrogLeapController* controller);

		bool genRandomSolution18_0(FrogLeapController* controller);

		bool genRandomSolution19_0(FrogLeapController* controller);

		bool genRandomSolution20_0(FrogLeapController* controller);

		bool genRandomSolution21_0(FrogLeapController* controller);

		Cluster* getClusterByIndex(int i);

		bool genRandomSolutionFromTestCase(FrogLeapController * controller);

		float normalRandomAssigment(FrogLeapController * controller);

		float assignRandomFeasibleDepot(FrogObjectCol * feasibleDepotList, FrogLeapController * controller, int customerIndex);

		float assignRandomFeasibleDepot2(FrogLeapController * controller, int customerIndex);

		float assignRandomFeasibleDepot3(FrogLeapController * controller, FrogObjectCol * localDepotCol, int customerIndex);		

		float assignRandomFeasibleDepot4(FrogLeapController * controller, FrogObjectCol * & localDepotCol, int customerIndex);

		void assignRandomFeasibleDepot5(FrogLeapController * controller, Pair * currentDepotPair);

		void assignRandomFeasibleDepot6(FrogLeapController * controller, Pair * currentDepotPair, FrogObjectCol * depotListOrderedByCapacity);

		void assignRandomFeasibleDepot7(FrogLeapController* controller, Pair* currentDepotPair, FrogObjectCol* depotListOrderedByCapacity);

		void assignRandomFeasibleDepot8(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity);

		void assignRandomFeasibleDepot9(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity);
		
		void assignRandomFeasibleDepot10(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity);

		void assignRandomFeasibleDepot11(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity);

		void assignRandomFeasibleDepot12(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity);

		void assignRandomFeasibleDepot13(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity);

		void assignRandomFeasibleDepot14(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity);

		void assignRandomFeasibleDepot15(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity);

		void assignRandomFeasibleDepot16(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity);

		void assignRandomFeasibleDepot17(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity);

		void assignRandomFeasibleDepot18(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity);

		void assignRandomFeasibleDepot19(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity);

		void assignRandomFeasibleDepot20(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity);

		void assignRandomFeasibleDepot21(FrogLeapController* controller, Cluster* currentCluster, FrogObjectCol* depotListOrderedByCapacity);

		void assignRemainingCustomersToClosestCluster(FrogLeapController * controller);		

		FrogObjectCol * initializeFeasibleDepotList(FrogLeapController * controller);

		DecodedFrogLeapSolution * decodeSolution(FrogLeapController * g);				

		DecodedFrogLeapSolution * decodeFrogLeapSolution(FrogLeapController * controller, bool adjustVehicleRoutes = false);

		DecodedFrogLeapSolution * decodeWholeSolutionWithAngularCriteria(FrogLeapController * controller);

		DecodedFrogLeapSolution * decodeFrogLeapWholeSolutionWithAngularCriteria(FrogLeapController * controller);
		
		DecodedFrogLeapSolution * decodeSolutionWithAngularCriteria(FrogLeapController * controller);

		DecodedFrogLeapSolution * decodeFrogLeapSolutionWithAngularCriteria(FrogLeapController * controller, bool adjustVehicleRoutes);		

		DecodedFrogLeapSolution * decodeWholeSolutionWithClosestNextCriteria(FrogLeapController * controller);

		DecodedFrogLeapSolution* decodeWholeSolutionWithClarkWrightCriteria2(FrogLeapController* controller);
		
		DecodedFrogLeapSolution* decodeWholeSolutionWithClarkWrightCriteria(FrogLeapController* controller);		

		DecodedFrogLeapSolution* decodeWholeSolutionWithSimplifiedClosestNextCriteria(FrogLeapController* controller);

		DecodedFrogLeapSolution* decodeWholeSolutionWithMixedCriteria(FrogLeapController* controller);

		DecodedFrogLeapSolution * decodeFrogLeapWholeSolutionWithSimplifiedClosestNextCriteria(FrogLeapController* controller, bool adjustVehicleRoutes);
		
		DecodedFrogLeapSolution * decodeFrogLeapWholeSolutionWithClosestNextCriteria(FrogLeapController * controller, bool adjustVehicleRoutes);

		DecodedFrogLeapSolution * decodeFrogLeapWholeSolutionWithMixedCriteria(FrogLeapController* controller, bool adjustVehicleRoutes);

		void setSolutionGenerationType(SolutionGenerationType v_sgt);

		SolutionGenerationType getSolutionGenerationType();

		void initCustomerSelection(FrogLeapController * controller);

		FeasibleSolution * initDepotSelection(FrogLeapController * controller);

		void destroyRandomCustomerSelectionList();

		int selectRandomCustomerIndex(int i, FrogLeapController * controller);

		int selectRandomDepotIndex(int i, FeasibleSolution * fs, FrogLeapController * controller);

		void assignMatchCustomerListToFLValues(FrogObjectCol * matchCustomerCol, FrogLeapController * controller);		

		void writeFrogLeapSolution(FrogLeapController * controller);

		void writeFLSWithSolutionDataHeader(FrogLeapController* controller);

		void printFLSWithSolutionDataHeader(FrogLeapController* controller);

		void writeFrogLeapSolutionWithSolutionData(FrogLeapController* controller);

		void printFrogLeapSolutionWithSolutionData(FrogLeapController* controller);

		void writeAssignedCustomers(Cluster * cluster, FILE * pFile);
	
		void printAssignedCustomers(Cluster* cluster);

		void printFrogLeapSolutionArray(FrogLeapController* controller);

		void printFLS_ArrayHeader();

		void printFLS_ArrayFooter();

		void printFLS_Record(int i, FrogLeapController * controller);

		void printClusterColInfo();

		void printClusterContentHeader();

		void setAllFLSWithValue(float value);

		void initClusterCollection(FrogLeapController* controller);

		void updateClusterCollectionWithDistanceType(DistanceType distance_t);

		bool isAMatch_Cluster(Pair* currentCustomerPair, Cluster* currentCluster, FrogLeapController* controller);
		
		FrogObjectCol* createClusterListOrderedByDistanceFromCustomer(Pair* currentCustomerPair, FrogLeapController* controller);
		
		int getCWHandlerColSize();

		FrogObjectCol * getCWHandlerCol();

		void exportClusterColToVRP(FrogLeapController * controlller);

		// abstract methods
		void printFrogObj();

		bool isTheSame(FrogObject * fs);
};

#endif
