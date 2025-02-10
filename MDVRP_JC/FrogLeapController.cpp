#include "stdafx.h"
#include "FrogLeapController.h"
#include <stdio.h>      /* printf */
#include <math.h>       /* floor */
#include <iostream>
#include <time.h>
#include <string>
#include "FrogObject.h"
#include "FrogObjectCol.h"
#include "DecodedFrogLeapSolution.h"
#include "TspLibEuc2D.h"
#include "TestCaseObj.h"
#include "Pair.h"
#include "DistanceTable.h"
#include "FloatDistanceTable.h"
#include "Graph.h"
#include "FrogLeapSolution.h"
#include "FeasibleSolution.h"
#include "Pair.h"
#include "IndexList.h"
#include "Vehicle.h"
#include "Cluster.h"
#include <limits>
#include <iostream>
#include <string>
#include <functional>
#include <random>
#include <utility>
#include <cstdlib> // for exit and abort

//using std::string;
//using std::cout;
//using std::endl;
//using std::cin;
//using std::string;
//using std::random_device;
//using std::default_random_engine;

using namespace std;

const int LINE_MAX = 256;

FrogLeapController::FrogLeapController()
{
	this->failAttempts = 0;

	this->successAttempts = 0;

	//this->timeSeedUsed = (unsigned)time(NULL);
	this->timeSeedUsed = 1537280770;
	srand(this->timeSeedUsed);

	this->minCostValue = std::numeric_limits<float>::max();;

	this->totalImprovements = 0;
	this->localSearchImprovements = 0;
	this->globalImprovements = 0;

	this->tspLibEud2DPtr = NULL;
	this->testCaseObjPtr = NULL;

	this->distTablePtr = NULL;
	this->floatDistTablePtr = NULL;

	this->graphPtr = NULL;

	this->customerList = new FrogObjectCol();
	this->depotList = new FrogObjectCol();
	this->vehiclePairList = new FrogObjectCol();
	this->ptrBestSolution = NULL;

	this->globalVehicleId = 0;	

	this->reassignCustomers = true;
}

FrogLeapController::~FrogLeapController()
{

	delete this->vehiclePairList;
	delete this->depotList;
	delete this->customerList;

	this->deleteArray(this->customerArray, this->getNumberOfCustomers());

	this->deleteArray(this->depotArray, this->getNumberOfDepots());

}

int FrogLeapController::getFailAttempts()
{
	return this->failAttempts;
}

void FrogLeapController::incFailAttempts()
{
	this->failAttempts++;
}

int FrogLeapController::getTotalImprovements()
{
	return this->localSearchImprovements + this->globalImprovements;
}

void FrogLeapController::incTotalImprovements()
{
	this->totalImprovements++;
}

int FrogLeapController::getLocalSearchImprovements()
{
	return this->localSearchImprovements;
}

void FrogLeapController::incLocalSearchImprovements()
{
	this->localSearchImprovements;
}

int FrogLeapController::getGlobalSearchImprovements()
{
	return this->globalImprovements;
}

void FrogLeapController::incGlobalSearchImprovements()
{
	this->globalImprovements++;
}

int FrogLeapController::getLocalGeneratedSolutions()
{
	return this->localGeneratedSolutions;
}

void FrogLeapController::incLocalGeneratedSolutions()
{
	this->localGeneratedSolutions++;
}

long int FrogLeapController::getNumberOfIterations()
{
	return 150000;
}

float FrogLeapController::getMinCostValue()
{
	return this->minCostValue;
}

void FrogLeapController::setMinCostValue(float cost)
{
	this->minCostValue = cost;
}

void FrogLeapController::setBestDecodedFrogLeapSolution(DecodedFrogLeapSolution * ptrSolution)
{
	DecodedFrogLeapSolution * oldSolution = this->ptrBestSolution;
	this->ptrBestSolution = ptrSolution;
	delete oldSolution;
	oldSolution = NULL;
}

DecodedFrogLeapSolution * FrogLeapController::getBestDecodedFrogLeapSolution()
{
	return this->ptrBestSolution;
}

void FrogLeapController::setRandomSeed()
{
	srand(this->timeSeedUsed);
}

void FrogLeapController::applyLocalSearch()
{
	float newCost = this->ptrBestSolution->applyLocalSearch(this);
	this->setMinCostValue(newCost);
}


void FrogLeapController::cw_printCtrlSolutionData(FrogLeapController* controller)
{
	FILE* pFile = this->getPFile();

	//fprintf(pFile, "\n \n SHOWING DATA OF FROGLEAPING CONTROLLER \n");
	if (this->ptrBestSolution != NULL)
	{
		this->ptrBestSolution->cw_printFrogObjWithSolutionData();
	}
	else
	{
		fprintf(pFile, "\n NO FEASIBLE SOLUTION FOUND: ptrBestSolution IS NULL \n");
	}

	//fprintf(pFile, "\n Summary of Best Found Solution \n");
	//fprintf(pFile, "	Time Seed used %lld \n", (long long)this->timeSeedUsed);
	//fprintf(pFile, "	Number of success attempts: %d \n", this->successAttempts);
	//fprintf(pFile, "	Number of fail attempts: %d \n", this->failAttempts);
	//fprintf(pFile, "	Number of TOTAL Improvements: %d \n", this->getTotalImprovements());
	//fprintf(pFile, "	Number of Global Search Improvements: %d \n", this->globalImprovements);
	//fprintf(pFile, "	Number of Local Search Improvements: %d \n", this->localSearchImprovements);
	//fprintf(pFile, "	Evaluation of best found solution is: %.3f \n \n", this->getMinCostValue());
}


void FrogLeapController::printCtrlSolutionData(FrogLeapController * controller)
{
	FILE* pFile = this->getPFile();
	
	//fprintf(pFile, "\n \n SHOWING DATA OF FROGLEAPING CONTROLLER \n");
	if(this->ptrBestSolution != NULL)
	{
		this->ptrBestSolution->printFrogObjWithSolutionData();
	}
	else
	{
		fprintf(pFile, "\n NO FEASIBLE SOLUTION FOUND: ptrBestSolution IS NULL \n");
	}
	
	//fprintf(pFile, "\n Summary of Best Found Solution \n");
	//fprintf(pFile, "	Time Seed used %lld \n", (long long)this->timeSeedUsed);
	//fprintf(pFile, "	Number of success attempts: %d \n", this->successAttempts);
	//fprintf(pFile, "	Number of fail attempts: %d \n", this->failAttempts);
	//fprintf(pFile, "	Number of TOTAL Improvements: %d \n", this->getTotalImprovements());
	//fprintf(pFile, "	Number of Global Search Improvements: %d \n", this->globalImprovements);
	//fprintf(pFile, "	Number of Local Search Improvements: %d \n", this->localSearchImprovements);
	//fprintf(pFile, "	Evaluation of best found solution is: %.3f \n \n", this->getMinCostValue());
}

void FrogLeapController::writeCtrl()
{
	FILE* pFile = this->getPFile();

	//fprintf(pFile, "\n \n SHOWING DATA OF FROGLEAPING CONTROLLER \n");
	if (this->ptrBestSolution != NULL)
	{
		this->ptrBestSolution->writeFrogObjWithSolutionData();
	}
	else
	{
		fprintf(pFile, "\n NO FEASIBLE SOLUTION FOUND: ptrBestSolution IS NULL \n");
	}

	//fprintf(pFile, "\n Summary of Best Found Solution \n");
	//fprintf(pFile, "	Time Seed used %lld \n", (long long)this->timeSeedUsed);
	//fprintf(pFile, "	Number of success attempts: %d \n", this->successAttempts);
	//fprintf(pFile, "	Number of fail attempts: %d \n", this->failAttempts);
	//fprintf(pFile, "	Number of TOTAL Improvements: %d \n", this->getTotalImprovements());
	//fprintf(pFile, "	Number of Global Search Improvements: %d \n", this->globalImprovements);
	//fprintf(pFile, "	Number of Local Search Improvements: %d \n", this->localSearchImprovements);
	//fprintf(pFile, "	Evaluation of best found solution is: %.3f \n \n", this->getMinCostValue());
}

void FrogLeapController::printCtrl()
{
	printf("\n \n SHOWING DATA OF FROGLEAPING CONTROLLER \n");
	if (this->ptrBestSolution != NULL)
	{
		//this->ptrBestSolution->printFrogObj();
	}
	else
	{
		printf("\n NO FEASIBLE SOLUTION FOUND: ptrBestSolution IS NULL \n");
	}

	printf("\n Summary of Best Found Solution \n");
	printf("	Time Seed used %lld \n", (long long)this->timeSeedUsed);
	printf("	Number of success attempts: %d \n", this->successAttempts);
	printf("	Number of fail attempts: %d \n", this->failAttempts);
	printf("	Number of TOTAL Improvements: %d \n", this->getTotalImprovements());
	printf("	Number of Global Search Improvements: %d \n", this->globalImprovements);
	printf("	Number of Local Search Improvements: %d \n", this->localSearchImprovements);
	printf("	Evaluation of best found solution is: %.3f \n \n", this->getMinCostValue());
}

void FrogLeapController::loadTSPEUC2D_Data(const char * fileName){
	
	FILE * filePtr;	
	
	TspLibEuc2D * tspLibEuc2DPtrAux = new TspLibEuc2D();
	
	//char * sectionTag = new char[50], *separatorChar = new char[1], buf[LINE_MAX];
	char sectionTag[50], separatorChar[10], buf[LINE_MAX];

	string ctrlSectionTagStr, ctrlSeparatorCharStr, sectionTagStr, separatorCharStr;

	ctrlSeparatorCharStr = string(":");
		
	// Opening file
	//if ((filePtr = fopen(fileName, "r")) != NULL)
	if ((fopen_s(&filePtr, fileName, "r")) == 0)
	{
		
		// Reading section tag NAME
		//char * name = new char[50];
		char name[50];

		if (fgets(buf, sizeof buf, filePtr) != NULL)
		{
			//sscanf(buf, "%s %s %s", sectionTag, separatorChar, name);
			sscanf_s(buf, "%s %s %s", sectionTag, (unsigned int)sizeof(sectionTag), separatorChar, (unsigned int)sizeof(separatorChar), name, (unsigned int)sizeof(name));

			printf("Section: %s %s %s \n", sectionTag, separatorChar, name);

			sectionTagStr = sectionTag;
			separatorCharStr = separatorChar;

			ctrlSectionTagStr = string("NAME");
			if (sectionTagStr.compare(ctrlSectionTagStr) != 0 || separatorCharStr.compare(ctrlSeparatorCharStr) != 0)
			{
				printf("Error in file format \n");
				return;
			}

			tspLibEuc2DPtrAux->setName(name);
		}
		else
		{
			printf("Error reading file \n");
		}

		// reading COMMENT
		//char * comment = new char[100];
		char comment[100];

		if (fgets(buf, sizeof buf, filePtr) != NULL)
		{
			//sscanf(buf, "%s %s %s", sectionTag, separatorChar, comment);
			sscanf_s(buf, "%s %s %s", sectionTag, (unsigned int)sizeof(sectionTag), separatorChar, (unsigned int)sizeof(separatorChar), comment, (unsigned int)sizeof(comment));

			printf("Section: %s %s %s \n", sectionTag, separatorChar, comment);

			sectionTagStr = sectionTag;
			separatorCharStr = separatorChar;
			ctrlSectionTagStr = string("COMMENT");
			if (sectionTagStr.compare(ctrlSectionTagStr) != 0 || separatorCharStr.compare(ctrlSeparatorCharStr) != 0)
			{
				printf("Error in file format \n");
				return;
			}

			tspLibEuc2DPtrAux->setComment(comment);
		}
		else
		{
			printf("Error reading file \n");
		}

		
		// reading type
		//char * type = new char[50];
		char type[50];

		if (fgets(buf, sizeof buf, filePtr) != NULL)
		{
			//sscanf(buf, "%s %s %s", sectionTag, separatorChar, type);
			sscanf_s(buf, "%s %s %s", sectionTag, (unsigned int)sizeof(sectionTag), separatorChar, (unsigned int)sizeof(separatorChar), type, (unsigned int)sizeof(type));

			printf("Section: %s %s %s \n", sectionTag, separatorChar, type);

			sectionTagStr = sectionTag;
			separatorCharStr = separatorChar;

			ctrlSectionTagStr = string("TYPE");
			if (sectionTagStr.compare(ctrlSectionTagStr) != 0 || separatorCharStr.compare(ctrlSeparatorCharStr) != 0)
			{
				printf("Error in file format \n");
				return;
			}

			tspLibEuc2DPtrAux->setType(type);
		}
		else
		{
			printf("Error reading file \n");
		}

		// reading DIMENSION
		int dimension;
		if (fgets(buf, sizeof buf, filePtr) != NULL)
		{
			//sscanf(buf, "%s %s %d", sectionTag, separatorChar, &dimension);
			sscanf_s(buf, "%49s %9s %d", sectionTag, (unsigned int)50, separatorChar, (unsigned int)10, &dimension);

			printf("Section: %s %s %d \n", sectionTag, separatorChar, dimension);

			sectionTagStr = sectionTag;
			separatorCharStr = separatorChar;

			ctrlSectionTagStr = string("DIMENSION");
			if (sectionTagStr.compare(ctrlSectionTagStr) != 0 || separatorCharStr.compare(ctrlSeparatorCharStr) != 0)
			{
				printf("Error in file format \n");
				return;
			}

			tspLibEuc2DPtrAux->setDimension(dimension);
		}
		else
		{
			printf("Error reading file \n");
		}
		
		// reading EDGE_WEIGHT_TYPE
		//char * edge_weight_type = new char[50];;
		char edge_weight_type[50];

		if (fgets(buf, sizeof buf, filePtr) != NULL)
		{
			//sscanf(buf, "%s %s %s", sectionTag, separatorChar, edge_weight_type);
			sscanf_s(buf, "%49s %9s %49s",
				sectionTag, (unsigned int)sizeof(sectionTag),
				separatorChar, (unsigned int)sizeof(separatorChar),
				edge_weight_type, (unsigned int)sizeof(edge_weight_type));

			printf("Section: %s %s %s \n", sectionTag, separatorChar, edge_weight_type);

			//sectionTagStr = sectionTag;
			//separatorCharStr = separatorChar;

			sectionTagStr = sectionTag;
			separatorCharStr = std::string(separatorChar, 1); // Convertir correctamente el carácter en un string


			ctrlSectionTagStr = string("EDGE_WEIGHT_TYPE");
			if (sectionTagStr.compare(ctrlSectionTagStr) != 0 || separatorCharStr.compare(ctrlSeparatorCharStr) != 0)
			{
				printf("Error in file format \n");
				return;
			}

			tspLibEuc2DPtrAux->setEdgeWeightType(edge_weight_type);
		}
		else
		{
			printf("Error reading file \n");
		}
		

		// reading CAPACITY
		int capacity;
		if (fgets(buf, sizeof buf, filePtr) != NULL)
		{
			//sscanf(buf, "%s %s %d", sectionTag, separatorChar, &capacity);
			sscanf_s(buf, "%49s %9s %d",
				sectionTag, (unsigned int)sizeof(sectionTag),
				separatorChar, (unsigned int)sizeof(separatorChar),
				&capacity);

			printf("Section: %s %s %d \n", sectionTag, separatorChar, capacity);

			//sectionTagStr = sectionTag;
			//separatorCharStr = separatorChar;

			sectionTagStr = sectionTag;
			separatorCharStr = std::string(separatorChar, 1); // Convertir correctamente el carácter en un string

			ctrlSectionTagStr = string("CAPACITY");
			if (sectionTagStr.compare(ctrlSectionTagStr) != 0 || separatorCharStr.compare(ctrlSeparatorCharStr) != 0)
			{
				printf("Error in file format \n");
				return;
			}

			tspLibEuc2DPtrAux->setCapacity(capacity);
		}
		else
		{
			printf("Error reading file \n");
		}

		// reading Node Coord Section
		//char * nodeCoordSection = new char[50];
		char nodeCoordSection[50];

		if (fgets(buf, sizeof buf, filePtr) != NULL)
		{
			//sscanf(buf, "%s ", nodeCoordSection);
			sscanf_s(buf, "%49s", nodeCoordSection, (unsigned int)sizeof(nodeCoordSection));

			printf("Section: %s \n", nodeCoordSection);

			//sectionTagStr = nodeCoordSection;
			sectionTagStr = std::string(nodeCoordSection);
			
			ctrlSectionTagStr = string("NODE_COORD_SECTION");
			if (sectionTagStr.compare(ctrlSectionTagStr) != 0)
			{
				printf("Error in file format \n");
				return;
			}

			// Loading coordinates (we load the node Id, x_coord and y_coord)
			this->loadCoordinates(filePtr, tspLibEuc2DPtrAux);

			// Loading demands (we load the node Id, demand). The Id is the same as the previous one
			this->loadDemand(filePtr, tspLibEuc2DPtrAux);

			this->loadDepots(filePtr, tspLibEuc2DPtrAux);

			this->tspLibEud2DPtr = tspLibEuc2DPtrAux;
			fclose(filePtr);
		}
		else
		{
			printf("Error reading file \n");
			fclose(filePtr);
		}		
	}
	else
	{
		printf("Error found opening file");
	}
}

