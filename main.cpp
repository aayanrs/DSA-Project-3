#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <utility>

using namespace std;


//Node class describes a movie or TV show and stores specific data members
struct Node{
    vector<string> genres;
    string title;
    string type; // movie, tv series, etc.
    string tconst; //specific, unique value of where it appears in the tsv file from IMDB
    string year;

    Node(string _tconst, string _type, string _title, string _year, vector<string> _genres){
        type = _type;
        title = _title;
        genres = _genres;
        tconst = _tconst;
        year = _year;
    }
};

class Graph{
private:
    // string as key that is a unique ID of the input from the user in the tsv file

public:
    unordered_map<string,vector<Node*>> graph;
    void insertEdge(string key, Node* to);

};

void Graph::insertEdge(string key, Node* to) {
    graph[key].push_back(to);
}

string readTSVFind(string filename, string& title, string& year, vector<string>& vec, bool& found, Graph& recommendations) {
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
            return tconst;
        }
        if (!found) {
            cout
                    << "Input was not found in database, please double check spelling and that the input and information is correct."
                    << endl;
        }
        inFile.close();
    }

}

int main() {
    Graph recommendations;
    string title;
    cout << "Enter the movie/series you want to find recommendations for: " << endl;
    cout << "Format: Name (Year) " << endl;

    getline(cin, title, '(');
    title = title.substr(0, title.length() - 1);
    string year;
    cin >> year;
    year = year.substr(0,year.length()-1);
    vector<string> vec;
    bool found = false;
    string key = readTSVFind("data.tsv",title,year,vec,found, recommendations); //returns unique value that pertains to the input given from the user


    fstream inFile2("data.tsv");
    if(inFile2.is_open()){
        string line;
        getline(inFile2,line);
        while(getline(inFile2,line)){
            istringstream stream(line);
            string tconst;
            getline(stream,tconst,'\t');
            string titleType;
            getline(stream, titleType, '\t');
            //program only wants movies and tv series
            if(titleType == "tvEpisode" ||titleType ==  "videoGame" ||titleType ==  "video" || titleType == "short" || titleType == "tvSpecial"
               || titleType == "tvMiniSeries" || titleType == "tvMovie"){
                continue;
            }
            string primaryTitle;
            getline(stream, primaryTitle, '\t');
            string originalTitle;
            getline(stream, originalTitle , '\t');
            string isAdult;
            getline(stream,isAdult,'\t');
            int isAdult1 = stoi(isAdult);
            if(isAdult1 == 1){ // this is for inappropriate movies/shows
                continue;
            }
            string startYear;
            getline(stream,startYear,'\t');
            string endYear;
            getline(stream, endYear,'\t');
            string runtime;
            getline(stream, runtime,'\t');
            string genres;
            getline(stream, genres,'\t');
            if(genres.at(1) == 'N'){ //if no genres, its labelled as "\N" so we will continue
                continue;
            }
            int commas = 0;
            for(char genre : genres){
                if(genre == ','){
                    commas++;
                }
            }
            vector<string> genresVector;
            if(commas == 0 ){
                genresVector.push_back(genres);
            }
            if(commas == 1){
                istringstream genre(genres);
                string firstGenre;
                getline(genre,firstGenre,',');
                string secondGenre;
                getline(genre,secondGenre,',');
                genresVector.push_back(firstGenre);
                genresVector.push_back(secondGenre);

            }
            if(commas == 2){
                istringstream genre(genres);
                string firstGenre;
                getline(genre,firstGenre,',');
                string secondGenre;
                getline(genre,secondGenre,',');
                string thirdGenre;
                getline(genre, thirdGenre, ',');
                genresVector.push_back(firstGenre);
                genresVector.push_back(secondGenre);
                genresVector.push_back(thirdGenre);
            }
            if(vec == genresVector){ // checks if the genres are equal so we can give an accurate recommendation
                Node* node = new Node(tconst,titleType,primaryTitle,startYear, genresVector);
                recommendations.insertEdge(key,node);
            }

        }
    }
    for(auto i : recommendations.graph){
        for(auto v : i.second){
            cout << v->title << endl;
        }
    }
    // to limit the amount of recommendations, sort by year closest to or by recommendations by using ratings.tsv
}
