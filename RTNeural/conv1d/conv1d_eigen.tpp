#include "conv1d_eigen.h"

namespace RTNeural
{

template <typename T>
Conv1D<T>::Conv1D(size_t in_size, size_t out_size, size_t kernel_size, size_t dilation)
    : Layer<T>(in_size, out_size)
    , dilation_rate(dilation)
    , kernel_size(kernel_size)
    , state_size(kernel_size * dilation)
{
    kernelWeights.resize(out_size);
    for(size_t i = 0; i < out_size; ++i)
        kernelWeights[i] = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>::Zero(in_size, state_size);

    bias = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>::Zero(out_size, 1);
    state = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>::Zero(in_size, 2 * state_size);
    inVec = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>::Zero(in_size, 1);
    outVec = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>::Zero(out_size, 1);
}

template <typename T>
Conv1D<T>::Conv1D(std::initializer_list<size_t> sizes)
    : Conv1D<T>(*sizes.begin(), *(sizes.begin() + 1), *(sizes.begin() + 2), *(sizes.begin() + 3))
{
}

template <typename T>
Conv1D<T>::Conv1D(const Conv1D<T>& other)
    : Conv1D<T>(other.in_size, other.out_size, other.kernel_size, other.dilation_rate)
{
}

template <typename T>
Conv1D<T>& Conv1D<T>::operator=(const Conv1D<T>& other)
{
    return *this = Conv1D<T>(other);
}

template <typename T>
Conv1D<T>::~Conv1D()
{
}

template <typename T>
void Conv1D<T>::reset()
{
    state_ptr = 0;
    state = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>::Zero(Layer<T>::in_size, 2 * state_size);
}

template <typename T>
void Conv1D<T>::setWeights(const std::vector<std::vector<std::vector<T>>>& weights)
{
    for(size_t i = 0; i < Layer<T>::out_size; ++i)
        for(size_t k = 0; k < Layer<T>::in_size; ++k)
            for(size_t j = 0; j < kernel_size; ++j)
                kernelWeights[i](k, j * dilation_rate) = weights[i][k][j];
}

template <typename T>
void Conv1D<T>::setBias(const std::vector<T>& biasVals)
{
    for(size_t i = 0; i < Layer<T>::out_size; ++i)
        bias(i, 0) = biasVals[i];
}

} // namespace RTNeural
