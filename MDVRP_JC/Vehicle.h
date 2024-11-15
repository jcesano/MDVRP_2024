#ifndef VEHICLE_H_   /* Include guard */
#define VEHICLE_H_

#include "FrogObject.h"

class FrogObjectCol;
class Graph;
class Pair;
class FeasibleSolution;
class FrogLeapController;
class SolutionData;

class Vehicle : public FrogObject
{
	private:
		FrogObjectCol * customers;
		int size;
		int depotIndex;
		int depotInternalId;
		long int capacity;
		long int remaining_capacity;
		int demand;
		bool isFeasible;
		
		bool localSearchApplied;
		float pathCost;

		FrogLeapController * ptrController;

		int * vehicleCustomerArray;

	public:
		Vehicle(int id);

		Vehicle(int id, FrogLeapController * controller);

		virtual ~Vehicle();

		void addCustomerPair(Pair * customerPair);

		void addCustomerPairDoubleOrdered(Pair * customerPair);

		void addLastCustomerPair(Pair * customerPair);

		void setDepotIndex(int depot_v);

		void unreferenceAndDeleteCustomerList();

		int getDepotIndex();

		void setDepotInternalId(int v_depotId);

		int getDepotInternalId();

		float cw2_evalPath(FrogLeapController* controller);

		float cw_evalPath(FrogLeapController* controller);
		
		float evalPath(FrogLeapController * controller);

		void setController(FrogLeapController * controller);

		FrogLeapController * getController();

		void setPathCost(float cost);

		float getPathCost();

		long int getCapacity();

		long int getRemainingCapacity();

		void decRemainingCapacity(int capacity_to_dec);

		int incDemand(int aditionalDemand);

		int getDemand();

		void setAsFeasible();

		void setAsUnFeasible();

		bool getIsFeasible();

		void cw_setupLocalSearch();
		
		void setupLocalSearch();

		float applyLocalSearch(FrogLeapController * controller);

		int ObtainCustomerInternalIdFromIndex(int position);

		int ObtainDepotInternalIdFromIndex();

		bool generateLocalSolutionsAndEvaluate(FrogLeapController * controller);

		void updateBestSolution(FeasibleSolution * fs, float cost);

		void printLocalSolution();

		void printGlobalSolution();

		void adjustCustomerRoute(FrogLeapController * controller);

		Vehicle* copy();

		
		Pair * getClosestIndexCustomerPairToDepot(int depotInternalId, FrogLeapController * controller);

		Pair * getClosestIndexCustomerPairToCustomer(int depotInternalId, FrogLeapController * controller);
		
		void writeFrogObj(FrogLeapController * controller, SolutionData * sd);

		void writeSolution(FrogLeapController * controller);

		void writeSolutionData(SolutionData * sd);

		void writeVehicleRoute(FILE* pFile, SolutionData* sd);

		void writeFrogObjWithCoordinates(FrogLeapController * controller);

		void writeSolutionWithCoordinates(FrogLeapController * controller);

		void writeNodeCoordinateInfo(FrogLeapController * controller, int nodeInternalId, int orderInPath, int prevInternalId);

		void cw_printFrogObj(FrogLeapController* controller, SolutionData* sd);

		void printFrogObj(FrogLeapController* controller, SolutionData* sd);

		void cw_printVehicleRoute(SolutionData* sd, FrogLeapController* controller);

		void printVehicleRoute(SolutionData* sd, FrogLeapController* controller);

		void cw_printSolutionData(SolutionData* sd, FrogLeapController* controller);
		
		void printSolutionData(SolutionData* sd, FrogLeapController* controller);

		// abstract methods
		void printFrogObj();

		bool isTheSame(FrogObject * fs);
};

#endif