DecodedFrogLeapSolution * FrogLeapController::loadTestCaseData(const char * fileName)
{
	FILE * filePtr;
	//char * sectionTag = new char[50], *separatorChar = new char[1], buf[LINE_MAX];
	char sectionTag[50], separatorChar[1], buf[LINE_MAX];
	string ctrlSectionTagStr, ctrlSeparatorCharStr, sectionTagStr, separatorCharStr;

	TestCaseObj * testCaseObjPtr = new TestCaseObj();

	ctrlSeparatorCharStr = string(":");

	//if ((filePtr = fopen(fileName, "r")) != NULL)
	
	if ((fopen_s(&filePtr, fileName, "r")) == 0)
	{
		// Reading section tag NAME
		//char * name = new char[50];
		char name[50];
		if (fgets(buf, sizeof buf, filePtr) != NULL)
		{
			//sscanf(buf, "%s %s %s", sectionTag, separatorChar, name);
			sscanf_s(buf, "%49s %49s %49s", sectionTag, (unsigned int)sizeof(sectionTag),
				separatorChar, (unsigned int)sizeof(separatorChar),
				name, (unsigned int)sizeof(name));

			printf("Section: %s %s %s \n", sectionTag, separatorChar, name);

			//sectionTagStr = sectionTag;
			//separatorCharStr = separatorChar;

			sectionTagStr = sectionTag;
			separatorCharStr = std::string(separatorChar);


			ctrlSectionTagStr = string("NAME");
			
			if (sectionTagStr.compare(ctrlSectionTagStr) != 0 || separatorCharStr.compare(ctrlSeparatorCharStr) != 0)
			{
				printf("Error in file format \n");
				return NULL;
			}

			testCaseObjPtr->setName(name);
		}
		else
		{
			printf("Error reading file \n");
			return NULL;
		}

		// reading COMMENT
		char * comment = new char[100];
		if (fgets(buf, sizeof buf, filePtr) != NULL)
		{
			//sscanf(buf, "%s %s %s", sectionTag, separatorChar, comment);
			sscanf_s(buf, "%49s %49s %49s",
				sectionTag, (unsigned int)sizeof(sectionTag),
				separatorChar, (unsigned int)sizeof(separatorChar),
				comment, (unsigned int)sizeof(comment));

			printf("Section: %s %s %s \n", sectionTag, separatorChar, comment);

			sectionTagStr = sectionTag;
			separatorCharStr = separatorChar;
			ctrlSectionTagStr = string("COMMENT");
			if (sectionTagStr.compare(ctrlSectionTagStr) != 0 || separatorCharStr.compare(ctrlSeparatorCharStr) != 0)
			{
				printf("Error in file format \n");
				return NULL;
			}

			testCaseObjPtr->setComment(comment);
		}
		else
		{
			printf("Error reading file \n");
			return NULL;
		}

		// reading DIMENSION
		int dimension;
		if (fgets(buf, sizeof buf, filePtr) != NULL)
		{
			//sscanf(buf, "%s %s %d", sectionTag, separatorChar, &dimension);
			sscanf_s(buf, "%49s %49s %d",
				sectionTag, (unsigned int)sizeof(sectionTag),
				separatorChar, (unsigned int)sizeof(separatorChar),
				&dimension);

			printf("Section: %s %s %d \n", sectionTag, separatorChar, dimension);

			sectionTagStr = sectionTag;
			separatorCharStr = separatorChar;

			ctrlSectionTagStr = string("DIMENSION");
			if (sectionTagStr.compare(ctrlSectionTagStr) != 0 || separatorCharStr.compare(ctrlSeparatorCharStr) != 0)
			{
				printf("Error in file format \n");
				return NULL;
			}

			testCaseObjPtr->setDimension(dimension);
		}
		else
		{
			printf("Error reading file \n");
			return NULL;
		}

		// reading type
		char * type = new char[50];
		if (fgets(buf, sizeof buf, filePtr) != NULL)
		{
			//sscanf(buf, "%s %s %s", sectionTag, separatorChar, type);
			sscanf_s(buf, "%49s %49s %49s",
				sectionTag, (unsigned int)sizeof(sectionTag),
				separatorChar, (unsigned int)sizeof(separatorChar),
				type, (unsigned int)sizeof(type));

			printf("Section: %s %s %s \n", sectionTag, separatorChar, type);

			sectionTagStr = sectionTag;
			separatorCharStr = separatorChar;

			ctrlSectionTagStr = string("TYPE");
			if (sectionTagStr.compare(ctrlSectionTagStr) != 0 || separatorCharStr.compare(ctrlSeparatorCharStr) != 0)
			{
				printf("Error in file format \n");
				return NULL;
			}

			testCaseObjPtr->setType(type);
		}
		else
		{
			printf("Error reading file \n");
			return NULL;
		}

		// reading CAPACITY
		int capacity;
		if (fgets(buf, sizeof buf, filePtr) != NULL)
		{
			//sscanf(buf, "%s %s %d", sectionTag, separatorChar, &capacity);
			sscanf_s(buf, "%49s %49s %d",
				sectionTag, (unsigned int)sizeof(sectionTag),
				separatorChar, (unsigned int)sizeof(separatorChar),
				&capacity);

			printf("Section: %s %s %d \n", sectionTag, separatorChar, capacity);

			sectionTagStr = sectionTag;
			separatorCharStr = separatorChar;

			ctrlSectionTagStr = string("CAPACITY");
			if (sectionTagStr.compare(ctrlSectionTagStr) != 0 || separatorCharStr.compare(ctrlSeparatorCharStr) != 0)
			{
				printf("Error in file format \n");
				return NULL;
			}

			testCaseObjPtr->setCapacity(capacity);
		}
		else
		{
			printf("Error reading file \n");
			return NULL;
		}

		// reading Assignation Section
		char * assignationSection = new char[50];;
		if (fgets(buf, sizeof buf, filePtr) != NULL)
		{
			//sscanf(buf, "%s ", assignationSection);
			sscanf_s(buf, "%49s", assignationSection, (unsigned int)sizeof(assignationSection));

			printf("Section: %s \n", assignationSection);

			sectionTagStr = assignationSection;

			ctrlSectionTagStr = string("ASSIGNATION");
			if (sectionTagStr.compare(ctrlSectionTagStr) != 0)
			{
				printf("Error in file format \n");
				return NULL;
			}

			DecodedFrogLeapSolution * dfls = NULL;
			FrogLeapSolution * fls = NULL;

			// Loading assignations (customers to vehicles)
			dfls = this->loadAssignations2(filePtr, testCaseObjPtr);
			//fls = this->loadAssignations3(filePtr, testCaseObjPtr);
			//dfls = fls->decodeFrogLeapSolution(this);
			//dfls = fls->decodeSolutionWithAngularCriteria(this);
			float evaluation = dfls->evalSolution();
			printf("Showing test evaluation: %.3f", evaluation);
			fclose(filePtr);
			return dfls;
		}
		else
		{
			printf("Error reading file \n");
			fclose(filePtr);

			return NULL;
		}
	}
}

void FrogLeapController::loadAssignations(FILE * filePtr, TestCaseObj * testCaseObjPtr)
{
	bool stopLoop = false, vehicleDataLoaded = false, assignationBlockLoaded = false;
	int nodeLabelId = 0, depot_capacity = 0, customer_load = 0, v_dimension = testCaseObjPtr->getDimension();
	char buf[LINE_MAX];
	Pair * currPair;

	while(stopLoop == false)
	{
		if (fgets(buf, sizeof buf, filePtr) != NULL)
		{
			if(vehicleDataLoaded == false)
			{
				//sscanf(buf, "%d %d %d", &nodeLabelId, &depot_capacity, &customer_load);
				sscanf_s(buf, "%d %d %d", &nodeLabelId, &depot_capacity, &customer_load);

				printf("Depot data: %d %d %d \n", nodeLabelId, depot_capacity, customer_load);

				currPair = new Pair(PairType::IntVsInt, nodeLabelId);
				currPair->set_i_IntValue(depot_capacity);
				currPair->set_j_IntValue(customer_load);
				currPair->setId(nodeLabelId);
				currPair->setValue(nodeLabelId);

				testCaseObjPtr->AddVehicleItem(currPair);
				vehicleDataLoaded = true;
				assignationBlockLoaded = false;
			}
			else
			{
				int customerLabelId;
				if(assignationBlockLoaded == false)
				{					
					//sscanf(buf, "%d ", &customerLabelId);
					sscanf_s(buf, "%d ", &customerLabelId);

					printf("Customer Label Id : %d \n", customerLabelId);
					
					if(customerLabelId == -1)
					{
						assignationBlockLoaded = true;
						vehicleDataLoaded = false;
					}
					else
					{
						if(customerLabelId == -2)
						{
							stopLoop = true;
						}
						else
						{
							currPair = new Pair(PairType::IntVsInt, customerLabelId);
							currPair->set_i_IntValue(customerLabelId);
							currPair->set_j_IntValue(nodeLabelId);
							currPair->setId(customerLabelId);
							currPair->setValue(customerLabelId);

							testCaseObjPtr->AddCustomerItem(currPair);
						}
					}
				}
			}
		}
		else
		{
			printf("Error reading file: Section Node Coordinates \n");
			return;
		}
	}
}

// This method creates an already decoded solution
DecodedFrogLeapSolution * FrogLeapController::loadAssignations2(FILE * filePtr, TestCaseObj * testCaseObjPtr)
{
	bool stopLoop = false, vehicleDataLoaded = false, assignationBlockLoaded = false;
	int depotLabelId = 0, vehicle_cost = 0, vehicle_load = 0, v_dimension = testCaseObjPtr->getDimension();
	char buf[LINE_MAX];
	char* context = nullptr;  // Variable para strtok_s
	Pair * currPair;
	float globalCounter = 0;
	int depotIndex;

	DecodedFrogLeapSolution * decodedSolution = new DecodedFrogLeapSolution(this->getNumberOfDepots(), this);
	Vehicle * veh = NULL;
	int count;
	Pair * customerPair = NULL;

	int prevDepotLabelId = -1;

	while (fgets(buf, sizeof buf, filePtr) != NULL)
	{
		//char* tok = strtok(buf, " ");
		char* tok = strtok_s(buf, " ", &context);

		// read data of the depot vehicle
		count = 0;
			
		// Loading the first three data of the vehicle depot
		while (tok != NULL && count < 3) // Correct logic to use to stop looping.
		{
			if(count == 0)
			{
				depotLabelId = atoi(tok);
				cout << depotLabelId << endl;  // Print the current token before getting the next token.
			}				

			if(count == 1)
			{
				vehicle_cost = atoi(tok);
				cout << vehicle_cost << endl;  // Print the current token before getting the next token.
			}

			if(count == 2)
			{
				vehicle_load = atoi(tok);
				cout << vehicle_load << endl;  // Print the current token before getting the next token.
			}
								
			//tok = strtok(NULL, " ");
			tok = strtok_s(NULL, " ", &context);
			count++;
		}

		veh = new Vehicle(this->getGlobalVehicleId(), this);
		veh->setDepotInternalId(depotLabelId);
		depotIndex = this->getDepotIndexByLabelId(depotLabelId);
		veh->setDepotIndex(depotIndex);

		decodedSolution->addVehicle(depotIndex, veh);

		count = 0;

		int customerLabelId, customerIndex;
		while (tok != NULL) 
		{
			customerLabelId = atoi(tok);
			customerIndex = this->getCustomerIndexByLabelId(customerLabelId);
			float fvalue = depotIndex + (globalCounter / 1000);
			customerPair = new Pair(PairType::IntVsFloat);
			customerPair->set_i_IntValue(customerIndex);
			customerPair->set_j_FloatValue(fvalue);
			customerPair->setValue(fvalue);
			customerPair->setId(customerIndex);
				
			veh->addLastCustomerPair(customerPair);

			//tok = strtok(NULL, " ");
			tok = strtok_s(NULL, " ", &context);

			globalCounter = globalCounter + 1;
		}

		globalCounter = 0;
		prevDepotLabelId = depotLabelId;
	}

	return decodedSolution;
}


