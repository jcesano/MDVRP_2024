#include "stdafx.h"
#include "ClarkWrightHandler.h"

ClarkWrightHandler::ClarkWrightHandler(FrogLeapController* controller, Cluster * v_cluster)
{			
	this->distanceTable_ptr = controller->getDistanceTable();	
	this->cluster = v_cluster;
	this->vehicleCapacity = controller->getVehicleCapacity();
	this->savingsList = new FrogObjectCol();
}

ClarkWrightHandler::~ClarkWrightHandler()
{
	deleteFinalRoutes();
	this->distanceTable_ptr = NULL;
	this->cluster = NULL;
	delete this->savingsList;
}

void ClarkWrightHandler::create_Routes()
{
	int n_Customers = this->cluster->getCustomerCol()->getSize();

	this->routes = new FrogObjectCol * [n_Customers];

	for(int i = 0; i < n_Customers; i++)
	{
		this->routes[i] = create_Route(i); // create an initial route with (depot, customer_i, depot)
	}
}

void ClarkWrightHandler::create_Null_Routes()
{
	int n_Customers = this->cluster->getCustomerCol()->getSize();

	this->routes = new FrogObjectCol * [n_Customers];

	for (int i = 0; i < n_Customers; i++)
	{
		this->routes[i] = NULL;//	create_Route(i); // create an initial route with (depot, customer_i, depot)
	}

	//this->printRoutesStats();
}

void ClarkWrightHandler::printRoutesStats()
{
	printf("Mostrando estadisticas de routas asociadas al Deposito: %d \n", this->cluster->getDepotPair()->getLabelId());

	int null_routes = this->getNumberOfNullRoutes();
	int not_null_routes = this->getNumberOfNotNullRoutes();

	printf("Cantidad de rutas nulas: %d \n", null_routes);
	printf("Cantidad de rutas NO nulas: %d \n", not_null_routes);
	printf("Cantidad de rutas total: %d \n", null_routes + not_null_routes);
}

int ClarkWrightHandler::getNumberOfNullRoutes()
{
	int customer_col_size = this->cluster->getCustomerCol()->getSize();
	// The initial size of routes is the same as the number of customers of the cluster. One route per customer
	int null_routes = 0;

	for (int i = 0; i < customer_col_size; i++)
	{
		if(this->routes[i] == NULL)
		{
			null_routes++;
		}
	}

	return null_routes;
}

int ClarkWrightHandler::getNumberOfNotNullRoutes() 
{
	int customer_col_size = this->cluster->getCustomerCol()->getSize();
	// The initial size of routes is the same as the number of customers of the cluster. One route per customer
	int not_null_routes = 0;

	for (int i = 0; i < customer_col_size; i++)
	{
		if (this->routes[i] != NULL)
		{
			not_null_routes++;
		}
	}

	return not_null_routes;
}

FrogObjectCol * ClarkWrightHandler:: create_Route(int i)
{
	FrogObjectCol * result = new FrogObjectCol();

	result->addLastFrogObject(this->cluster->getDepotPair());
	result->addLastFrogObject(this->cluster->getCustomerCol()->getFrogObject(i));
	result->addLastFrogObject(this->cluster->getDepotPair());

	return result;
}

void ClarkWrightHandler::deleteFinalRoutes()
{
	int nFinalRoutes = this->final_routes_size;

	for(int i = 0; i < nFinalRoutes; i++)
	{
		this->final_routes[i]->unReferenceFrogObjectCol();
		delete this->final_routes[i];
	}
}
void ClarkWrightHandler::select_final_routes()
{
	int n_Customers = this->cluster->getCustomerCol()->getSize();	
	this->final_routes = new FrogObjectCol * [n_Customers];
	int final_index = 0;

	for(int i = 0; i < n_Customers; i++)
	{
		if(this->routes[i] != NULL)
		{
			this->final_routes[final_index] = this->routes[i];
			final_index++;
			this->routes[i] = NULL;
		}
	}

	this->final_routes_size = final_index;
}

