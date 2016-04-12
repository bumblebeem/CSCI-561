#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>
#include <deque>
#include <queue>
#include <algorithm>
using namespace std;

//用一个类Case来存储每个测试用例的成员，便于处理
struct Case{
	string task;
	string source;
	string destinations;
	string middle_nodes;
	int pipes;
	string graph;
	int start_time;

	//构造函数，初始化初值
	Case(){
		task = "";
		source = "";
		destinations = "";
		middle_nodes = "";
		pipes = 0;
		graph = "";
		start_time = 0;
	}

};

int num_of_cases;
string trash;             //用于处理使用getline()的时候遇到的多余的换行符

//读取即将处理的用例的信息，存储到Case类的对象中并返回
Case ReadInput(ifstream &input){
	Case res = Case();

	getline(input, res.task);
	getline(input, res.source);
	getline(input, res.destinations);
	getline(input, res.middle_nodes);
	input >> res.pipes;
	getline(input, trash);
	for (int i = res.pipes; i > 0; i--){
		string temp;
		getline(input, temp);
		res.graph = res.graph + temp + "*";
	}
	input >> res.start_time;
	getline(input, trash);
	getline(input, trash);

	return res;
}

//定义图的顶点类
struct Vertex{
	vector<string> vertexs;
	size_t num_of_vertexs;

	//从测试用例中取出所有顶点，并存放到一个vector中
	vector<string> GetVertex(Case case_now){
		vector<string> vertexs;

		//处理source，存入vertexs
		vertexs.push_back(case_now.source);

		//处理destinations，存入vertexs
		string temp = "";
		for (size_t i = 0; i < case_now.destinations.size(); i++){
			if (case_now.destinations[i] != ' '){
				temp += case_now.destinations[i];
			}
			else{
				vertexs.push_back(temp);
				temp.clear();
			}
		}
		vertexs.push_back(temp);

		//处理middle_nodes，存入vertexs
		if (case_now.middle_nodes != ""){
			temp.clear();
			for (size_t i = 0; i < case_now.middle_nodes.size(); i++){
				if (case_now.middle_nodes[i] != ' '){
					temp += case_now.middle_nodes[i];
				}
				else{
					vertexs.push_back(temp);
					temp.clear();
				}
			}
			vertexs.push_back(temp);
		}

		/*
		//测试是否将所有图的顶点信息存储正确
		for (size_t i = 0; i < vertexs.size(); i++){
		cout << "vertexs " << i + 1 << "is" << vertexs[i] << endl;
		}
		*/

		return vertexs;
	}

	//取得顶点名为ch的顶点的坐标
	int GetLocation(string ch){
		for (size_t i = 0; i < num_of_vertexs; i++){
			if (ch == vertexs[i]){
				return i;
			}
		}
		return -1;
	}

	//顶点类Vertex的构造函数
	Vertex(Case c){
		vertexs = GetVertex(c);
		num_of_vertexs = vertexs.size();
	}

};

//定义图的节点类
struct Node{
	string state;
	int depth;      //在BFS和DFS里边用
	int cost_by_now;        //在UCS里边用

	Node(string s, int d){
		state = s;
		depth = d;
		cost_by_now = -1;
	}

	Node(string s, int d, int c){
		state = s;
		depth = -1;
		cost_by_now = c;
	}

};

//尼玛，搞清楚了alphabetical order的时机之后，原来一切如此简单。。。
//辅助BFS节点在tie situations时候排序
bool BFSAlphaBeticalSortHelp(const Node &a, const Node &b){
	return a.state < b.state;
}

//辅助DFS节点在tie situations时候排序
bool DFSAlphaBeticalSortHelp(const Node &a, const Node &b){
	return a.state > b.state;
}

//辅助UCS中的优先队列排序,同时保证如果cost相同，先输出字母序小的
struct cmp{
	bool operator() (Node x, Node y){
		if (x.cost_by_now == y.cost_by_now){
			return x.state > y.state;
		}
		else{
			return x.cost_by_now > y.cost_by_now;
		}
	}
};

