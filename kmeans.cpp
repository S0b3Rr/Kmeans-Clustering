#include <algorithm>
#include <cmath>
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <float.h>
#include <iomanip>

using namespace std;

class Point {
private:
    int pointId, clusterId;
    int dimensions; //used to store the dimension of the point
    vector<double> values; //store the coordinates of the point

    vector<double> inputToVector(string& line) {//store each points's value into a vector from input.txt
        string temp = "";
        vector<double> values;

        for (int i = 0; i < line.size(); i++) {
            if ((48 <= int(line[i]) && int(line[i]) <= 57) || line[i] == '.' || line[i] == '+' || line[i] == '-' || line[i] == 'e') {
                //supports different kinds of value inputs
                temp += line[i];
            }
            else if (temp.size() > 0) { 
                values.push_back(stod(temp)); //stod() -- Convert string to double
                temp = "";
            }
        }
        if (temp.size() > 0) { 
            values.push_back(stod(temp));
            temp = "";
        }
        return values;
    }

public:
    Point(int id, string line) { //Constructor
        pointId = id;
        values = inputToVector(line);
        dimensions = values.size(); //get dimension by the number of the coordinates of the value
        clusterId = 0; // set 0 to mean that the point is not assigned to any of the cluster initially
    }

    int getDimensions() {
        return dimensions;
    }
    int getCluster() {
        return clusterId;
    }
    int getID() {
        return pointId;
    }
    void setCluster(int cId) {
        clusterId = cId;
    }
    double getVal(int axis) {
        return values[axis];
    }
};

class Cluster {
private:
    int clusterId;
    vector<double> centroid;
    vector<Point> points;

public:
    Cluster(int cId, Point centro) { //Constructor
        clusterId = cId;
        for (int i = 0; i < centro.getDimensions(); i++) {
            centroid.push_back(centro.getVal(i));
        }
        this -> addPoint(centro);
    }

    void addPoint(Point p) {  //add a point to the cluster
        p.setCluster(this -> clusterId);
        points.push_back(p);
    }

    void removeAllPoints() {
        points.clear();
    }
    int getId() {
        return clusterId;
    }
    Point getPoint(int index) { //get the point by index in the cluster
        return points[index];
    }
    int getSize() { //get the size of the cluster
        return points.size();
    }
    double getCentroidByAxis(int axis) {
        return centroid[axis];
    }
    void setCentroidByIndex(int axis, double val) {
        this -> centroid[axis] = val;
    }
};

class KMeans {
private:
    int K, dimensions, numOfAllPoints;
    vector<Cluster> clusters;
    
    void clearClusters() {
        for (int i = 0; i < K; i++) {
            clusters[i].removeAllPoints();
        }
    }

    int getNearestClusterId(Point point) {
        double sum = 0.0, min_dist = DBL_MAX; //set the initial min distance to be max
        int NearestClusterId;

        //loop to find 
        for (int i = 0; i < K; i++) {
            double dist;
            sum = 0.0;

            if (dimensions == 1) {
                min_dist = abs(clusters[i].getCentroidByAxis(0) - point.getVal(0));
            }
            else {
                for (int j = 0; j < dimensions; j++) 
                    sum += pow(clusters[i].getCentroidByAxis(j) - point.getVal(j), 2.0);
                dist = sqrt(sum);
            }
            if (dist < min_dist) { //compare if the current dist is smaller than min dist
                min_dist = dist;
                NearestClusterId = clusters[i].getId();
            }
        }
        return NearestClusterId;
    }

public:
    KMeans(int K) {
        this -> K = K;
    }

