/*
 * Application Mapping Optimizer for Embedded System Design
 * =========================================================
 *
 * This program implements an optimization model for mapping applications to
 * heterogeneous cores in an embedded system. The objective is to minimize
 * total energy consumption while satisfying memory capacity and deadline constraints.
 *
 * Author: Educational Example for ESD Course
 * Date: March 2026
 *
 * Compile:
 *   g++ -std=c++11 -I${GUROBI_HOME}/include -L${GUROBI_HOME}/lib \
 *       application_mapping_cpp.cpp -o application_mapping \
 *       -lgurobi_c++ -lgurobi130 -lm
 *
 * Run:
 *   ./application_mapping application_mapping_data.txt
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include "gurobi_c++.h"

using namespace std;


/**
 * Class to represent the problem data
 */
class ProblemData {
public:
    int num_applications;
    int num_cores;
    vector<string> application_names;
    vector<string> core_names;
    vector<vector<double>> energy_matrix;      // [app][core]
    vector<vector<double>> time_matrix;        // [app][core]
    vector<double> memory_requirements;        // [app]
    vector<double> memory_capacity;            // [core]
    vector<double> deadlines;                  // [app]
    
    ProblemData() : num_applications(0), num_cores(0) {}
    
    /**
     * Read problem data from file
     */
    bool readFromFile(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: Could not open file " << filename << endl;
            return false;
        }
        
        string line;
        while (getline(file, line)) {
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#') continue;
            
            istringstream iss(line);
            string keyword;
            iss >> keyword;
            
            if (keyword == "NUM_APPLICATIONS") {
                iss >> num_applications;
            }
            else if (keyword == "NUM_CORES") {
                iss >> num_cores;
            }
            else if (keyword == "APPLICATION_NAMES") {
                application_names.clear();
                for (int i = 0; i < num_applications; i++) {
                    getline(file, line);
                    if (!line.empty() && line[0] != '#') {
                        application_names.push_back(line);
                    }
                }
            }
            else if (keyword == "CORE_NAMES") {
                core_names.clear();
                for (int i = 0; i < num_cores; i++) {
                    getline(file, line);
                    if (!line.empty() && line[0] != '#') {
                        core_names.push_back(line);
                    }
                }
            }
            else if (keyword == "ENERGY_MATRIX") {
                energy_matrix.clear();
                energy_matrix.resize(num_applications);
                for (int i = 0; i < num_applications; i++) {
                    getline(file, line);
                    istringstream row_stream(line);
                    double value;
                    while (row_stream >> value) {
                        energy_matrix[i].push_back(value);
                    }
                }
            }
            else if (keyword == "TIME_MATRIX") {
                time_matrix.clear();
                time_matrix.resize(num_applications);
                for (int i = 0; i < num_applications; i++) {
                    getline(file, line);
                    istringstream row_stream(line);
                    double value;
                    while (row_stream >> value) {
                        time_matrix[i].push_back(value);
                    }
                }
            }
            else if (keyword == "MEMORY_REQUIREMENTS") {
                memory_requirements.clear();
                for (int i = 0; i < num_applications; i++) {
                    getline(file, line);
                    if (!line.empty() && line[0] != '#') {
                        memory_requirements.push_back(stod(line));
                    }
                }
            }
            else if (keyword == "MEMORY_CAPACITY") {
                memory_capacity.clear();
                for (int i = 0; i < num_cores; i++) {
                    getline(file, line);
                    if (!line.empty() && line[0] != '#') {
                        memory_capacity.push_back(stod(line));
                    }
                }
            }
            else if (keyword == "DEADLINES") {
                deadlines.clear();
                for (int i = 0; i < num_applications; i++) {
                    getline(file, line);
                    if (!line.empty() && line[0] != '#') {
                        deadlines.push_back(stod(line));
                    }
                }
            }
        }
        
        file.close();
        return true;
    }
    
    /**
     * Validate the data
     */
    bool validate() const {
        if (num_applications <= 0 || num_cores <= 0) {
            cerr << "Error: Invalid problem dimensions" << endl;
            return false;
        }
        if ((int)application_names.size() != num_applications) {
            cerr << "Error: Application names count mismatch" << endl;
            return false;
        }
        if ((int)core_names.size() != num_cores) {
            cerr << "Error: Core names count mismatch" << endl;
            return false;
        }
        if ((int)energy_matrix.size() != num_applications) {
            cerr << "Error: Energy matrix row count mismatch" << endl;
            return false;
        }
        for (int i = 0; i < num_applications; i++) {
            if ((int)energy_matrix[i].size() != num_cores) {
                cerr << "Error: Energy matrix column count mismatch at row " << i << endl;
                return false;
            }
        }
        return true;
    }
};


