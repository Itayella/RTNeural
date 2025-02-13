#include "gru_xsimd.h"

namespace RTNeural
{

template <typename T>
GRULayer<T>::GRULayer(size_t in_size, size_t out_size)
    : Layer<T>(in_size, out_size)
    , zWeights(in_size, out_size)
    , rWeights(in_size, out_size)
    , cWeights(in_size, out_size)
{
    ht1.resize(out_size, (T)0);
    zVec.resize(out_size, (T)0);
    rVec.resize(out_size, (T)0);
    cVec.resize(out_size, (T)0);
    cTmp.resize(out_size, (T)0);

    prod_in.resize(in_size, (T)0);
    prod_out.resize(out_size, (T)0);

    ones.resize(out_size, (T)1);
}

template <typename T>
GRULayer<T>::GRULayer(std::initializer_list<size_t> sizes)
    : GRULayer<T>(*sizes.begin(), *(sizes.begin() + 1))
{
}

template <typename T>
GRULayer<T>::GRULayer(const GRULayer<T>& other)
    : GRULayer<T>(other.in_size, other.out_size)
{
}

template <typename T>
GRULayer<T>& GRULayer<T>::operator=(const GRULayer<T>& other)
{
    return *this = GRULayer<T>(other);
}

template <typename T>
GRULayer<T>::~GRULayer()
{
}

template <typename T>
GRULayer<T>::WeightSet::WeightSet(size_t in_size, size_t out_size)
    : out_size(out_size)
{
    W = vec2_type(out_size, vec_type(in_size, (T)0));
    U = vec2_type(out_size, vec_type(out_size, (T)0));

    b[0].resize(out_size, (T)0);
    b[1].resize(out_size, (T)0);
}

template <typename T>
GRULayer<T>::WeightSet::~WeightSet()
{
}

template <typename T>
void GRULayer<T>::setWVals(const std::vector<std::vector<T>>& wVals)
{
    for(size_t i = 0; i < Layer<T>::in_size; ++i)
    {
        for(size_t k = 0; k < Layer<T>::out_size; ++k)
        {
            zWeights.W[k][i] = wVals[i][k];
            rWeights.W[k][i] = wVals[i][k + Layer<T>::out_size];
            cWeights.W[k][i] = wVals[i][k + Layer<T>::out_size * 2];
        }
    }
}

template <typename T>
void GRULayer<T>::setWVals(T** wVals)
{
    for(size_t i = 0; i < Layer<T>::in_size; ++i)
    {
        for(size_t k = 0; k < Layer<T>::out_size; ++k)
        {
            zWeights.W[k][i] = wVals[i][k];
            rWeights.W[k][i] = wVals[i][k + Layer<T>::out_size];
            cWeights.W[k][i] = wVals[i][k + Layer<T>::out_size * 2];
        }
    }
}

template <typename T>
void GRULayer<T>::setUVals(const std::vector<std::vector<T>>& uVals)
{
    for(size_t i = 0; i < Layer<T>::out_size; ++i)
    {
        for(size_t k = 0; k < Layer<T>::out_size; ++k)
        {
            zWeights.U[k][i] = uVals[i][k];
            rWeights.U[k][i] = uVals[i][k + Layer<T>::out_size];
            cWeights.U[k][i] = uVals[i][k + Layer<T>::out_size * 2];
        }
    }
}

template <typename T>
void GRULayer<T>::setUVals(T** uVals)
{
    for(size_t i = 0; i < Layer<T>::out_size; ++i)
    {
        for(size_t k = 0; k < Layer<T>::out_size; ++k)
        {
            zWeights.U[k][i] = uVals[i][k];
            rWeights.U[k][i] = uVals[i][k + Layer<T>::out_size];
            cWeights.U[k][i] = uVals[i][k + Layer<T>::out_size * 2];
        }
    }
}

template <typename T>
void GRULayer<T>::setBVals(const std::vector<std::vector<T>>& bVals)
{
    for(size_t i = 0; i < 2; ++i)
    {
        for(size_t k = 0; k < Layer<T>::out_size; ++k)
        {
            zWeights.b[i][k] = bVals[i][k];
            rWeights.b[i][k] = bVals[i][k + Layer<T>::out_size];
            cWeights.b[i][k] = bVals[i][k + Layer<T>::out_size * 2];
        }
    }
}

template <typename T>
void GRULayer<T>::setBVals(T** bVals)
{
    for(size_t i = 0; i < 2; ++i)
    {
        for(size_t k = 0; k < Layer<T>::out_size; ++k)
        {
            zWeights.b[i][k] = bVals[i][k];
            rWeights.b[i][k] = bVals[i][k + Layer<T>::out_size];
            cWeights.b[i][k] = bVals[i][k + Layer<T>::out_size * 2];
        }
    }
}

template <typename T>
T GRULayer<T>::getWVal(size_t i, size_t k) const noexcept
{
    T** set = zWeights.W;
    if(k > 2 * Layer<T>::out_size)
    {
        k -= 2 * Layer<T>::out_size;
        set = cWeights.W;
    }
    else if(k > Layer<T>::out_size)
    {
        k -= Layer<T>::out_size;
        set = rWeights.W;
    }

    return set[i][k];
}

template <typename T>
T GRULayer<T>::getUVal(size_t i, size_t k) const noexcept
{
    T** set = zWeights.U;
    if(k > 2 * Layer<T>::out_size)
    {
        k -= 2 * Layer<T>::out_size;
        set = cWeights.U;
    }
    else if(k > Layer<T>::out_size)
    {
        k -= Layer<T>::out_size;
        set = rWeights.U;
    }

    return set[i][k];
}

template <typename T>
T GRULayer<T>::getBVal(size_t i, size_t k) const noexcept
{
    T** set = zWeights.b;
    if(k > 2 * Layer<T>::out_size)
    {
        k -= 2 * Layer<T>::out_size;
        set = cWeights.b;
    }
    else if(k > Layer<T>::out_size)
    {
        k -= Layer<T>::out_size;
        set = rWeights.b;
    }

    return set[i][k];
}

//====================================================
template <typename T, size_t in_sizet, size_t out_sizet>
GRULayerT<T, in_sizet, out_sizet>::GRULayerT()
{
    for(size_t i = 0; i < v_out_size; ++i)
    {
        // single-input kernel weights
        Wz_1[i] = v_type((T)0);
        Wr_1[i] = v_type((T)0);
        Wh_1[i] = v_type((T)0);

        // biases
        bz[i] = v_type((T)0);
        br[i] = v_type((T)0);
        bh0[i] = v_type((T)0);
        bh1[i] = v_type((T)0);

        // intermediate vars
        zt[i] = v_type((T)0);
        rt[i] = v_type((T)0);
        ct[i] = v_type((T)0);
        ht[i] = v_type((T)0);

        reset();
    }

    for(size_t i = 0; i < out_size; ++i)
    {
        // recurrent weights
        for(size_t k = 0; k < v_out_size; ++k)
        {
            Uz[i][k] = v_type((T)0);
            Ur[i][k] = v_type((T)0);
            Uh[i][k] = v_type((T)0);
        }

        // kernel weights
        for(size_t k = 0; k < v_in_size; ++k)
        {
            Wz[i][k] = v_type((T)0);
            Wr[i][k] = v_type((T)0);
            Wh[i][k] = v_type((T)0);
        }
    }
}

template <typename T, size_t in_sizet, size_t out_sizet>
void GRULayerT<T, in_sizet, out_sizet>::reset()
{
    // reset output state
    for(size_t i = 0; i < v_out_size; ++i)
        outs[i] = v_type((T)0);
}

// kernel weights
template <typename T, size_t in_sizet, size_t out_sizet>
void GRULayerT<T, in_sizet, out_sizet>::setWVals(const std::vector<std::vector<T>>& wVals)
{
    for(size_t i = 0; i < in_size; ++i)
    {
        for(size_t j = 0; j < out_size; ++j)
        {
            Wz[j][i / v_size] = set_value(Wz[j][i / v_size], i % v_size, wVals[i][j]);
            Wr[j][i / v_size] = set_value(Wr[j][i / v_size], i % v_size, wVals[i][j + out_size]);
            Wh[j][i / v_size] = set_value(Wh[j][i / v_size], i % v_size, wVals[i][j + 2 * out_size]);
        }
    }

    for(size_t j = 0; j < out_size; ++j)
    {
        Wz_1[j / v_size] = set_value(Wz_1[j / v_size], j % v_size, wVals[0][j]);
        Wr_1[j / v_size] = set_value(Wr_1[j / v_size], j % v_size, wVals[0][j + out_size]);
        Wh_1[j / v_size] = set_value(Wh_1[j / v_size], j % v_size, wVals[0][j + 2 * out_size]);
    }
}

// recurrent weights
template <typename T, size_t in_sizet, size_t out_sizet>
void GRULayerT<T, in_sizet, out_sizet>::setUVals(const std::vector<std::vector<T>>& uVals)
{
    for(size_t i = 0; i < out_size; ++i)
    {
        for(size_t j = 0; j < out_size; ++j)
        {
            Uz[j][i / v_size] = set_value(Uz[j][i / v_size], i % v_size, uVals[i][j]);
            Ur[j][i / v_size] = set_value(Ur[j][i / v_size], i % v_size, uVals[i][j + out_size]);
            Uh[j][i / v_size] = set_value(Uh[j][i / v_size], i % v_size, uVals[i][j + 2 * out_size]);
        }
    }
}

// biases
template <typename T, size_t in_sizet, size_t out_sizet>
void GRULayerT<T, in_sizet, out_sizet>::setBVals(const std::vector<std::vector<T>>& bVals)
{
    for(size_t k = 0; k < out_size; ++k)
    {
        bz[k / v_size] = set_value(bz[k / v_size], k % v_size, bVals[0][k] + bVals[1][k]);
        br[k / v_size] = set_value(br[k / v_size], k % v_size, bVals[0][k + out_size] + bVals[1][k + out_size]);
        bh0[k / v_size] = set_value(bh0[k / v_size], k % v_size, bVals[0][k + 2 * out_size]);
        bh1[k / v_size] = set_value(bh1[k / v_size], k % v_size, bVals[1][k + 2 * out_size]);
    }
}

} // namespace RTNeural