FrogLeapSolution * FrogLeapController::loadAssignations3(FILE * filePtr, TestCaseObj * testCaseObjPtr)
{
	bool stopLoop = false, vehicleDataLoaded = false, assignationBlockLoaded = false;
	int depotLabelId = 0, vehicle_cost = 0, vehicle_load = 0, v_dimension = testCaseObjPtr->getDimension();
	char buf[LINE_MAX];
	char* context = nullptr;  // Variable para strtok_s
	Pair * currPair = NULL;
	float globalCounter = 0, fvalue;
	int depotIndex, customerLabelId, customerIndex;
	FrogLeapSolution * frogLeapSolution = new FrogLeapSolution(SolutionGenerationType::FrogLeaping, this->getSourceType(), this->getNumberOfCustomers(), this->getNumberOfDepots(), 0);
		
	int count;
	Pair * customerPair = NULL;
		
	while (fgets(buf, sizeof buf, filePtr) != NULL)
	{
		//char* tok = strtok(buf, " ");
		char* tok = strtok_s(buf, " ", &context);

		// read data of the depot vehicle
		count = 0;

		// Loading the first three data of the vehicle depot
		while (tok != NULL && count < 3) // Correct logic to use to stop looping.
		{
			if (count == 0)
			{
				depotLabelId = atoi(tok);
				cout << depotLabelId << endl;  // Print the current token before getting the next token.
			}

			if (count == 1)
			{
				vehicle_cost = atoi(tok);
				cout << vehicle_cost << endl;  // Print the current token before getting the next token.
			}

			if (count == 2)
			{
				vehicle_load = atoi(tok);
				cout << vehicle_load << endl;  // Print the current token before getting the next token.
			}

			//tok = strtok(NULL, " ");
			tok = strtok_s(NULL, " ", &context);
			count++;
		}

		depotIndex = this->getDepotIndexByLabelId(depotLabelId);
		count = 0;

		while (tok != NULL)
		{
			customerLabelId = atoi(tok);
			customerIndex = this->getCustomerIndexByLabelId(customerLabelId);
			fvalue = depotIndex + (globalCounter / 1000);
			frogLeapSolution->setFLValue(customerIndex, fvalue);

			//tok = strtok(NULL, " ");
			tok = strtok_s(NULL, " ", &context);

			globalCounter = globalCounter + 1;
		}		
	}

	return frogLeapSolution;
}


void FrogLeapController::loadCoordinates(FILE * filePtr, TspLibEuc2D * tspLibEuc2DPtr)
{
	bool stopLoop = false;
	int nodeId = 0, x_coord = 0, y_coord = 0, v_dimension = tspLibEuc2DPtr->getDimension();
	char buf[LINE_MAX];

	for(int i=0; i < v_dimension;i++)
	{
		if (fgets(buf, sizeof buf, filePtr) != NULL)
		{
			//sscanf(buf, "%d %d %d", &nodeId, &x_coord, &y_coord);
			sscanf_s(buf, "%d %d %d", &nodeId, &x_coord, &y_coord);

			printf("Coordinate: %d %d %d \n", nodeId, x_coord, y_coord);

			Pair * currPair = new Pair(PairType::IntVsInt, nodeId);

			currPair->set_i_IntValue(x_coord);
			currPair->set_j_IntValue(y_coord);
			currPair->setId(nodeId);
			currPair->setValue(nodeId);

			tspLibEuc2DPtr->AddNodeCoordItem(currPair);
		}
		else
		{
			printf("Error reading file: Section Node Coordinates \n");
			return;
		}
	}
}

void FrogLeapController::loadDemand(FILE * filePtr, TspLibEuc2D * tspLibEuc2DPtr)
{	
	int nodeId = -1, demand = 0;
	//char * demandSection = new char[50];
	char demandSection[50];
	char buf [LINE_MAX];

	if (fgets(buf, sizeof buf, filePtr) != NULL)
	{
		//sscanf(buf, "%s ", demandSection);
		sscanf_s(buf, "%s", demandSection, (unsigned int)sizeof(demandSection));

		printf("Section: %s \n", demandSection);
	}

	string sectionTagStr = demandSection;

	string ctrlSectionTagStr = string("DEMAND_SECTION");

	if (sectionTagStr.compare(ctrlSectionTagStr) != 0)
	{
		printf("Error in file format \n");
		return;
	}

	int dimension = tspLibEuc2DPtr->getDimension();

	for (int i = 0; i < dimension; i++)
	{
		if (fgets(buf, sizeof buf, filePtr) != NULL)
		{
			//sscanf(buf, "%d %d", &nodeId, &demand);
			sscanf_s(buf, "%d %d", &nodeId, &demand);

			printf("Demand: %d %d \n", nodeId, demand);

	
			Pair * currPair = new Pair(PairType::IntVsInt, nodeId);

			currPair->set_i_IntValue(nodeId);
			currPair->set_j_IntValue(demand);
			currPair->setId(nodeId);
			currPair->setValue(nodeId);

			tspLibEuc2DPtr->AddDemandItem(currPair);
		}
		else
		{
			printf("Error reading file: Section Node Coordinates \n");
			return;
		}
	}
}

void FrogLeapController::loadDepots(FILE * filePtr, TspLibEuc2D * tspLibEuc2DPtr)
{
	int depotId = -2;
	//char * depotSection = new char[50];
	char depotSection[50];
	char buf[LINE_MAX];

	if (fgets(buf, sizeof buf, filePtr) != NULL)
	{
		//sscanf(buf, "%s ", depotSection);
		sscanf_s(buf, "%s", depotSection, (unsigned int)sizeof(depotSection));

		printf("Section: %s \n", depotSection);
	}

	string sectionTagStr = depotSection;

	string ctrlSectionTagStr = string("DEPOT_SECTION");

	if (sectionTagStr.compare(ctrlSectionTagStr) != 0)
	{
		printf("Error in file format \n");
		return;
	}
	
	bool stopLoop = false;

	while(!stopLoop)	
	{
		if (fgets(buf, sizeof buf, filePtr) != NULL)
		{
			//sscanf(buf, "%d ", &depotId);
			sscanf_s(buf, "%d ", &depotId);

			printf("%d \n", depotId);

			if(depotId != -1)
			{
				tspLibEuc2DPtr->AddDepotItem(depotId);
			}
			else
			{
				stopLoop = true;
			}
		}
		else
		{
			printf("Error reading file: Section Node Coordinates \n");
			return;
		}
	}
}

DistanceTable * FrogLeapController::loadDistanceTable()
{
	int dimension = this->tspLibEud2DPtr->getDimension();
	float floatDistance;

	DistanceTable * fdt = new DistanceTable(dimension);

	for(int i = 0; i < dimension; i++)
	{
		for(int j = i + 1; j < dimension; j++)
		{
			
			if(i==208 && j==250)
			{
				printf("parar aca \n");
			}

			floatDistance = this->tspLibEud2DPtr->getEucDistance(i, j);			

			// we add two edges (a[u][v] and a[v][u])
			fdt->addEdge(i, j, floatDistance);			

			//if (i == 54 && j == 260)
			//{
			//	printf("Testing distance table: dt[%d, %d] = %f", i, j, fdt->getEdge(i,j));
			//}

		}
	}

	for (int k = 0; k < dimension; k++)
	{
		fdt->addDiagEdge(k, 0);
	}

	this->distTablePtr = fdt;

	return fdt;
}

void FrogLeapController::setDistanceTable(DistanceTable * t)
{
	this->distTablePtr = t;
}

DistanceTable * FrogLeapController::getDistanceTable()
{
	return this->distTablePtr;
}

void FrogLeapController::setFloatDistanceTable(FloatDistanceTable * t)
{
	this->floatDistTablePtr = t;
}

FloatDistanceTable * FrogLeapController::getFloatDistanceTable()
{
	return this->floatDistTablePtr;
}

int FrogLeapController::getNumberOfDepots()
{
	//if(this->source_t == SourceType::Graph)
	//{
	//	return this->depotList->getSize();
	//}
	//else
	//{
	//	return this->tspLibEud2DPtr->getNumberOfDepots();
	//}	
	return this->depotList->getSize();
}

Pair * FrogLeapController::getDepotPairByIndex(int position)
{
	return this->depotArray[position];
}


//Pair * FrogLeapController::getMatchedDepotPair(Pair* customerPair) 
//{
//	// search for the i_th (depotToMatch) closest depot to customerPair
//	//int depotToMatch = customerPair->getDepotIndexToMatch();
//
//	// create a depot list ordered by distance from customerPair
//	FrogObjectCol* depotList = this->createDepotListOrderedByDistanceFromCustomer(customerPair);
//	
//	// get the i_th(depotToMatch) closest depot to customerPair
//	//Pair * matchedDepotPair = (Pair *)depotList->getFrogObject(depotToMatch);
//
//	
//	FrogObjectCol* customerList = this->createCustomerListOrderedByDistanceFromDepot(matchedDepotPair);
//	Pair* closestCustPair = (Pair *)customerList->getFrogObject(0);
//
//	// check if the (depotIndexToMatch)th closest depot has the customerPair as the closest
//	bool doesItMatch = (closestCustPair->getId() == customerPair->getId());
//}

Pair * FrogLeapController::getCustomerPairByIndex(int position)
{
	return this->customerArray[position];
}

Pair * FrogLeapController::getPairByInternalId(int internalId)
{
	return (Pair *) this->tspLibEud2DPtr->getNodeCoordSection()->getFrogObject(internalId);	
}

void FrogLeapController::setAngularValues(Pair * veh_customerPair, int customerInternalId, int depotInternalId)
{
	float theta = this->tspLibEud2DPtr->getAngularTheta(customerInternalId, depotInternalId);
	float rho = this->tspLibEud2DPtr->getAngularRho(customerInternalId, depotInternalId);

	veh_customerPair->setValue(theta);
	veh_customerPair->setValue2(rho);
}

void FrogLeapController::setUpVehiclesPerDepot()
{
	int numOfDepots = this->depotList->getSize();
	Pair * depotPair = NULL;
	int depotDemand, depotId;

	this->vehiclePairList = new FrogObjectCol();

	for (int i = 0; i < numOfDepots; i++)
	{
		depotPair = this->depotArray[i];
		depotId = depotPair->get_i_IntValue();
		depotDemand = depotPair->get_j_IntValue();

		assignVehiclesToDepots(depotId, depotDemand);
	}

	setUpVehiclePairList();
}

void FrogLeapController::assignVehiclesToDepots(int depotId, int depotDemand)
{
	int remainingDemand = depotDemand;
	Pair * vehiclePair = NULL;
	int currentId = this->vehiclePairList->getSize();
	
	while (remainingDemand > 0)
	{
		vehiclePair = new Pair(PairType::IntVsInt);
		vehiclePair->setId(currentId);		

		vehiclePair->set_i_IntValue(this->getVehicleCapacity());
		vehiclePair->set_j_IntValue(depotId);
		this->vehiclePairList->addFrogObjectOrdered(vehiclePair);

		if(remainingDemand >= this->getVehicleCapacity())
		{
			remainingDemand = remainingDemand - this->getVehicleCapacity();
		}
		else
		{
			remainingDemand = 0;
		}
	}
}

int FrogLeapController::getNumberOfCustomers()
{
	//if (this->source_t == SourceType::Graph)
	//{
	//	return this->customerList->getSize();
	//}
	//else
	//{
	//	return this->tspLibEud2DPtr->getNumberOfCustomers();
	//}
	return this->customerList->getSize();
}

void FrogLeapController::setGraph(Graph * gPtr)
{
	this->graphPtr = gPtr;
}

Graph * FrogLeapController::getGraph()
{
	return this->graphPtr;
}

void FrogLeapController::setSourceType(SourceType source)
{
	this->source_t = source;
}

SourceType FrogLeapController::getSourceType()
{
	return this->source_t;
}

void FrogLeapController::setSolutionGenerationType(SolutionGenerationType v_sgt)
{
	this->sgt = v_sgt;
}

SolutionGenerationType FrogLeapController::getSolutionGenerationType()
{
	return this->sgt;
}

void FrogLeapController::readTSPSection(FILE * filePtr, char * ctrlSectionTag, char * ctrlSeparatorChar, int * off_set)
{
	char * sectionTag = new char[50], *separatorChar = new char[1], buf[LINE_MAX];

	string ctrlSectionTagStr = ctrlSectionTag, ctrlSeparatorCharStr = ctrlSeparatorChar;

	if (fgets(buf, sizeof buf, filePtr) != NULL)
	{
		//sscanf(buf, "%s %s", sectionTag, separatorChar);
		sscanf_s(buf, "%s %s", sectionTag, (unsigned int)sizeof(sectionTag), separatorChar, (unsigned int)sizeof(separatorChar));

		printf("Section: %s %s", sectionTag, separatorChar);

		string sectionTagStr = sectionTag;
		string separatorCharStr = separatorChar;

		if (sectionTagStr.compare(ctrlSectionTagStr) != 0 || separatorCharStr.compare(ctrlSeparatorCharStr) != 0)
		{
			printf("Error in file format \n");
			return;
		}

		loadTSPSection(buf, sectionTag);
	}
	else
	{
		printf("Error buffering file content \n");
	}
}

