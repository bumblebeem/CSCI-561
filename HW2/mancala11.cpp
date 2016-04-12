#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>
#include <string>
#include <queue>
#include <climits>
#include <string.h>
using namespace std;

int num_of_pits;
int cut_off_depth;
int max_player;

struct GameInfo {
	int task;
	int player;
	int cut_off_dep;
	int state_of_2[11];
	int state_of_1[11];
	int stones_in_mancala2;
	int stones_in_mancala1;

	GameInfo() {
		task = -1;
		player = -1;
		cut_off_dep = -1;
		memset(state_of_2, -1, 11 * sizeof(int));
		memset(state_of_1, -1, 11 * sizeof(int));
		stones_in_mancala2 = -1;
		stones_in_mancala1 = -1;
	}
};

void TransferStateInfoIntoArray(string state, int a[]) {
	num_of_pits = 0;
	string temp = "";
	for (size_t i = 0; i < state.length(); i++) {
		if (state.at(i) != ' ') {
			temp += state.at(i);
			continue;
		}
		num_of_pits++;
		a[num_of_pits] = atoi(temp.c_str());
		temp.clear();
	}
	num_of_pits++;
	a[num_of_pits] = atoi(temp.c_str());
}

GameInfo ReadInput(ifstream &input) {
	GameInfo game = GameInfo();
	string trash, state2, state1;

	input >> game.task;
	input >> game.player;
	input >> game.cut_off_dep;
	getline(input, trash);
	getline(input, state2);
	getline(input, state1);
	input >> game.stones_in_mancala2;
	input >> game.stones_in_mancala1;

	TransferStateInfoIntoArray(state2, game.state_of_2);
	TransferStateInfoIntoArray(state1, game.state_of_1);

	return game;
}

struct Node {
	int player;
	int state_of_2[11];
	int state_of_1[11];
	int stones_in_mancala2;
	int stones_in_mancala1;
	int depth;
	int value;
	int to_move;
	bool depth_change;
	string parent_name;
	int parent_depth;
	int parent_value;
	int show_depth;

	Node(GameInfo g, int d) {
		player = g.player;
		for (int i = 1; i <= num_of_pits; i++) {
			state_of_1[i] = g.state_of_1[i];
			state_of_2[i] = g.state_of_2[i];
		}
		stones_in_mancala1 = g.stones_in_mancala1;
		stones_in_mancala2 = g.stones_in_mancala2;
		depth = d;
		value = INT_MIN;
		to_move = -1;
		depth_change = true;
		parent_depth = -1;
		parent_name = "";
		show_depth = d;
	}

	Node() {
		player = -1;
		memset(state_of_2, -1, 11 * sizeof(int));
		memset(state_of_1, -1, 11 * sizeof(int));
		stones_in_mancala2 = -1;
		stones_in_mancala1 = -1;
		depth = -1;
		value = INT_MIN;
		to_move = -1;
		depth_change = true;
		parent_depth = -1;
		parent_name = "";
		show_depth = -1;
	}
};

int minValue(Node node, ofstream &output, Node* p, bool print_log);
int maxValue(Node node, ofstream &output, Node* p, bool print_log);
int alphaValue(Node node, ofstream &output, Node* p, int a, int b, bool print_log);
int betaValue(Node node, ofstream &output, Node* p, int a, int b, bool print_log);
Node global_move;
int globalmove;

string PrintInfinity(int i) {
	if (i == INT_MAX) {
		return "Infinity";
	}
	else if (i == INT_MIN) {
		return "-Infinity";
	}
	else {
		return to_string(i);
	}
}

int Evaluation(Node node) {
	if (max_player == 1) {
		return node.stones_in_mancala1 - node.stones_in_mancala2;
	}
	if (max_player == 2) {
		return node.stones_in_mancala2 - node.stones_in_mancala1;
	}
}

bool IsEndGame(Node node) {
	int mark1 = 0;
	int mark2 = 0;

	for (int i = 1; i <= num_of_pits; i++) {
		if (node.state_of_1[i] != 0) {
			mark1 = 1;
		}
	}
	for (int i = 1; i <= num_of_pits; i++) {
		if (node.state_of_2[i] != 0) {
			mark2 = 1;
		}
	}

	if (mark1 * mark2 != 0) {
		return false;
	}
	else {
		return true;
	}
}

