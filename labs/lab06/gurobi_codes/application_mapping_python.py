"""
Application Mapping Optimizer for Embedded System Design
=========================================================

This module implements an optimization model for mapping applications to
heterogeneous cores in an embedded system. The objective is to minimize
total energy consumption while satisfying memory capacity and deadline constraints.

Author: Educational Example for ESD Course
Date: March 2026
"""

import gurobipy as gp
from gurobipy import GRB


class ApplicationMappingOptimizer:
    """
    Optimizer for mapping applications to heterogeneous cores in embedded systems.
    
    This class implements a Mixed-Integer Linear Programming (MILP) model to find
    the optimal assignment of applications to processing cores, minimizing energy
    consumption while respecting memory and timing constraints.
    """
    
    def __init__(self, env):
        """
        Initialize the optimizer with a Gurobi environment.
        
        Args:
            env: Gurobi environment object
        """
        self.env = env
        self.model = None
        
        # Data attributes (to be set via set_data method)
        self.num_applications = 0
        self.num_cores = 0
        self.application_names = []
        self.core_names = []
        self.energy_matrix = []      # energy[i][j]: energy of app i on core j
        self.time_matrix = []         # time[i][j]: execution time of app i on core j
        self.memory_requirements = [] # memory_req[i]: memory needed by app i
        self.memory_capacity = []     # mem_cap[j]: memory capacity of core j
        self.deadlines = []           # deadline[i]: deadline for app i
        
        # Decision variables (to be created in build_model)
        self.x = None  # x[i,j]: binary variable (1 if app i assigned to core j)
        
    def __enter__(self):
        """Enter context manager."""
        return self
        
    def __exit__(self, exc_type, exc_val, exc_tb):
        """Exit context manager and dispose model."""
        if self.model is not None:
            self.model.dispose()
        return False
    
    def set_data(self, num_applications, num_cores, application_names, core_names,
                 energy_matrix, time_matrix, memory_requirements, 
                 memory_capacity, deadlines):
        """
        Set the problem data.
        
        Args:
            num_applications: Number of applications to map
            num_cores: Number of available cores
            application_names: List of application names
            core_names: List of core names
            energy_matrix: 2D list [num_applications][num_cores] of energy values
            time_matrix: 2D list [num_applications][num_cores] of execution times
            memory_requirements: List of memory requirements per application
            memory_capacity: List of memory capacity per core
            deadlines: List of deadline values per application
        """
        self.num_applications = num_applications
        self.num_cores = num_cores
        self.application_names = application_names
        self.core_names = core_names
        self.energy_matrix = energy_matrix
        self.time_matrix = time_matrix
        self.memory_requirements = memory_requirements
        self.memory_capacity = memory_capacity
        self.deadlines = deadlines
        
    def build_model(self):
        """
        Build the complete optimization model.
        
        Creates the Gurobi model with all variables, constraints, and objective function.
        """
        # Create model
        self.model = gp.Model("ApplicationMapping", env=self.env)
        
        # Build model components
        self._create_variables()
        self._add_constraints()
        self._set_objective()
        
    def _create_variables(self):
        """
        Create decision variables.
        
        Creates binary variables x[i,j] indicating whether application i 
        is assigned to core j.
        """
        # Create binary assignment variables
        # x[i,j] = 1 if application i is assigned to core j, 0 otherwise
        self.x = {}
        for i in range(self.num_applications):
            for j in range(self.num_cores):
                var_name = f"x_{self.application_names[i]}_{self.core_names[j]}"
                self.x[i, j] = self.model.addVar(
                    vtype=GRB.BINARY,
                    name=var_name
                )
    
    def _add_constraints(self):
        """
        Add all constraints to the model.
        
        Adds:
        1. Assignment constraints (each app assigned to exactly one core)
        2. Memory capacity constraints (total memory per core <= capacity)
        3. Deadline constraints (execution time <= deadline for each app)
        """
        # Constraint 1: Each application must be assigned to exactly one core
        for i in range(self.num_applications):
            self.model.addConstr(
                gp.quicksum(self.x[i, j] for j in range(self.num_cores)) == 1,
                name=f"assign_{self.application_names[i]}"
            )
        
        # Constraint 2: Memory capacity constraint for each core
        for j in range(self.num_cores):
            self.model.addConstr(
                gp.quicksum(
                    self.memory_requirements[i] * self.x[i, j] 
                    for i in range(self.num_applications)
                ) <= self.memory_capacity[j],
                name=f"memory_{self.core_names[j]}"
            )
        
        # Constraint 3: Deadline constraint for each application
        for i in range(self.num_applications):
            self.model.addConstr(
                gp.quicksum(
                    self.time_matrix[i][j] * self.x[i, j] 
                    for j in range(self.num_cores)
                ) <= self.deadlines[i],
                name=f"deadline_{self.application_names[i]}"
            )
    
    def _set_objective(self):
        """
        Set the objective function.
        
        Minimizes total energy consumption across all application-core assignments.
        """
        # Objective: Minimize total energy consumption
        total_energy = gp.quicksum(
            self.energy_matrix[i][j] * self.x[i, j]
            for i in range(self.num_applications)
            for j in range(self.num_cores)
        )
        
        self.model.setObjective(total_energy, GRB.MINIMIZE)
    
    def solve(self):
        """
        Solve the optimization model.
        
        Returns:
            Optimization status (e.g., GRB.OPTIMAL, GRB.INFEASIBLE)
        """
        self.model.optimize()
        return self.model.Status
    
    def get_solution(self):
        """
        Extract the solution from the model.
        
        Returns:
            Dictionary containing:
                - 'status': Optimization status
                - 'objective_value': Total energy consumption (if optimal)
                - 'assignments': List of (app_index, core_index) tuples
                - 'assignment_details': List of dicts with detailed info
        """
        solution = {
            'status': self.model.Status,
            'objective_value': None,
            'assignments': [],
            'assignment_details': []
        }
        
        if self.model.Status == GRB.OPTIMAL:
            solution['objective_value'] = self.model.ObjVal
            
            # Extract assignments
            for i in range(self.num_applications):
                for j in range(self.num_cores):
                    if self.x[i, j].X > 0.5:  # Binary variable is 1
                        solution['assignments'].append((i, j))
                        solution['assignment_details'].append({
                            'application': self.application_names[i],
                            'core': self.core_names[j],
                            'energy': self.energy_matrix[i][j],
                            'execution_time': self.time_matrix[i][j],
                            'memory_used': self.memory_requirements[i]
                        })
        
        return solution
    
    def display_results(self, solution=None):
        """
        Display the optimization results in a readable format.
        
        Args:
            solution: Solution dictionary (if None, will be extracted automatically)
        """
        if solution is None:
            solution = self.get_solution()
        
        print("\n" + "="*70)
        print("APPLICATION MAPPING OPTIMIZATION RESULTS")
        print("="*70)
        
        if solution['status'] == GRB.OPTIMAL:
            print(f"\nStatus: OPTIMAL SOLUTION FOUND")
            print(f"Total Energy Consumption: {solution['objective_value']:.2f} mJ")
            
            print("\n" + "-"*70)
            print("APPLICATION-TO-CORE MAPPING:")
            print("-"*70)
            print(f"{'Application':<25} {'Core':<25} {'Energy (mJ)':<12} {'Time (ms)'}")
            print("-"*70)
            
            for detail in solution['assignment_details']:
                print(f"{detail['application']:<25} {detail['core']:<25} "
                      f"{detail['energy']:<12.1f} {detail['execution_time']:.1f}")
            
            print("-"*70)
            
            # Display memory utilization per core
            print("\nMEMORY UTILIZATION PER CORE:")
            print("-"*70)
            print(f"{'Core':<25} {'Used (MB)':<15} {'Capacity (MB)':<15} {'Utilization'}")
            print("-"*70)
            
            memory_used = [0.0] * self.num_cores
            for detail in solution['assignment_details']:
                core_idx = self.core_names.index(detail['core'])
                memory_used[core_idx] += detail['memory_used']
            
            for j in range(self.num_cores):
                utilization = (memory_used[j] / self.memory_capacity[j]) * 100
                print(f"{self.core_names[j]:<25} {memory_used[j]:<15.1f} "
                      f"{self.memory_capacity[j]:<15.1f} {utilization:.1f}%")
            
            print("="*70 + "\n")
            
        elif solution['status'] == GRB.INFEASIBLE:
            print("\nStatus: INFEASIBLE - No solution exists that satisfies all constraints")
            print("\nPossible reasons:")
            print("  - Memory capacity too small for the applications")
            print("  - Deadlines too tight (no core can meet the deadline)")
            print("\nTry: Use model.computeIIS() to find the source of infeasibility")
            
        elif solution['status'] == GRB.UNBOUNDED:
            print("\nStatus: UNBOUNDED - The objective can be improved indefinitely")
            print("This should not happen for this problem formulation.")
            
        else:
            print(f"\nStatus: {solution['status']}")
            print("Optimization did not complete successfully.")


