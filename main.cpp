#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <queue>

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

    /*
    vector<bool> visited(graph.numVertices, false);
    vector<int> d(graph.numVertices, INT_MAX);
    vector<int> p(graph.numVertices, -1);

    d[0] = 0; //src vertex is index 0
    visited[0] = true;

    for (int i = 1; i < graph.graph[src->tconst].size(); i++) {
        //
        d[i] = graph.graph[src->tconst][i - 1].second; //ex: index of 1 is the first thing the src is connected to, but the weight is attributed with the index preceding it (i-1)
        p[i] = 0; //aka setting to src
    }
    for (int i = 0; i < graph.numVertices - 1; i++) {
        int smallest = INT_MAX;
        int u;
        for (int j = 0; j < graph.numVertices; j++) {
            if (!visited[j] && d[j] <= smallest) {
                smallest = d[j];
                u = j;
            }
        }
        visited[u] = true;
        for (int v = 0; v < graph.graph[graph.graph[src->tconst][u - 1].first->tconst].size(); v++) {

        }
        // graph.graph[graph.graph[src->tconst][u-1].first->tconst]
        /*
        for (auto v: graph.graph[graph.graph[src->tconst][u-1].first->tconst]) { //this is accessing nodes adjacent to u
            if (visited[v.first] == false && d[u] + v.second < d[v.first]) {
                d[v.first] = d[u] + v.second;
                p[v.first] = u;
            }
        }
         */
    //  }
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

bool readTSVFind(string filename, string& title, string& year, vector<string>& vec, bool& found, string& key) {
    fstream inFile(filename);
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

            if (startYear != year || primaryTitle != title) {
                continue;

            }
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
            found = true;
            vec = genresVector;
            key = tconst;
            return found;
        }

        inFile.close();
    }
    return false;
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

Graph makeGraph(bool& ratingsOn, string filename, vector<string>& sourceGenres, string& key, int& minPopularity, unordered_map<string,pair<double,int>>& ratingsMap){
    Graph recommendations;
    fstream inFile(filename);
    if (inFile.is_open()) {
        string line;
        getline(inFile, line);
        while (getline(inFile, line)) {
            istringstream stream(line);
            string tconst;
            getline(stream, tconst, '\t');
            if (ratingsMap[tconst].second <= minPopularity) {
                continue;
            }
            string titleType;
            getline(stream, titleType, '\t');
            //program only wants movies and tv series
            if (titleType == "tvEpisode" || titleType == "videoGame" || titleType == "video" || titleType == "short" || titleType == "tvSpecial"
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
            if (sourceGenres == genresVector) { // checks if the genres are equal so we can give an accurate recommendation
                Node* node = new Node(tconst, titleType, primaryTitle, startYear, genresVector);
                if(ratingsOn){
                    recommendations.insertEdge(key, node, 10 - ratingsMap[tconst].first); //highest rated = least weight
                }
                else{
                    recommendations.insertEdge(key, node, 3000000 - ratingsMap[tconst].second); //most popular = least weight
                }
            }
        }
    }
    return recommendations;
}

int main() {
    // If not found, ask for another input
    string title;
    unordered_map<string, pair<double, int>> ratingsMap = readRatings("ratings.tsv");
    cout << "Welcome to Recommendation Finder!" << endl;

    cout << "Enter the movie/series you want to find recommendations for: " << endl;
    cout << "Format: Name (Year) " << endl;
    int minPopularity = 10000;
    getline(cin, title, '(');
    title = title.substr(0, title.length() - 1);
    string year;
    cin >> year;
    year = year.substr(0, year.length() - 1);

    vector<string> sourceGenres;
    string key; //returns unique value that pertains to the input given from the user
    bool found = false;
    while(!found){ // yo fix this shit
        found = readTSVFind("data.tsv", title, year, sourceGenres, found, key);
        if(found){
            break;
        }
        else{
            cout << "Input was not found, please ensure everything is inputted correctly with the correct information." << endl;
            cout << "Please enter information again: " << endl;
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
    Graph recommendations = makeGraph(ratingsOn,"data.tsv",sourceGenres,key,minPopularity,ratingsMap);

    for (auto i : recommendations.graph) {
        for (auto v : i.second) {
            cout << "Title: " << v.first->title << endl;
            cout << "tconst: " << v.first->tconst << endl;
            cout << "Weight: " << v.second << endl;
        }
        cout << "Size: " << recommendations.numVertices << endl;
    }
    unordered_map<string, double> test = dijkstra(recommendations, key);
    vector<double> sorted = sortByValue(test);
    cout << "======ALIGN VALUE CHECK======== " << endl;
    vector<string> test2 = alignMapValues(test,sorted);

    for(int i = 0; i < 20; i++){
        cout << test2.at(i) << " " << sorted.at(i) << endl;
    }
    // to limit the amount of recommendations, sort by year closest to or by recommendations by using ratings.tsv
    // idea: weight as popularity or ratings, bfs vs dijkstra
    // make boolean where user can choose if they want it based on ratings or popularity
    //bfs when we come back
    cout << "======= BFS =======" << endl;
    vector<string> test3 = bfs(recommendations,key);
    for(int i = 0; i < test3.size(); i++){
        cout << test3.at(i)<< endl;
    }
    // For both dfs and dijkstra, it stores the src at the front of the vector, so take note of this
    // add the couts so that it couts the title, the year, and rating as a list that goes from like 1 to 20 or less or whatever
    // ex 1. Naruto, 2002, 8.3
    return 0;

}