void ClarkWrightHandler::create_SavingsList()
{
	Pair* pair_ptr = NULL, * currentCustomer_i = NULL, * currentCustomer_j = NULL;

	// testing code
	int depotLabelId = this->cluster->getDepotPair()->getLabelId();

	if(depotLabelId == 255)
	{
		//printf("PARAR ACA \n");
	}
	// end of testing code

	int n = this->cluster->getCustomerCol()->getSize();
	int aux = 0;
	// create Savings
	for (int i = 0; i < n; i++)
	{
		for (int j = i + 1; j < n; j++)
		{
			currentCustomer_i = (Pair*)this->cluster->getCustomerCol()->getFrogObject(i);
			currentCustomer_j = (Pair*)this->cluster->getCustomerCol()->getFrogObject(j);
			pair_ptr = new Pair(PairType::IntVsInt, aux);
			aux++;
			pair_ptr->set_i_IntValue(currentCustomer_i->getId());
			pair_ptr->set_j_IntValue(currentCustomer_j->getId());

			// calculate savings = distance(depot, customer_i) + distance(depot, customer_j) - distance(customer_i, customer_j)
			float distance_Customer_i_Customer_j = this->distanceTable_ptr->getEdge(currentCustomer_i->getId(), currentCustomer_j->getId());
			float distance_Depot_Customer_i = this->distanceTable_ptr->getEdge(this->cluster->getDepotPair()->getId(), currentCustomer_i->getId());
			float distance_Depot_Customer_j = this->distanceTable_ptr->getEdge(this->cluster->getDepotPair()->getId(), currentCustomer_j->getId());
			float save_Distance = distance_Depot_Customer_i + distance_Depot_Customer_j - distance_Customer_i_Customer_j;
			
			pair_ptr->set_i_FloatValue(save_Distance);
			pair_ptr->setValue(save_Distance);

			savingsList->addFrogObjectDescOrdered(pair_ptr);
		}
	}

	//savingsList->printFrogObjCol();
}

void ClarkWrightHandler::merge_Routes()
{
	int savingsListSize = this->savingsList->getSize();
	LocationType result_i, result_j, testing_result;
	int route_demand_i, route_demand_j, node_demand;

	int routeIndex, internalId_node_to_add;
	LocationType position_to_add;

	for(int i = 0; i < savingsListSize;i++)
	{
		Pair* current_Saving = (Pair *)this->savingsList->getFrogObject(i);
		int route_index_i = findInRoutes(current_Saving->get_i_IntValue(), result_i);
		int route_index_j = findInRoutes(current_Saving->get_j_IntValue(), result_j);

		int routeWithDepotInside = findDepotInRoutes(testing_result);

		//if(i == 44)
		//{
		//	printf("PARA ACA \n");
		//	printf("mostrando current_saving: i = %d, j = %d  \n\n", current_Saving->get_i_IntValue(), current_Saving->get_j_IntValue());
		//}
		
		//if(testing_result == LocationType::At_Middle)
		//{
		//	printf("PARAR ACA \n");
		//}
		
		if(internalIdsExistInDifferentRoutes(route_index_i, route_index_j,result_i, result_j) == true)
		{
			route_demand_i = this->getRouteDemand(this->routes[route_index_i]);
			route_demand_j = this->getRouteDemand(this->routes[route_index_j]);
			if(route_demand_i + route_demand_j <= this->vehicleCapacity)
			{
				merge_diff_routes(route_index_i, route_index_j, result_i, result_j, current_Saving->get_i_IntValue(), current_Saving->get_j_IntValue());
			}
			
			continue;
		}

		if(internalIdsExistInSameRoute(route_index_i, route_index_j) == true)
		{
			continue;
		}

		if(internalIdsDoNotExist(result_i, result_j) == true)
		{
			// creates a route and adds it only if the route demand <= vehicle_capacity
			add_New_Route(route_index_i, route_index_j, result_i, result_j, current_Saving->get_i_IntValue(), current_Saving->get_j_IntValue());
			continue;
		}

		if(onlyInternalId_i_ExistsAtBegin(result_i, result_j) == true)
		{
			//add_Node_j_AtBegin_into_route_i
			internalId_node_to_add = current_Saving->get_j_IntValue();			
			position_to_add = LocationType::At_Begin;
			routeIndex = route_index_i;

			add_Node_AtPosition_into_route(routeIndex, position_to_add, internalId_node_to_add);
			continue;
		}

		if (onlyInternalId_i_ExistsAtEnd(result_i, result_j) == true)
		{
			// add_Node_j_AtLast_into_route_i
			internalId_node_to_add = current_Saving->get_j_IntValue();
			position_to_add = LocationType::At_Last;
			routeIndex = route_index_i;

			add_Node_AtPosition_into_route(routeIndex, position_to_add, internalId_node_to_add);
			continue;
		}

		if (onlyInternalId_j_ExistsAtBegin(result_i, result_j) == true)
		{
			// add_Node_i_AtBegin_into_route_j
			internalId_node_to_add = current_Saving->get_i_IntValue();
			position_to_add = LocationType::At_Begin;
			routeIndex = route_index_j;

			add_Node_AtPosition_into_route(routeIndex, position_to_add, internalId_node_to_add);
			continue;
		}

		if (onlyInternalId_j_ExistsAtEnd(result_i, result_j) == true)
		{
			// add_Node_i_AtBegin_into_route_j
			internalId_node_to_add = current_Saving->get_i_IntValue();
			position_to_add = LocationType::At_Last;
			routeIndex = route_index_j;

			add_Node_AtPosition_into_route(routeIndex, position_to_add, internalId_node_to_add);
			continue;
		}
	}
}