def read_data_from_file(filename):
    """
    Read problem data from a text file.
    
    Args:
        filename: Path to the data file
        
    Returns:
        Dictionary containing all problem data
    """
    data = {}
    
    with open(filename, 'r') as f:
        lines = [line.strip() for line in f.readlines() if line.strip() and not line.startswith('#')]
    
    i = 0
    while i < len(lines):
        line = lines[i]
        
        if line.startswith('NUM_APPLICATIONS'):
            data['num_applications'] = int(lines[i].split()[1])
            i += 1
            
        elif line.startswith('NUM_CORES'):
            data['num_cores'] = int(lines[i].split()[1])
            i += 1
            
        elif line == 'APPLICATION_NAMES':
            i += 1
            data['application_names'] = []
            for _ in range(data['num_applications']):
                data['application_names'].append(lines[i])
                i += 1
                
        elif line == 'CORE_NAMES':
            i += 1
            data['core_names'] = []
            for _ in range(data['num_cores']):
                data['core_names'].append(lines[i])
                i += 1
                
        elif line == 'ENERGY_MATRIX':
            i += 1
            data['energy_matrix'] = []
            for _ in range(data['num_applications']):
                row = [float(x) for x in lines[i].split()]
                data['energy_matrix'].append(row)
                i += 1
                
        elif line == 'TIME_MATRIX':
            i += 1
            data['time_matrix'] = []
            for _ in range(data['num_applications']):
                row = [float(x) for x in lines[i].split()]
                data['time_matrix'].append(row)
                i += 1
                
        elif line == 'MEMORY_REQUIREMENTS':
            i += 1
            data['memory_requirements'] = []
            for _ in range(data['num_applications']):
                data['memory_requirements'].append(float(lines[i]))
                i += 1
                
        elif line == 'MEMORY_CAPACITY':
            i += 1
            data['memory_capacity'] = []
            for _ in range(data['num_cores']):
                data['memory_capacity'].append(float(lines[i]))
                i += 1
                
        elif line == 'DEADLINES':
            i += 1
            data['deadlines'] = []
            for _ in range(data['num_applications']):
                data['deadlines'].append(float(lines[i]))
                i += 1
        else:
            i += 1
    
    return data


