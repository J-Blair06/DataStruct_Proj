#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <limits>


using namespace std;

const int INF = 1000000000; //used to represent infinity

/*******************************************************************************************************/

template <typename T>
class MinHeap {
public:
    MinHeap() {}
    void insert(const T& val); 
    T delete_min();
    bool empty() const { return data.empty(); }

private:
    vector<T> data;

    void swap(T& v1, T& v2) {
        T tmp = v1;
        v1 = v2;
        v2 = tmp;
    }

    void percolate_down(int i);
    int min_index(int i1, int i2) const;
};

/*******************************************************************************************************/

template<typename T>
void MinHeap<T>::insert(const T& val) {
    data.push_back(val);
    int n = data.size() - 1; // index of the last node
    // percolate UP
    while (n > 0 && data[n] < data[(n - 1) / 2]) {
        swap(data[n], data[(n - 1) / 2]);
        n = (n - 1) / 2;
    }
}

/*******************************************************************************************************/

template<typename T>
T MinHeap<T>::delete_min() {
    if (data.empty()) {
        throw string("delete_min: Empty Heap\n");
    }    
    T res = data[0];
    data[0] = data[data.size() - 1];
    data.pop_back();
    percolate_down(0);
    return res;
}

/*******************************************************************************************************/

template<typename T>
void MinHeap<T>::percolate_down(int i) {
    if (data.empty() || i >= (int)data.size() || i < 0) return;

    int parent_index = i;
    int kids_min_index = i;

    do {
        if (parent_index * 2 + 1 >= (int)data.size()) break; // no kids

        if (parent_index * 2 + 2 < (int)data.size()) { // two kids
            kids_min_index = min_index(parent_index * 2 + 1, parent_index * 2 + 2);
        } else if (parent_index * 2 + 1 < (int)data.size()) { // only left kid
            kids_min_index = parent_index * 2 + 1;
        }

        if (data[kids_min_index] < data[parent_index]) {
            swap(data[parent_index], data[kids_min_index]);
            parent_index = kids_min_index;
        } else {
            break;
        }
    } while (true);
}

/*******************************************************************************************************/

template<typename T>
int MinHeap<T>::min_index(int i1, int i2) const {
    return (data[i1] < data[i2] ? i1 : i2);
}

/*******************************************************************************************************/
 
struct HeapNode {
    int dist;
    int node;
    bool operator<(const HeapNode& other) const {
        return dist < other.dist;   // smaller distance = higher priority
    }
};

/*******************************************************************************************************/

//Edge struct - represents a directed flight from one airport to another
//stores both weights (distance and cost)

struct Edge {
    int to; //index of airport the plane is flying to
    int dist; //distance in miles
    int cost; //how much the ticket costs
};

/*******************************************************************************************************/

//UEdge struct - only used for undirected graph and MST algorithms
// represents undirected connection between two different airports

struct UEdge {
    int u; //smaller index airport
    int v; //larger index airport
    int cost; //minimum cost between the two directed edges
};

/*******************************************************************************************************/

// Conn struct - direct connnections count (stores airport
// code and total inbound + outbound direct flights)
struct Conn {
    string code;
    int cnt;
};

/*******************************************************************************************************/

//PathDisplay struct - stores and displays results

struct PathDisplay {
    string pathStr;
    int length;
    int cost;
};

/*******************************************************************************************************/

//function to reverse vectors
void reverseVector(vector<int>& v){
    int i = 0;
    int j = (int)v.size() - 1;
    while(i < j) {
        int tmp = v[i];
        v[i] = v[j];
        v[j] = tmp;
        i++;
        j--;
    }
}


//function CutOutBlanks which will remove blank spaces
// from the beginning and end of a string
//is used when parsing CSV city names

string CutOutBlanks(const string& str) {
    //finds the first entry in a string that is not " "
    size_t first = str.find_first_not_of(" \t"); 

    if(first == string::npos){
        return "";
    }

    //finds the last character in a string that is not " "
    size_t last = str.find_last_not_of(" \t");

    //creates new string starting from the first non blank character
    // to the last non blank character
    return str.substr(first, last - first + 1);
}

/*******************************************************************************************************/

//function to pull the 2 letter state code from the city, ST format
// EX: "lakeland, FL" -> "FL"

string takeOutState(string city){

    //position variable which locates the , in the city, ST format
    size_t pos = city.rfind(',');

    //this is checking if the , was found at the index pos
    if(pos != string::npos) {
        
        //extracts a new substring starting from position
        // immediately after pos until the end of the string
        string state = city.substr(pos + 1);
        return CutOutBlanks(state);
    }

    return CutOutBlanks(city);
}

