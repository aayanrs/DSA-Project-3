#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <queue>
#include <chrono>

int minPopularity = 10000;

using namespace std;

//Node class describes a movie or TV show and stores specific data members
struct Node {
    vector<string> genres;
    string title;
    string type; // movie, tv series, etc.
    string tconst; //specific, unique value of where it appears in the tsv file from IMDB
    string year;

    Node(string _tconst, string _type, string _title, string _year, vector<string> _genres) {
        type = _type;
        title = _title;
        genres = _genres;
        tconst = _tconst;
        year = _year;
    }
};

class Graph {
private:
    // string as key that is a unique ID of the input from the user in the tsv file

public:
    int numVertices = 1;
    unordered_map<string, vector<pair<Node*, double>>> graph;
    void insertEdge(string key, Node* to, double weight);

};

void Graph::insertEdge(string key, Node* to, double weight) {
    graph[key].push_back(make_pair(to, weight));
    numVertices++;
}

vector<double> sortByValue(unordered_map<string,double> &map){
    vector<double> vec;
    for(auto i : map ){
        vec.push_back(i.second);
    }

    sort(vec.begin(), vec.end());

    return vec;
}

vector<string> alignMapValues(unordered_map<string,double> &original, vector<double> &sorted){
    vector<string> ret;
    for(int i = 0; i < sorted.size(); i++){
        for(auto v: original){
            if(sorted.at(i)==v.second){
                ret.push_back(v.first);
            }
        }
    }

    return ret;
}

unordered_map<string, double> dijkstra(Graph& graph, string src) {
    unordered_map<string, bool> visited;
    unordered_map<string, double> distances;
    unordered_map<string, string> previous;

    distances[src] = 0; // Using tconst as unique value for each vertex
    visited[src] = true;

    for (auto iter = graph.graph.begin(); iter != graph.graph.end(); iter++) { // Sets default distances to INT_MAX for all vertices
        for (auto v : iter->second) {
            distances[v.first->tconst] = INT_MAX;
        }
    }

    for (auto v : graph.graph[src]) { // For everything connected to src
        distances[v.first->tconst] = v.second;
        previous[v.first->tconst] = src;
    }

    for (int i = 0; i < graph.numVertices - 1; i++) {
        double smallest = INT_MAX;
        string u;
        for (auto iter = distances.begin(); iter != distances.end(); iter++) { // Finds next smallest distance
            if (visited[iter->first] == false && iter->second <= smallest) {
                smallest = iter->second;
                u = iter->first;
            }
        }
        visited[u] = true;
        for (auto v: graph.graph[u]) {
            if (visited[v.first->tconst] == false && distances[u] + v.second < distances[v.first->tconst]) {
                distances[v.first->tconst] = distances[u] + v.second;
                previous[v.first->tconst] = u;
            }
        }
    }

    return distances;
}

vector<string> bfs(Graph& graph, string src){
    unordered_map<string,bool> visited;
    queue<string>q;
    vector<string> ret;
    visited[src] = true;
    q.push(src);
    while(!q.empty()){
        string u = q.front();
        ret.push_back(u);
        q.pop();
        for(auto v : graph.graph[u]){
            if(!visited[v.first->tconst]){
                visited[v.first->tconst] = true;
                q.push(v.first->tconst);
            }
        }
    }
    return ret;
}

unordered_map<string,Node*> readTitle(string filename){
    unordered_map<string,Node*> titleMap;
    fstream inFile(filename);
    string line;
    getline(inFile,line);
    if (inFile.is_open()) {
        string line;
        getline(inFile, line);
        while (getline(inFile, line)) {
            istringstream stream(line);
            string tconst;
            getline(stream, tconst, '\t');
            string titleType;
            getline(stream, titleType, '\t');
            //program only wants movies and tv series
            if (titleType == "tvEpisode" || titleType == "videoGame" || titleType == "video" || titleType == "short" ||
                titleType == "tvSpecial"
                || titleType == "tvMiniSeries" || titleType == "tvMovie") {
                continue;
            }
            string primaryTitle;
            getline(stream, primaryTitle, '\t');
            string originalTitle;
            getline(stream, originalTitle, '\t');
            string isAdult;
            getline(stream, isAdult, '\t');
            int isAdult1 = stoi(isAdult);
            if (isAdult1 == 1) { // this is for inappropriate movies/shows
                continue;
            }
            string startYear;
            getline(stream, startYear, '\t');

            string endYear;
            getline(stream, endYear, '\t');
            string runtime;
            getline(stream, runtime, '\t');
            string genres;
            getline(stream, genres, '\t');
            if (genres.at(1) == 'N') { //if no genres, its labelled as "\N" so we will continue
                continue;
            }
            int commas = 0;
            for (char genre : genres) {
                if (genre == ',') {
                    commas++;
                }
            }
            vector<string> genresVector;
            if (commas == 0) {
                genresVector.push_back(genres);
            }
            if (commas == 1) {
                istringstream genre(genres);
                string firstGenre;
                getline(genre, firstGenre, ',');
                string secondGenre;
                getline(genre, secondGenre, ',');
                genresVector.push_back(firstGenre);
                genresVector.push_back(secondGenre);

            }
            if (commas == 2) {
                istringstream genre(genres);
                string firstGenre;
                getline(genre, firstGenre, ',');
                string secondGenre;
                getline(genre, secondGenre, ',');
                string thirdGenre;
                getline(genre, thirdGenre, ',');
                genresVector.push_back(firstGenre);
                genresVector.push_back(secondGenre);
                genresVector.push_back(thirdGenre);
            }
            Node *node = new Node(tconst, titleType, primaryTitle, startYear, genresVector);
            titleMap[tconst] = node;
        }
    }

    return titleMap;
}