Node MakeMove(int to_move, Node parent) {
	Node child;
	string parent_name = "";
	if (parent.player == 2) {
		if (parent.depth_change == true) {
			parent_name += "B";
		}
		else {
			parent_name += "A";
		}
	}
	else {
		if (parent.depth_change == true) {
			parent_name += "A";
		}
		else {
			parent_name += "B";
		}
	}
	parent_name += to_string(parent.to_move + 1);
	child.parent_name = parent_name;
	child.parent_depth = parent.depth;

	child.stones_in_mancala1 = parent.stones_in_mancala1;
	child.stones_in_mancala2 = parent.stones_in_mancala2;
	for (int i = 1; i <= num_of_pits; i++) {
		child.state_of_1[i] = parent.state_of_1[i];
		child.state_of_2[i] = parent.state_of_2[i];
	}
	child.to_move = to_move;

	int total = 2 * num_of_pits + 1;
	int rounds = 0, remainder = 0;

	if (parent.player == 1) {
		int rounds = child.state_of_1[to_move] / total;
		int remainder = child.state_of_1[to_move] % total;

		child.state_of_1[to_move] = 0;
		if (rounds > 0) {
			for (int i = 1; i <= num_of_pits; i++) {
				child.state_of_1[i] += rounds;
				child.state_of_2[i] += rounds;
			}
			child.stones_in_mancala1 += rounds;
		}

		for (int i = 1; i <= remainder; i++) {
			if (to_move + i <= num_of_pits) {
				child.state_of_1[to_move + i] ++;
			}
			else if (to_move + i == num_of_pits + 1) {
				child.stones_in_mancala1++;
			}
			else if (to_move + i <= 2 * num_of_pits + 1) {
				child.state_of_2[2 * num_of_pits - to_move - i + 2] ++;
			}
			else {
				child.state_of_1[2 * (to_move - num_of_pits) - 3 + i] ++;
			}
		}

		if (rounds == 0 && to_move + remainder <= num_of_pits && child.state_of_1[to_move + remainder] == 1) {
			child.stones_in_mancala1 += child.state_of_1[to_move + remainder];
			child.stones_in_mancala1 += child.state_of_2[to_move + remainder];
			child.state_of_1[to_move + remainder] = 0;
			child.state_of_2[to_move + remainder] = 0;
		}

		if (to_move + remainder == num_of_pits + 1) {
			child.player = 1;
			child.depth = parent.depth;
			child.depth_change = false;
		}
		else {
			child.player = 2;
			child.depth = parent.depth + 1;
			child.depth_change = true;
		}

		if (parent.depth_change == false) {
			child.show_depth = parent.show_depth;
		}
		else {
			child.show_depth = parent.show_depth + 1;
		}

		if (IsEndGame(child)) {
			for (int i = 1; i <= num_of_pits; i++) {
				child.stones_in_mancala2 += child.state_of_2[i];
				child.state_of_2[i] = 0;
			}
		}

		if (child.player == max_player) {
			child.value = INT_MIN;
		}
		else {
			child.value = INT_MAX;
		}

		if (child.depth == cut_off_depth) {
			child.value = Evaluation(child);
		}
	}

	if (parent.player == 2) {
		int rounds = child.state_of_2[to_move] / total;
		int remainder = child.state_of_2[to_move] % total;

		child.state_of_2[to_move] = 0;
		if (rounds > 0) {
			for (int i = 1; i <= num_of_pits; i++) {
				child.state_of_1[i] += rounds;
				child.state_of_2[i] += rounds;
			}
			child.stones_in_mancala2 += rounds;
		}

		for (int i = 1; i <= remainder; i++) {
			if (num_of_pits - to_move + 1 + i <= num_of_pits) {
				child.state_of_2[to_move - i] ++;
			}
			else if (num_of_pits - to_move + 1 + i == num_of_pits + 1) {
				child.stones_in_mancala2++;
			}
			else if (num_of_pits - to_move + 1 + i <= 2 * num_of_pits + 1) {
				child.state_of_1[i - to_move] ++;
			}
			else {
				child.state_of_2[2 * num_of_pits + to_move - i + 1] ++;
			}
		}

		if (rounds == 0 && num_of_pits - to_move + 1 + remainder <= num_of_pits && child.state_of_2[to_move - remainder] == 1) {
			child.stones_in_mancala2 += child.state_of_1[to_move - remainder];
			child.stones_in_mancala2 += child.state_of_2[to_move - remainder];
			child.state_of_1[to_move - remainder] = 0;
			child.state_of_2[to_move - remainder] = 0;
		}

		if (num_of_pits - to_move + 1 + remainder == num_of_pits + 1) {
			child.player = 2;
			child.depth = parent.depth;
			child.depth_change = false;
		}
		else {
			child.player = 1;
			child.depth = parent.depth + 1;
			child.depth_change = true;
		}

		if (parent.depth_change == false) {
			child.show_depth = parent.show_depth;
		}
		else {
			child.show_depth = parent.show_depth + 1;
		}

		if (IsEndGame(child)) {
			for (int i = 1; i <= num_of_pits; i++) {
				child.stones_in_mancala1 += child.state_of_1[i];
				child.state_of_1[i] = 0;
			}
		}

		if (child.player == max_player) {
			child.value = INT_MIN;
		}
		else {
			child.value = INT_MAX;
		}

		if (child.depth == cut_off_depth) {
			child.value = Evaluation(child);
		}
	}

	if (IsEndGame(child)) {
		child.value = Evaluation(child);
	}

	return child;
}