//测试当前节点是否是goal
bool IsGoalState(string state, vector<string> destination_vertexs){
	for (size_t i = 0; i < destination_vertexs.size(); i++){
		if (state == destination_vertexs[i]){
			return true;
		}
	}
	return false;
}

//测试当前节点是否在froniter中
bool IsInFrontier(Node node, deque<Node> frontier){
	queue<Node> temp;
	bool mark = false;
	for (size_t i = 0; i < frontier.size(); i++){
		if (node.state == frontier.front().state){
			mark = true;
			break;     //以前没加break，感觉应该是加上才对，没测试过
		}
		temp.push(frontier.front());
		frontier.pop_front();
	}
	for (size_t i = 0; i < temp.size(); i++){
		frontier.push_back(temp.front());
		temp.pop();
	}
	return mark;
}

//测试当前节点是否在froniter中，在的话返回当前cost
int IsInFrontierReturnCost(Node node, priority_queue<Node, vector<Node>, cmp> frontier){
	queue<Node> temp;
	int cost = -1;
	for (size_t i = 0; i < frontier.size(); i++){
		if (node.state == frontier.top().state){
			cost = frontier.top().cost_by_now;
			break;
		}
		temp.push(frontier.top());
		frontier.pop();
	}
	for (size_t i = 0; i < temp.size(); i++){
		frontier.push(temp.front());
		temp.pop();
	}
	return cost;
}

//从frontier中删除节点
//注意这里的frontier必须加上&(传引用)，否则修改不了！！
void DeleteNodeFromFrontier(Node node, priority_queue<Node, vector<Node>, cmp> &frontier){
	queue<Node> temp;
	for (size_t i = 0; i < frontier.size(); i++){
		if (node.state == frontier.top().state){
			frontier.pop();
			break;
		}
		temp.push(frontier.top());
		frontier.pop();
	}
	for (size_t i = 0; i < temp.size(); i++){
		frontier.push(temp.front());
		temp.pop();
	}
}

//测试当前节点是否在explored中
bool IsInExplored(Node node, vector<Node> explored){
	for (vector<Node>::iterator i = explored.begin(); i != explored.end(); i++){
		if (node.state == i->state){
			return true;
		}
	}
	return false;
}

//测试当前节点是否在explored中,在的话返回当前cost
int IsInExploredReturnCost(Node node, vector<Node> explored){
	int cost = -1;
	for (vector<Node>::iterator i = explored.begin(); i != explored.end(); i++){
		if (node.state == i->state){
			return i->cost_by_now;
		}
	}
	return cost;
}

//从explored中删除节点
//注意这里的explored必须加上&(传引用)，否则修改不了！！
void DeleteNodeFromExplored(Node node, vector<Node> &explored){
	for (vector<Node>::iterator i = explored.begin(); i != explored.end(); i++){
		if (node.state == i->state){
			i = explored.erase(i);
			break;
		}
	}
}

