#ifndef DECODEDFROGLEAPSOLUTION_H_   /* Include guard */
#define DECODEDFROGLEAPSOLUTION_H_

#include "FrogObject.h"
#include <vector>
using namespace std;


class FrogObjectCol;
class Vehicle;
class Graph;
class FrogLeapController;
class FrogLeapSolution;
class Pair;


class DecodedFrogLeapSolution : public FrogObject
{
	private:
		FrogObjectCol * * vehicles;
		FrogObjectCol * * assignedCustomers;

		//vector<FrogObjectCol> vehicles;
		//FrogObjectCol * vehicles;

		FrogLeapController * ptrController;

		bool isFeasibleSolution;

		int notAddedCustomer;

		bool localSearchApplied;	
		bool unReferenceItemsBeforeDelete;

		int numDepots;
	
	public:

		DecodedFrogLeapSolution(int n_depots);

		DecodedFrogLeapSolution(int n_depots, FrogLeapController * controller);

		virtual ~DecodedFrogLeapSolution();

		void deleteArrayOfFrogObjectCols(FrogObjectCol ** arrayPtr, int v_size);

		void addVehicle(int depotIndex, Vehicle * v);

		//Vehicle * getVehicle(int pos);

		int decodeFixedFloatFrogLeapValue(float fvalue, int numberOfVehicles);

		int decodeFrogLeapValue(float fvalue, int numberOfVehicles);

		//bool decodeFloatFixedFrogLeapItem(float fvalue, int customerIndex, int numberOfDepots, int numberOfVehicles);

		bool decodeFrogLeapItem(FrogLeapController * controller, float fvalue, int customerIndex, int numberOfDepots);		

		bool decodeFrogLeapItemToListWithAngularCriteria(FrogLeapController * controller, float fvalue, int customerIndex, int numberOfDepots);

		bool decodeFrogLeapItemWithAngularCriteria(FrogLeapController * controller, float fvalue, int customerIndex, int numberOfDepots);

		bool decodeFrogLeapItemToListWithClosestNextCriteria(FrogLeapController * controller, float fvalue, int customerIndex, int numberOfDepots);

		void assignDecodedCustomersToVehicles(FrogLeapController * controller);

		void assignDecodedCustomersToDepotVehicles(int depotIndex, FrogLeapController * controller);

		void assignCustomersToDepotLists(FrogLeapController * controller, FrogLeapSolution * fls);

		void assignCustomerToDepotList(FrogLeapController * controller, FrogLeapSolution * fls, int customerIndex);

		void orderDepotCustomersWithClosestNextCriteria(int depotIndex, FrogLeapController * controller);

		void orderCustomersWithClosestNextCriteria(FrogLeapController * controller);

		Pair * getClosestCustomerIndexToDepot(int depotIndex, FrogLeapController * controller);

		Pair * getClosestCustomerIndexToCustomer(int customerIndex, int depotIndex, FrogLeapController * controller);

		float evalSolution();

		void setController(FrogLeapController * controller);

		FrogLeapController * getController();

		void setIsFeasibleSolution(bool v_isFeasible);

		bool getIsFeasibleSolution();

		float applyLocalSearch(FrogLeapController * controller);		
		
		int getNotAddedCustomer();

		void setNotAddedCustomer(int customerId);

		Vehicle * getFirstUpperValueVehicle(int customerDemand, int depotIndex);

		void adjustVehicleRoutes(FrogLeapController * controller);

		void adjustDepotVehicleRoutes(FrogObjectCol * customerList, FrogLeapController * controller);

		//Pair * calculateAngularValue(int customerIndex, int depotIndex, FrogLeapController * controller);

		void setUnReferenceBeforeDelete(bool v_bol);

		bool getUnReferenceBeforeDelete();
		void writeDecodedFrogLeapSolution(FrogLeapController * controller);

		void writeDecodedFrogLeapSolutionWithCoordinates(FrogLeapController * controller);

		// abstract methods
		void printFrogObj();

		bool isTheSame(FrogObject * fs);
};

#endif