    //execute k-mean cluerting
    void execute(vector<Point>& allPoints, vector<int>& centroids) {
        numOfAllPoints = allPoints.size();
        dimensions = allPoints[0].getDimensions();

        vector<int> processedCentroidIds;
        auto centroidPtr = centroids.begin();
        cout << endl;
        
        //initialize clusters and set centroid of clusters
        for (int i = 1; i <= K; i++) {
            while (true) {
                int index;
                if (centroidPtr != centroids.end() && !centroids.empty()) { //check if the centroids are specified
                    index = *centroidPtr - 1; //set controied to be user specified
                    //index = *centroidPtr - 1; is set to be 1-indexed to satisfy the requirment of the assignment
                    if (centroidPtr != centroids.end())
                        centroidPtr++;
                }
                else 
                    index = rand() % numOfAllPoints; //no specified centroid, generate centroids randomly

                if (find(processedCentroidIds.begin(), processedCentroidIds.end(), index) == processedCentroidIds.end()) { 
                    processedCentroidIds.push_back(index);
                    allPoints[index].setCluster(i);
                    Cluster cluster(i, allPoints[index]); // create cluster instance and pass the centroid to the constructer
                    cout << "Cluster " << i << " initialized with centroid : ";
                    for (int j = 0; j < dimensions; j++) {
                        cout << allPoints[index].getVal(j) << " ";
                    }
                    cout << endl;
                    clusters.push_back(cluster);
                    break;
                }
            }
        }
        cout << "Clusters initialized successfully with K = " << clusters.size() << endl << endl;

        //iterate: continuesly do k means clustering
        while (true) {
            bool done = true;
            for (int i = 0; i < numOfAllPoints; i++) {
                int currentClusterId = allPoints[i].getCluster();
                int nearestClusterId = getNearestClusterId(allPoints[i]);

                if (currentClusterId != nearestClusterId) {
                    allPoints[i].setCluster(nearestClusterId);
                    done = false;
                }
            }

            //clear all clusters to relocate all points into empty clusters
            clearClusters();

            //relocate all points into their clusters respectively
            for (int i = 0; i < numOfAllPoints; i++) 
                clusters[allPoints[i].getCluster() - 1].addPoint(allPoints[i]);  // cluster index is 1-indexed

            //recalculate the centroid of each cluster
            for (int i = 0; i < K; i++) {
                int clusterSize = clusters[i].getSize();

                for (int j = 0; j < dimensions; j++) {
                    double sum = 0.0;
                    if (clusterSize > 0) {
                        for (int n = 0; n < clusterSize; n++) 
                            sum += clusters[i].getPoint(n).getVal(j);
                        clusters[i].setCentroidByIndex(j, sum / (double)clusterSize);
                    }
                }
            }

            //check if all points are in its nearest cluster. if so, we are finished with clustering
            if (done)
                break;
        }

        //print the answer to the consel. 
        for (int i = 0; i < K; i++) {
            cout << "Cluster " << clusters[i].getId() << " centroid : ";
            cout << setprecision(5);
            for (int j = 0; j < dimensions; j++)
                cout << clusters[i].getCentroidByAxis(j) << " "; 
            cout << endl;
        }
    }
};

//Tested on department's gateway server: gateway.cs.cityu.edu.hk
int main(int argc, char** argv) {
    
    //Input example: ./kmeans input.txt 3 Y
    if (argc != 4) {
        cout << "Warning: wrong instruction. Please retry. \n ./kmeans <INPUT FILEPATH NAME> <K> <IF_SPECIFY_CENTROID(Y/N)>" << endl;
        return 1;
    }

    string fileName = argv[1];
    int K = atoi(argv[2]); //atoi() -- Convert string to integer
    string ifSpecifyCentroid = argv[3];

    ifstream inFile(fileName.c_str());
    if (!inFile.is_open()) {
        cout << "Error: File cannot be opened." << endl;
        return 1;
    }

    int pointId = 1;
    vector<Point> allPoints;
    string line;

    //process the input file line by line
    while (getline(inFile, line)) {
        Point point(pointId, line);
        allPoints.push_back(point);
        pointId++;
    }

    inFile.close();

    vector<int> centroidIndex;

    if (ifSpecifyCentroid == "Y") { // check if centroids are specified
        cout << "Please input the index of your initial centroid (input -1 to finish): " << endl;
        int ind;
        while (cin >> ind) {
            if (ind == -1)
                break;
            centroidIndex.push_back(ind);
        }
    }

    if (allPoints.size() < K) {
        cout << "Error: Number of clusters is greater than number of points." << endl;
        return 1;
    }

    KMeans kmeans(K);
    kmeans.execute(allPoints, centroidIndex);
    return 0;
}