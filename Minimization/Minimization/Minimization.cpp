#include "stdafx.h"
#include "pch.h"
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <algorithm>

void ReadMili(int countX, int countQ, std::ifstream & input, int countY);
void ReadMur(int countX, int countQ, std::ifstream & input);
void PrintMur(std::vector<int> & classes, std::vector<std::vector<int>> & transitions, std::vector<std::vector<int>> & tableY, int countQ, int countX);
void PrintMili(std::vector<int> & classes, std::vector<std::vector<int>> & transitions, std::vector<std::vector<int>> & tableY, int countQ, int countX, int countY);
std::vector<int> FirstStep(std::vector<std::vector<int>> & table, int countQ);
void Minimization(std::vector<int> & classes, std::vector<std::vector<int>> & transitions, int countX, int countQ);


using namespace std;
void CreateDotFile(vector<vector<int>>& matrix, vector<vector<int>> & matrixY)
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
	for (size_t i = 0; i < matrix.size(); i++)
	{
		for (size_t j = 1; j < matrix[0].size(); j++)
		{
			Edge edge(i, matrix[i][j]);
			edges.push_back(edge);
			weights.push_back(std::to_string(j) + "/" + std::to_string(matrixY[i][j]));
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
	std::ifstream inputFile("in.txt");
	int type, countX, countY, countQ;
	inputFile >> type >> countX >> countY >> countQ;
	if (type == 1) {
		ReadMur(countX, countQ, inputFile);
	}
	else if (type == 2) {
		ReadMili(countX, countQ, inputFile, countY);
	}
	else {
		std::cout << "Err";
		return 1;
	}
	return 0;
}

void ReadMili(const int countX, const int countQ, std::ifstream & input, int countY) {
	struct Cell {
		int q;
		int y;
	};
	Cell cell;
	std::vector<std::vector<int>> table(countQ, std::vector<int>(countX + 1));
	std::vector<std::vector<int>> states(countQ, std::vector<int >(countX + 1));
	for (int x = 1; x <= countX; x++) {
		for (int q = 0; q < countQ; q++) {
			input >> cell.q >> cell.y;
			table[q][x] = cell.y;
			states[q][x] = cell.q;
		}
	}
	std::vector<int> transitions = FirstStep(table, countQ);
	Minimization(transitions, states, countX, countQ);
	PrintMili(transitions, states, table, countQ, countX, countY);
}

void ReadMur(int countX, int countQ, std::ifstream & input) {
	std::vector<std::vector<int>> table(countQ, std::vector<int>(countX + 1));
	std::vector<std::vector<int>> tableS(countQ, std::vector<int>(countX + 1, 0));

	for (int q = 0; q < countQ; q++) {
		input >> tableS[q][1];
	}

	for (int x = 1; x <= countX; x++) {
		for (int q = 0; q < countQ; q++) {
			input >> table[q][x];
		}
	}
	std::vector<int> transitions = FirstStep(tableS, countQ);
	Minimization(transitions, table, countX, countQ);
	PrintMur(transitions, table, tableS, countQ, countX);
}

std::vector<int> FirstStep(std::vector<std::vector<int>> & table, int countQ) {
	std::vector<int> transitions(countQ);
	bool stop = false;
	std::vector<bool> ver(countQ, true);
	int countClasses = -1;
	while (!stop) {
		auto match = std::find(ver.begin(), ver.end(), true);
		if (match != ver.end()) {
			const int index = match - ver.begin();
			ver[index] = false;
			countClasses++;
			transitions[index] = countClasses;
			for (int q = index + 1; q < countQ; q++) {
				if (table[q] == table[index]) {
					ver[q] = false;
					transitions[q] = countClasses;
				}
			}
		}
		else {
			stop = true;
		}
	}
	return transitions;
}

void Minimization(std::vector<int> & classes, std::vector<std::vector<int>> & transitions, int countX, int countQ) {
	std::vector<std::vector<int>> min(countQ, std::vector<int>(countX + 1));
	for (int x = 1; x <= countX; x++) {
		for (int q = 0; q < countQ; q++) {
			min[q][x] = classes[transitions[q][x]];
		}
	}

	bool changes = true;
	while (changes) {
		std::vector<bool> ver(countQ, true);
		std::vector<int> newClasses(countQ, -1);
		bool stop = false;
		int countClasses = -1;
		while (!stop) {
			auto match = std::find(ver.begin(), ver.end(), true);
			std::vector<std::vector<int>> tmp(countQ, std::vector<int>(countX + 1, -1));
			int k = 0;
			if (match != ver.end()) {
				const int index = match - ver.begin();
				ver[index] = false;
				countClasses++;
				newClasses[index] = countClasses;
				for (int q = index + 1; q < countQ; q++) {
					if (min[index] == min[q] && classes[q] == classes[index]) {
						ver[q] = false;
						newClasses[q] = countClasses;
					}
					else if (min[index] != min[q] && classes[q] == classes[index]) {
						//ищем в tmp
						bool found = false;
						for (int i = 0; i < tmp.size(); i++) {
							if (tmp[i] == min[q]) {
								newClasses[q] = i;
								found = true;
							}
						}
						if (!found) {
							k++;
							newClasses[q] = *std::max_element(classes.begin(), classes.end()) + k;
							tmp[newClasses[q]] = min[q];
						}
						ver[q] = false;
					}
				}
			}
			else {
				stop = true;
			}
		}
		for (int x = 1; x <= countX; x++) {
			for (int q = 0; q < countQ; q++) {
				min[q][x] = newClasses[transitions[q][x]];
			}
		}
		if (classes == newClasses) {
			changes = false;
		}
		else {
			classes = newClasses;
		}
	}
}

void PrintMur(std::vector<int> & classes, std::vector<std::vector<int>> & transitions, std::vector<std::vector<int>> & tableY, int countQ, int countX) {
	int countClasses = std::distance(classes.begin(), std::max_element(classes.begin(), classes.end()));
	std::vector<std::vector<int>> result(classes[countClasses] + 1, std::vector<int>(countX + 1));
	std::vector<bool> wasChecked(classes[countClasses] + 1, false);
	std::vector<std::vector<int>> resultY(classes[countClasses] + 1, std::vector<int>(countX + 1));
	std::ofstream out("out.txt");

	for (int q = 0; q < countQ; q++) {
		if (wasChecked[classes[q]] == false) {
			wasChecked[classes[q]] = true;
			for (int x = 1; x <= countX; x++) {
				result[classes[q]][x] = classes[transitions[q][x]];
				resultY[classes[q]][x] = tableY[q][1];
			}
		}
	}

	out << "1\n" << countX << "\n" << resultY.size() << "\n" << result.size() << "\n";
	for (int y = 0; y < resultY.size(); y++) {
		out << resultY[y][1] << " ";
	}
	out << "\n";
	for (int x = 1; x <= countX; x++) {
		for (int q = 0; q < result.size(); q++) {
			out << result[q][x] << " ";
		}
		out << "\n";
	}

	CreateDotFile(result, resultY);
}

void PrintMili(std::vector<int> & classes, std::vector<std::vector<int>> & transitions, std::vector<std::vector<int>> & tableY, int countQ, int countX, int countY) {
	int countClasses = std::distance(classes.begin(), std::max_element(classes.begin(), classes.end()));
	std::vector<std::vector<int>> result(classes[countClasses] + 1, std::vector<int>(countX + 1));
	std::vector<std::vector<int>> resultY(classes[countClasses] + 1, std::vector<int>(countX + 1));
	std::ofstream out("out.txt");
	std::vector<bool> wasChecked(classes[countClasses] + 1, false);
	for (int q = 0; q < countQ; q++) {
		if (wasChecked[classes[q]] == false) {
			wasChecked[classes[q]] = true;
			for (int x = 1; x <= countX; x++) {
				result[classes[q]][x] = classes[transitions[q][x]];
				resultY[classes[q]][x] = tableY[q][x];
			}
		}
	}

	out << "2\n" << countX << "\n" << countY << "\n" << result.size() << "\n";
	for (int x = 1; x <= countX; x++) {
		for (int q = 0; q < result.size(); q++) {
			out << result[q][x] << " " << resultY[q][x] << " ";
		}
		out << "\n";
	}

	CreateDotFile(result, resultY);
}