//BFS搜索算法
string BFS(Case case_now){
	Vertex v = Vertex(case_now);
	vector<vector<int> > graph_information(v.num_of_vertexs, vector<int>(v.num_of_vertexs));

	//初始化邻接矩阵, good job!!!
	string x = "", y = "", value = "";
	int mark = 0;
	size_t i = 0;
	string temp = "";
	while (i < case_now.graph.size()){
		if (mark % 4 == 0){
			temp += case_now.graph[i];
			i++;
			if (case_now.graph[i] == ' '){
				mark = (mark + 1) % 4;
				i++;
				x = temp;
				temp.clear();
			}
		}
		if (mark % 4 == 1){
			temp += case_now.graph[i];
			i++;
			if (case_now.graph[i] == ' '){
				mark = (mark + 1) % 4;
				i++;
				y = temp;
				temp.clear();
			}
		}
		if (mark % 4 == 2){
			temp += case_now.graph[i];
			i++;
			if (case_now.graph[i] == ' '){
				mark = (mark + 1) % 4;
				i++;
				value = temp;
				temp.clear();
				//注意下一行中atoi()函数的使用，将C字符串转换成int!
				graph_information[v.GetLocation(x)][v.GetLocation(y)] = atoi(value.c_str());
				x.clear();
				y.clear();
				value.clear();
			}
		}
		if (mark % 4 == 3){
			if (case_now.graph[i] == '*'){
				mark = (mark + 1) % 4;
				i++;
			}
			else{
				i++;
			}
		}
	}

	/*测试邻接矩阵是否正确
	cout << "test information for graph_information:" << endl;
	for (size_t i = 0; i < v.num_of_vertexs; i++){
	for (size_t j = 0; j < v.num_of_vertexs; j++){
	cout << graph_information[i][j] <<"\t";
	}
	cout << endl;
	}
	*/

	//将所有作为destination的顶点存储到destination_vertexs这个vector中，便于之后处理
	temp.clear();
	vector<string> destination_vertexs;
	for (size_t i = 0; i < case_now.destinations.size(); i++){
		if (case_now.destinations[i] != ' '){
			temp += case_now.destinations[i];
		}
		else{
			destination_vertexs.push_back(temp);
			temp.clear();
		}
	}
	destination_vertexs.push_back(temp);

	//正式开始BFS。。。
	Node current_node(case_now.source, 0);
	int time = case_now.start_time;
	if (IsGoalState(current_node.state, destination_vertexs)){
		return case_now.source + " " + to_string(time);
	}
	deque<Node> frontier;
	frontier.push_back(current_node);
	vector<Node> explored;
	vector<Node> alphabetical_order_helper;
	while (!frontier.empty()){
		current_node = frontier.front();
		//cout << current_node.state << " is being explored!" << endl;
		//注意下边这段对explored的操作！
		if (IsInExplored(current_node, explored)){
			frontier.pop_front();
			continue;
		}
		else{
			explored.push_back(current_node);
			frontier.pop_front();
		}
		string state_name = "";
		for (size_t i = 0; i < v.num_of_vertexs; i++){
			if (graph_information[v.GetLocation(current_node.state)][i] != 0){
				state_name = v.vertexs[i];
				Node child = Node(state_name, current_node.depth + 1);
				if (!IsInFrontier(child, frontier) && !IsInExplored(child, explored)){
					alphabetical_order_helper.push_back(child);
				}
			}
		}
		//排序alphabetical_order_helper,之后再push进frontier!!!!!
		//注意sort函数第三个参数的用法，之后还要注意#include <algorithm>
		sort(alphabetical_order_helper.begin(), alphabetical_order_helper.end(), BFSAlphaBeticalSortHelp);
		for (vector<Node>::iterator iter = alphabetical_order_helper.begin(); iter != alphabetical_order_helper.end(); iter++){
			//cout << iter->state << " is pushing in frontier!!" << endl;
			if (IsGoalState(iter->state, destination_vertexs)){
				return iter->state + " " + to_string((time + iter->depth) % 24);
			}
			frontier.push_back(*iter);
		}
		alphabetical_order_helper.clear();
	}

	return "None";
}

