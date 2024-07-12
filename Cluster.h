#ifndef CLUSTER_H_   /* Include guard */
#define CLUSTER_H_

//=================================
// included dependencies
#include "FrogObject.h"

class FrogObject;
class FrogObjectCol;
class Pair;

class Cluster : public FrogObject
{

	int assignedDepotIndex;

	FrogObjectCol * customerPairsCol;

public:

	Cluster(Pair * depotPair, int id);

	Cluster(int pairId);

	virtual ~Cluster();

	PairType getType();

	FrogObjectCol * getCustomerPairCol();

	void setCustomerPairCol(FrogObjectCol * v_pairCol);	

	Pair * createCopy();

	// abstract methods
	void printFrogObj();

	bool isTheSame(FrogObject * fs);
};
#endif
#pragma once