void ClarkWrightHandler::add_Node_AtPosition_into_route(int routeIndex, LocationType position_to_add, int internalId_node_to_add)
{
	//Note:Add the node in nodeIndex in position = LocationType (Begin/End) into the route with routeIndex
	// Always check if routes do not exceed the vehicle capacity
	
	// getting the reference of the node to be added in the route with routeIndex
	Pair * node_to_add = (Pair*)this->cluster->getCustomerCol()->getFrogObjectById(internalId_node_to_add);

	int node_demand = node_to_add->get_i_IntValue();

	int demand = node_demand + this->getRouteDemand(this->routes[routeIndex]);
	
	if(demand <= this->vehicleCapacity)
	{
		// get the reference to the depot that is at the begining and at the end of any route
		Pair* depot = (Pair*)this->routes[routeIndex]->getFrogObject(0);

		// this function removes the first and the last objects of the list. Recall that depot appears at the begining and at the end	
		this->routes[routeIndex]->removeFrogObject(depot);

		if (position_to_add == LocationType::At_Begin)
		{
			this->routes[routeIndex]->addLastFrogObject(depot); // restore depot in the last position

			// adding node_j to the first position of the route before node_i that is at the begining (is adyascent to the depot)
			this->routes[routeIndex]->addFrogObject(node_to_add);

			// restore the depot reference at the begining of the route
			this->routes[routeIndex]->addFrogObject(depot);
			return;
		}

		if (position_to_add == LocationType::At_Last)
		{
			this->routes[routeIndex]->addFrogObject(depot); // restore depot at the begining position

			// adding node_j to the first position of the route before node_i that is at the begining (is adyascent to the depot)
			this->routes[routeIndex]->addLastFrogObject(node_to_add);

			// restore the depot reference at the begining of the route
			this->routes[routeIndex]->addLastFrogObject(depot);
			return;
		}
	}

	return;
}

bool ClarkWrightHandler::onlyInternalId_i_ExistsAtBegin(LocationType result_i, LocationType result_j)
{
	return (result_i == LocationType::At_Begin && result_j != LocationType::At_Begin && result_j != LocationType::At_Last && result_j != LocationType::At_Middle);
}

bool ClarkWrightHandler::onlyInternalId_j_ExistsAtBegin(LocationType result_i, LocationType result_j)
{
	return (result_j == LocationType::At_Begin && result_i != LocationType::At_Begin && result_i != LocationType::At_Last && result_i != LocationType::At_Middle);
}

bool ClarkWrightHandler::onlyInternalId_i_ExistsAtEnd(LocationType result_i, LocationType result_j)
{
	return (result_i == LocationType::At_Last && result_j != LocationType::At_Begin && result_j != LocationType::At_Last && result_j != LocationType::At_Middle);
}

bool ClarkWrightHandler::onlyInternalId_j_ExistsAtEnd(LocationType result_i, LocationType result_j)
{
	return (result_j == LocationType::At_Last && result_i != LocationType::At_Begin && result_i != LocationType::At_Last && result_i != LocationType::At_Middle);
}

