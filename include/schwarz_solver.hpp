#ifndef schwarz_solver_hpp
#define schwarz_solver_hpp

#include <cmath>
#include <fstream>
#include <memory>

#include <communicate.hpp>
#include <initialization.hpp>
#include <schwarz/config.hpp>
#include <solve.hpp>

#if SCHWARZ_USE_DEALII
#include <deal.II/lac/sparse_matrix.h>
#endif

/**
 * The Schwarz wrappers namespace
 *
 * @ingroup schwarz_wrappers
 */
namespace SchwarzWrappers {

/**
 * The Base solver class is meant to be the class implementing the common
 * implementations for all the schwarz methods. It derives from the
 * Initialization class, the Communication class and the Solve class all of
 * which are templated.
 *
 * @tparam ValueType  The type of the floating point values.
 * @tparam IndexType  The type of the index type values.
 */
template <typename ValueType = gko::default_precision,
          typename IndexType = gko::int32>
class SolverBase : public Initialize<ValueType, IndexType>,
                   public Communicate<ValueType, IndexType>,
                   public Solve<ValueType, IndexType> {
public:
    /**
     * The constructor that takes in the user settings and a metadata struct
     * containing the solver metadata.
     *
     * @param settings  The settings struct.
     * @param metadata  The metadata struct.
     */
    SolverBase(Settings &settings, Metadata<ValueType, IndexType> &metadata);

#if SCHWARZ_USE_DEALII
    void initialize(const dealii::SparseMatrix<ValueType> &matrix,
                    const dealii::Vector<ValueType> &system_rhs);
#else
    void initialize();
#endif

    /**
     * The function that runs the actual solver and obtains the final solution.
     *
     * @param solution  The solution vector.
     */
    void run(std::shared_ptr<gko::matrix::Dense<ValueType>> &solution);

    /**
     * The auxiliary function that prints a passed in vector.
     *
     * @param vector  The vector to be printed.
     * @param subd  The subdomain on which the vector exists.
     * @param name  The name of the vector as a string.
     */
    void print_vector(
        const std::shared_ptr<gko::matrix::Dense<ValueType>> &vector, int subd,
        std::string name);

    /**
     * The auxiliary function that prints a passed in CSR matrix.
     *
     * @param matrix  The matrix to be printed.
     * @param subd  The subdomain on which the vector exists.
     * @param name  The name of the matrix as a string.
     */
    void print_matrix(
        const std::shared_ptr<gko::matrix::Csr<ValueType, IndexType>> &matrix,
        int rank, std::string name);

    /**
     * The local subdomain matrix.
     */
    std::shared_ptr<gko::matrix::Csr<ValueType, IndexType>> local_matrix;

    /**
     * The local triangular factor used for the triangular solves.
     */
    std::shared_ptr<gko::matrix::Csr<ValueType, IndexType>> triangular_factor;

    /**
     * The local interface matrix.
     */
    std::shared_ptr<gko::matrix::Csr<ValueType, IndexType>> interface_matrix;

    /**
     * The global matrix.
     */
    std::shared_ptr<gko::matrix::Csr<ValueType, IndexType>> global_matrix;

    /**
     * The local right hand side.
     */
    std::shared_ptr<gko::matrix::Dense<ValueType>> local_rhs;

    /**
     * The global right hand side.
     */
    std::shared_ptr<gko::matrix::Dense<ValueType>> global_rhs;

    /**
     * The local solution vector.
     */
    std::shared_ptr<gko::matrix::Dense<ValueType>> local_solution;

    /**
     * The global solution vector.
     */
    std::shared_ptr<gko::matrix::Dense<ValueType>> global_solution;

protected:
    /**
     * The settings struct used to store the solver and other auxiliary
     * settings.
     */
    Settings &settings;

    /**
     * The metadata struct used to store the solver metadata.
     */
    Metadata<ValueType, IndexType> &metadata;

    /**
     * The communication struct used to store the metadata and arrays needed for
     * the communication bewtween subdomains.
     */
    struct Communicate<ValueType, IndexType>::comm_struct comm_struct;
};


/**
 * An implementation of the solver interface using the RAS solver.
 *
 * @tparam ValueType  The type of the floating point values.
 * @tparam IndexType  The type of the index type values.
 * @ingroup SchwarzWrappers
 */
template <typename ValueType = gko::default_precision,
          typename IndexType = gko::int32>
class SolverRAS : public SolverBase<ValueType, IndexType> {
public:
    /**
     * Standardized data struct to pipe additional data to the solver.
     */
    struct AdditionalData {};

    /**
     * The constructor that takes in the user settings and a metadata struct
     * containing the solver metadata.
     *
     * @param settings  The settings struct.
     * @param metadata  The metadata struct.
     * @param data  The additional data struct.
     */
    SolverRAS(Settings &settings, Metadata<ValueType, IndexType> &metadata,
              const AdditionalData &data = AdditionalData());

    void setup_local_matrices(
        Settings &settings, Metadata<ValueType, IndexType> &metadata,
        std::vector<unsigned int> &partition_indices,
        std::shared_ptr<gko::matrix::Csr<ValueType, IndexType>> &global_matrix,
        std::shared_ptr<gko::matrix::Csr<ValueType, IndexType>> &local_matrix,
        std::shared_ptr<gko::matrix::Csr<ValueType, IndexType>>
            &interface_matrix) override;

    void setup_comm_buffers() override;

    void setup_windows(
        const Settings &settings,
        const Metadata<ValueType, IndexType> &metadata,
        std::shared_ptr<gko::matrix::Dense<ValueType>> &main_buffer) override;

    void exchange_boundary(const Settings &settings,
                           const Metadata<ValueType, IndexType> &metadata,
                           std::shared_ptr<gko::matrix::Dense<ValueType>>
                               &solution_vector) override;

    void update_boundary(
        const Settings &settings,
        const Metadata<ValueType, IndexType> &metadata,
        std::shared_ptr<gko::matrix::Dense<ValueType>> &local_solution,
        const std::shared_ptr<gko::matrix::Dense<ValueType>> &local_rhs,
        const std::shared_ptr<gko::matrix::Dense<ValueType>> &solution_vector,
        std::shared_ptr<gko::matrix::Dense<ValueType>> &global_old_solution,
        const std::shared_ptr<gko::matrix::Csr<ValueType, IndexType>>
            &interface_matrix) override;

    /**
     * Store a copy of the flags for this particular solver.
     */
    const AdditionalData additional_data;
};


}  // namespace SchwarzWrappers


#endif  // schwarz_solver.hpp