/*******************************************************************************************************/

vector<string> parseCSVLine(const string& line) {
    vector<string> fields;
    string field; // current field being built
    bool inQuotes = false; //variable to determine if we are inside quotes

    for(char c : line) {
        if(c == '"'){
            //toggles the inquotes to be true and skip the quote character
            inQuotes = !inQuotes;
            continue;
        }

        if(c == ',' && !inQuotes){
            fields.push_back(field);
            field.clear();
        } else {
            field += c;
        }
    }
    if (!field.empty() || !fields.empty()) fields.push_back(field);
    return fields;
}

/*******************************************************************************************************/

//function addAirport - adds a new airport if it doesn't exist and returns its index

int addAirport(const string& code, const string& city, vector<string>& codes, vector<string>& statesv) {
    string cleanCode = CutOutBlanks(code);
    for (size_t i = 0; i < codes.size(); ++i)
        if (codes[i] == cleanCode) return i;
    codes.push_back(cleanCode);
    statesv.push_back(takeOutState(city));
    return codes.size() - 1;
}

/*******************************************************************************************************/

//getIndex - returns the index of an airport by its 3 - letter code.
//uses linear search
int getIndex(const string& code, const vector<string>& codes) {
    string clean = CutOutBlanks(code);
    for (size_t i = 0; i < codes.size(); ++i)
        if (codes[i] == clean) return i;
    return -1;
}

/*******************************************************************************************************/

//reconstructPath - builds the path from the prev[] array produced by Dijkstra
// starts at the goal and goes backwards to start using the prev links

vector<int> reconstructPath(const vector<int>&prev, int start, int goal){
    vector<int> path;
    for (int at = goal; at != -1; at = prev[at]){
        path.push_back(at);
        if (at == start) break;
    }
    if (!path.empty() && path.back() != start) return {};
    reverseVector(path);
    return path;
}

/*******************************************************************************************************/

//calculatePathCost - function is given a path, then sums the flight costs
// this is done by looking up each edge in the adjacency list

int calculatePathCost(const vector<int>& path, 
const vector<vector<Edge>>& adj) {
    if (path.size() < 2) return 0;
    int total = 0;
    for (size_t i = 0; i +1 < path.size(); ++i){
        int u = path[i], v = path[i + 1];
        for( const auto& e : adj[u]) {
            if (e.to == v) { total += e.cost; break; }
        }
    }
    return total;
}

/*******************************************************************************************************/

//dijkstraShortest - shortest path by distance using MinHeap
//returns the path and the total distance (length)

pair<vector<int>, int> dijkstraShortest(int start, int goal,
const vector<vector<Edge>>& adj, int n) {
    vector<int> dist(n, INF);
    vector<int> prev(n, -1);
    vector<bool> visited(n, false);
    MinHeap<HeapNode> pq;

    dist[start] = 0;
    pq.insert({0, start});

 while (!pq.empty()) {
        HeapNode curr = pq.delete_min();
        int u = curr.node;
        if (visited[u]) continue;
        visited[u] = true;

        for (const auto& e : adj[u]) {
            int v = e.to;
            if (dist[u] + (int)e.dist < dist[v]) {
                dist[v] = dist[u] + (int)e.dist;
                prev[v] = u;
                pq.insert({dist[v], v});
            }
        }
    }

    vector<int> path = reconstructPath(prev, start, goal);
    int length = (path.empty() ? INF : dist[goal]);
    return {path, length};
}

/*******************************************************************************************************/

//shortestWithStops - finds the shortest path with exactly k stops
//dp[edges][v] = minimum distance to v

pair<vector<int>, int> shortestWithStops(int start, int goal, int kStops,
const vector<vector<Edge>>& adj, int n) {
    if (kStops < 0) return {{}, INF};
    int maxEdges = kStops + 1;
    vector<vector<int>> dp(maxEdges + 1, vector<int> (n, INF));
    vector<vector<int>> pre(maxEdges +1, vector<int>(n, -1));
    dp[0][start] = 0;

    for(int edges = 1; edges <= maxEdges; ++edges) {
        for(int u = 0; u < n; ++u){
            if(dp[edges - 1][u] == INF) continue;
            for( const auto& e : adj[u]){
                int v = e.to;
                int newDist = dp[edges - 1][u]+ e.dist;
                if (newDist < dp[edges][v]){
                    dp[edges][v] = newDist;
                    pre[edges][v] = u;
                }
            }
        }
    }

    if (dp[maxEdges][goal] == INF) return {{}, INF};

    vector<int> path;
    int currV = goal, currE = maxEdges;
    while (currE >= 0) {
        path.push_back(currV);
        if(currE == 0) break;
        currV = pre[currE][currV];
        if(currV == -1) return {{}, INF};
        --currE;
    }

    reverseVector(path);
    if(path.empty() || path[0] != start) return {{}, INF};
    return {path, dp[maxEdges][goal]};
}

