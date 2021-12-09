#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <queue>
#include <chrono>
#include <SFML/Graphics.hpp>
#include "TextureManager.h"

using namespace std;
int minPopularity = 10000;
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

vector<double> sortByValue(unordered_map<string, double>& map) {
    vector<double> vec;
    for (auto i : map) {
        vec.push_back(i.second);
    }

    sort(vec.begin(), vec.end());

    return vec;
}

vector<string> alignMapValues(unordered_map<string, double>& original, vector<double>& sorted) {
    vector<string> ret;
    for (int i = 0; i < sorted.size(); i++) {
        for (auto v : original) {
            if (sorted.at(i) == v.second) {
                if (find(ret.begin(), ret.end(), v.first) != ret.end()) {
                    continue;
                }
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
        for (auto v : graph.graph[u]) {
            if (visited[v.first->tconst] == false && distances[u] + v.second < distances[v.first->tconst]) {
                distances[v.first->tconst] = distances[u] + v.second;
                previous[v.first->tconst] = u;
            }
        }
    }

    return distances;
}

vector<string> bfs(Graph& graph, string src) {
    unordered_map<string, bool> visited;
    queue<string>q;
    vector<string> ret;
    visited[src] = true;
    q.push(src);
    while (!q.empty()) {
        string u = q.front();
        ret.push_back(u);
        q.pop();
        for (auto v : graph.graph[u]) {
            if (!visited[v.first->tconst]) {
                visited[v.first->tconst] = true;
                q.push(v.first->tconst);
            }
        }
    }
    return ret;
}

unordered_map<string, Node*> readTitle(string filename) {
    unordered_map<string, Node*> titleMap;
    fstream inFile(filename);
    string line;
    getline(inFile, line);
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
            Node* node = new Node(tconst, titleType, primaryTitle, startYear, genresVector);
            titleMap[tconst] = node;
        }
    }
    return titleMap;
}

// !
bool findEntry(string& title, string& year, vector<string>& vec, string& key, unordered_map<string, Node*> titleMap) {
    for (auto i : titleMap) {
        if (i.second->title == title && i.second->year == year) {

            key = i.first;
            vec = i.second->genres;
            return true;
        }

    }
    return false;

}

