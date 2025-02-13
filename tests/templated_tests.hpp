#pragma once

#include <iostream>
#include "load_csv.hpp"
#include "test_configs.hpp"

template <typename T, typename ModelType>
int runTestTemplated(const TestConfig& test)
{
    std::cout << "TESTING " << test.name << " TEMPLATED IMPLEMENTATION..." << std::endl;

    std::ifstream jsonStream(test.model_file, std::ifstream::binary);
    ModelType model;
    model.parseJson(jsonStream, true);
    model.reset();

    std::ifstream pythonX(test.x_data_file);
    auto xData = load_csv::loadFile<T>(pythonX);

    std::ifstream pythonY(test.y_data_file);
    const auto yRefData = load_csv::loadFile<T>(pythonY);

    std::vector<T> yData(xData.size(), (T)0);
    for(size_t n = 0; n < xData.size(); ++n)
    {
        T input[] = { xData[n] };
        yData[n] = model.forward(input);
    }

    size_t nErrs = 0;
    T max_error = (T)0;
    for(size_t n = 0; n < xData.size(); ++n)
    {
        auto err = std::abs(yData[n] - yRefData[n]);
        if(err > test.threshold)
        {
            max_error = std::max(err, max_error);
            nErrs++;

            // For debugging purposes
            // std::cout << "ERR: " << err << ", idx: " << n << std::endl;
            // std::cout << yData[n] << std::endl;
            // std::cout << yRefData[n] << std::endl;
            // break;
        }
    }

    if(nErrs > 0)
    {
        std::cout << "FAIL: " << nErrs << " errors!" << std::endl;
        std::cout << "Maximum error: " << max_error << std::endl;
        return 1;
    }

    std::cout << "SUCCESS" << std::endl;
    return 0;
}

int templatedTests(std::string arg)
{
    using namespace RTNeural;
    using TestType = double;

#if USE_XSIMD
    int result = 0;

    if(arg == "dense")
    {
        using ModelType = ModelT<TestType, 1, 1,
                            DenseT<TestType, 1, 8>,
                            TanhActivationT<TestType, 8>,
                            DenseT<TestType, 8, 8>,
                            ReLuActivationT<TestType, 8>,
                            DenseT<TestType, 8, 8>,
                            SoftmaxActivationT<TestType, 8>,
                            DenseT<TestType, 8, 1>>;
        result |= runTestTemplated<TestType, ModelType>(tests.at(arg));
    }

    if(arg == "gru")
    {
        using ModelType = ModelT<TestType, 1, 1,
                            DenseT<TestType, 1, 8>,
                            TanhActivationT<TestType, 8>,
                            GRULayerT<TestType, 8, 8>,
                            DenseT<TestType, 8, 8>,
                            SigmoidActivationT<TestType, 8>,
                            DenseT<TestType, 8, 1>>;
        result |= runTestTemplated<TestType, ModelType>(tests.at(arg));
    }

    return result;

#else // @TODO
    return 0;
#endif
}