bool ClarkWrightHandler::internalIdsDoNotExist(LocationType result_i, LocationType result_j)
{
	return (result_i == LocationType::No_Exists && result_j == LocationType::No_Exists);
}

void ClarkWrightHandler::merge_diff_routes(int routeIndex_i, int routeIndex_j, LocationType result_i, LocationType result_j, int internalId_route_i, int internalId_route_j)
{
	//int route_i_loc = routeItem_LocationInRoute(routeIndex_i, internalId_route_i); // -1 = no exists, position in list otherwise
	//int route_j_loc = routeItem_LocationInRoute(routeIndex_j, internalId_route_j); // -1 = no exists, position in list otherwise

	if(result_i == LocationType::At_Begin && result_j == LocationType::At_Begin)
	{
		//printf("mostrando ruta i \n");
		//this->routes[routeIndex_i]->printFrogObjCol();

		//printf("mostrando ruta j \n");
		//this->routes[routeIndex_j]->printFrogObjCol();

		// reverse route to concatenate routes with adjascent nodes.
		this->routes[routeIndex_i]->reverse();
		//this->routes[routeIndex_i]->printFrogObjCol();
		
		// remove last item of list because corresponds to the depot of the cluster
		this->routes[routeIndex_i]->removeLastItem();
		//this->routes[routeIndex_i]->printFrogObjCol();

		// remove first item of list because corresponds to the depot of the cluster
		//this->routes[routeIndex_j]->printFrogObjCol();
		this->routes[routeIndex_j]->removeFirstItem();
		//this->routes[routeIndex_j]->printFrogObjCol();

		//concatenate routes
		this->routes[routeIndex_i]->addLastAllFrogObjects(this->routes[routeIndex_j]);
		//this->routes[routeIndex_i]->printFrogObjCol();

		// erase route_j
		this->routes[routeIndex_j]->unReferenceFrogObjectCol();
		delete this->routes[routeIndex_j];
		this->routes[routeIndex_j] = NULL;
		
		return;
	}

	if (result_i == LocationType::At_Begin && result_j == LocationType::At_Last)
	{
		this->routes[routeIndex_i]->removeFirstItem();
		this->routes[routeIndex_j]->removeLastItem();
		
		this->routes[routeIndex_j]->addLastAllFrogObjects(this->routes[routeIndex_i]);

		// erase route_i
		this->routes[routeIndex_i]->unReferenceFrogObjectCol();
		delete this->routes[routeIndex_i];
		this->routes[routeIndex_i] = NULL;
		
		return;
	}

	if (result_i == LocationType::At_Last && result_j == LocationType::At_Begin)
	{
		// remove last item from result_i
		this->routes[routeIndex_i]->removeLastItem();

		// remove first item from result_j
		this->routes[routeIndex_j]->removeFirstItem();

		// add items to route_i
		this->routes[routeIndex_i]->addLastAllFrogObjects(this->routes[routeIndex_j]);

		// erase route_j
		this->routes[routeIndex_j]->unReferenceFrogObjectCol();
		delete this->routes[routeIndex_j];
		this->routes[routeIndex_j] = NULL;

		return;
	}

	if (result_i == LocationType::At_Last && result_j == LocationType::At_Last)
	{
		// remove last item from result_i
		this->routes[routeIndex_i]->removeLastItem();

		// remove last item from route_j
		this->routes[routeIndex_j]->removeLastItem();

		// reverse route_j
		this->routes[routeIndex_j]->reverse();

		// add items to route_i
		this->routes[routeIndex_i]->addLastAllFrogObjects(this->routes[routeIndex_j]);
		
		// erase route_j
		this->routes[routeIndex_j]->unReferenceFrogObjectCol();
		delete this->routes[routeIndex_j];
		this->routes[routeIndex_j] = NULL;

		return;
	}
}

int ClarkWrightHandler::routeItem_LocationInRoute(int route_index, int node_InternalId)
{
	FrogObjectCol* route = this->routes[route_index];
	int n_size = this->routes[route_index]->getSize();
	int result = -1;

	for(int i = 0; i < n_size; i++)
	{
		if(node_InternalId == this->routes[route_index]->getFrogObject(i)->getId())
		{
			result = i;
		}
	}

	return result;
}