void FrogLeapController::loadTSPSection(char * buf, char * sectionTag)
{
	char * auxContentCharPtr = new char[50];
	string auxContentStr, sectionTagStr;
	int auxShortInt;

	sectionTagStr = sectionTag;

	if (sectionTagStr.compare("NAME") == 0)
	{
		//sscanf(buf, "%s", auxContentCharPtr);
		sscanf_s(buf, "%49s", auxContentCharPtr, (unsigned int)sizeof(auxContentCharPtr));

		this->tspLibEud2DPtr->setName(auxContentCharPtr);
		return;
	}

	if (sectionTagStr.compare("TYPE") == 0)
	{
		//sscanf(buf, "%s", auxContentCharPtr);
		sscanf_s(buf, "%49s", auxContentCharPtr, (unsigned int)sizeof(auxContentCharPtr));
		
		this->tspLibEud2DPtr->setType(auxContentCharPtr);
		return;
	}

	if (sectionTagStr.compare("DIMENSION") == 0)
	{
		//sscanf(buf, "%hu", &auxShortInt);
		sscanf_s(buf, "%d", &auxShortInt);  // Se usa %d en lugar de %hu para short int en seguridad

		this->tspLibEud2DPtr->setDimension(auxShortInt);
		return;
	}

	if (sectionTagStr.compare("EDGE_WEIGHT_TYPE") == 0)
	{
		//sscanf(buf, "%s", auxContentCharPtr);
		sscanf_s(buf, "%49s", auxContentCharPtr, (unsigned int)sizeof(auxContentCharPtr));

		this->tspLibEud2DPtr->setEdgeWeightType(auxContentCharPtr);
		return;
	}

	if (sectionTagStr.compare("CAPACITY") == 0)
	{
		//sscanf(buf, "%hu", &auxShortInt);
		sscanf_s(buf, "%d", &auxShortInt);  // Se usa %d en lugar de %hu para evitar problemas
		
		this->tspLibEud2DPtr->setCapacity(auxShortInt);
		return;
	}

	printf("Error in file format: Section does not exist");

	return;
}


void FrogLeapController::setFailAttempts(int vfailAttempts)
{
	this->failAttempts = vfailAttempts;
}

int FrogLeapController::getSuccessAttempts()
{
	return this->successAttempts;
}

void FrogLeapController::incSuccessAttempts()
{
	this->successAttempts++;
}

void FrogLeapController::setSuccessAttempts(int vsucessAttempts)
{
	this->successAttempts = vsucessAttempts;
}

void FrogLeapController::setAsCustomer(int customerInternalId, int demand)
{
	Pair* customerPair = new Pair(PairType::IntVsInt);
	customerPair->set_i_IntValue(demand);
	customerPair->set_j_IntValue(demand);
	customerPair->setValue(customerInternalId);
	customerPair->setId(customerInternalId);
	customerPair->setLabelId(-1);

	this->customerList->addFrogObjectOrdered(customerPair);
}

void FrogLeapController::setAsCustomer(int customerInternalId, int demand, int labelId)
{
	Pair * customerPair = new Pair(PairType::IntVsInt);
	customerPair->set_i_IntValue(demand);
	customerPair->set_j_IntValue(demand);
	customerPair->setValue(customerInternalId);
	customerPair->setId(customerInternalId);
	customerPair->setLabelId(labelId);

	this->customerList->addLastFrogObject(customerPair);
}

void FrogLeapController::setAsDepot(int depotInternalId, int capacity)
{

	Pair* depotPair = new Pair(PairType::IntVsInt);
	depotPair->set_i_IntValue(capacity);
	depotPair->set_j_IntValue(capacity);
	depotPair->setValue(depotInternalId);
	depotPair->setId(depotInternalId);
	depotPair->setLabelId(-1);

	this->depotList->addLastFrogObject(depotPair);
}

void FrogLeapController::setAsDepot(int depotInternalId, int capacity, int labelId)
{

	Pair * depotPair = new Pair(PairType::IntVsInt);
	depotPair->set_i_IntValue(capacity);
	depotPair->set_j_IntValue(capacity);
	depotPair->setValue(depotInternalId);
	depotPair->setId(depotInternalId);
	depotPair->setLabelId(labelId);

	this->depotList->addLastFrogObject(depotPair);
}

void FrogLeapController::setUpCustomerList()
{
	int n_customers = this->getNumberOfCustomers();

	this->customerArray = new Pair *[n_customers];

	for (int i = 0; i < n_customers; i++)
	{
		this->customerArray[i] = (Pair *) this->customerList->getFrogObject(i);
	}
}

void FrogLeapController::setUpDepotList()
{
	int n_depots = this->getNumberOfDepots();

	this->depotArray = new Pair *[n_depots];

	for (int i = 0; i < n_depots; i++)
	{
		depotArray[i] = (Pair *)this->depotList->getFrogObject(i);
	}
}

void FrogLeapController::loadCustomerAndDepotList()
{
	int size = this->tspLibEud2DPtr->getDimension(), demandOrCapacity;

	IndexList * depotListSection = this->tspLibEud2DPtr->getDepotSection();

	// i is the internal id of the node in the nodeCoordSection collection
	for (int i = 0; i < size; i++)
	{			
		Pair * nodeCoord = (Pair *) this->tspLibEud2DPtr->getNodeCoordSection()->getFrogObject(i);

		int nodeIdLabel = nodeCoord->getId();

		Pair * demandPair = (Pair *) this->tspLibEud2DPtr->getDemandSection()->getFrogObjectById(nodeIdLabel);

		if(demandPair != NULL)
		{
			demandOrCapacity = demandPair->get_j_IntValue();
		}		
		else 
		{
			demandOrCapacity = -1;
		}

		//if does not exist, then it is a customer
		if (depotListSection->getItemById(nodeIdLabel) == -1)
		{				
			this->setAsCustomer(i, demandOrCapacity, nodeIdLabel);
		}
		else //is a depot
		{
			this->setAsDepot(i, demandOrCapacity, nodeIdLabel);
		}
	}
}

int FrogLeapController::getLabelId(int internalId)
{
	if(this->source_t == SourceType::Graph)
	{
		return this->graphPtr->getVertexIdByPosition(internalId);
	}
	else
	{
		if(this->source_t == SourceType::Tsp2DEuc)
		{
			return this->tspLibEud2DPtr->getNodeCoordSection()->getFrogObject(internalId)->getId();
		}
		else
		{
			FrogObject * item = this->customerList->getFrogObjectById(internalId);
			if(item != NULL)
			{
				return item->getId();
			}
			else
			{
				item = this->depotList->getFrogObjectById(internalId);
				return item->getId();
			}
		}		
	}
	
}

int FrogLeapController::getDepotListIndexByInternal(int depotInternalId)
{

	int size = this->depotList->getSize();

	for (int i = 0; i < size; i++)
	{
		if (this->depotArray[i]->getId() == depotInternalId)
		{			
			return i;
		}
	}

	return -1;
}

int FrogLeapController::getCustomerListIndexByInternal(int customerInternalId)
{

	for (int i = 0; i < this->customerList->getSize(); i++)
	{
		if (this->customerArray[i]->getId() == customerInternalId)
		{
			return i;
		}
	}

	return -1;
}

int FrogLeapController::getDepotIndexByLabelId(int depotLabelId)
{
	
	int depotInternalIndex = -1;
	Pair * depotPair = NULL;
	Pair * nodeCoord = NULL;
	int nDepots = this->getNumberOfDepots();

	for(int i = 0; i < nDepots; i++)
	{
		depotPair = (Pair *)this->depotArray[i];

		if(depotPair != NULL)
		{
			depotInternalIndex = depotPair->getId();
			nodeCoord = (Pair *) this->tspLibEud2DPtr->getNodeCoordSection()->getFrogObject(depotInternalIndex);
			if(nodeCoord != NULL)
			{
				int nodeIdLabel = nodeCoord->getId();
				if(nodeIdLabel == depotLabelId)
				{
					return i;
				}
			}
		}		
	}

	return -1;
}

Pair * FrogLeapController::getNodeCoordPairById(int pairId)
{
	Pair* nodeCoord = NULL;

	nodeCoord = (Pair*)this->tspLibEud2DPtr->getNodeCoordSection()->getFrogObject(pairId);

	return nodeCoord;
}

Pair* FrogLeapController::getNodeDemandPairById(int pairId)
{
	Pair* nodeCoord = NULL;

	nodeCoord = (Pair*)this->tspLibEud2DPtr->getDemandSection()->getFrogObject(pairId);

	return nodeCoord;
}

int FrogLeapController::getCustomerIndexByLabelId(int customerLabelId)
{
	int customerInternalIndex = -1;
	Pair * customerPair = NULL;
	Pair * nodeCoord = NULL;

	for (int i = 0; i < this->getNumberOfCustomers(); i++)
	{
		customerPair = (Pair *)this->customerArray[i];

		if (customerPair != NULL)
		{
			customerInternalIndex = customerPair->getId();
			nodeCoord = (Pair *) this->tspLibEud2DPtr->getNodeCoordSection()->getFrogObject(customerInternalIndex);
			if (nodeCoord != NULL)
			{
				int nodeIdLabel = nodeCoord->getId();
				if (nodeIdLabel == customerLabelId)
				{
					return i;
				}
			}
		}
	}

	return -1;
}

FrogObjectCol * FrogLeapController::getTestCustomerSectionList()
{
	return this->testCaseObjPtr->getCustomerSection();
}

void FrogLeapController::setUpCustomerAndDepotLists()
{
	if(this->getSourceType() == SourceType::Tsp2DEuc)
	{
		loadCustomerAndDepotList();		
	}

	setUpCustomerList();
	setUpDepotList();
}

void FrogLeapController::setUpVehicleCapacity()
{
	this->vehicle_capacity = this->tspLibEud2DPtr->getCapacity();
}

void FrogLeapController::setUpVehicleCapacity(int vehicleCapacity)
{
	this->vehicle_capacity = vehicleCapacity;
}

long int FrogLeapController::getVehicleCapacity()
{
	return this->vehicle_capacity;
}

void FrogLeapController::setUpVehiclePairList()
{
	int n_vehiclePairs = this->vehiclePairList->getSize();

	this->vehiclePairArray = new Pair *[n_vehiclePairs];

	for (int i = 0; i < n_vehiclePairs; i++)
	{
		vehiclePairArray[i] = (Pair *)this->vehiclePairList->getFrogObject(i);
	}
}

int FrogLeapController::getNumberOfVehicles()
{
	return this->vehiclePairList->getSize();
}

int FrogLeapController::getCustomerInternalId(int position)
{
	return this->customerArray[position]->getId();
}

int FrogLeapController::getCustomerLabelId(int position)
{
	return this->customerArray[position]->getLabelId();
}


int FrogLeapController::getCustomerDemandByIndex(int position)
{
	return this->customerArray[position]->get_i_IntValue();
}

int FrogLeapController::getDepotInternalId(int position)
{
	
	return this->depotArray[position]->getId();
}

int  FrogLeapController::getDepotCapacityByIndex(int position)
{
	return this->depotArray[position]->get_i_IntValue();
}

int  FrogLeapController::getDepotRemainingCapacityByIndex(int position)
{
	return this->depotArray[position]->get_j_IntValue();
}

void FrogLeapController::decRemainingDepotCapacity(int position, int capacity_to_dec)
{
	int oldValue = this->depotArray[position]->get_j_IntValue();
	this->depotArray[position]->set_j_IntValue(oldValue - capacity_to_dec);
}

void  FrogLeapController::setDepotRemainingCapacityByIndex(int position, int remaining_capacity)
{
	this->depotArray[position]->set_j_IntValue(remaining_capacity);
}

FrogLeapSolution * FrogLeapController::genRandomFrogLeapSolution(FrogLeapController * controller)
{
	FrogLeapSolution * result = new FrogLeapSolution(this->sgt, this->source_t, this->getNumberOfCustomers(), this->getNumberOfDepots(), 0);

	result->genRandomSolution(controller);
	return result;
}

long int FrogLeapController::getTope()
{
	return TOPE_ITERATIONS;
}

FrogObjectCol * FrogLeapController::createDepotListOrderedByCapacity()
{
	FrogObjectCol * localDepotCol = new FrogObjectCol();
	Pair * current = NULL;

	for(int i = 0; i < this->getNumberOfDepots(); i++)
	{
		current = this->depotArray[i];
		current->setValue(current->get_i_IntValue());
		localDepotCol->addFrogObjectOrdered(current);
	}

	return localDepotCol;
}

FrogObjectCol * FrogLeapController::createDepotListDescOrderedByCapacity()
{
	FrogObjectCol * localDepotCol = new FrogObjectCol();
	Pair * current = NULL;

	for (int i = 0; i < this->getNumberOfDepots(); i++)
	{
		current = this->depotArray[i];
		current->setValue(current->get_i_IntValue());
		localDepotCol->addFrogObjectDescOrdered(current);
	}

	return localDepotCol;
}

FrogObjectCol * FrogLeapController::createDepotListOrderedByRemainingCapacity()
{
	FrogObjectCol * localDepotCol = new FrogObjectCol();
	Pair * current = NULL;

	for (int i = 0; i < this->getNumberOfDepots(); i++)
	{
		current = this->depotArray[i];
		current->setValue(current->get_j_IntValue());
		localDepotCol->addFrogObjectOrdered(current);
	}

	return localDepotCol;
}

// create a list ordered by demand in descendant order
FrogObjectCol * FrogLeapController::createCustomerListOrderedByDemandDesc()
{
	FrogObjectCol * localCustomerCol = new FrogObjectCol();
	Pair * current = NULL;

	for (int i = 0; i < this->getNumberOfCustomers(); i++)
	{
		current = this->customerArray[i];
		current->setValue(current->get_i_IntValue());
		localCustomerCol->addFrogObjectDescOrdered(current);
	}

	return localCustomerCol;
}

// create a list ordered by demand in descendant order
FrogObjectCol * FrogLeapController::createCustomerListOrderedByDemand()
{
	FrogObjectCol * localCustomerCol = new FrogObjectCol();
	Pair * current = NULL;

	for (int i = 0; i < this->getNumberOfCustomers(); i++)
	{
		current = this->customerArray[i];
		localCustomerCol->addFrogObjectOrdered(current);
	}

	return localCustomerCol;
}

void FrogLeapController::resetDepotRemainingCapacities()
{
	int numberOfDepots = this->getNumberOfDepots();
	for (int i = 0; i < numberOfDepots; i++)
	{		
		resetCapacityOrDemand(this->depotArray[i]);
	}
}