/**
 * Class to manage the optimization model
 */
class ApplicationMappingOptimizer {
private:
    GRBEnv* env;
    GRBModel* model;
    const ProblemData* data;
    
    // Decision variables: x[i][j] = 1 if app i is assigned to core j
    vector<vector<GRBVar>> x;
    
public:
    /**
     * Constructor
     */
    ApplicationMappingOptimizer(GRBEnv* gurobi_env, const ProblemData* problem_data) 
        : env(gurobi_env), data(problem_data) {
        // Create model
        model = new GRBModel(*env);
        model->set(GRB_StringAttr_ModelName, "ApplicationMapping");
    }
    
    /**
     * Destructor
     */
    ~ApplicationMappingOptimizer() {
        delete model;
    }
    
    /**
     * Build the complete optimization model
     */
    void buildModel() {
        createVariables();
        addConstraints();
        setObjective();
    }
    
    /**
     * Create decision variables
     */
    void createVariables() {
        // Initialize the x variable matrix
        x.resize(data->num_applications);
        
        // Create binary assignment variables
        // x[i][j] = 1 if application i is assigned to core j, 0 otherwise
        for (int i = 0; i < data->num_applications; i++) {
            x[i].resize(data->num_cores);
            for (int j = 0; j < data->num_cores; j++) {
                string var_name = "x_" + data->application_names[i] + "_" + 
                                 data->core_names[j];
                x[i][j] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, var_name);
            }
        }
    }
    
    /**
     * Add all constraints to the model
     */
    void addConstraints() {
        // Constraint 1: Each application must be assigned to exactly one core
        for (int i = 0; i < data->num_applications; i++) {
            GRBLinExpr expr = 0;
            for (int j = 0; j < data->num_cores; j++) {
                expr += x[i][j];
            }
            string constr_name = "assign_" + data->application_names[i];
            model->addConstr(expr == 1, constr_name);
        }
        
        // Constraint 2: Memory capacity constraint for each core
        for (int j = 0; j < data->num_cores; j++) {
            GRBLinExpr expr = 0;
            for (int i = 0; i < data->num_applications; i++) {
                expr += data->memory_requirements[i] * x[i][j];
            }
            string constr_name = "memory_" + data->core_names[j];
            model->addConstr(expr <= data->memory_capacity[j], constr_name);
        }
        
        // Constraint 3: Deadline constraint for each application
        for (int i = 0; i < data->num_applications; i++) {
            GRBLinExpr expr = 0;
            for (int j = 0; j < data->num_cores; j++) {
                expr += data->time_matrix[i][j] * x[i][j];
            }
            string constr_name = "deadline_" + data->application_names[i];
            model->addConstr(expr <= data->deadlines[i], constr_name);
        }
    }
    
    /**
     * Set the objective function
     */
    void setObjective() {
        // Objective: Minimize total energy consumption
        GRBLinExpr total_energy = 0;
        for (int i = 0; i < data->num_applications; i++) {
            for (int j = 0; j < data->num_cores; j++) {
                total_energy += data->energy_matrix[i][j] * x[i][j];
            }
        }
        
        model->setObjective(total_energy, GRB_MINIMIZE);
    }
    
    /**
     * Solve the optimization model
     */
    int solve() {
        model->optimize();
        return model->get(GRB_IntAttr_Status);
    }
    
    /**
     * Display the optimization results
     */
    void displayResults() {
        int status = model->get(GRB_IntAttr_Status);
        
        cout << "\n" << string(70, '=') << endl;
        cout << "APPLICATION MAPPING OPTIMIZATION RESULTS" << endl;
        cout << string(70, '=') << endl;
        
        if (status == GRB_OPTIMAL) {
            double obj_val = model->get(GRB_DoubleAttr_ObjVal);
            
            cout << "\nStatus: OPTIMAL SOLUTION FOUND" << endl;
            cout << "Total Energy Consumption: " << fixed << setprecision(2) 
                 << obj_val << " mJ" << endl;
            
            cout << "\n" << string(70, '-') << endl;
            cout << "APPLICATION-TO-CORE MAPPING:" << endl;
            cout << string(70, '-') << endl;
            cout << left << setw(25) << "Application" 
                 << setw(25) << "Core"
                 << setw(12) << "Energy (mJ)"
                 << "Time (ms)" << endl;
            cout << string(70, '-') << endl;
            
            // Extract and display assignments
            vector<double> memory_used(data->num_cores, 0.0);
            
            for (int i = 0; i < data->num_applications; i++) {
                for (int j = 0; j < data->num_cores; j++) {
                    if (x[i][j].get(GRB_DoubleAttr_X) > 0.5) {
                        cout << left << setw(25) << data->application_names[i]
                             << setw(25) << data->core_names[j]
                             << setw(12) << fixed << setprecision(1) 
                             << data->energy_matrix[i][j]
                             << data->time_matrix[i][j] << endl;
                        
                        memory_used[j] += data->memory_requirements[i];
                    }
                }
            }
            
            cout << string(70, '-') << endl;
            
            // Display memory utilization
            cout << "\nMEMORY UTILIZATION PER CORE:" << endl;
            cout << string(70, '-') << endl;
            cout << left << setw(25) << "Core"
                 << setw(15) << "Used (MB)"
                 << setw(15) << "Capacity (MB)"
                 << "Utilization" << endl;
            cout << string(70, '-') << endl;
            
            for (int j = 0; j < data->num_cores; j++) {
                double utilization = (memory_used[j] / data->memory_capacity[j]) * 100.0;
                cout << left << setw(25) << data->core_names[j]
                     << setw(15) << fixed << setprecision(1) << memory_used[j]
                     << setw(15) << data->memory_capacity[j]
                     << setprecision(1) << utilization << "%" << endl;
            }
            
            cout << string(70, '=') << endl << endl;
            
        } else if (status == GRB_INFEASIBLE) {
            cout << "\nStatus: INFEASIBLE - No solution exists that satisfies all constraints" << endl;
            cout << "\nPossible reasons:" << endl;
            cout << "  - Memory capacity too small for the applications" << endl;
            cout << "  - Deadlines too tight (no core can meet the deadline)" << endl;
            cout << "\nTry: Use model->computeIIS() to find the source of infeasibility" << endl;
            
        } else if (status == GRB_UNBOUNDED) {
            cout << "\nStatus: UNBOUNDED - The objective can be improved indefinitely" << endl;
            cout << "This should not happen for this problem formulation." << endl;
            
        } else {
            cout << "\nStatus: " << status << endl;
            cout << "Optimization did not complete successfully." << endl;
        }
    }
};