bool ClarkWrightHandler::internalIdsExistInDifferentRoutes(int index_route_i, int index_route_j, LocationType result_i, LocationType result_j)
{
	bool index_result = index_route_i != index_route_j && index_route_i != -1 && index_route_j != -1;
	bool location_result_i = result_i != LocationType::No_Exists && result_i != LocationType::At_Middle;
	bool location_result_j = result_j != LocationType::No_Exists && result_j != LocationType::At_Middle;

	return index_result && location_result_i && location_result_j;
}

bool ClarkWrightHandler::internalIdsExistInSameRoute(int index_route_i, int index_route_j)
{
	return (index_route_i != -1 && index_route_j != -1 && index_route_i == index_route_j);
}

bool ClarkWrightHandler::noIndexExists(int index_route_i, int index_route_j)
{
	return ((index_route_i == -1) && (index_route_j == -1));
}

int ClarkWrightHandler::findDepotInRoutes(LocationType & result_i)
{
	int nCustomers = this->cluster->getCustomerCol()->getSize();
	int routes_index = -1;
	result_i = LocationType::No_Exists;
	int i = 0;
	while (i < nCustomers && result_i == LocationType::No_Exists)
	{
		result_i = findDepotInRoute(this->routes[i]);
		if (result_i != LocationType::No_Exists)
		{
			routes_index = i;
		}

		i++;
	}

	return routes_index;

}

// returns the index of the first route which includes the customerIndex
int ClarkWrightHandler::findInRoutes(int customerClusterInternalId, LocationType & result_i)
{
	int nCustomers = this->cluster->getCustomerCol()->getSize();
	int routes_index = -1;
	result_i = LocationType::No_Exists;
	int i = 0;
	while (i < nCustomers && result_i == LocationType::No_Exists)
	{		
		result_i = findInRoute(this->routes[i], customerClusterInternalId);
		if(result_i != LocationType::No_Exists)
		{
			routes_index = i;
		}

		i++;
	}

	return routes_index;
}

LocationType ClarkWrightHandler::findDepotInRoute(FrogObjectCol* route)
{	
	int int_result = -1;
	LocationType result = LocationType::No_Exists;
	int i = 0;
	int depotInternalId = this->cluster->getDepotPair()->getId();

	if (route != NULL)
	{
		int routeSize = route->getSize();

		while (i < routeSize && int_result == -1)
		{
			Pair* route_item = (Pair*)route->getFrogObject(i);			
			
			if (route_item->getId() == depotInternalId)
			{
				int_result = i;
			}

			i++;
		}

		if (int_result == -1 || int_result == 0 || int_result == (routeSize - 1))
		{
			result = LocationType::No_Exists;
			return result;
		};

		if (int_result == 1)
		{
			result = LocationType::At_Begin;
			return result;
		}

		if (int_result == routeSize - 2)
		{
			result = LocationType::At_Last;
			return result;
		}

		if (int_result >= 1 && int_result <= routeSize - 2)
		{
			result = LocationType::At_Middle;
			return result;
		}
	}

	return result;

}

LocationType ClarkWrightHandler::findInRoute(FrogObjectCol * route, int customerInternalId)
{
	int int_result = -1;
	LocationType result = LocationType::No_Exists;	
	int i = 0;

	if(route != NULL)
	{
		int routeSize = route->getSize();

		while (i < routeSize && int_result == -1)
		{
			Pair* route_item = (Pair*)route->getFrogObject(i);
			if (route_item->getId() == customerInternalId)
			{
				int_result = i;
			}

			i++;
		}

		if (int_result == -1 || int_result == 0 || int_result == (routeSize - 1))
		{
			result = LocationType::No_Exists;
			return result;
		};

		if (int_result == 1)
		{
			result = LocationType::At_Begin;
			return result;
		}

		if (int_result == routeSize - 2)
		{
			result = LocationType::At_Last;
			return result;
		}

		if (int_result >= 1 && int_result <= routeSize - 2)
		{
			result = LocationType::At_Middle;
			return result;
		}
	}

	return result;
}

