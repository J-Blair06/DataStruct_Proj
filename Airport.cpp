#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <limits>

using namespace std;

const long long INF = 1LL << 60; //used to represent infinity

/*******************************************************************************************************/

//Stack class using templates and vectors
template <typename T>
class Stack {
private:
    vector<T> data;
public:
    void push (const T& val) { data.push_back(val); }
    void pop () { if (!empty()) data.pop_back(); }
    T top() const {return data.back(); }
    bool empty() const { return data.empty(); }
    size_t size() const { return data.size(); }    
};

/*******************************************************************************************************/

// queue class using templates and vectors
template <typename T>
class Queue {
private:
    vector<T> data;
    size_t frontIdx = 0;
public:
    void push(const T& val) { data.push_back(val); }
    void pop() { if (!empty()) ++frontIdx; }
    T front() const { return data[frontIdx]; }
    bool empty() const { return frontIdx >= data.size(); }
    size_t size() const {return data.size() - frontIdx; }
};

/*******************************************************************************************************/

//HeapNode struct
struct HeapNode {
    long long dist;
    int node;
};

/*******************************************************************************************************/

//MinHeap class using templates and vectors
class MinHeap {
private:
    vector<HeapNode> heap;

//heapify up function
    void heapifyUp(size_t idx) {
        while (idx > 0) {
            size_t parent = (idx - 1) / 2;
            if(heap[idx].dist < heap[parent].dist) {
                swap(heap[idx], heap[parent]);
                idx = parent;
            }
            else break;
        }
    }
//heapify down function
    void heapifyDown(size_t idx) {
        size_t size = heap.size();
        while (true) {
            size_t left = 2 * idx + 1;
            size_t right = 2 * idx + 2;
            size_t smallest = idx;

            if(left < size && heap[left].dist < heap[smallest].dist) {
                smallest = left;
            }
            if(right < size && heap[right].dist < heap[smallest].dist) {
                smallest = right;
            }
            if (smallest != idx) {
                swap(heap[idx], heap[smallest]);
                idx = smallest;
            }
            else break;
        }
    }
public:
    void push(HeapNode node) {
        heap.push_back(node);
        heapifyUp(heap.size() - 1);
    }

    HeapNode pop() {
        if (heap.empty()) return {INF, -1};
        HeapNode root = heap[0];
        heap[0] = heap.back();
        heap.pop_back();
        if(!heap.empty()) heapifyDown(0);
        return root;
    }

    bool empty() const { return heap.empty(); }
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
    long long length;
    long long cost;
};

/*******************************************************************************************************/

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

        if(c == "," && !inQuotes){
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

int addAirport(const string& code, const string& city, vector<string>& codes, 
vector<string>& statesv) {
    for (size_t i = 0; i < codes.size(); ++i){
        if (codes[i] == code) return i;
    }
    codes.push_back(code);
    statesv.push_back(takeOutState(city));
    return codes.size() - 1;
}

/*******************************************************************************************************/

//getIndex - returns the index of an airport by its 3 - letter code.
//uses linear search


