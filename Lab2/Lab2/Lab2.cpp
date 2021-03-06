// Lab2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "pch.h"
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <queue>

std::vector<int> Split(const std::string & str);
void Read(std::vector<std::vector<std::string>> & table, std::ifstream & inputFile);
std::vector<std::vector<int>> MakeNew(std::vector<std::vector<std::string>> & table, int signalCount);
std::vector<std::vector<int>> StringVectorToInt(std::vector<std::vector<std::string>> & strVector, std::vector<std::string> & matchStates);
std::string SortString(std::string & str);
std::string makeState(std::string & first, std::string & second);

void CreateDotFile(std::vector<std::vector<int>>& matrix)
{
	using Edge = std::pair<int, int>;
	using Graph = boost::adjacency_list<boost::vecS,
		boost::vecS, boost::directedS,
		boost::property<boost::vertex_color_t,
		boost::default_color_type>,
		boost::property<boost::edge_weight_t, std::string>>;

	std::vector<Edge> edges;
	std::vector<std::string> weights;
	const int VERTEX_COUNT = matrix.size();
	for (size_t i = 0; i < matrix.size(); ++i)
	{
		for (size_t j = 0; j < matrix[0].size(); ++j)
		{
			if (matrix[i][j] != -1) {
				Edge edge(i, matrix[i][j]);
				edges.push_back(edge);
				weights.push_back(std::to_string(j));
			}
		}
	}

	Graph graph(edges.begin(), edges.end(), weights.begin(),
		VERTEX_COUNT);

	boost::dynamic_properties dp;
	dp.property("weight", boost::get(boost::edge_weight, graph));
	dp.property("label", boost::get(boost::edge_weight, graph));
	dp.property("node_id", boost::get(boost::vertex_index, graph));
	std::ofstream ofs("test.dot");
	boost::write_graphviz_dp(ofs, graph, dp);
}

int main()
{
	std::ifstream inputFile("in4.txt");
	int signalCount, stateCount, finalStateCount;
	inputFile >> signalCount >> stateCount >> finalStateCount;
	std::vector<int> finalStates(finalStateCount);
	for (int i = 0; i < finalStateCount; i++) {
		inputFile >> finalStates[i];
	}
	std::vector<std::vector<std::string>> table(stateCount, std::vector<std::string>(signalCount, "-1"));
	Read(table, inputFile);
	std::vector<std::vector<int>> result = MakeNew(table, signalCount);
	CreateDotFile(result);
	return 0;
}

void Read(std::vector<std::vector<std::string>> & table, std::ifstream & inputFile) {
	std::string str;
	int strNum = 0, signal, state;
	std::getline(inputFile, str);
	while (std::getline(inputFile, str))
	{
		std::stringstream ss(str);
		while (!ss.eof() && !str.empty()) {
			ss >> state >> signal;
			if (table[strNum][signal] == "-1") {
				table[strNum][signal] = std::to_string(state) + " ";
			}
			else {
				table[strNum][signal] += std::to_string(state) + " ";
			}
		}
		strNum++;
	}
	for (int i = 0; i < table.size(); i++) {
		for (int j = 0; j < table[0].size(); j++) {
			table[i][j] = SortString(table[i][j]);
		}
	}
}

std::string SortString(std::string & str) {
	std::vector<int> states = Split(str);
	std::sort(states.begin(), states.end());
	std::string result = "";
	for (int i = 0; i < states.size(); i++) {
		if (states[i] != -1) {
			result += std::to_string(states[i]) + " ";
		}
		else {
			result = "-1";
		}
	}
	return result;
}

std::vector<std::vector<int>> MakeNew(std::vector<std::vector<std::string>> & table, int signalCount) {
	std::queue<std::string> queue;
	std::vector<std::string> matchStates;
	std::vector<std::string> existingStates;
	std::vector<std::vector<std::string>> newTable;
	std::vector<std::string> newTableLine(signalCount);

	newTable.push_back(table[0]);
	existingStates.push_back("0 ");
	matchStates.push_back("0 ");
	for (int i = 0; i < signalCount; i++) {
		if (table[0][i] != "-1") {
			queue.push(table[0][i]);
		}
	}

	std::vector<int> states;
	while (!queue.empty()) {
		const std::string item = queue.front();
		queue.pop();
		auto match = std::find(existingStates.begin(), existingStates.end(), item);
		if (match == existingStates.end()) {
			existingStates.push_back(item);
			states = Split(item);
			matchStates.push_back(item);
			for (int i = 0; i < signalCount; i++) {
				std::string resultState = "";
				for (int j = 0; j < states.size(); j++) {
					if (table[states[j]][i] != "-1") {
						resultState = makeState(table[states[j]][i], resultState);
					}
				}
				if (resultState == "") {
					newTableLine[i] = "-1";
				}
				else {
					resultState = SortString(resultState);
					newTableLine[i] = resultState;
					queue.push(resultState);
				}
			}
			newTable.push_back(newTableLine);
		}
	}
	
	return StringVectorToInt(newTable, matchStates);
}

std::string makeState(std::string & first, std::string & second) {
	std::vector<int> firstVector = Split(first);
	std::vector<int> secondVector = Split(second);
	firstVector.insert(firstVector.end(), secondVector.begin(), secondVector.end());
	sort(firstVector.begin(), firstVector.end());
	firstVector.resize(std::unique(firstVector.begin(), firstVector.end()) - firstVector.begin());
	std::string state = "";
	for (int i = 0; i < firstVector.size(); i++) {
		state += std::to_string(firstVector[i]) + " ";
	}
	return state;
}

std::vector<std::vector<int>> StringVectorToInt(std::vector<std::vector<std::string>> & strVector, std::vector<std::string> & matchStates) {
	std::vector<std::vector<int>> result(strVector.size(), std::vector<int>(strVector[0].size()));;

	for (int i = 0; i < strVector.size(); i++) {
		for (int j = 0; j < strVector[0].size(); j++) {
			if (strVector[i][j] != "-1") {
				auto newState = find(matchStates.begin(), matchStates.end(), strVector[i][j]);
				result[i][j] = (newState - matchStates.begin());
			}
			else {
				result[i][j] = -1;
			}

		}
	}

	return result;
}

std::vector<int> Split(const std::string & str) {
	std::istringstream iss(str);
	std::vector<std::string> result;
	std::vector<int> resultInt;
	std::copy(std::istream_iterator<std::string>(iss),
		std::istream_iterator<std::string>(),
		std::back_inserter<std::vector<std::string> >(result));
	for (int i = 0; i < result.size(); i++) {
		resultInt.push_back(stoi(result[i]));
	}

	return resultInt;
}