//DFS搜索算法
string DFS(Case case_now){
	Vertex v = Vertex(case_now);
	vector<vector<int> > graph_information(v.num_of_vertexs, vector<int>(v.num_of_vertexs));

	//初始化邻接矩阵, good job!!!
	string x = "", y = "", value = "";
	int mark = 0;
	size_t i = 0;
	string temp = "";
	while (i < case_now.graph.size()){
		if (mark % 4 == 0){
			temp += case_now.graph[i];
			i++;
			if (case_now.graph[i] == ' '){
				mark = (mark + 1) % 4;
				i++;
				x = temp;
				temp.clear();
			}
		}
		if (mark % 4 == 1){
			temp += case_now.graph[i];
			i++;
			if (case_now.graph[i] == ' '){
				mark = (mark + 1) % 4;
				i++;
				y = temp;
				temp.clear();
			}
		}
		if (mark % 4 == 2){
			temp += case_now.graph[i];
			i++;
			if (case_now.graph[i] == ' '){
				mark = (mark + 1) % 4;
				i++;
				value = temp;
				temp.clear();
				//注意下一行中atoi()函数的使用，将C字符串转换成int!
				graph_information[v.GetLocation(x)][v.GetLocation(y)] = atoi(value.c_str());
				x.clear();
				y.clear();
				value.clear();
			}
		}
		if (mark % 4 == 3){
			if (case_now.graph[i] == '*'){
				mark = (mark + 1) % 4;
				i++;
			}
			else{
				i++;
			}
		}
	}

	//将所有作为destination的顶点存储到destination_vertexs这个vector中，便于之后处理
	temp.clear();
	vector<string> destination_vertexs;
	for (size_t i = 0; i < case_now.destinations.size(); i++){
		if (case_now.destinations[i] != ' '){
			temp += case_now.destinations[i];
		}
		else{
			destination_vertexs.push_back(temp);
			temp.clear();
		}
	}
	destination_vertexs.push_back(temp);

	//正式开始DFS。。。
	Node current_node(case_now.source, 0);
	int time = case_now.start_time;
	if (IsGoalState(current_node.state, destination_vertexs)){
		return case_now.source + " " + to_string(time);
	}
	deque<Node> frontier;
	frontier.push_front(current_node);
	vector<Node> explored;
	vector<Node> alphabetical_order_helper;
	while (!frontier.empty()){
		current_node = frontier.front();
		//cout << current_node.state << " is being explored!" << endl;
		if (IsGoalState(current_node.state, destination_vertexs)){
			return current_node.state + " " + to_string((time + current_node.depth) % 24);
		}
		//注意下边这段对explored的操作！
		if (IsInExplored(current_node, explored)){
			frontier.pop_front();
			continue;
		}
		else{
			explored.push_back(current_node);
			frontier.pop_front();
		}
		string state_name = "";
		for (size_t i = 0; i < v.num_of_vertexs; i++){
			if (graph_information[v.GetLocation(current_node.state)][i] != 0){
				state_name = v.vertexs[i];
				Node child = Node(state_name, current_node.depth + 1);
				if (!IsInFrontier(child, frontier) && !IsInExplored(child, explored)){
					alphabetical_order_helper.push_back(child);
				}
			}
		}
		//排序alphabetical_order_helper,之后再push进frontier!!!!!
		sort(alphabetical_order_helper.begin(), alphabetical_order_helper.end(), DFSAlphaBeticalSortHelp);
		for (vector<Node>::iterator iter = alphabetical_order_helper.begin(); iter != alphabetical_order_helper.end(); iter++){
			//cout << iter->state << " is pushing in frontier!!" << endl;
			frontier.push_front(*iter);
		}
		alphabetical_order_helper.clear();
	}

	return "None";
}

//定义记录路径的关闭时间的类
//其中off_peroid用set之类不允许有重复元素的容器更好
struct Path{
	string from;
	string to;
	vector<int> off_period;

	Path(string f, string t, int a, int b){
		from = f;
		to = t;
		for (int i = a; i <= b; i++){
			off_period.push_back(i);
		}
	}
};

//检查time_now的时候，node是否可用
bool PipeIsOff(Node from, Node to, vector<Path> paths, int start_time){
	//cout << "we are now examining this path: " << from.state << " - " << to.state << endl;
	int time_now = (from.cost_by_now + start_time) % 24;
	for (size_t i = 0; i < paths.size(); i++){
		if (from.state == paths[i].from && to.state == paths[i].to){
			for (size_t j = 0; j < paths[i].off_period.size(); j++){
				if (time_now == paths[i].off_period[j]){
					//cout << "off occur!" << endl;
					return true;
				}
			}
		}
	}
	//cout << "off did not occur!" << endl;
	return false;
}