/**
 * Main function
 */
int main(int argc, char* argv[]) {
    try {
        // Check command line arguments
        if (argc < 2) {
            cerr << "Usage: " << argv[0] << " <data_file>" << endl;
            return 1;
        }
        
        string filename = argv[1];
        
        // Read problem data
        cout << "Reading data from file: " << filename << endl;
        ProblemData data;
        if (!data.readFromFile(filename)) {
            return 1;
        }
        
        // Validate data
        if (!data.validate()) {
            return 1;
        }
        
        cout << "Problem size: " << data.num_applications << " applications, " 
             << data.num_cores << " cores" << endl;
        
        // Create Gurobi environment
        GRBEnv env = GRBEnv(true);
        env.set(GRB_IntParam_OutputFlag, 1);  // Enable solver output
        env.set(GRB_DoubleParam_TimeLimit, 300);  // Set time limit to 5 minutes
        env.start();
        
        // Create optimizer
        ApplicationMappingOptimizer optimizer(&env, &data);
        
        // Build model
        cout << "\nBuilding optimization model..." << endl;
        optimizer.buildModel();
        
        // Solve
        cout << "Solving optimization model..." << endl;
        int status = optimizer.solve();
        
        // Display results
        optimizer.displayResults();
        
        // Additional discussion questions
        cout << "\n" << string(70, '=') << endl;
        cout << "DISCUSSION QUESTIONS FOR STUDENTS:" << endl;
        cout << string(70, '=') << endl;
        cout << "1. How would the solution change if we minimized execution time instead?" << endl;
        cout << "2. What happens if we reduce the memory capacity of Core3 to 500 MB?" << endl;
        cout << "3. Can you formulate a multi-objective version (energy + time)?" << endl;
        cout << "4. How would you add task dependencies (e.g., App1 must run before App2)?" << endl;
        cout << string(70, '=') << endl << endl;
        
        return 0;
        
    } catch (GRBException& e) {
        cerr << "Gurobi exception: Error code = " << e.getErrorCode() << endl;
        cerr << e.getMessage() << endl;
        return 1;
    } catch (...) {
        cerr << "Unknown exception caught" << endl;
        return 1;
    }
}