void FrogLeapController::resetCustomerRemainingDemands()
{
	int nCustomers = this->getNumberOfCustomers();
	for (int i = 0; i < nCustomers; i++)
	{
		resetCapacityOrDemand(this->customerArray[i]);
	}
}

void FrogLeapController::resetCapacityOrDemand(Pair * nodePair)
{
	
	int demandOrCap = nodePair->get_i_IntValue();
	nodePair->set_j_IntValue(demandOrCap);	
}

void FrogLeapController::deleteArray(Pair ** arrayPtr, int v_size) {
	int size = v_size;

	for (int i = 0; i < size; i++)
	{
		//delete arrayPtr[i];
		arrayPtr[i] = NULL;
	}

	delete[] arrayPtr;
}

long long int FrogLeapController::getGlobalVehicleId()
{
	long long int result = this->globalVehicleId;

	this->globalVehicleId++;

	return result;
}

int FrogLeapController::getCloserIndexToDepot(int depotIndex, int lowBoundIndex, int topBoundIndex, FrogObjectCol * localNodeCol)
{
	float closerDistance = FLT_MAX;
	int closerNodeIndex = INT_MAX;

	for (int i = lowBoundIndex; i < topBoundIndex; i++)
	{
		Pair * currentPair = (Pair *)localNodeCol->getFrogObject(i);
		Pair * depotPair = (Pair *) this->depotArray[depotIndex];
		int nodeInternalId = currentPair->getId();
		int depotInternalId = depotPair->getId();
		DistanceTable * dt = this->getDistanceTable();
		float currentDistance = dt->getEdge(nodeInternalId, depotInternalId);
		if (currentDistance < closerDistance)
		{
			closerDistance = currentDistance;
			closerNodeIndex = i;
		}
	}

	return closerNodeIndex;
}


// returns the index in the local collection passed as parameter
int FrogLeapController::getClosestCustomerLocalIndexWithCapacityToDepot(int depotIndex, int lowBoundIndex, int topBoundIndex, FrogObjectCol* localCustomerIndexesCol, int veh_rem_capacity)
{
	float closerDistance = FLT_MAX;
	int closerNodeIndex = -1;
		
	for (int i = lowBoundIndex; i < topBoundIndex; i++)
	{
		Pair* customerPair = (Pair*)localCustomerIndexesCol->getFrogObject(i);
		
		if(!customerPair->isAlreadyAssignedToVehicle())
		{
			int customerDemand = customerPair->getCustomerDemand();

			if (customerDemand <= veh_rem_capacity)
			{
				Pair* depotPair = (Pair*)this->depotArray[depotIndex];
				int customerInternalId = customerPair->getId();
				int depotInternalId = depotPair->getId();
				DistanceTable* dt = this->getDistanceTable();

				float currentDistance = dt->getEdge(customerInternalId, depotInternalId);
				if (currentDistance < closerDistance)
				{
					closerDistance = currentDistance;
					closerNodeIndex = i;
				}
			}
		}
	}

	return closerNodeIndex;
}


// returns the index in the local collection passed as parameter
int FrogLeapController::getClosestCustomerLocalIndexToDepot(int depotIndex, int lowBoundIndex, int topBoundIndex, FrogObjectCol * localCustomerIndexesCol)
{
	float closerDistance = FLT_MAX;
	int closerNodeIndex = INT_MAX;

	for (int i = lowBoundIndex; i < topBoundIndex; i++)
	{
		Pair * customerPair = (Pair *)localCustomerIndexesCol->getFrogObject(i);
		Pair * depotPair = (Pair *) this->depotArray[depotIndex];
		int customerInternalId = customerPair->getId();
		int depotInternalId = depotPair->getId();
		DistanceTable * dt = this->getDistanceTable();

		float currentDistance = dt->getEdge(customerInternalId, depotInternalId);
		if (currentDistance < closerDistance)
		{
			closerDistance = currentDistance;
			closerNodeIndex = i;
		}
	}

	return closerNodeIndex;
}

// returns the index in the local collection passed as parameter considering the remaining capacity of the vehicle
int FrogLeapController::getClosestCustomerLocalIndexWithCapacityToCustomer(int targetCustomerInternalId, int lowBoundIndex, int topBoundIndex, FrogObjectCol* localCustomerIndexesCol, int veh_rem_capacity)
{
	float closerDistance = FLT_MAX;
	int closerNodeIndex = -1;

	for (int i = lowBoundIndex; i < topBoundIndex; i++)
	{
		Pair* customerPair = (Pair*)localCustomerIndexesCol->getFrogObject(i);

		if(!customerPair->isAlreadyAssignedToVehicle())
		{
			int customerDemand = customerPair->getCustomerDemand();

			if (customerDemand <= veh_rem_capacity && customerPair->isAlreadyAssignedToVehicle() == false)
			{
				int customerInternalId = customerPair->getId();			DistanceTable* dt = this->getDistanceTable();
				float currentDistance = dt->getEdge(customerInternalId, targetCustomerInternalId);

				if (currentDistance < closerDistance)
				{
					closerDistance = currentDistance;
					closerNodeIndex = i;
				}
			}
		}
	}

	return closerNodeIndex;
}

// returns the index in the local collection passed as parameter
int FrogLeapController::getClosestCustomerLocalIndexToCustomer(int targetCustomerInternalId, int lowBoundIndex, int topBoundIndex, FrogObjectCol * localCustomerIndexesCol)
{
	float closerDistance = FLT_MAX;
	int closerNodeIndex = INT_MAX;

	for (int i = lowBoundIndex; i < topBoundIndex; i++)
	{
		Pair * customerPair = (Pair *)localCustomerIndexesCol->getFrogObject(i);
		
		int customerInternalId = customerPair->getId();		
		DistanceTable * dt = this->getDistanceTable();
		float currentDistance = dt->getEdge(customerInternalId, targetCustomerInternalId);

		if (currentDistance < closerDistance)
		{
			closerDistance = currentDistance;
			closerNodeIndex = i;
		}
	}
	
	return closerNodeIndex;
}

int FrogLeapController::getNextClosestFittingCustomerIndexFrom(Pair* item_pair, int remaining_capacity, int depotIndex, int lowBoundIndex, int topBoundIndex, FrogObjectCol* localCustomerIndexesCol, FrogLeapController* controller)
{
	float closerDistance = FLT_MAX;
	int closerNodeIndex = -1;

	for (int i = lowBoundIndex; i < topBoundIndex; i++)
	{
		Pair* customerPair = (Pair*)localCustomerIndexesCol->getFrogObject(i);

		int customer_demand = customerPair->getCustomerDemand();

		if (customer_demand <= remaining_capacity)
		{
			int customerInternalId = customerPair->getId();
			DistanceTable* dt = this->getDistanceTable();
			int targetCustomerInternalId = item_pair->getId();

			float currentDistance = dt->getEdge(customerInternalId, targetCustomerInternalId);

			if (currentDistance < closerDistance)
			{
				closerDistance = currentDistance;
				closerNodeIndex = i;
			}
		}
	}

	return closerNodeIndex;
}

bool FrogLeapController::existInLocalDepotList(int assignedDepotIndex, FrogObjectCol * localDepotCol, int low, int top)
{
	for(int i = low; i < top; i++)
	{
		Pair * currentPair = (Pair *)localDepotCol->getFrogObject(i);
		int depotInternalId = currentPair->getId();
		int depotIndex = this->getDepotListIndexByInternal(depotInternalId);
		
		if(assignedDepotIndex == depotIndex)
		{
			return true;
		}		
	}

	return false;	
}

int FrogLeapController::getDepotIndexOfClosestAssignedCustomer (int targetCustomerIndex, FrogObjectCol * localDepotCol, int low, int top, float & distanceToCustomer)
{
	float closestDistance = FLT_MAX, currentDistance;
	int result = -1;
	Pair * customerPair = NULL;
	int assignedDepotIndex;
	int customerInternalId;
	Pair * targetCustomerPair;
	int targetCustomerInternalId;
	DistanceTable * dt = NULL;

	for(int i = 0; i < this->getNumberOfCustomers(); i++)
	{
		customerPair = this->customerArray[i];

		assignedDepotIndex = customerPair->getAssignedDepotIndex();

		if(assignedDepotIndex != -1)
		{
			if(existInLocalDepotList(assignedDepotIndex, localDepotCol, low, top) == true)
			{
				customerInternalId = customerPair->getId();
				targetCustomerPair = (Pair *)this->customerArray[targetCustomerIndex];
				targetCustomerInternalId = targetCustomerPair->getId();
				dt = this->getDistanceTable();

				currentDistance = dt->getEdge(targetCustomerInternalId, customerInternalId);

				if(currentDistance < closestDistance)
				{
					closestDistance = currentDistance;
					result = customerPair->getAssignedDepotIndex();
					distanceToCustomer = closestDistance;
				}
			}
		}
	} //end for

	return result;
}

bool FrogLeapController::unassignedCustomerPairExists() 
{
	int nCustomers = this->getNumberOfCustomers();
	bool result = false;

	int i = 0;

	while (i < nCustomers && result == false)
	{
		int currentDepotIndex = this->customerArray[i]->getAssignedDepotIndex();
		if (currentDepotIndex == -1) {
			result = true;
		}
		
		i++;
	}

	return result;
}

int FrogLeapController::numberOfUnassignedCustomers()
{
	int nCustomers = this->getNumberOfCustomers();
	int result = 0;

	int i = 0;

	while (i < nCustomers)
	{
		int currentDepotIndex = this->customerArray[i]->getAssignedDepotIndex();
		if (currentDepotIndex == -1) {
			result++;
		}
		i++;
	}

	return result;
}

FrogObjectCol * FrogLeapController::getListOfUnassignedCustomers()
{
	FrogObjectCol* unassignedCustomers = new FrogObjectCol();
	int nCustomers = this->getNumberOfCustomers();
	int result = 0;

	int i = 0;

	while (i < nCustomers)
	{
		int currentDepotIndex = this->customerArray[i]->getAssignedDepotIndex();
		if (currentDepotIndex == -1) {
			unassignedCustomers->addLastFrogObject(this->customerArray[i]);
		}
		i++;
	}

	return unassignedCustomers;
}

bool FrogLeapController::getReassignCustomersSettings()
{
	return this->reassignCustomers;
}

void FrogLeapController::setRassignCustomerSettings(bool reassignCustomers)
{
	this->reassignCustomers = reassignCustomers;
}

void FrogLeapController::setCustomerPairAsAssigned(int customerIndex, int depotIndex)
{
	Pair * customerPair = this->customerArray[customerIndex];

	customerPair->setAssignedDepotIndex(depotIndex);
}

// Set every pair in customerArray as not assigned. -1 value is assigned in each item.
void FrogLeapController::resetCustomersAsNotAssigned()
{
	int nCustomers = this->getNumberOfCustomers();

	for(int i = 0; i < nCustomers; i++)
	{
		Pair * customerPair = this->customerArray[i];
		customerPair->setAssignedDepotIndex(-1);
	}
}

int FrogLeapController::getClosestLocalDepotIndexToCustomer(int customerIndex, int lowBoundIndex, int topBoundIndex, FrogObjectCol * localDepotCol, float & distance)
{
	float closestDistance = FLT_MAX;
	int closestNodeIndex = INT_MAX;

	for(int i = lowBoundIndex; i < topBoundIndex; i++ )
	{
		Pair * currentDepotPair = (Pair *)localDepotCol->getFrogObject(i);
		Pair * customerPair = (Pair *) this->customerArray[customerIndex];
		int depotInternalId = currentDepotPair->getId();
		int customerInternalId = customerPair->getId();
		DistanceTable * dt = this->getDistanceTable();
		float currentDistance = dt->getEdge(depotInternalId, customerInternalId);

		if(currentDistance < closestDistance)
		{
			closestDistance = currentDistance;
			closestNodeIndex = i;
		}
	}

	distance = closestDistance;

	return closestNodeIndex;	
}

float FrogLeapController::genRandomFloatingNumber(float a, float b)
{
	float randNumber = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	
	float result = a + diff*randNumber;
		 
	//this->writeRandomInfo(a, b, result);
		
	return result;
}

int FrogLeapController::genRandomIntNumber(int a, int b)
{
	float randNumber = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;

	float result = a + diff*randNumber;

	//this->writeRandomInfo(a, b, result);

	int finalResult = floor(result);

	if (finalResult == b)
		finalResult--;

	return finalResult;
}

int FrogLeapController::getX_Coord(int nodeInternalId)
{
	Pair * nodePair = (Pair *)this->tspLibEud2DPtr->getNodeCoordSection()->getFrogObject(nodeInternalId);
	
	return nodePair->get_i_IntValue();
}

int FrogLeapController::getY_Coord(int nodeInternalId)
{
	Pair * nodePair = (Pair *)this->tspLibEud2DPtr->getNodeCoordSection()->getFrogObject(nodeInternalId);

	return nodePair->get_j_IntValue();
}

// create a list of customers that satisfies for each customer of the list that 
// 1) the depotPair is the closest available one of all.
// The returned list is ordered by distance from the depotpair to all customers that satisfy condition 1)
FrogObjectCol * FrogLeapController::createMatchCustomerList(Pair * currentDepotPair)
{
	FrogObjectCol * matchCol = new FrogObjectCol();

	FrogObjectCol * distanceCustList = createCustomerListOrderedByDistanceFromDepot(currentDepotPair);

	int size = distanceCustList->getSize();

	for(int i = 0; i < size; i++)
	{
		Pair * currentCustomerPair = (Pair *)distanceCustList->getFrogObject(i);

		bool itIsAMatch = isAMatch(currentCustomerPair, currentDepotPair);

		if(itIsAMatch)
		{
			matchCol->addLastFrogObject(currentCustomerPair);
		}
	}

	distanceCustList->unReferenceFrogObjectCol();
	delete distanceCustList;

	return matchCol;
}