//UCS搜索算法
string UCS(Case case_now){
	Vertex v = Vertex(case_now);
	vector<vector<int> > graph_information(v.num_of_vertexs, vector<int>(v.num_of_vertexs));
	vector<Path> paths;
	paths.clear();

	//初始化邻接矩阵, good job!!!加上了对关闭路径的处理，犀利！！！
	string from = "", to = "", value = "";
	int mark = 0;
	size_t i = 0;
	string temp = "";
	int nums_of_closed_time = 0;
	vector<int> offtimes;
	offtimes.clear();
	while (i < case_now.graph.size()){
		if (mark % 4 == 0){
			temp += case_now.graph[i];
			i++;
			if (case_now.graph[i] == ' '){
				mark = (mark + 1) % 4;
				i++;
				from = temp;
				temp.clear();
			}
		}
		if (mark % 4 == 1){
			temp += case_now.graph[i];
			i++;
			if (case_now.graph[i] == ' '){
				mark = (mark + 1) % 4;
				i++;
				to = temp;
				temp.clear();
			}
		}
		if (mark % 4 == 2){
			temp += case_now.graph[i];
			i++;
			if (case_now.graph[i] == ' '){
				mark = (mark + 1) % 4;
				i++;
				value = temp;
				temp.clear();
				//注意下一行中atoi()函数的使用，将C字符串转换成int!
				graph_information[v.GetLocation(from)][v.GetLocation(to)] = atoi(value.c_str());
				value.clear();
				nums_of_closed_time = 0;
			}
		}
		if (mark % 4 == 3){
			while (case_now.graph[i] != '*' && case_now.graph[i] != ' '){
				temp += case_now.graph[i];
				i++;
			}
			nums_of_closed_time = atoi(temp.c_str());
			temp.clear();
			i++;
			if (nums_of_closed_time == 0){
				mark = (mark + 1) % 4;
			}
			while (nums_of_closed_time-- && ((mark % 4) == 3)){
				if (nums_of_closed_time > 0){
					while (case_now.graph[i] != '-'){
						temp += case_now.graph[i];
						i++;
					}
					offtimes.push_back(atoi(temp.c_str()));
					temp.clear();
					i++;
					while (case_now.graph[i] != ' '){
						temp += case_now.graph[i];
						i++;
					}
					offtimes.push_back(atoi(temp.c_str()));
					temp.clear();
					i++;
					sort(offtimes.begin(), offtimes.end());
					Path p = Path(from, to, offtimes[0], offtimes[1]);
					paths.push_back(p);
					offtimes.clear();
					continue;
				}
				else{
					while (case_now.graph[i] != '-'){
						temp += case_now.graph[i];
						i++;
					}
					offtimes.push_back(atoi(temp.c_str()));
					temp.clear();
					i++;
					while (case_now.graph[i] != '*'){
						temp += case_now.graph[i];
						i++;
					}
					offtimes.push_back(atoi(temp.c_str()));
					temp.clear();
					i++;
					sort(offtimes.begin(), offtimes.end());
					Path p = Path(from, to, offtimes[0], offtimes[1]);
					paths.push_back(p);
					offtimes.clear();
				}
			}
			if ((mark % 4) == 3){
				mark = (mark + 1) % 4;
			}
			from.clear();
			to.clear();
		}

	}

	/*
	//测试邻接矩阵是否正确
	cout << "test information for graph_information:" << endl;
	for (size_t i = 0; i < v.num_of_vertexs; i++){
	for (size_t j = 0; j < v.num_of_vertexs; j++){
	cout << graph_information[i][j] << "\t";
	}
	cout << endl;
	}
	*/

	/*
	//测试paths是否正确
	for (size_t i = 0; i < paths.size(); i++){
	cout << paths[i].from << "-" << paths[i].to << "is off during: " << endl;
	for (size_t j = 0; j < paths[i].off_period.size(); j++){
	cout << paths[i].off_period[j] << "*" << endl;
	}
	}
	*/

	//将所有作为destination的顶点存储到destination_vertexs这个vector中，便于之后处理
	temp.clear();
	vector<string> destination_vertexs;
	for (size_t i = 0; i < case_now.destinations.size(); i++){
		if (case_now.destinations[i] != ' '){
			temp += case_now.destinations[i];
		}
		else{
			destination_vertexs.push_back(temp);
			temp.clear();
		}
	}
	destination_vertexs.push_back(temp);

	//正式开始UCS。。。
	Node current_node(case_now.source, -1, 0);
	int start_time = case_now.start_time;
	priority_queue<Node, vector<Node>, cmp> frontier;
	frontier.push(current_node);
	vector<Node> explored;
	vector<Node> children;
	while (!frontier.empty()){
		current_node = frontier.top();
		frontier.pop();
		//cout << current_node.state << "is being explored!" << endl;
		if (IsGoalState(current_node.state, destination_vertexs)){
			return current_node.state + " " + to_string((start_time + current_node.cost_by_now) % 24);
		}
		string state_name = "";
		for (size_t i = 0; i < v.num_of_vertexs; i++){
			if (graph_information[v.GetLocation(current_node.state)][i] != 0){
				state_name = v.vertexs[i];
				Node child = Node(state_name, -1, current_node.cost_by_now + graph_information[v.GetLocation(current_node.state)][i]);
				if (PipeIsOff(current_node, child, paths, start_time)){
					//cout << child.state << child.cost_by_now << " was threw away!!" << endl;
					continue;
				}
				if ((IsInFrontierReturnCost(child, frontier) == -1) && (IsInExploredReturnCost(child, explored) == -1)){
					frontier.push(child);
					//cout << child.state << child.cost_by_now << " entered the frontier!!" << endl;
				}
				else if ((IsInFrontierReturnCost(child, frontier) != -1) &&
					(child.cost_by_now < IsInFrontierReturnCost(child, frontier))){
					DeleteNodeFromFrontier(child, frontier);
					frontier.push(child);
					//cout << child.state << child.cost_by_now << " entered the frontier!!" << endl;
				}
				/*
				//根据书上的算法实现的话，不判断是不是在explored里边，所以最后这个else if最后要去掉
				else if ((IsInExploredReturnCost(child, explored) != -1) &&
				(child.cost_by_now < IsInExploredReturnCost(child, explored))){
				//cout << child.state << "is in explored" << endl;
				DeleteNodeFromExplored(child, explored);
				frontier.push(child);
				//cout << child.state << child.cost_by_now << " entered the frontier!!" << endl;
				}
				*/
			}
		}
		explored.push_back(current_node);
	}

	return "None";
}

int main(int argc, char *argv[]){
	if (argc > 1){
		ifstream in_file(argv[2]);
		cout << "Open File Successfully!" << endl;
		ofstream out_file("output.txt");

		if (in_file.is_open() && in_file.good()){
			in_file >> num_of_cases;
			getline(in_file, trash);

			while (num_of_cases--){
				Case case_now = ReadInput(in_file);
				if (case_now.task == "BFS"){
					out_file << BFS(case_now) << endl;
				}
				if (case_now.task == "DFS"){
					out_file << DFS(case_now) << endl;
				}
				if (case_now.task == "UCS"){
					out_file << UCS(case_now) << endl;
				}
			}

			in_file.close();
			out_file.close();
			return 0;
		}
	}
	else{
		cout << "Open File Failed!!" << endl;
		return -1;
	}
}