queue<Node> ExpandNode(Node n) {
	queue<Node> result;
	for (int i = 1; i <= num_of_pits; i++) {
		if (n.player == 1 && n.state_of_1[i] == 0) {
			continue;
		}
		if (n.player == 2 && n.state_of_2[i] == 0) {
			continue;
		}
		result.push(MakeMove(i, n));
	}
	return result;
}

int alphabetaDecision(Node node, ofstream &output, bool print_log) {
	Node* temp = &node;
	int a = INT_MIN;
	int b = INT_MAX;
	int value = alphaValue(node, output, temp, a, b, print_log);

	return value;
}

int alphaValue(Node node, ofstream &output, Node* p, int a, int b, bool print_log) {
	if (print_log) {
		if (node.depth == 0 && node.to_move == -1) {
			output << "root,0," << PrintInfinity(node.value) << "," << PrintInfinity(a) << "," << PrintInfinity(b) << endl;
		}
		else if (node.depth_change == true) {
			if (node.player == 2) {
				output << "B" << node.to_move + 1 << ",";
			}
			else {
				output << "A" << node.to_move + 1 << ",";
			}
			output << node.show_depth << "," << PrintInfinity(node.value) << "," << PrintInfinity(a) << "," << PrintInfinity(b) << endl;
		}
		else {
			if (node.player == 2) {
				output << "A" << node.to_move + 1 << ",";
			}
			else {
				output << "B" << node.to_move + 1 << ",";
			}
			output << node.show_depth << "," << PrintInfinity(node.value) << "," << PrintInfinity(a) << "," << PrintInfinity(b) << endl;
		}
	}

	if ((node.depth_change == true && node.depth == cut_off_depth) || IsEndGame(node)) {
		return Evaluation(node);
	}

	int best_value = INT_MIN;
	queue<Node> children;
	children = ExpandNode(node);
	Node* parent = &node;
	string parent_name = "";
	if (node.player == 2) {
		if (node.depth_change == true) {
			parent_name += "B";
		}
		else {
			parent_name += "A";
		}
	}
	else {
		if (node.depth_change == true) {
			parent_name += "A";
		}
		else {
			parent_name += "B";
		}
	}
	parent_name += to_string(node.to_move + 1);
	parent->parent_name = parent_name;
	parent->parent_depth = node.depth;

	while (!children.empty()) {
		Node child_node = children.front();

		if (child_node.player == node.player) {
			int value = alphaValue(child_node, output, parent, a, b, print_log);
			if (value > best_value) {
				best_value = value;
				globalmove = child_node.to_move;
				global_move.to_move = child_node.to_move;
				for (int i = 1; i <= num_of_pits; i++) {
					global_move.state_of_1[i] = child_node.state_of_1[i];
					global_move.state_of_2[i] = child_node.state_of_2[i];
				}
				global_move.stones_in_mancala1 = child_node.stones_in_mancala1;
				global_move.stones_in_mancala2 = child_node.stones_in_mancala2;
			}

			if (best_value >= b) {
				if (print_log) {
					if (node.depth == 0 && node.to_move == -1) {
						output << "root,0," << PrintInfinity(best_value) << "," << PrintInfinity(a) << "," << PrintInfinity(b) << endl;
					}
					else {
						output << parent->parent_name << "," << parent->show_depth << "," << PrintInfinity(best_value) << "," << PrintInfinity(a) << "," << PrintInfinity(b) << endl;
					}
				}
				return best_value;
			}

			if (print_log) {
				if (node.depth == 0 && node.to_move == -1) {
					output << "root,0," << PrintInfinity(best_value) << "," << PrintInfinity(best_value) << "," << PrintInfinity(b) << endl;
				}
				else {
					output << parent->parent_name << "," << parent->show_depth << "," << PrintInfinity(best_value) << "," << PrintInfinity(best_value) << "," << PrintInfinity(b) << endl;
				}
			}

		}
		else {
			int value = betaValue(child_node, output, parent, a, b, print_log);
			if (value > best_value) {
				best_value = value;
				globalmove = child_node.to_move;
				global_move.to_move = child_node.to_move;
				for (int i = 1; i <= num_of_pits; i++) {
					global_move.state_of_1[i] = child_node.state_of_1[i];
					global_move.state_of_2[i] = child_node.state_of_2[i];
				}
				global_move.stones_in_mancala1 = child_node.stones_in_mancala1;
				global_move.stones_in_mancala2 = child_node.stones_in_mancala2;
			}

			if (best_value >= b) {
				if (print_log) {
					if (node.depth == 0 && node.to_move == -1) {
						output << "root,0," << PrintInfinity(best_value) << "," << PrintInfinity(a) << "," << PrintInfinity(b) << endl;
					}
					else {
						output << parent->parent_name << "," << parent->show_depth << "," << PrintInfinity(best_value) << "," << PrintInfinity(a) << "," << PrintInfinity(b) << endl;
					}
				}

				return best_value;
			}

			if (print_log) {
				if (node.depth == 0 && node.to_move == -1) {
					output << "root,0," << PrintInfinity(best_value) << "," << PrintInfinity(best_value) << "," << PrintInfinity(b) << endl;
				}
				else {
					output << parent->parent_name << "," << parent->show_depth << "," << PrintInfinity(best_value) << "," << PrintInfinity(best_value) << "," << PrintInfinity(b) << endl;
				}
			}

		}

		if (best_value > a) {
			a = best_value;
		}


		children.pop();
	}

	return best_value;
}