//!
Graph makeGraph(bool& ratingsOn, vector<string>& sourceGenres, string& key, unordered_map<string, pair<double, int>>& ratingsMap, unordered_map<string, Node*>& titleMap) {
    Graph recommendations;
    for (auto i : titleMap) {
        if (i.second->genres == sourceGenres) {
            if (ratingsMap[i.first].second < minPopularity) {
                continue;
            }
            Node* node = new Node(i.first, i.second->type, i.second->title, i.second->year, i.second->genres);
            if (ratingsOn) {
                recommendations.insertEdge(key, node, 10 - ratingsMap[i.first].first);
            }
            else {
                recommendations.insertEdge(key, node, 3000000 - ratingsMap[i.first].second);
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
std::string findReccomendations(sf::String& title, sf::String& year, bool sort, bool algorithm, int& state, unordered_map<string, Node*>& titleMap, unordered_map<string, pair<double, int>>& ratingsMap) {
    std::string searchTitle = title.toAnsiString();
    std::string searchYear = year.toAnsiString();
    vector<string> sourceGenres;
    string key; //returns unique value that pertains to the input given from the user
    bool found = false;
    std::string results;
    while (!found) {
        found = findEntry(searchTitle, searchYear, sourceGenres, key, titleMap);
        if (found) {
            break;
        }
        else {
            return "Movie was not found. Please ensure the title and year were\ninputted correctly.";
        }
    }
    Graph recommendations = makeGraph(sort, sourceGenres, key, ratingsMap, titleMap);
    int quantity = 0;
    auto start = chrono::high_resolution_clock::now();
    if (algorithm == 0) {
        unordered_map<string, double> test = dijkstra(recommendations, key);
        vector<double> sorted = sortByValue(test);
        vector<string> dijkstraVector = alignMapValues(test, sorted);
        quantity = (dijkstraVector.size() < 20) ? dijkstraVector.size() : 20;
        for (int i = 0; i < quantity; i++) {
            if (dijkstraVector.at(i) == key) {
                continue;
            }
            results += titleMap[dijkstraVector.at(i)]->title + " (" + titleMap[dijkstraVector.at(i)]->year + ") " + to_string(ratingsMap[dijkstraVector.at(i)].first).substr(0, 4) + '\n';
        }
    }
    // to limit the amount of recommendations, sort by year closest to or by recommendations by using ratings.tsv
    // idea: weight as popularity or ratings, bfs vs dijkstra
    // make boolean where user can choose if they want it based on ratings or popularity
    //bfs when we come back
    else {
        vector<string> bfsVector = bfs(recommendations, key);
        quantity = (bfsVector.size() < 20) ? bfsVector.size() : 20;
        for (int i = 0; i < quantity; i++) {
            if (bfsVector.at(i) == key) {
                continue;
            }
            results += titleMap[bfsVector.at(i)]->title + " (" + titleMap[bfsVector.at(i)]->year + ") " + to_string(ratingsMap[bfsVector.at(i)].first).substr(0, 4) + '\n';
        }

    }
    auto finish = chrono::high_resolution_clock::now();
    auto execTime = chrono::duration_cast<chrono::microseconds>(finish - start);
    
    return results + "Time for  algorithm: " + to_string(execTime.count()) + " microseconds\n";
}

int main() {
    const int WIDTH = 1920;
    const int HEIGHT = 1080;
    unordered_map<string, Node*> titleMap = readTitle("databases/data.tsv");
    unordered_map<string, pair<double, int>> ratingsMap = readRatings("databases/ratings.tsv");
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Sure-Shot Show Selector");
    sf::Sprite sprites[20];
    sf::Font arial;
    sf::Font modenine;
    sf::String titleInput = "Title";
    sf::String yearInput = "Year";
    sf::String output;
    sf::Text titleBox;
    sf::Text yearBox;
    sf::Text outputBox;
    int state = 0;
    // 1 for ratings, 0 for popularity
    bool sort = 1;
    // 1 for BFS, 0 for Dijkstra
    bool algorithm = 1;

    // Import background image (background.jpg)
    sprites[0].setTexture(TextureManager::getTexture("background.jpg"));
    sprites[0].setScale(WIDTH / sprites[0].getLocalBounds().width, HEIGHT / sprites[0].getLocalBounds().height);

    // Import tint visual (tint.png)
    sprites[1].setTexture(TextureManager::getTexture("glass.png"));
    sprites[1].setScale(WIDTH / sprites[1].getLocalBounds().width, HEIGHT / sprites[1].getLocalBounds().height);

    // Import glass visual (tint.png)
    sprites[2].setTexture(TextureManager::getTexture("tint.png"));
    sprites[2].setScale(WIDTH / sprites[2].getLocalBounds().width, HEIGHT / sprites[2].getLocalBounds().height);

    //Import screen visual (pane.png)
    sprites[3].setTexture(TextureManager::getTexture("pane.png"));
    sprites[3].setScale(WIDTH / sprites[3].getLocalBounds().width, HEIGHT / sprites[3].getLocalBounds().height);

    //Import logo visual (logo.png)
    sprites[4].setTexture(TextureManager::getTexture("logo.png"));
    sprites[4].setPosition(WIDTH / 6, HEIGHT / 6);

    //Import slogan visual (slogan.png)
    sprites[5].setTexture(TextureManager::getTexture("slogan.png"));
    sprites[5].setOrigin(sprites[5].getLocalBounds().width / 2, sprites[5].getLocalBounds().height / 2);
    sprites[5].setPosition(WIDTH / 2, 161 * HEIGHT / 200);

    //Import target visual (target.png)
    sprites[6].setTexture(TextureManager::getTexture("target.png"));
    sprites[6].setOrigin(sprites[6].getLocalBounds().width / 2, sprites[6].getLocalBounds().height / 2);
    sprites[6].setPosition(WIDTH / 2, HEIGHT / 2);

    //Import title input textbox background (textbox.png)
    sprites[7].setTexture(TextureManager::getTexture("textbox.png"));
    sprites[7].setScale(WIDTH / sprites[7].getLocalBounds().width / 1.8, 0.3);
    sprites[7].setOrigin(sprites[7].getLocalBounds().width / 2, 0);
    sprites[7].setPosition(WIDTH / 2, 197 * HEIGHT / 320);

    //Import year input textbox background (textbox.png)
    sprites[8].setTexture(TextureManager::getTexture("textbox.png"));
    sprites[8].setScale(WIDTH / sprites[8].getLocalBounds().width / 1.8, 0.3);
    sprites[8].setOrigin(sprites[8].getLocalBounds().width / 2, 0);
    sprites[8].setPosition(WIDTH / 2, 11 * HEIGHT / 16);

    //Import checked checkbox (checked.png)
    sprites[9].setTexture(TextureManager::getTexture("checked.png"));
    sprites[9].setScale(0.5, 0.5);
    sprites[9].setPosition(11 * WIDTH / 65, 2 * HEIGHT / 5);

    //Import unchecked checkbox (unchecked.png)
    sprites[10].setTexture(TextureManager::getTexture("unchecked.png"));
    sprites[10].setScale(0.5, 0.5);
    sprites[10].setPosition(11 * WIDTH / 65, 37 * HEIGHT / 80);

    //Import sort header text (sort_header.png)
    sprites[11].setTexture(TextureManager::getTexture("sort_header.png"));
    sprites[11].setPosition(111 * WIDTH / 650, 11 * HEIGHT / 32);

    //Import ratings label text (ratings.png)
    sprites[12].setTexture(TextureManager::getTexture("ratings.png"));
    sprites[12].setScale(0.5, 0.5);
    sprites[12].setPosition(WIDTH / 5, 131 * HEIGHT / 320);

    //Import popularity label text (popularity.png)
    sprites[13].setTexture(TextureManager::getTexture("popularity.png"));
    sprites[13].setScale(0.5, 0.5);
    sprites[13].setPosition(WIDTH / 5, 151 * HEIGHT / 320);

    //Import checked algorithm checkbox (a_checked.png)
    sprites[14].setTexture(TextureManager::getTexture("a_checked.png"));
    sprites[14].setScale(0.5, 0.5);
    sprites[14].setPosition(46 * WIDTH / 65, 2 * HEIGHT / 5);

    //Import unchecked algorithm checkbox (a_unchecked.png)
    sprites[15].setTexture(TextureManager::getTexture("a_unchecked.png"));
    sprites[15].setScale(0.5, 0.5);
    sprites[15].setPosition(46 * WIDTH / 65, 37 * HEIGHT / 80);

    //Import BFS label text (bfs.png)
    sprites[16].setTexture(TextureManager::getTexture("bfs.png"));
    sprites[16].setScale(0.5, 0.5);
    sprites[16].setPosition(37 * WIDTH / 50, 131 * HEIGHT / 320);

    //Import dijkstra label text (dijkstra.png)
    sprites[17].setTexture(TextureManager::getTexture("dijkstra.png"));
    sprites[17].setScale(0.5, 0.5);
    sprites[17].setPosition(37 * WIDTH / 50, 151 * HEIGHT / 320);

    //Import algorithm header text (algorithm_header.png)
    sprites[18].setTexture(TextureManager::getTexture("algorithm_header.png"));
    sprites[18].setPosition(46 * WIDTH / 65, 11 * HEIGHT / 32);

    //Import display visual (display.png)
    sprites[19].setTexture(TextureManager::getTexture("display.png"));
    sprites[19].setScale(0.35, 0.35);
    sprites[19].setOrigin(sprites[19].getLocalBounds().width / 2, sprites[19].getLocalBounds().height / 2);
    sprites[19].setPosition(WIDTH / 2, 90 * HEIGHT / 200);

    //Import font (arial.ttf and modenine.ttf)
    arial.loadFromFile("fonts/arial.ttf");
    modenine.loadFromFile("fonts/modenine.ttf");

    //Set up title input textbox
    titleBox.setFont(arial);
    titleBox.setPosition(WIDTH / 2, 19 * HEIGHT / 30);

    //Set up year textbox
    yearBox.setFont(arial);
    yearBox.setPosition(WIDTH / 2, 17 * HEIGHT / 24);

    //Set up output textbox
    outputBox.setFont(modenine);
    outputBox.setCharacterSize(11);
    outputBox.setPosition(WIDTH / 2 - WIDTH / 9.5, 3.05 * HEIGHT / 9);
    outputBox.setFillColor(sf::Color::White);

    while (window.isOpen()) {
        sf::Event event;
        sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (sprites[9].getGlobalBounds().contains(mousePosition.x, mousePosition.y)) {
                        sort = 1;
                        sprites[9].setTexture(TextureManager::getTexture("checked.png"));
                        sprites[10].setTexture(TextureManager::getTexture("unchecked.png"));
                        if (state == 3) {
                            output = findReccomendations(titleInput, yearInput, sort, algorithm, state, titleMap, ratingsMap);
                            outputBox.setString("Reccomendations:\n" + output);
                        }
                    }
                    else if (sprites[10].getGlobalBounds().contains(mousePosition.x, mousePosition.y)) {
                        sort = 0;
                        sprites[10].setTexture(TextureManager::getTexture("checked.png"));
                        sprites[9].setTexture(TextureManager::getTexture("unchecked.png"));
                        if (state == 3) {
                            output = findReccomendations(titleInput, yearInput, sort, algorithm, state, titleMap, ratingsMap);
                            outputBox.setString("Reccomendations:\n" + output);
                        }
                    }
                    if (sprites[14].getGlobalBounds().contains(mousePosition.x, mousePosition.y)) {
                        algorithm = 1;
                        sprites[14].setTexture(TextureManager::getTexture("a_checked.png"));
                        sprites[15].setTexture(TextureManager::getTexture("a_unchecked.png"));
                        if (state == 3) {
                            output = findReccomendations(titleInput, yearInput, sort, algorithm, state, titleMap, ratingsMap);
                            outputBox.setString("Reccomendations:\n" + output);
                        }
                    }
                    else if (sprites[15].getGlobalBounds().contains(mousePosition.x, mousePosition.y)) {
                        algorithm = 0;
                        sprites[15].setTexture(TextureManager::getTexture("a_checked.png"));
                        sprites[14].setTexture(TextureManager::getTexture("a_unchecked.png"));
                        if (state == 3) {
                            output = findReccomendations(titleInput, yearInput, sort, algorithm, state, titleMap, ratingsMap);
                            outputBox.setString("Reccomendations:\n" + output);
                        }
                    }
                }
            }
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::BackSpace) {
                    if (yearInput.getSize() == 0 && state == 2 || state == 3) --state;
                    if (state == 1 && titleInput.getSize() > 0) {
                        titleInput = titleInput.substring(0, titleInput.getSize() - 1);
                        titleBox.setOrigin(titleBox.getLocalBounds().width / 2, 0);
                    }
                    else if (state == 2) {
                        yearInput = yearInput.substring(0, yearInput.getSize() - 1);
                        yearBox.setOrigin(yearBox.getLocalBounds().width / 2, 0);
                    }
                }
                if (event.key.code == sf::Keyboard::Enter && state < 3) { ++state; }
                if (state == 3) {
                    output = findReccomendations(titleInput, yearInput, sort, algorithm, state, titleMap, ratingsMap);
                    outputBox.setString("Reccomendations:\n" + output);
                }
            }
            else if (event.type == sf::Event::TextEntered) {
                if (state == 0) {
                    titleInput = yearInput = "";
                    ++state;
                }
                if (std::isprint(event.text.unicode)) {
                    if (state == 1) {
                        titleInput += event.text.unicode;
                        titleBox.setString(titleInput);
                        titleBox.setOrigin(titleBox.getLocalBounds().width / 2, 0);
                    }
                    else if (state == 2) {
                        yearInput += event.text.unicode;
                        yearBox.setString(yearInput);
                        yearBox.setOrigin(yearBox.getLocalBounds().width / 2, 0);
                    }
                }
            }
        }
        titleBox.setString(titleInput);
        yearBox.setString(yearInput);
        window.clear();
        window.draw(sprites[0]);
        window.draw(sprites[1]);
        window.draw(sprites[2]);
        window.draw(sprites[3]);
        window.draw(sprites[4]);
        window.draw(sprites[5]);
        window.draw(sprites[6]);
        window.draw(sprites[7]);
        window.draw(sprites[8]);
        window.draw(sprites[9]);
        window.draw(sprites[10]);
        window.draw(sprites[11]);
        window.draw(sprites[12]);
        window.draw(sprites[13]);
        window.draw(sprites[14]);
        window.draw(sprites[15]);
        window.draw(sprites[16]);
        window.draw(sprites[17]);
        window.draw(sprites[18]);
        window.draw(titleBox);
        window.draw(yearBox);
        if (state == 3) {
            window.draw(sprites[19]);
            window.draw(outputBox);
        }
        window.display();
    }
    return 0;
}