// create a list of customers that satisfies for each customer of the list that 
// 1) the depotPair is the closest available (with remaining capacity enough) one of all.
// The returned list is ordered by distance from the depotpair to all customers that satisfy condition 1)
FrogObjectCol* FrogLeapController::createFullMatchCustomerList(Pair* currentDepotPair)
{
	FrogObjectCol* matchCol = new FrogObjectCol();

	// creates a customer list ordered by distance from depot considering only not assigned customers
	// we do not create copies so we need to unreference the list before deleting it.
	FrogObjectCol* distanceCustList = createCustomerListOrderedByDistanceFromDepot(currentDepotPair);

	int size = distanceCustList->getSize();

	for (int i = 0; i < size; i++)
	{
		Pair* currentCustomerPair = (Pair*)distanceCustList->getFrogObject(i);

		// get the demand of current customer pair 
		int customerDemand = this->getCustomerDemandByIndex(this->getCustomerListIndexByInternal(currentCustomerPair->getId()));

		// get remaining capacity of current depot
		int depotRemainingCapacity = this->getDepotRemainingCapacityByIndex(this->getDepotListIndexByInternal(currentDepotPair->getId()));

		if(depotRemainingCapacity >= customerDemand)
		{
			bool itIsAMatch = isAFullMatch(currentCustomerPair, currentDepotPair);

			if (itIsAMatch)
			{
				matchCol->addLastFrogObject(currentCustomerPair);
			}
		}
	}

	distanceCustList->unReferenceFrogObjectCol();
	delete distanceCustList;

	return matchCol;
}

FrogObjectCol* FrogLeapController::reorderCustomerListByDemandPerDistance(FrogObjectCol * matchCustomerCol, Cluster * cluster)
{
	FrogObjectCol* result = new FrogObjectCol();

	int size = matchCustomerCol->getSize();

	DistanceTable* dt = this->getDistanceTable();

	for(int i = 0; i < size; i++)
	{
		Pair * currentCustomerPair = (Pair *)matchCustomerCol->getFrogObject(0);
		matchCustomerCol->removeFrogObjectByPosition(0);

		int currentCustomerPair_Index = this->getCustomerListIndexByInternal(currentCustomerPair->getId());
		float distanceToCluster = cluster->getDistanceToCustomer(currentCustomerPair_Index, this);
		
		float demandPerDistance = (float)currentCustomerPair->get_i_IntValue() / distanceToCluster;

		currentCustomerPair->setValue(demandPerDistance);
		result->addFrogObjectOrdered(currentCustomerPair);
	}

	matchCustomerCol->unReferenceFrogObjectCol();
	delete matchCustomerCol;

	return result;
}

// create a list of customers that satisfies for each customer of the list that 
// 1) the depotPair is the closest one of all.
// The returned list is ordered by distance from the depotpair to all customers that satisfy condition 1)
FrogObjectCol* FrogLeapController::createMatchCustomerList_Cluster(Cluster* currentCluster, FrogLeapSolution* fls)
{
	FrogObjectCol* matchCol = new FrogObjectCol();

	FrogObjectCol* distanceCustList = createCustomerListOrderedByDistanceFromCluster(currentCluster, fls);

	int size = distanceCustList->getSize();

	for (int i = 0; i < size; i++)
	{
		Pair* currentCustomerPair = (Pair*)distanceCustList->getFrogObject(i);

		//if(currentCustomerPair->getLabelId() == 15)
		//{
		//	printf("showing current cluster information \n");
		//	currentCluster->printFrogObj();
		//	printf("\n");
		//	printf("showing currentCustomerPair information \n");
		//	currentCustomerPair->printFrogObj();
		//	printf("\n");


		//	printf("Parar acá \n");

		//	DistanceTable* dt = this->getDistanceTable();
		//	float distance = dt->getEdge(currentCluster->getDepotPair()->getId(), currentCustomerPair->getId());
		//	printf("Distance from customer LabelId = %d to depot LabelId = %d is %.2f \n", currentCustomerPair->getLabelId(), currentCluster->getDepotPair()->getLabelId(), distance);

		//	printf("Showing distanceCustList size: %d \n", size);
		//	distanceCustList->printFrogObjCol();
		//	printf("\n");

		//}

		
		bool itIsAMatch = fls->isAMatch_Cluster(currentCustomerPair, currentCluster, this);

		if (itIsAMatch)
		{
			//if (this->getCustomerListIndexByInternal(currentCustomerPair->getId()) == 19)
			//{
				//printf("Stop in createMatchCustomerList: customer 19 found \n");
			//}

			matchCol->addLastFrogObject(currentCustomerPair);
		}
	}

	distanceCustList->unReferenceFrogObjectCol();
	delete distanceCustList;

	return matchCol;
}

FrogObjectCol* FrogLeapController::createCustomerListOrderedByDistanceFromCluster(Cluster* currentCluster, FrogLeapSolution * fls)
{
	FrogObjectCol * distanceCustomerList = new FrogObjectCol();
	Pair * customerPair_i = NULL;
	Pair * clusterDepot = NULL;
	int nCustomers = this->getNumberOfCustomers();
	float currentDistance;

	for (int i = 0; i < nCustomers; i++)
	{
		customerPair_i = this->getCustomerPairByIndex(i);
		
		clusterDepot = currentCluster->getDepotPair();
		int clusterDepotIndex = this->getDepotListIndexByInternal(clusterDepot->getId());

		if(this->getReassignCustomersSettings() == true)
		{
			if (customerPair_i->getAssignedDepotIndex() == clusterDepotIndex)
			{ //si ya está asignado, verifico si está asignado al currentCluster. Si es, desasigno momentáneamente para hacer el cálculo y vuelvo a asignar

				// this is momentarily just to recalculate the distance from currentCustomer to the rest of members of its cluster.
				unassignCustomerFromCluster(customerPair_i, currentCluster);

				currentDistance = currentCluster->getDistanceToCustomer(i, this);
				customerPair_i->setValue(currentDistance);
				distanceCustomerList->addFrogObjectOrdered(customerPair_i);

				// assigning again the customer to its original cluster
				assignCustomerToClusterByCustomerIndex(currentCluster, i);
			}
			else
			{
				currentDistance = currentCluster->getDistanceToCustomer(i, this);
				customerPair_i->setValue(currentDistance);
				distanceCustomerList->addFrogObjectOrdered(customerPair_i);				
			}
		}
		else
		{
			if (customerPair_i->getAssignedDepotIndex() == -1)
			{
				currentDistance = currentCluster->getDistanceToCustomer(i, this);
				customerPair_i->setValue(currentDistance);
				distanceCustomerList->addFrogObjectOrdered(customerPair_i);
			}
		}
	}

	return distanceCustomerList;
}

void FrogLeapController::assignCustomerToClusterByCustomerIndex(Cluster* currentCluster, int customerIndex)
{
	// we need to assign the customer with customerIndex to the depot of the currentCluster
	Pair * customerPair = this->getCustomerPairByIndex(customerIndex);
	Pair* clusterDepotPair = currentCluster->getDepotPair();
	int clusterDepotIndex = this->getDepotListIndexByInternal(clusterDepotPair->getId());
	

	customerPair->setAssignedDepotIndex(clusterDepotIndex);
	clusterDepotPair->upDateRemainingCapacity(customerPair);
	currentCluster->addCustomer(customerIndex, this);	
}

void FrogLeapController::unassignCustomerFromCluster(Pair* customerPair, Cluster* currentCluster)
{
	int depotIndex = customerPair->getAssignedDepotIndex();
	Pair* depotPair = this->getDepotPairByIndex(depotIndex);
	depotPair->restoreRemainingCapacity(customerPair);
	customerPair->setAssignedDepotIndex(-1);
	currentCluster->unassignCustomerFromCluster(customerPair);
}

/// revisado
FrogObjectCol * FrogLeapController::createCustomerListOrderedByDistanceFromDepot(Pair * currentDepotPair)
{
	FrogObjectCol * distanceCustomerList = new FrogObjectCol();

	int customerSetSize = this->getNumberOfCustomers();

	for(int i = 0; i < customerSetSize; i++)
	{
		Pair * customerPair = this->customerArray[i];

		// check if customerPair is already assigned
		int assignedDepotIndex = customerPair->getAssignedDepotIndex();

		if (assignedDepotIndex == -1)
		{
			int customerInternalId = customerPair->getId();			
			int currentDepotInternalId = currentDepotPair->getId();
			DistanceTable * dt = this->getDistanceTable();

			float currentDistance = dt->getEdge(currentDepotInternalId, customerInternalId);

			// add customerPair ordered by distance
			customerPair->setValue(currentDistance);

			distanceCustomerList->addFrogObjectOrdered(customerPair);
		}
	}

	return distanceCustomerList;
}

// revisado
// this collection must be completely destroyed since it creates copies of elements. 
// values of objects represents the distance from currentCustomer
FrogObjectCol * FrogLeapController::createDepotListOrderedByDistanceFromCustomer(Pair * currentCustomerPair)
{
	FrogObjectCol * distanceDepotList = new FrogObjectCol();

	int depotSetSize = this->getNumberOfDepots();

	for (int i = 0; i < depotSetSize; i++)
	{
		Pair * depotPair = this->depotArray[i];

		int depotInternalId = depotPair->getId();
		int currentCustomerInternalId = currentCustomerPair->getId();

		DistanceTable * dt = this->getDistanceTable();

		float currentDistance = dt->getEdge(currentCustomerInternalId, depotInternalId);

		Pair * depotPairCopy = depotPair->createCopy();

		depotPairCopy->setValue(currentDistance);

		distanceDepotList->addFrogObjectOrdered(depotPairCopy);
	};

	return distanceDepotList;
}

// checks if currentDepotPair = the closest Depot to currentCustomerPair
bool FrogLeapController::isAMatch(Pair* currentCustomerPair, Pair* currentDepotPair)
{
	bool result = false;
	FrogObjectCol* depotListOrdered = currentCustomerPair->getPairCol();

	if (depotListOrdered == NULL)
	{
		depotListOrdered = this->createDepotListOrderedByDistanceFromCustomer(currentCustomerPair);
		currentCustomerPair->setPairCol(depotListOrdered);
	}

	Pair* closestDepotPair = (Pair*)depotListOrdered->getFrogObject(0);

	result = (currentDepotPair->getId() == closestDepotPair->getId());
	return result;
}

// checks if currentDepotPair = the closest Depot to currentCustomerPair with remaining capacity enough 
bool FrogLeapController::isAFullMatch(Pair * currentCustomerPair, Pair * currentDepotPair)
{
	bool result = false;
	FrogObjectCol* depotListOrdered = currentCustomerPair->getPairCol();

	if (depotListOrdered == NULL)
	{
		// this list has copies of depotPair. They must be destroyed
		depotListOrdered = this->createDepotListOrderedByDistanceFromCustomer(currentCustomerPair);
		currentCustomerPair->setPairCol(depotListOrdered);
	}
	else {
		result = false; // código para debug solamente
	};

	// get the demand of current customer
	int customerDemand = this->getCustomerDemandByIndex(this->getCustomerListIndexByInternal(currentCustomerPair->getId()));
	
	// get the closest depot with remaining capacity enough to attend currentCustomer
	Pair* closestDepotPairWithCapacity = (Pair *)this->getFirstDepotPairWithRemainingCapacity(depotListOrdered, customerDemand);

	if (closestDepotPairWithCapacity != nullptr) {
		result = (currentDepotPair->getId() == closestDepotPairWithCapacity->getId());
	}
	else {
		result = false;
	}
	
	return result;
}

Pair * FrogLeapController::getFirstDepotPairWithRemainingCapacity(FrogObjectCol* depotListOrdered, int customerDemand)
{
	int size = depotListOrdered->getSize();

	for (int i = 0; i < size; i++)
	{
		Pair * currentDepot = (Pair *)depotListOrdered->getFrogObject(i);
		
		int currentDepotRemainingCapacity = this->getDepotRemainingCapacityByIndex(this->getDepotListIndexByInternal(currentDepot->getId()));

		if(currentDepotRemainingCapacity >= customerDemand)
		{
			return currentDepot;
		}
	}

	return nullptr;
}

Cluster * FrogLeapController::getFirstClusterWithRemainingCapacity(FrogObjectCol* clusterListOrdered, Pair * currentCustomerPair, FrogLeapController * controller)
{
	int customerIndex = controller->getCustomerListIndexByInternal(currentCustomerPair->getId());
	int customerDemand = controller->getCustomerDemandByIndex(customerIndex);
	
	int customerDepotIndex = currentCustomerPair->getAssignedDepotIndex();

	int size = clusterListOrdered->getSize();

	for (int i = 0; i < size; i++)
	{
		Cluster* currentCluster = (Cluster*)clusterListOrdered->getFrogObject(i);

		Pair* currentClusterDepot = currentCluster->getDepotPair();
		int clusterDepotIndex = this->getDepotListIndexByInternal(currentClusterDepot->getId());

		if(customerDepotIndex == clusterDepotIndex)
		{
			return currentCluster;
		}
		else
		{
			int currentDepotRemainingCapacity = this->getDepotRemainingCapacityByIndex(this->getDepotListIndexByInternal(currentClusterDepot->getId()));

			if (currentDepotRemainingCapacity >= customerDemand)
			{
				return currentCluster;
			}
		}
	}

	return nullptr;
}

int FrogLeapController::getTotalDemandOrCapacity(FrogObjectCol * pairCol)
{
	float result = 0;

	int size = pairCol->getSize();

	for (int i = 0; i < size; i++)
	{
		Pair * currentPair = (Pair *)pairCol->getFrogObject(i);
		result = result + currentPair->get_i_IntValue();
	}

	return result;
}

void FrogLeapController::assignDepotToCustomerPairs(Pair * depotPair, FrogObjectCol * customerCol)
{
	int size = customerCol->getSize();
	
	for(int i = 0; i < size; i++)
	{
		Pair * customerPair = (Pair *) customerCol->getFrogObject(i);

		this->assignDepotToCustomer(depotPair, customerPair);

		depotPair->upDateRemainingCapacity(customerPair);		
	}
}

float FrogLeapController::assignRandomToDepotSelected(int depotIndex)
{
	float randnum, result;

	do
	{
		randnum = this->genRandomFloatingNumber(0, 1);
	} while (randnum >= 1);

	result = depotIndex + randnum;

	return  result;
}

bool FrogLeapController::isCustomerAssignedToCluster(Pair* customerPair, Cluster* cluster)
{
	bool result = false;

	int customerDepotIndex = customerPair->getAssignedDepotIndex();
	Pair* clusterDepot = cluster->getDepotPair();
	int clusterDepotIndex = this->getDepotListIndexByInternal(clusterDepot->getId());

	result = (customerDepotIndex == clusterDepotIndex);

	return result;
}