void ClarkWrightHandler::add_New_Route(int routeIndex_i, int routeIndex_j, LocationType result_i, LocationType result_j, int internalId_route_i, int internalId_route_j) 
{	
	FrogObjectCol * new_Route = NULL;
	Pair * depotPair = NULL;
	Pair* customer_i = (Pair *)this->cluster->getCustomerCol()->getFrogObjectById(internalId_route_i);
	Pair* customer_j = (Pair*)this->cluster->getCustomerCol()->getFrogObjectById(internalId_route_j);

	// calculate the demand of this new route (customer_i_demand + customer_j_demand) and check if it is <= vechicle_capacity
	if(customer_i->get_i_IntValue() + customer_j->get_i_IntValue() <= this->vehicleCapacity)
	{		
		new_Route = new FrogObjectCol();
		depotPair = this->cluster->getDepotPair();
		new_Route->addLastFrogObject(depotPair);
		new_Route->addLastFrogObject(customer_i);
		new_Route->addLastFrogObject(customer_j);
		new_Route->addLastFrogObject(depotPair);

		int empty_index = findEmptyRouteIndex();
		if (empty_index != -1)
		{
			this->routes[empty_index] = new_Route;
		}			
		return;
	}

	return;
}

int ClarkWrightHandler::findEmptyRouteIndex()
{
	int nSize = this->cluster->getCustomerCol()->getSize();

	for(int i = 0; i < nSize; i++)
	{
		if(this->routes[i] == NULL)
		{
			return i;
		}
	}

	return -1;
}

int ClarkWrightHandler:: getRouteDemand(FrogObjectCol* route)
{	
	int nSize = route->getSize();
	int route_demand = 0;

	// we exclude the first and the last items because they are references to the depot of the route. 
	// every route starts and ends with the depot of the corresponding cluster
	for(int i = 1; i < nSize-1; i++)
	{
		Pair * current_pair = (Pair *)route->getFrogObject(i);		
		
		// get the demand of customer in the route
		route_demand += current_pair->get_i_IntValue();
	}

	return route_demand;
}

float ClarkWrightHandler::eval_final_route(int i)
{
	float result = 0;

	int route_size = this->final_routes[i]->getSize();

	Pair* prevPair = (Pair *)this->final_routes[i]->getFrogObject(0);
	Pair* currentPair = NULL;

	for(int k=1; k<route_size;k++)
	{
		currentPair = (Pair*)this->final_routes[i]->getFrogObject(k);
		float current_distance = this->distanceTable_ptr->getEdge(prevPair->getId(), currentPair->getId());
		result = result + current_distance;

		prevPair = currentPair;
	}

	return result;
}

void ClarkWrightHandler::print_final_route(int i) 
{
	int customerLabelId; 
	float routeCost = eval_final_route(i), currentEdge = 0;
	Pair * currentPair = NULL, * prevPair = NULL;

	printf("Cost of route %d is %.2f \n", i, routeCost);
	int route_size = this->final_routes[i]->getSize();

	printf("NodeLabelId; Cost\n");
	prevPair = (Pair*)this->final_routes[i]->getFrogObject(0);
	printf("%d; 0\n", prevPair->getLabelId());

	for(int k =1; k<route_size; k++)
	{
		currentPair = (Pair*)this->final_routes[i]->getFrogObject(k);

		float currentEdge = this->distanceTable_ptr->getEdge(prevPair->getId(), currentPair->getId());
		printf("%d; %.2f  \n", currentPair->getLabelId(), currentEdge);

		prevPair = currentPair;
	}
}

void ClarkWrightHandler::print_final_routes()
{
	printf("Showing final routes \n");
	int size = this->final_routes_size;

	for(int i=0; i<size; i++)
	{
		print_final_route(i);
	}
	printf("FINISH OF Showing final routes \n");
}

void ClarkWrightHandler::cw_execute()
{
	create_SavingsList();
	create_Routes();
	//create_Null_Routes();
	merge_Routes();
	select_final_routes();
	//print_final_routes();
}


FrogObjectCol* ClarkWrightHandler::getRoute(int route_index)
{
	if(route_index >= 0 && route_index < this->final_routes_size)
	{
		return this->final_routes[route_index];
	}
	return NULL;
}

int ClarkWrightHandler::getFinalRoutesSize()
{
	return this->final_routes_size;
}

// abstract methods
void ClarkWrightHandler::printFrogObj()
{
}

bool ClarkWrightHandler::isTheSame(FrogObject* fs)
{
	return (this == fs);
}