int betaValue(Node node, ofstream &output, Node* p, int a, int b, bool print_log) {
	if (print_log) {
		if (node.depth_change == true) {
			if (node.player == 2) {
				output << "B" << node.to_move + 1 << ",";
			}
			else {
				output << "A" << node.to_move + 1 << ",";
			}
			output << node.show_depth << "," << PrintInfinity(node.value) << "," << PrintInfinity(a) << "," << PrintInfinity(b) << endl;
		}
		else {
			if (node.player == 2) {
				output << "A" << node.to_move + 1 << ",";
			}
			else {
				output << "B" << node.to_move + 1 << ",";
			}
			output << node.show_depth << "," << PrintInfinity(node.value) << "," << PrintInfinity(a) << "," << PrintInfinity(b) << endl;
		}
	}


	if ((node.depth_change == true && node.depth == cut_off_depth) || IsEndGame(node)) {
		return Evaluation(node);
	}

	int best_value = INT_MAX;
	queue<Node> children;
	children = ExpandNode(node);
	Node* parent = &node;
	string parent_name = "";
	if (node.player == 2) {
		if (node.depth_change == true) {
			parent_name += "B";
		}
		else {
			parent_name += "A";
		}
	}
	else {
		if (node.depth_change == true) {
			parent_name += "A";
		}
		else {
			parent_name += "B";
		}
	}
	parent_name += to_string(node.to_move + 1);
	parent->parent_name = parent_name;
	parent->parent_depth = node.depth;

	while (!children.empty()) {
		Node child_node = children.front();

		if (child_node.player == node.player) {
			int value = betaValue(child_node, output, parent, a, b, print_log);
			if (value < best_value) {
				best_value = value;
			}

			if (best_value <= a) {
				if (print_log) {
					output << parent->parent_name << "," << parent->show_depth << "," << PrintInfinity(best_value) << "," << PrintInfinity(a) << "," << PrintInfinity(b) << endl;
				}

				return best_value;
			}
			if (print_log) {
				output << parent->parent_name << "," << parent->show_depth << "," << PrintInfinity(best_value) << "," << PrintInfinity(a) << "," << PrintInfinity(best_value) << endl;
			}


		}
		else {
			int value = alphaValue(child_node, output, parent, a, b, print_log);
			if (value < best_value) {
				best_value = value;
			}

			if (best_value <= a) {
				if (print_log) {
					output << parent->parent_name << "," << parent->show_depth << "," << PrintInfinity(best_value) << "," << PrintInfinity(a) << "," << PrintInfinity(b) << endl;
				}

				return best_value;
			}
			if (print_log) {
				output << parent->parent_name << "," << parent->show_depth << "," << PrintInfinity(best_value) << "," << PrintInfinity(a) << "," << PrintInfinity(best_value) << endl;
			}

		}

		if (best_value < b) {
			b = best_value;
		}

		children.pop();
	}

	return best_value;
}