// assigns each customer to depot until remaining capacity = 0 and return a list of all assigned customers
// assigns each customer of the list until remaining capacity = 0
FrogObjectCol * FrogLeapController::assignDepotToCustomerPairsUntilCapacityIsComplete(Pair * depotPair, FrogObjectCol * customerCol, FrogLeapSolution * fs)
{
	int size = customerCol->getSize(), i = 0;
	Pair* customerPair;


	bool demandCompleted = false;
	
	FrogObjectCol * newCol = new FrogObjectCol();

	while (i < size  && demandCompleted == false)
	{
		
		customerPair = (Pair*)customerCol->getFrogObject(i);
		
		int customerDemand = customerPair->get_i_IntValue();
		int currentDepotRemainingCapacity = depotPair->get_j_IntValue();

		if (currentDepotRemainingCapacity == 0)
		{
			demandCompleted = true;
		}
		else
		{
			if (customerDemand <= currentDepotRemainingCapacity)
			{
				this->assignDepotToCustomerAndUpdateRemainingCapacity(depotPair, customerPair, fs);
				
				newCol->addLastFrogObject(customerPair);
			}
	
			//customerCol->removeFrogObject(customerPair);	

		}	

		i++;
	}

	//customerCol->unReferenceFrogObjectCol();
	//delete customerCol;

	return newCol;
}

FrogObjectCol* FrogLeapController::assignCustomersToClusterUntilCapacityIsComplete(Cluster* cluster, FrogObjectCol* customerCol, FrogLeapSolution* fls)
{
	float u;	
	float remainingDepotCapacity = -1;
	Pair* customerPair = NULL;
	Pair* clusterDepotPair = NULL;
	int depotIndex = -1;
	int customerIndex = -1;
	float customerDemand = -1;
	FrogObjectCol* assignedCustomersInCurrentPhase = new FrogObjectCol();

	int size = customerCol->getSize();
	for (int i = 0; i < size; i++)
	{
		customerPair = (Pair*)customerCol->getFrogObject(i);

		clusterDepotPair = cluster->getDepotPair();
		depotIndex = this->getDepotListIndexByInternal(clusterDepotPair->getId());
		customerIndex = this->getCustomerListIndexByInternal(customerPair->getId());

		if(!this->isCustomerAssignedToCluster(customerPair, cluster))
		{			
			remainingDepotCapacity = this->getDepotRemainingCapacityByIndex(depotIndex);			
			customerDemand = this->getCustomerDemandByIndex(customerIndex);

			if (remainingDepotCapacity >= customerDemand) {
				// if its fits then customer needs to be assigned to this cluster

				if (customerPair->getAssignedDepotIndex() != -1)
				{ //if it is previously assigned to another cluster, then it needs to be unassigned.
					this->unassignCustomerFromItsCluster(customerPair, fls);					
				}

				cluster->addCustomer(customerIndex, this);
				this->assignDepotToCustomer(clusterDepotPair, customerPair);
				clusterDepotPair->upDateRemainingCapacity(customerPair);
				
				u = this->assignRandomToDepotSelected(depotIndex);
				fls->setFLValue(customerIndex, u);

				assignedCustomersInCurrentPhase->addFrogObject(customerPair);
			}

			if (customerIndex == 8)
			{
				//printf("Stop: customer 8 found in assignCustomersToCluster \n");
			}
		}
		else // if it is already a member of the cluster
		{
			u = this->assignRandomToDepotSelected(depotIndex);
			fls->setFLValue(customerIndex, u);
			assignedCustomersInCurrentPhase->addFrogObject(customerPair);
		}				
	}

	return assignedCustomersInCurrentPhase;
}

FrogObjectCol* FrogLeapController::assignDepotRndToCustomerPairsUntilCapacityIsComplete(Pair* depotPair, FrogObjectCol* customerCol, FrogLeapSolution* fs)
{
	int size = customerCol->getSize(), i = 0, rnd_i;
	Pair* customerPair;

	FeasibleSolution* indexRndArray = this->initFeasibleSolution(size);

	bool demandCompleted = false;

	FrogObjectCol* newCol = new FrogObjectCol();

	while (i < size && demandCompleted == false)
	{
		rnd_i = this->selectRandomIndex(i, indexRndArray);
		
		customerPair = (Pair*)customerCol->getFrogObject(rnd_i);

		int customerDemand = customerPair->get_i_IntValue();
		int currentDepotRemainingCapacity = depotPair->get_j_IntValue();

		if (currentDepotRemainingCapacity == 0)
		{
			demandCompleted = true;
		}
		else
		{
			if (customerDemand <= currentDepotRemainingCapacity)
			{
				this->assignDepotToCustomerAndUpdateRemainingCapacity(depotPair, customerPair, fs);

				newCol->addLastFrogObject(customerPair);
			}

			//customerCol->removeFrogObject(customerPair);	

		}

		i++;
	}

	//customerCol->unReferenceFrogObjectCol();
	//delete customerCol;
	delete indexRndArray;

	return newCol;

}

void FrogLeapController::upDateRemainingCapacity(Pair * depotPair, Pair * customerPair)
{
	int depotIndex = this->getDepotListIndexByInternal(depotPair->getId());
	Pair* localDepotPair = this->getDepotPairByIndex(depotIndex);

	localDepotPair->upDateRemainingCapacity(customerPair);
}

FrogObjectCol * FrogLeapController::selectCustomerPairsUntilDemandComplete(Pair * depotPair, FrogObjectCol * customerCol)
{
	int size = customerCol->getSize();

	bool demandCompleted = false;
	int i = 0;
	FrogObjectCol * newCol = new FrogObjectCol();

	while (i < size && demandCompleted == false)
	{

		Pair * customerPair = (Pair *)customerCol->getFrogObject(0);

		int customerDemand = customerPair->get_i_IntValue();
		int currentDepotRemainingCapacity = depotPair->get_j_IntValue();

		if (currentDepotRemainingCapacity == 0)
		{
			demandCompleted = true;
		}
		else
		{
			if (customerDemand <= currentDepotRemainingCapacity)
			{
				depotPair->upDateRemainingCapacity(customerPair);
				//this->assignDepotToCustomer(depotPair, customerPair);
				newCol->addLastFrogObject(customerPair);
			}

			customerCol->removeFrogObject(customerPair);
			i++;
		}
	}

	// reset capacity of depot
	this->resetCapacityOrDemand(depotPair);

	customerCol->unReferenceFrogObjectCol();
	delete customerCol;

	return newCol;
}

void FrogLeapController::assignCustomersToDepot(Pair * depotPair, FrogObjectCol * & customerCol, FrogObjectCol * depotListOrderedByCapacity, FrogLeapSolution * fs)
{
	int size = customerCol->getSize();



	for (int i = 0; i < size; i++)
	{
		int rand_int = this->genRandomIntNumber(0, customerCol->getSize());
		Pair * customerPair = (Pair *)customerCol->getFrogObject(rand_int);

		int customerIndex = this->getCustomerListIndexByInternal(customerPair->getId());

		float u = fs->assignRandomFeasibleDepot4(this, depotListOrderedByCapacity, customerIndex);

		fs->setFLValue(customerIndex, u);

		customerCol->removeFrogObject(customerPair);
	}
}

void FrogLeapController::assignCustomersToDepot2(Pair* depotPair, FrogObjectCol*& customerCol, FrogObjectCol* depotListOrderedByCapacity, FrogLeapSolution* fs)
{
	int size = customerCol->getSize();



	for (int i = 0; i < size; i++)
	{
		int rand_int = this->genRandomIntNumber(0, customerCol->getSize());
		Pair* customerPair = (Pair*)customerCol->getFrogObject(rand_int);

		int customerIndex = this->getCustomerListIndexByInternal(customerPair->getId());

		float u = fs->assignRandomFeasibleDepot4(this, depotListOrderedByCapacity, customerIndex);

		fs->setFLValue(customerIndex, u);

		customerCol->removeFrogObject(customerPair);
	}
}

FrogObjectCol * FrogLeapController::orderCustomerPairListByNthClosestDepotDesc(int n, FrogObjectCol * customerPairCol)
{
	int size = customerPairCol->getSize();
	Pair * customerPair = NULL;
	FrogObjectCol * customerPairFrogObjectCol = NULL;
	FrogObjectCol * newCol = new FrogObjectCol();
	
	for (int i = 0; i < size; i++)
	{
		customerPair = (Pair *)customerPairCol->getFrogObject(0);
		customerPairFrogObjectCol = customerPair->getPairCol();
		if(customerPairFrogObjectCol != NULL)
		{
			Pair * localPair = (Pair *)customerPairFrogObjectCol->getFrogObject(n - 1);
			float nthFurthestDistance = localPair->getValue();
			customerPair->setValue(nthFurthestDistance);

			customerPairCol->removeFrogObject(customerPair);
			newCol->addFrogObjectDescOrdered(customerPair);
		}
		else {
			int result = 1;
		}
	}

	customerPairCol->unReferenceFrogObjectCol();
	delete customerPairCol;

	return newCol;
}

void FrogLeapController::assignDepotToCustomer(Pair * depotPair, Pair * customerPair)
{
	int depotIndex = this->getDepotListIndexByInternal(depotPair->getId());
	customerPair->setAssignedDepotIndex(depotIndex);
}

void FrogLeapController::unassignCustomerFromItsCluster(Pair* customerPair, FrogLeapSolution * fls)
{
	int depotIndex = customerPair->getAssignedDepotIndex();
	Pair* depotPair = this->getDepotPairByIndex(depotIndex);
	depotPair->restoreRemainingCapacity(customerPair);
	customerPair->setAssignedDepotIndex(-1);
	
	Cluster* currentCluster = fls->getClusterByIndex(depotIndex);
	currentCluster->unassignCustomerFromCluster(customerPair);	
}

float FrogLeapController::addRandomNumberToInt(int index)
{
	float result = 0;
	float randnum;

	do
	{
		randnum = this->genRandomFloatingNumber(0, 1);
	} while (randnum >= 1);

	result = index + randnum;

	return  result;	
}

bool FrogLeapController::isCustomerPairAssigned(Pair * customerPair)
{
	if (customerPair->getAssignedDepotIndex() == -1)
	{
		return false;
	}
	else
	{
		return true;
	};		
}

bool FrogLeapController::isCustomerPairAssigned(int customerIndex)
{
	Pair * customerPair = this->getCustomerPairByIndex(customerIndex);

	if (customerPair->getAssignedDepotIndex() == -1)
		return false;
	else
		return true;
}

// Assigns depot index to customerPair. 
// Assigns depot value into FrogLeaping FL solution in customer index. This means that into the array of customers represented by
// the FL solution, in customer index we assign the depot index plus a value between 0 and 1.
void FrogLeapController::assignDepotToCustomerAndUpdateRemainingCapacity(Pair* depotPair, Pair * customerPair, FrogLeapSolution * fs)
{
	this->upDateRemainingCapacity(depotPair, customerPair);
	// assing the depot index to customerPair in its attribute
	this->assignDepotToCustomer(depotPair, customerPair);
	int depotIndex = this->getDepotListIndexByInternal(depotPair->getId());
	int customerIndex = this->getCustomerListIndexByInternal(customerPair->getId());

	// assing the depot index to customer index place in the customer array represented by fs 
	fs->setFLValue(customerIndex, depotIndex);
}

//void FrogLeapController::setEngine(std::default_random_engine * generator)
//{
//	this->engine = generator;
//}

//std::default_random_engine * FrogLeapController::getEngine()
//{
//	return this->engine;
//}

void FrogLeapController::openOutPutFile()
{
	//outPutFileName = "outPutFL2.txt";
	////pFile = fopen(outPutFileName, "w");
	//fopen_s(&pFile, outPutFileName, "w");

	//if(pFile != NULL)
	//{
	//	fputs("MDVRP ALGORITHM OUTPUT \n", pFile);
	//}


	const char* outPutFileName = "outPutFL2.txt";

	// Intentar abrir el archivo de salida de forma segura
	errno_t err = fopen_s(&pFile, outPutFileName, "w");

	if (err == 0 && pFile != nullptr) // Verificar que fopen_s no haya fallado
	{
		fputs("MDVRP ALGORITHM OUTPUT \n", pFile);
	}
	else
	{
		printf("Error al abrir el archivo de salida: %s\n", outPutFileName);
	}
}

void FrogLeapController::closeOutPutFile()
{
	fclose(this->pFile);
}

FILE * FrogLeapController::openFile(char* fileName)
{
	//FILE * pFile = fopen(fileName, "w");
	//FILE* pFile = NULL;
	//
	//fopen_s(&pFile, fileName, "w");

	//if (pFile == NULL)
	//{
	//	std::cerr << "Error: Null pointer detected. Terminating program." << std::endl;
	//	// Terminate the program
	//	exit(EXIT_FAILURE);
	//}

	//return pFile;


	FILE* pFile = NULL;
	errno_t err = fopen_s(&pFile, fileName, "w");

	if (err != 0 || pFile == NULL)
	{
		std::cerr << "Error: Could not open file " << fileName << ". Terminating program." << std::endl;
		exit(EXIT_FAILURE);
	}

	return pFile;
}

void FrogLeapController::closeFile(FILE* filePtr)
{
	fclose(filePtr);
}


FILE * FrogLeapController::getPFile()
{
	return this->pFile;
}

char * FrogLeapController::getTestCaseName()
{
	return this->tspLibEud2DPtr->getName();	
}

char * FrogLeapController::getTestCaseComment()
{
	return this->tspLibEud2DPtr->getComment();
}

int FrogLeapController::getTestCaseDimension()
{
	return this->tspLibEud2DPtr->getDimension();
}

char * FrogLeapController::getTestCaseType()
{
	return this->tspLibEud2DPtr->getType();
}

int FrogLeapController::getTestCaseCapacity()
{
	return this->tspLibEud2DPtr->getCapacity();
}