// !
bool findEntry(string& title, string& year, vector<string>& vec, string& key, unordered_map<string,Node*> titleMap) {

    for(auto i : titleMap){
        if(i.second->title == title && i.second->year == year){

            key = i.first;
            vec = i.second->genres;
            return true;
        }

    }
    return false;

}

//!
Graph makeGraph(bool& ratingsOn, vector<string>& sourceGenres, string& key, unordered_map<string,pair<double,int>>& ratingsMap, unordered_map<string,Node*>& titleMap){
    Graph recommendations;
    for(auto i : titleMap){
        if(i.second->genres == sourceGenres){
            if(ratingsMap[i.first].second < minPopularity){
                continue;
            }
            Node* node = new Node(i.first,i.second->type,i.second->title, i.second->year, i.second->genres);
            if(ratingsOn){
                recommendations.insertEdge(key,node,10-ratingsMap[i.first].first);
            }
            else{
                recommendations.insertEdge(key,node,3000000 - ratingsMap[i.first].second);
            }
        }
    }

    return recommendations;
}

unordered_map<string, pair<double, int>> readRatings(string filename) {
    unordered_map<string, pair<double, int>> ratingsMap;
    fstream inFile(filename);
    string line;

    getline(inFile, line);
    if (!inFile.is_open()) {
        cout << "Ratings file not opened correctly" << endl;
    }
    while (getline(inFile, line)) {
        istringstream stream(line);
        string tconstStr;
        getline(stream, tconstStr, '\t');
        string ratingStr;
        getline(stream, ratingStr, '\t');
        double rating = stod(ratingStr);
        string popularityStr;
        getline(stream, popularityStr, '\t');
        int popularity = stoi(popularityStr);

        ratingsMap[tconstStr] = make_pair(rating, popularity);
    }

    return ratingsMap;
}

int main() {
    // If not found, ask for another input
    string title;
    unordered_map<string, pair<double, int>> ratingsMap = readRatings("ratings.tsv");
    unordered_map<string,Node*> titleMap = readTitle("data.tsv");

    cout << "Welcome to Recommendation Finder!" << endl;
    cout << "Enter the movie/series you want to find recommendations for: " << endl;
    cout << "Format: Name (Year) " << endl;

    getline(cin, title, '(');
    title = title.substr(0, title.length() - 1);
    string year;
    cin >> year;
    year = year.substr(0, year.length() - 1);

    vector<string> sourceGenres;
    string key; //returns unique value that pertains to the input given from the user
    bool found = false;
    while(!found){
        found = findEntry(title,year,sourceGenres,key,titleMap);
        if(found){
            break;
        }
        else{
            cout << "Input was not found, please ensure everything is inputted correctly with the correct information." << endl;
            cout << "Please enter information again: " << endl;
            string extra;
            getline(cin, extra);
            getline(cin, title, '(');
            title = title.substr(0, title.length() - 1);
            cin >> year;
            year = year.substr(0, year.length() - 1);
        }
    }
    cout << "Now, enter what you would like to base your recommendations more strongly towards: Ratings or Popularity " << endl;
    cout << "Type 0 for Ratings, and 1 for Popularity: " << endl;
    int ratOrPop;
    bool ratingsOn;
    cin >> ratOrPop;

    if((ratOrPop) == 0){
        ratingsOn = true;
    }
    else{
        ratingsOn = false;
    }

    Graph recommendations = makeGraph(ratingsOn,sourceGenres,key,ratingsMap, titleMap);

    /*
    cout << "printing graph: " << endl;
    for(auto i : recommendations.graph){
        for(auto v : i.second){
            cout << v.first->title << " " << v.second << endl;
        }
    }
    */

    auto start = chrono::high_resolution_clock::now();

    unordered_map<string, double> test = dijkstra(recommendations, key);
    vector<double> sorted = sortByValue(test);


    cout << "======DIJKSTRA======== " << endl;
    vector<string> dijkstraVector = alignMapValues(test, sorted);
    for(int i = 0; i < 20; i++){
        if(dijkstraVector.at(i) == key){
            continue;
        }
        cout << titleMap[dijkstraVector.at(i)]->title << " " << titleMap[dijkstraVector.at(i)]->year << " " << ratingsMap[dijkstraVector.at(i)].first << endl;
    }
    auto finish = chrono::high_resolution_clock::now();
    auto execTime = chrono::duration_cast<chrono::microseconds>(finish-start);
    cout << "Time for Dijkstra's algorithm: " << execTime.count() << " microseconds" << endl;
    // to limit the amount of recommendations, sort by year closest to or by recommendations by using ratings.tsv
    // idea: weight as popularity or ratings, bfs vs dijkstra
    // make boolean where user can choose if they want it based on ratings or popularity
    //bfs when we come back
    cout << "======= BFS =======" << endl;
    start = chrono::high_resolution_clock::now();
    vector<string> bfsVector = bfs(recommendations, key);
    for(int i = 0; i < 20; i++){
        if(bfsVector.at(i) == key){
            continue;
        }
        cout << titleMap[bfsVector.at(i)]->title << " " << titleMap[bfsVector.at(i)]->year << " " << ratingsMap[bfsVector.at(i)].first << endl;
    }
    finish = chrono::high_resolution_clock::now();
    execTime = chrono::duration_cast<chrono::microseconds>(finish-start);
    cout << "Time for BFS: " << execTime.count() << " microseconds" << endl;

    return 0;

}