int minmaxDecision(Node node, ofstream &output, bool print_log) {
	Node* temp = &node;
	int value = maxValue(node, output, temp, print_log);

	return value;
}

int maxValue(Node node, ofstream &output, Node* p, bool print_log) {
	if (print_log) {
		if (node.depth == 0 && node.to_move == -1) {
			output << "root,0," << PrintInfinity(node.value) << endl;
		}
		else if (node.depth_change == true) {
			if (node.player == 2) {
				output << "B" << node.to_move + 1 << ",";
			}
			else {
				output << "A" << node.to_move + 1 << ",";
			}
			output << node.show_depth << "," << PrintInfinity(node.value) << endl;
		}
		else {
			if (node.player == 2) {
				output << "A" << node.to_move + 1 << ",";
			}
			else {
				output << "B" << node.to_move + 1 << ",";
			}
			output << node.show_depth << "," << PrintInfinity(node.value) << endl;
		}
	}

	if ((node.depth_change == true && node.depth == cut_off_depth) || IsEndGame(node)) {
		return Evaluation(node);
	}

	int best_value = INT_MIN;
	queue<Node> children;
	children = ExpandNode(node);
	Node* parent = &node;
	string parent_name = "";
	if (node.player == 2) {
		if (node.depth_change == true) {
			parent_name += "B";
		}
		else {
			parent_name += "A";
		}
	}
	else {
		if (node.depth_change == true) {
			parent_name += "A";
		}
		else {
			parent_name += "B";
		}
	}
	parent_name += to_string(node.to_move + 1);
	parent->parent_name = parent_name;
	parent->parent_depth = node.depth;

	while (!children.empty()) {
		Node child_node = children.front();

		if (child_node.player == node.player) {
			int value = maxValue(child_node, output, parent, print_log);
			if (value > best_value) {
				best_value = value;
				globalmove = child_node.to_move;
				global_move.to_move = child_node.to_move;
				for (int i = 1; i <= num_of_pits; i++) {
					global_move.state_of_1[i] = child_node.state_of_1[i];
					global_move.state_of_2[i] = child_node.state_of_2[i];
				}
				global_move.stones_in_mancala1 = child_node.stones_in_mancala1;
				global_move.stones_in_mancala2 = child_node.stones_in_mancala2;
			}
			if (print_log) {
				if (node.depth == 0 && node.to_move == -1) {
					output << "root,0," << PrintInfinity(best_value) << endl;
				}
				else {
					output << parent->parent_name << "," << parent->show_depth << "," << PrintInfinity(best_value) << endl;
				}
			}
		}
		else {
			int value = minValue(child_node, output, parent, print_log);
			if (value > best_value) {
				best_value = value;
				globalmove = child_node.to_move;
				global_move.to_move = child_node.to_move;
				for (int i = 1; i <= num_of_pits; i++) {
					global_move.state_of_1[i] = child_node.state_of_1[i];
					global_move.state_of_2[i] = child_node.state_of_2[i];
				}
				global_move.stones_in_mancala1 = child_node.stones_in_mancala1;
				global_move.stones_in_mancala2 = child_node.stones_in_mancala2;
			}
			if (print_log) {
				if (node.depth == 0 && node.to_move == -1) {
					output << "root,0," << PrintInfinity(best_value) << endl;
				}
				else {
					output << parent->parent_name << "," << parent->show_depth << "," << PrintInfinity(best_value) << endl;
				}
			}
		}

		children.pop();
	}

	return best_value;
}