void FrogLeapController::cw_loadDistanceTable()
{
	int vertCount = 8;
	this->distTablePtr = new DistanceTable(vertCount);

	this->distTablePtr->addEdge(0, 1, 14.2);
	this->distTablePtr->addEdge(0, 2, 11.4);
	this->distTablePtr->addEdge(0, 3, 13.9);
	this->distTablePtr->addEdge(0, 4, 8.0);
	this->distTablePtr->addEdge(0, 5, 14.6);
	this->distTablePtr->addEdge(0, 6, 8.2);
	this->distTablePtr->addEdge(0, 7, 11.2);
	this->distTablePtr->addEdge(1, 2, 12.0);
	this->distTablePtr->addEdge(1, 3, 7.2);
	this->distTablePtr->addEdge(1, 4, 21.0);
	this->distTablePtr->addEdge(1, 5, 8.6);
	this->distTablePtr->addEdge(1, 6, 13.0);
	this->distTablePtr->addEdge(1, 7, 11.0);
	this->distTablePtr->addEdge(2, 3, 17.1);
	this->distTablePtr->addEdge(2, 4, 19.2);
	this->distTablePtr->addEdge(2, 5, 18.4);
	this->distTablePtr->addEdge(2, 6, 17.0);
	this->distTablePtr->addEdge(2, 7, 1.0);
	this->distTablePtr->addEdge(3, 4, 18.4);
	this->distTablePtr->addEdge(3, 5, 1.4);
	this->distTablePtr->addEdge(3, 6, 8.6);
	this->distTablePtr->addEdge(3, 7, 16.2);
	this->distTablePtr->addEdge(4, 5, 18.4);
	this->distTablePtr->addEdge(4, 6, 10.0);
	this->distTablePtr->addEdge(4, 7, 19.1);
	this->distTablePtr->addEdge(5, 6, 8.5);
	this->distTablePtr->addEdge(5, 7, 17.5);
	this->distTablePtr->addEdge(6, 7, 16.4);

	int nVertices = this->distTablePtr->getNumberOfVertices();
	for (int i = 0; i < nVertices; i++)
	{
		this->distTablePtr->addDiagEdge(i, 0.0);
	}
}

void FrogLeapController::cw_loadDistanceTableValues(DistanceTable * dt)
{
	dt->addEdge(0, 1, 14.2);
	dt->addEdge(0, 2, 11.4);
	dt->addEdge(0, 3, 13.9);
	dt->addEdge(0, 4, 8.0);
	dt->addEdge(0, 5, 14.6);
	dt->addEdge(0, 6, 8.2);
	dt->addEdge(0, 7, 11.2);
	dt->addEdge(1, 2, 12.0);
	dt->addEdge(1, 3, 7.2);
	dt->addEdge(1, 4, 21.0);
	dt->addEdge(1, 5, 8.6);
	dt->addEdge(1, 6, 13.0);
	dt->addEdge(1, 7, 11.0);
	dt->addEdge(2, 3, 17.1);
	dt->addEdge(2, 4, 19.2);
	dt->addEdge(2, 5, 18.4);
	dt->addEdge(2, 6, 17.0);
	dt->addEdge(2, 7, 1.0);
	dt->addEdge(3, 4, 18.4);
	dt->addEdge(3, 5, 1.4);
	dt->addEdge(3, 6, 8.6);
	dt->addEdge(3, 7, 16.2);
	dt->addEdge(4, 5, 18.4);
	dt->addEdge(4, 6, 10.0);
	dt->addEdge(4, 7, 19.1);
	dt->addEdge(5, 6, 8.5);
	dt->addEdge(5, 7, 17.5);
	dt->addEdge(6, 7, 16.4);

	int nVertices = dt->getNumberOfVertices();
	for(int i = 0; i < nVertices;i++)
	{
		dt->addDiagEdge(i, 0.0);
	}
}

void FrogLeapController::cw_loadCustomersAndDepots()
{
	// the first node corresponds to the unique depot for testing this example of Clark and Wright algorithm
	this->setAsDepot(0, 1000);
	
	// loading 7 customers.
	this->setAsCustomer(1, 46);
	this->setAsCustomer(2, 30);
	this->setAsCustomer(3, 34);
	this->setAsCustomer(4, 49);
	this->setAsCustomer(5, 18);
	this->setAsCustomer(6, 34);
	this->setAsCustomer(7, 28);

	// create the arrays of customers and depots
	this->setUpCustomerList();
	this->setUpDepotList();
}

void FrogLeapController::cw_loadVehicleCapacity()
{
	this->setUpVehicleCapacity(150);
}

void FrogLeapController::writeSeed()
{
	char buffer[256] = { 0 };

	//sprintf(buffer, "SEED USED: ; %lld \n", this->timeSeedUsed);
	sprintf_s(buffer, sizeof(buffer), "SEED USED: ; %lld \n", this->timeSeedUsed);
	fputs(buffer, this->pFile);
}

void FrogLeapController::writeFrogLeapSolution(FrogLeapSolution * fls)
{ 
	//fputs("FrogLeapSolution \n", this->pFile);
	
	fputs("FLS Customer Index: ;", this->pFile);

	for(int i = 0; i < fls->getSize(); i++)
	{
		fprintf(this->pFile, "%d ; ", i);
	}

	fprintf(this->pFile, "\n");

	fputs("FLS Depot Index: ;", this->pFile);

	for (int i = 0; i < fls->getSize(); i++)
	{
		fprintf(this->pFile, "%.3f ;", fls->getFLValue(i));
	}

	fprintf(this->pFile, "\n");
}



void FrogLeapController::writeIterationInfo(long long int i, float currentValue)
{
	char buffer[300] = { 0 };

	printf("\n\n\n Iteration Number i = %lld ; MinCostValue = %.3f ; CurrentCostValue = %.3f \n\n\n", i, this->getMinCostValue(), currentValue);
	//sprintf(buffer, "Iteration Number i = ; %lld ; MinCostValue = ; %.3f ; CurrentCostValue = ; %.3f \n", i, this->getMinCostValue(), currentValue);
	sprintf_s(buffer, sizeof(buffer), "Iteration Number i = ; %lld ; MinCostValue = ; %.3f ; CurrentCostValue = ; %.3f \n", i, this->getMinCostValue(), currentValue);
	fputs(buffer, this->pFile);	
}

void FrogLeapController::writeRandomInfo(float a, float b, float finalRandom)
{
	char buffer[300] = { 0 };

	printf("Random Info a = %.3f b = %.3f finalRandom = %.3f \n", a, b, finalRandom);
	//sprintf(buffer, "Random Info a = ; %.3f ; b = ; %.3f m = ; %.3f finalRandom = ; %.3f \n", a, b, finalRandom);
	sprintf_s(buffer, sizeof(buffer), "Random Info a = ; %.3f ; b = ; %.3f m = ; %.3f finalRandom = ; %.3f \n", a, b, finalRandom);
	fputs(buffer, this->pFile);
}

void FrogLeapController::writeExecutionInfo()
{
	char buffer[300] = { 0 };

	//sprintf(buffer, "\n \n SHOWING DATA OF FROGLEAPING CONTROLLER \n");
	sprintf_s(buffer, sizeof(buffer), "\n \n SHOWING DATA OF FROGLEAPING CONTROLLER \n");

	buffer[300] = { 0 };
	fputs(buffer, this->pFile);

	if (this->ptrBestSolution != NULL)
	{
		this->ptrBestSolution->printFrogObj();
	}
	else
	{
		//sprintf(buffer,"\n NO FEASIBLE SOLUTION FOUND: ptrBestSolution IS NULL \n");
		sprintf_s(buffer, sizeof(buffer), "\n NO FEASIBLE SOLUTION FOUND: ptrBestSolution IS NULL \n");
		buffer[300] = { 0 };
	}

	//sprintf(buffer, "\n Summary of Best Found Solution \n");
	sprintf_s(buffer, sizeof(buffer), "\n Summary of Best Found Solution \n");
	buffer[300] = { 0 };
	fputs(buffer, this->pFile);
	//sprintf(buffer, "	Time Seed used %lld \n", (long long)this->timeSeedUsed);
	sprintf_s(buffer, sizeof(buffer), "	Time Seed used %lld \n", (long long)this->timeSeedUsed);
	buffer[300] = { 0 };
	fputs(buffer, this->pFile);
	//sprintf(buffer, "	Number of success attempts: %d \n", this->successAttempts);
	sprintf_s(buffer, sizeof(buffer), "	Number of success attempts: %d \n", this->successAttempts);
	buffer[300] = { 0 };
	fputs(buffer, this->pFile);
	sprintf_s(buffer, sizeof(buffer), "	Number of fail attempts: %d \n", this->failAttempts);
	buffer[300] = { 0 };
	fputs(buffer, this->pFile);
	//sprintf(buffer, "	Number of TOTAL Improvements: %d \n", this->getTotalImprovements());
	sprintf_s(buffer, sizeof(buffer),"	Number of TOTAL Improvements: %d \n", this->getTotalImprovements());
	buffer[300] = { 0 };
	fputs(buffer, this->pFile);
	//sprintf(buffer, "	Number of Global Search Improvements: %d \n", this->globalImprovements);
	sprintf_s(buffer, sizeof(buffer), "	Number of Global Search Improvements: %d \n", this->globalImprovements);
	buffer[300] = { 0 };
	fputs(buffer, this->pFile);
	sprintf_s(buffer, sizeof(buffer), "	Number of Local Search Improvements: %d \n", this->localSearchImprovements);
	buffer[300] = { 0 };
	fputs(buffer, this->pFile);
	sprintf_s(buffer, sizeof(buffer), "	Evaluation of best found solution is: %.3f \n \n", this->getMinCostValue());
	buffer[300] = { 0 };
	fputs(buffer, this->pFile);
}


//choose the closest item: available depot or the depot of the closest assigned customer
float FrogLeapController::assignRandomFeasibleDepot4(FrogObjectCol * & localDepotCol, int customerIndex)
{
	float u = -1, result = -1;
	Pair * depotPairSelected = NULL;
	int lowBoundIndex = -1;

	// get the customer demand
	int customerDemand = this->getCustomerDemandByIndex(customerIndex);

	// get the index of the first depot with capacity enough to attend customer demand
	localDepotCol->getFirstHigherValueFrogObjectIndex(customerDemand, lowBoundIndex);

	// if there is not any depot then return -1
	if (lowBoundIndex == -1)
	{
		return result;
	}

	// choose between the available depots with sufficient capacity to attend the customer demand
	int numberOfDepots = this->getNumberOfDepots();

	float distanceToDepot;
	int positionSelected = this->getClosestLocalDepotIndexToCustomer(customerIndex, lowBoundIndex, localDepotCol->getSize(), localDepotCol, distanceToDepot);

	float distanceToDepotCustomer;

	// get the depot index of the closest already assigned customer. Capacity of depot is checked
	int closestCustomerDepotIndex = this->getDepotIndexOfClosestAssignedCustomer(customerIndex, localDepotCol, lowBoundIndex, localDepotCol->getSize(), distanceToDepotCustomer);

	if (closestCustomerDepotIndex != -1)
	{
		if (distanceToDepotCustomer < distanceToDepot)
		{
			depotPairSelected = this->getDepotPairByIndex(closestCustomerDepotIndex);
		}
		else
		{
			// get the depotPair selected in the list of available depots
			depotPairSelected = (Pair *)localDepotCol->getFrogObject(positionSelected);
		}
	}
	else
	{
		// get the depotPair selected in the list of available depots
		depotPairSelected = (Pair *)localDepotCol->getFrogObject(positionSelected);
	}

	//update remaining capacity of depot pair
	int depotRemainingCap = depotPairSelected->get_j_IntValue();
	int newCapacity = depotRemainingCap - customerDemand;
	depotPairSelected->set_j_IntValue(newCapacity);
	depotPairSelected->setValue(newCapacity);

	localDepotCol->reorderFrogObject(depotPairSelected);

	// get the depot index in the controller of the selected depot
	int depotId = depotPairSelected->getId();
	int depotIndex = this->getDepotListIndexByInternal(depotId);

	this->setCustomerPairAsAssigned(customerIndex, depotIndex);

	// assign a random number to the depot selected
	float randnum;

	do
	{
		randnum = this->genRandomFloatingNumber(0, 1);
	} while (randnum >= 1);

	result = depotIndex + randnum;

	if(depotIndex == 6)
	{
		//printf("ojo 2");
	}

	return  result;
}

time_t FrogLeapController::getSeedUsed()
{
	return this->timeSeedUsed;
}

int FrogLeapController::getDemandFromCustomerList(FrogObjectCol* customerList)
{
	int totalDemand = 0, nSize = customerList->getSize();
	
	for(int i = 0; i < nSize; i++)
	{
		Pair* customer = (Pair*)customerList->getFrogObject(i);
		totalDemand += customer->get_i_IntValue();
	}

	return totalDemand;
}

bool FrogLeapController::isCurrentSolutionFeasibleForCustomer(int customerIndex)
{		
	int nDepots = this->getNumberOfDepots();
	int customerDemand = this->getCustomerDemandByIndex(customerIndex);

	bool isCustomerAssigned = this->isCustomerPairAssigned(customerIndex);

	if (isCustomerAssigned == false) 
	{
		for (int i = 0; i < nDepots; i++)
		{
			int remainingCapacity = this->getDepotRemainingCapacityByIndex(i);

			if (remainingCapacity >= customerDemand)
			{
				return true;
			}
		}

		return false;
	}

	return true;
}

bool FrogLeapController::isCurrentSolutionFeasible()
{	
	int nCustomers = this->getNumberOfCustomers();
	for (int i = 0; i < nCustomers; i++)
	{
		bool currentResult = isCurrentSolutionFeasibleForCustomer(i);

		if (currentResult == false) 
		{
			return false;
		}
	}

	return true;
}

// create a feasibleSolution object (an array) with value i in position i
FeasibleSolution * FrogLeapController::initFeasibleSolution(int size) {

	FeasibleSolution * selectionArray = new FeasibleSolution(size);

	for (int i = 0; i < size; i++)
	{
		selectionArray->setSolFactValue(i, i);
	}

	return selectionArray;
}

int FrogLeapController::selectRandomIndex(int i, FeasibleSolution* fs)
{
	// select a random depot between the set of customers
	int size = fs->getSize();
	int randomIndex;
	float u;

	do
	{
		u = this->genRandomFloatingNumber(i, size);
	} while (u >= size);

	int positionSelected = floor(u);

	randomIndex = fs->getSolFactValue(positionSelected);

	fs->swapItems(i, positionSelected);

	return randomIndex;
}

int FrogLeapController::getMinValueIteration()
{
	return this->minValueIteration;
}

void FrogLeapController::setMinValueIteration(int min_i)
{
	this->minValueIteration = min_i;
}


