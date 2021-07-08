/**
 * @file methods/ann/layer/linear_impl.hpp
 * @author Marcus Edel
 *
 * Implementation of the Linear layer class also known as fully-connected layer
 * or affine transformation.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#ifndef MLPACK_METHODS_ANN_LAYER_LINEAR_IMPL_HPP
#define MLPACK_METHODS_ANN_LAYER_LINEAR_IMPL_HPP

// In case it hasn't yet been included.
#include "linear.hpp"

namespace mlpack {
namespace ann /** Artificial Neural Network. */ {

template<typename InputType, typename OutputType, typename RegularizerType>
LinearType<InputType, OutputType, RegularizerType>::LinearType() :
    Layer<InputType, OutputType>(),
    inSize(0),
    outSize(0)
{
  // Nothing to do here.
}

template<typename InputType, typename OutputType, typename RegularizerType>
LinearType<InputType, OutputType, RegularizerType>::LinearType(
    const size_t outSize,
    RegularizerType regularizer) :
    Layer<InputType, OutputType>(),
    inSize(0), // This will be computed in ComputeOutputDimensions().
    outSize(outSize),
    regularizer(regularizer)
{
  weights.set_size(WeightSize(), 1);
}

template<typename InputType, typename OutputType, typename RegularizerType>
void LinearType<InputType, OutputType, RegularizerType>::SetWeights(
    typename OutputType::elem_type* weightsPtr)
{
  weights = OutputType(weightsPtr, outSize * inSize + outSize, 1, false, false);
  weight = OutputType(weightsPtr, outSize, inSize, false, false);
  bias = OutputType(weightsPtr + weight.n_elem, outSize, 1, false, false);
}

template<typename InputType, typename OutputType, typename RegularizerType>
void LinearType<InputType, OutputType, RegularizerType>::Forward(
    const InputType& input, OutputType& output)
{
  output = weight * input;
  output.each_col() += bias;
}

template<typename InputType, typename OutputType, typename RegularizerType>
void LinearType<InputType, OutputType, RegularizerType>::Backward(
    const InputType& /* input */, const OutputType& gy, OutputType& g)
{
  g = weight.t() * gy;
}

template<typename InputType, typename OutputType, typename RegularizerType>
void LinearType<InputType, OutputType, RegularizerType>::Gradient(
    const InputType& input,
    const OutputType& error,
    OutputType& gradient)
{
  gradient.submat(0, 0, weight.n_elem - 1, 0) = arma::vectorise(
      error * input.t());
  gradient.submat(weight.n_elem, 0, gradient.n_elem - 1, 0) =
      arma::sum(error, 1);

  regularizer.Evaluate(weights, gradient);
}

template<typename InputType, typename OutputType, typename RegularizerType>
template<typename Archive>
void LinearType<InputType, OutputType, RegularizerType>::serialize(
    Archive& ar, const uint32_t /* version */)
{
  ar(cereal::base_class<Layer<InputType, OutputType>>(this));

  ar(CEREAL_NVP(inSize));
  ar(CEREAL_NVP(outSize));
  ar(CEREAL_NVP(regularizer));
}

} // namespace ann
} // namespace mlpack

#endif