/*******************************************************************************************************/

// sortConns - bubble sort : sorts airports by total connection
// descending then by code ascending

void sortConns(vector<Conn>& conns) {
    int sz = conns.size();
    for (int i = 0; i < sz; ++i){
        for(int j = 0; j < sz - 1 - i; ++j) {
            bool swapNeeded = (conns[j].cnt < conns[j + 1].cnt)
            || (conns[j].cnt == conns[j + 1].cnt &&
            conns[j].code > conns[j + 1].code);
            if (swapNeeded) swap(conns[j], conns[j + 1]);
        }
    }
}

/*******************************************************************************************************/

//sortUEdges - bubble sort for Kruskal's algorithm
//sorts undirected edges by cost ascending

void sortUEdges(vector<UEdge>& edges){
    int sz = edges.size();
    for(int i = 0; i < sz; ++i){
        for(int j = 0; j < sz - 1 - i; ++j){
            if(edges[j].cost > edges[j + 1].cost) swap(edges[j], edges[j + 1]);
        }
    }
}

/*******************************************************************************************************/

//main
//interactive menu and graph loading

int main() {
    vector<string> codes;
    vector<string> statesv;
    vector<vector<Edge>> adj;

    ifstream fin("airports.csv");
    if (!fin.is_open()) {
        cout << "Error: Cannot open airports.csv" << endl;
        return 1;
    }

    vector<string> allLines;
    string line;
    while (getline(fin, line)) {
    if (!line.empty()) {
        allLines.push_back(line); }
    }
    fin.close();

    for(const auto& l : allLines){
        auto fields = parseCSVLine(l);
        if (fields.size() < 6 || fields[0] == "Origin_airport") {
            continue;
        }
        addAirport(fields[0], fields[2], codes, statesv);
        addAirport(fields[1], fields[3], codes, statesv);
    }

    int n = codes.size();
    adj.assign(n, vector<Edge>());
    vector<vector<int>> dirCost(n, vector<int>(n, INF));
    vector<int> inDegree(n, 0);

    for(const auto& l : allLines) {
        auto fields = parseCSVLine(l);
        if(fields.size() < 6 || fields[0] == "Origin_airport"){
            continue;
        }
        int u = getIndex(fields[0], codes);
        int v = getIndex(fields[1], codes);
        if (u == -1 || v == -1) continue;
        int distVal = stoi(fields[4]);
        int costVal = stoi(fields[5]);
        adj[u].push_back({v, distVal, costVal});
        if (dirCost[u][v] == INF || costVal < dirCost[u][v]){
            dirCost[u][v] = costVal;
        }
        inDegree[v]++;
    }

    cout << "Graph loaded successfully! " << n << "airports, " <<
    (allLines.size() - 1) << " flights.\n" << endl;

    while (true) {
        cout << "\n=== Airport Graph System ===" << endl;
        cout << "2. Shortest path by distance" << endl;
        cout << "3. Shortest paths to all airports in a state" << endl;
        cout << "4. Shortest path with exact number of stops" << endl;
        cout << "5. Direct flight connections count" << endl;
        cout << "6. Build and show Undirected Graph Gu" << endl;
        cout << "7. Prim's Minimal Spanning Tree" << endl;
        cout << "8. Kruskal's Minimal Spanning Tree/Forest" << endl;
        cout << "0. Exit" << endl;
        cout << "Enter choice: ";
        int choice;
        cin >>choice;
        if(choice == 0) break;

/*******************************************************************************************************/

        if(choice == 2) {
            string orig, dest;
            cout << "Enter origin airport code: ";
            cin >> orig;
            cout << "Enter destination airport code: ";
            cin >> dest;
            int start = getIndex(orig, codes);
            int goal = getIndex(dest, codes);
            if(start == -1 || goal == -1) {cout << "Invalid airport code(s)." << endl; continue; }
            
            pair<vector<int>, int> result = dijkstraShortest(start, goal, adj, n);
            vector<int> path = result.first;
            int length = result.second;
            
            if(path.empty() || length == INF) {
                cout << "Shortest route from " << orig << " to " <<
                dest << ": None" << endl;
            } else {
                cout << "Shortest route from " << orig << " to " << dest << ": ";
                for(size_t i = 0; i < path.size(); ++i){
                    cout << codes[path[i]];
                    if (i + 1 < path.size()) cout << " -> ";
                }
                int pCost = calculatePathCost(path, adj);
                cout << ". The length is " << length << ". The cost is " << pCost << "." << endl;
            }
        }

/*******************************************************************************************************/

        else if (choice == 3) {
            string orig, state;
            cout << "Enter origin airport code: ";
            cin >> orig;
            cout << "Enter destination state: ";
            cin >> state;
            int start = getIndex(orig, codes);
            if (start == -1) {cout << "Invalid origin." << endl; continue; }

            vector<int> dist(n, INF);
            vector<int> prev(n, -1);
            vector<bool> visited(n, false);
            dist[start] = 0;
            MinHeap<HeapNode> pq;
            pq.insert({0, start});

            while(!pq.empty()) {
                HeapNode curr = pq.delete_min();
                int u = curr.node;
                if(visited[u]) continue;
                visited[u] = true;
                for(const auto& e : adj[u]){
                    int v = e.to;
                    if (dist[u] + (int)e.dist < dist[v]) {
                        dist[v] = dist[u] + (int)e.dist;
                        prev[v] = u;
                        pq.insert({dist[v], v});
                    }
                }
            }

            vector<PathDisplay> results;
            for(int i = 0; i< n; ++i){
                if (i == start || statesv[i] != state || dist[i] == INF) continue;
                vector<int> pth = reconstructPath(prev, start, i);
                if(pth.empty()) continue;
                int len = dist[i];
                int pc = calculatePathCost(pth, adj);
                string pStr;
                for(size_t j = 0; j < pth.size(); ++j) {
                    pStr += codes[pth[j]];
                    if (j + 1 < pth.size()) pStr += "->";
                }
                results.push_back({pStr, len, pc});
            }
            if(results.empty()){
                cout << "Shortest paths from " << orig << " to "<< state <<
                " state airports: None" << endl;
            } else {
                cout << "Shortest paths from " << orig << " to " << state <<
                " state airports are:" << endl;
                cout << "Path\t\tLength\tCost" << endl;
                for (const auto& r : results) {
                    cout << r.pathStr << "\t" << r.length << "\t" << r.cost << endl;
                }
            }
        }
/*******************************************************************************************************/

        else if (choice == 4) {
            string orig, dest;
            int k;
            cout << "Enter origin airpot code: ";
            cin >> orig;
            cout << "Enter destination airport code: ";
            cin >> dest;
            cout << "Enter number of stops: ";
            cin >> k;
            int start = getIndex(orig, codes);
            int goal = getIndex(dest, codes);
            if (start == -1 || goal == -1) {cout << "Invalid airport code(s)." << endl; continue; }
            pair<vector<int>, int> result = shortestWithStops(start, goal, k, adj, n);
            vector<int> path = result.first;
            int length = result.second;
            if(path.empty() || length == INF) {
                cout << "Shortest rout from "<< orig << " to " << dest <<
                " with" << k << " stops: None" << endl;
            } else {
                cout << "Shortest route from " << orig << " to " << dest << " with"
                << k << " stops: ";
                for(size_t i = 0; i < path.size(); ++i) {
                    cout << codes[path[i]];
                    if(i + 1 < path.size()) cout << " -> ";
                }
                int pCost = calculatePathCost(path, adj);
                cout << ". The length is " << length << ". The cost is " << pCost << "." << endl;
            }
        }

/*******************************************************************************************************/

        else if(choice == 5) {
            vector<Conn> conns(n);
            for(int i = 0; i < n; ++i){
                conns[i].code = codes[i];
                conns[i].cnt = inDegree[i] + (int)adj[i].size();
            }
            sortConns(conns);
            cout << "Airport\tConnections" << endl;
            for(const auto& c : conns) cout << c.code << "\t" << c.cnt << endl;
        }

/*******************************************************************************************************/

        else if (choice == 6) {
            cout << "\n===Undirected Graph G_u ===\n";
            vector<UEdge> undirEdges;
            for(int i = 0; i < n; ++i){
                for(int j = i + 1; j < n; ++j){
                    int c1 = (dirCost[i][j] == INF ? INF : dirCost[i][j]);
                    int c2 = (dirCost[j][i] == INF ? INF : dirCost[j][i]);
                    if(c1 == INF && c2 == INF) continue;
                    int minC = (c1 == INF ? c2 : (c2 == INF ? c1 : min(c1, c2)));
                    undirEdges.push_back({i, j, minC});
                }
            }
            for(const auto& e: undirEdges) {
                cout << codes[e.u] << " -- " << codes[e.v] << "(cost = " << e.cost << ")\n";
            }
            cout << "Total undirected edges in G_u: "<< undirEdges.size() << endl;
        }

/*******************************************************************************************************/

        else if(choice == 7) {
            vector<UEdge> undirEdges;
            for(int i = 0; i < n; ++i){
                for ( int j = i + 1; j < n; ++j) {
                    int c1 = (dirCost[i][j] == INF ? INF : dirCost[i][j]);
                    int c2 = (dirCost[j][i] == INF ? INF : dirCost[j][i]);

                    if(c1 == INF && c2 == INF) continue;
                    int minC = (c1 == INF ? c2 : (c2 == INF ? c1 : min(c1, c2)));
                    undirEdges.push_back({i, j, minC});
                }
            }
            vector<vector<UEdge>> undirAdj(n);
            for(const auto& e : undirEdges) {
                undirAdj[e.u].push_back({e.v, 0, e.cost});
                undirAdj[e.v].push_back({e.u, 0, e.cost});
            }

            vector<int> parent(n, -1);
            vector<int> minC(n, INF);
            vector<bool> inMST(n, false);
            minC[0] = 0;

            for(int i = 0; i < n; ++i){
                int u = -1;
                int minVal = INF;
                for(int j = 0; j < n; ++j){
                    if(!inMST[j] && minC[j] < minVal){
                        minVal = minC[j];
                        u = j;
                    }
                }
                if(u == -1) break;
                inMST[u] = true;
                for(const auto & e : undirAdj[u]){
                    int v = e.u;
                    int w = e.cost;
                    if(!inMST[v] && (int)w < minC[v]){
                        minC[v] = w;
                        parent[v] = u;
                    }
                }
            }
            int connectedCount = 0;
            for(bool b : inMST) if (b) ++connectedCount;
            if(connectedCount < n){
                cout << "The graph is disconnected. An MST cannot be formed."<< endl;
            } else {
                cout << "Minimal Spanning Tree:"<< endl;
                cout << "Edge\tWeight" << endl;
                int totalCost = 0;
                for(int i = 0; i < n; ++i){
                    if(parent[i] != -1){
                        int a = min(parent[i], i);
                        int b = max(parent[i], i);
                        cout << codes[a] << " - " << codes[b] << "\t" << minC[i] << endl;
                        totalCost += minC[i];
                    }
                }
                cout << "Total Cost of MST: " << totalCost << endl;
            }
        }

/*******************************************************************************************************/

        else if(choice == 8) {
            vector<UEdge> undirEdges;
            for(int i = 0; i < n; ++i) {
                for(int j = i + 1; j < n; ++j){
                int c1 = (dirCost[i][j] == INF ? INF : dirCost[i][j]);
                int c2 = (dirCost[j][i] == INF ? INF : dirCost[j][i]);
                if (c1 == INF && c2 == INF) continue;
                int minC = (c1 == INF ? c2 : (c2 == INF ? c1 : min(c1, c2)));
                undirEdges.push_back({i, j, minC});
            }
        }
        sortUEdges(undirEdges);

        vector<int> parent(n);
        for(int i = 0; i < n; ++i) parent[i] = i;

        auto find = [&](int x) -> int {
                int root = x;
                while (parent[root] != root) root = parent[root];
                // path compression
                while (x != root) {
                    int next = parent[x];
                    parent[x] = root;
                    x = next;
                }
                return root;
            };

        vector<UEdge> mstEdges;
        int totalCost = 0;
        for(const auto& e : undirEdges){
            int pu = find(e.u);
            int pv = find(e.v);
            if(pu != pv) {
                parent[pu] = pv;
                mstEdges.push_back(e);
                totalCost += e.cost;
            }
        }

        cout << "Minimal Spanning Tree:" << endl;
        cout << "Edge\tWeight" << endl;
        for (const auto& e : mstEdges) {
            cout << codes [e.u] <<" - "<<codes[e.v] << "\t" << e.cost << endl;
        }
        cout << "Total Cost of MST: " << totalCost << endl;
        if(mstEdges.size() < n - 1) {
            cout << "(This is a minimum spanning forest - graph is disconnected)" << endl;
        }
    }
}
return 0;
}