def main():
    """
    Main function to run the application mapping optimization.
    """
    # Read data from file
    print("Reading data from file...")
    data = read_data_from_file('application_mapping_data.txt')
    
    print(f"Problem size: {data['num_applications']} applications, {data['num_cores']} cores")
    
    # Create Gurobi environment with context manager
    with gp.Env(empty=True) as env:
        # Configure environment parameters
        env.setParam('OutputFlag', 1)  # Enable solver output
        env.setParam('TimeLimit', 300)  # Set time limit to 5 minutes
        env.start()
        
        # Create and use optimizer with context manager
        with ApplicationMappingOptimizer(env) as optimizer:
            # Set data
            optimizer.set_data(
                num_applications=data['num_applications'],
                num_cores=data['num_cores'],
                application_names=data['application_names'],
                core_names=data['core_names'],
                energy_matrix=data['energy_matrix'],
                time_matrix=data['time_matrix'],
                memory_requirements=data['memory_requirements'],
                memory_capacity=data['memory_capacity'],
                deadlines=data['deadlines']
            )
            
            # Build model
            print("\nBuilding optimization model...")
            optimizer.build_model()
            
            # Solve
            print("Solving optimization model...")
            status = optimizer.solve()
            
            # Display results
            optimizer.display_results()
            
            # Additional analysis: What if we had different objectives?
            print("\n" + "="*70)
            print("DISCUSSION QUESTIONS FOR STUDENTS:")
            print("="*70)
            print("1. How would the solution change if we minimized execution time instead?")
            print("2. What happens if we reduce the memory capacity of Core3 to 500 MB?")
            print("3. Can you formulate a multi-objective version (energy + time)?")
            print("4. How would you add task dependencies (e.g., App1 must run before App2)?")
            print("="*70 + "\n")


if __name__ == "__main__":
    main()
