#ifndef DENSEEIGEN_H_INCLUDED
#define DENSEEIGEN_H_INCLUDED

#include "../Layer.h"
#include <Eigen/Dense>

namespace RTNeural
{

template <typename T>
class Dense : public Layer<T>
{
public:
    Dense(size_t in_size, size_t out_size)
        : Layer<T>(in_size, out_size)
    {
        weights = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>::Zero(out_size, in_size);
        bias = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>::Zero(out_size, 1);

        inVec = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>::Zero(in_size, 1);
        outVec = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>::Zero(out_size, 1);
    }

    Dense(std::initializer_list<size_t> sizes)
        : Dense(*sizes.begin(), *(sizes.begin() + 1))
    {
    }

    Dense(const Dense& other)
        : Dense(other.in_size, other.out_size)
    {
    }

    Dense& operator=(const Dense& other)
    {
        return *this = Dense(other);
    }

    virtual ~Dense() { }

    inline void forward(const T* input, T* out) override
    {
        inVec = Eigen::Map<const Eigen::Matrix<T, Eigen::Dynamic, 1>, Eigen::Aligned16>(
            input, Layer<T>::in_size, 1);
        outVec = weights * inVec + bias;

        std::copy(outVec.data(), outVec.data() + Layer<T>::out_size, out);
    }

    void setWeights(const std::vector<std::vector<T>>& newWeights)
    {
        for(size_t i = 0; i < Layer<T>::out_size; ++i)
            for(size_t k = 0; k < Layer<T>::in_size; ++k)
                weights(i, k) = newWeights[i][k];
    }

    void setWeights(T** newWeights)
    {
        for(size_t i = 0; i < Layer<T>::out_size; ++i)
            for(size_t k = 0; k < Layer<T>::in_size; ++k)
                weights(i, k) = newWeights[i][k];
    }

    void setBias(T* b)
    {
        for(size_t i = 0; i < Layer<T>::out_size; ++i)
            bias(i, 0) = b[i];
    }

    T getWeight(size_t i, size_t k) const noexcept { return weights(i, k); }

    T getBias(size_t i) const noexcept { return bias(i, 0); }

private:
    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> weights;
    Eigen::Matrix<T, Eigen::Dynamic, 1> bias;

    Eigen::Matrix<T, Eigen::Dynamic, 1> inVec;
    Eigen::Matrix<T, Eigen::Dynamic, 1> outVec;
};

} // namespace RTNeural

#endif // DENSEEIGEN_H_INCLUDED