int minValue(Node node, ofstream &output, Node* p, bool print_log) {
	if (print_log) {
		if (node.depth_change == true) {
			if (node.player == 2) {
				output << "B" << node.to_move + 1 << ",";
			}
			else {
				output << "A" << node.to_move + 1 << ",";
			}
			output << node.show_depth << "," << PrintInfinity(node.value) << endl;
		}
		else {
			if (node.player == 2) {
				output << "A" << node.to_move + 1 << ",";
			}
			else {
				output << "B" << node.to_move + 1 << ",";
			}
			output << node.show_depth << "," << PrintInfinity(node.value) << endl;
		}
	}

	if ((node.depth_change == true && node.depth == cut_off_depth) || IsEndGame(node)) {
		return Evaluation(node);
	}

	int best_value = INT_MAX;
	queue<Node> children;
	children = ExpandNode(node);
	Node* parent = &node;
	string parent_name = "";
	if (node.player == 2) {
		if (node.depth_change == true) {
			parent_name += "B";
		}
		else {
			parent_name += "A";
		}
	}
	else {
		if (node.depth_change == true) {
			parent_name += "A";
		}
		else {
			parent_name += "B";
		}
	}
	parent_name += to_string(node.to_move + 1);
	parent->parent_name = parent_name;
	parent->parent_depth = node.depth;

	while (!children.empty()) {
		Node child_node = children.front();

		if (child_node.player == node.player) {
			int value = minValue(child_node, output, parent, print_log);
			if (value < best_value) {
				best_value = value;
			}
			if (print_log) {
				output << parent->parent_name << "," << parent->show_depth << "," << PrintInfinity(best_value) << endl;
			}
		}
		else {
			int value = maxValue(child_node, output, parent, print_log);
			if (value < best_value) {
				best_value = value;
			}
			if (print_log) {
				output << parent->parent_name << "," << parent->show_depth << "," << PrintInfinity(best_value) << endl;
			}
		}

		children.pop();
	}

	return best_value;
}

int main(void) {
	ifstream input;
	input.open("d:\\input.txt");
	ofstream nextstate("d:\\next_state.txt");
	ofstream tlog("d:\\traverse_log.txt");
	GameInfo game = ReadInput(input);

	if (game.task == 1) {
		cut_off_depth = 1;
		max_player = game.player;
		Node node = Node(game, 0);
		minmaxDecision(node, tlog, true);
		Node result = MakeMove(globalmove, node);
		while (result.depth_change == false) {
			minmaxDecision(result, tlog, false);
			result = MakeMove(globalmove, result);
		}

		for (int i = 1; i <= num_of_pits; i++) {
			nextstate << result.state_of_2[i] << " ";
		}
		nextstate << endl;
		for (int i = 1; i <= num_of_pits; i++) {
			nextstate << result.state_of_1[i] << " ";
		}
		nextstate << endl;
		nextstate << result.stones_in_mancala2 << endl << result.stones_in_mancala1 << endl;
	}

	if (game.task == 2) {
		tlog << "Node,Depth,Value" << endl;
		cut_off_depth = game.cut_off_dep;
		max_player = game.player;
		Node node = Node(game, 0);
		minmaxDecision(node, tlog, true);
		Node result = MakeMove(globalmove, node);
		while (result.depth_change == false) {
			minmaxDecision(result, tlog, false);
			result = MakeMove(globalmove, result);
		}

		for (int i = 1; i <= num_of_pits; i++) {
			nextstate << result.state_of_2[i] << " ";
		}
		nextstate << endl;
		for (int i = 1; i <= num_of_pits; i++) {
			nextstate << result.state_of_1[i] << " ";
		}
		nextstate << endl;
		nextstate << result.stones_in_mancala2 << endl << result.stones_in_mancala1 << endl;
	}

	if (game.task == 3) {
		tlog << "Node,Depth,Value,Alpha,Beta" << endl;
		cut_off_depth = game.cut_off_dep;
		max_player = game.player;
		Node node = Node(game, 0);
		alphabetaDecision(node, tlog, true);
		Node result = MakeMove(globalmove, node);
		while (result.depth_change == false) {
			alphabetaDecision(result, tlog, false);
			result = MakeMove(globalmove, result);
		}

		for (int i = 1; i <= num_of_pits; i++) {
			nextstate << result.state_of_2[i] << " ";
		}
		nextstate << endl;
		for (int i = 1; i <= num_of_pits; i++) {
			nextstate << result.state_of_1[i] << " ";
		}
		nextstate << endl;
		nextstate << result.stones_in_mancala2 << endl << result.stones_in_mancala1 << endl;
	}

	input.close();
	nextstate.close();
	tlog.close();

	return 0;
}
