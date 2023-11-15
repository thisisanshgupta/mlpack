/**
 * @file tests/layer/multihead_attention.cpp
 * @author Marcus Edel
 * @author Praveen Ch
 *
 * Tests the multihead_attention layer.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#include <mlpack/core.hpp>
#include <mlpack/methods/ann.hpp>

#include "../../test_catch_tools.hpp"
#include "../../catch.hpp"
#include "../../serialization.hpp"
#include "../ann_test_tools.hpp"

using namespace mlpack;

/**
 * Simple Multihead Attention test.
 */
TEST_CASE("SimpleMultiheadAttentionTest", "[ANNLayerTest]")
{
  size_t tLen = 5;
  size_t sLen = tLen;
  size_t embedDim = 4;
  size_t numHeads = 2;
  size_t bsz = 3;

  arma::mat query = 0.1 * arma::randu(embedDim * tLen, bsz);
  arma::mat output;

  arma::mat attnMask = arma::zeros(tLen, sLen);
  for (size_t i = 0; i < tLen; ++i)
  {
    for (size_t j = 0; j < sLen; ++j)
    {
      if (i < j)
        attnMask(i, j) = std::numeric_limits<double>::lowest();
    }
  }

  arma::mat keyPaddingMask = arma::zeros(1, sLen);
  keyPaddingMask(sLen - 1) = std::numeric_limits<double>::lowest();

  MultiheadAttention module(tLen, numHeads);
  module.InputDimensions() = std::vector<size_t>({ embedDim, 2 * sLen + tLen });
  module.ComputeOutputDimensions();
  arma::mat weights(module.WeightSize(), 1);
  weights.randu();
  module.SetWeights(weights.memptr());

  module.AttentionMask() = attnMask;
  module.KeyPaddingMask() = keyPaddingMask;

  // Forward test.
  arma::mat input = arma::join_cols(arma::join_cols(query, query), query);

  module.Forward(input, output);
  REQUIRE(output.n_rows == embedDim * tLen);
  REQUIRE(output.n_cols == bsz);

  // Backward test.
  arma::mat gy = 0.01 * arma::randu(embedDim * tLen, bsz);
  arma::mat g;
  module.Backward(input, output, gy, g);
  REQUIRE(g.n_rows == input.n_rows);
  REQUIRE(g.n_cols == input.n_cols);

  // Gradient test.
  arma::mat error = 0.05 * arma::randu(embedDim * tLen, bsz);
  arma::mat gradient;
  module.Gradient(input, error, gradient);
  REQUIRE(gradient.n_rows == module.Parameters().n_rows);
  REQUIRE(gradient.n_cols == module.Parameters().n_cols);
}

/**
 * Jacobian MultiheadAttention module test.
 */
TEST_CASE("JacobianMultiheadAttentionTest", "[ANNLayerTest]")
{
  // Check when query = key = value.
  for (size_t i = 0; i < 5; ++i)
  {
    const size_t tgtSeqLen = 2;
    const size_t embedDim = 4;
    const size_t nHeads = 2;
    const size_t batchSize = 1;

    arma::mat query = arma::randu(embedDim * tgtSeqLen, batchSize);
    arma::mat input = arma::join_cols(arma::join_cols(query, query), query);

    MultiheadAttention module(tgtSeqLen, nHeads);
    module.InputDimensions() = std::vector<size_t>({ embedDim, 3 * tgtSeqLen });
    module.ComputeOutputDimensions();
    arma::mat weights(module.WeightSize(), 1);
    weights.randu();
    module.SetWeights(weights.memptr());

    double error = CustomJacobianTest(module, input);
    REQUIRE(error <= 1e-5);
  }

  // Check when key = value.
  for (size_t i = 0; i < 5; ++i)
  {
    const size_t tgtSeqLen = 2;
    const size_t srcSeqLen = RandInt(2, 5);
    const size_t embedDim = 4;
    const size_t nHeads = 2;
    const size_t batchSize = 1;

    arma::mat query = arma::randu(embedDim * tgtSeqLen, batchSize);
    arma::mat key = 0.091 * arma::randu(embedDim * srcSeqLen, batchSize);
    arma::mat input = arma::join_cols(arma::join_cols(query, key), key);

    MultiheadAttention module(tgtSeqLen, nHeads);
    module.InputDimensions() = std::vector<size_t>({ embedDim, 2 * srcSeqLen + tgtSeqLen });
    module.ComputeOutputDimensions();
    arma::mat weights(module.WeightSize(), 1);
    weights.randu();
    module.SetWeights(weights.memptr());

    double error = CustomJacobianTest(module, input);
    REQUIRE(error <= 1e-5);
  }

  // Check when query, key and value are not same.
  for (size_t i = 0; i < 5; ++i)
  {
    const size_t tgtSeqLen = 2;
    const size_t srcSeqLen = RandInt(2, 5);
    const size_t embedDim = 4;
    const size_t nHeads = 2;
    const size_t batchSize = 1;

    arma::mat query = arma::randu(embedDim * tgtSeqLen, batchSize);
    arma::mat key = 0.091 * arma::randu(embedDim * srcSeqLen, batchSize);
    arma::mat value = 0.045 * arma::randu(embedDim * srcSeqLen, batchSize);
    arma::mat input = arma::join_cols(arma::join_cols(query, key), value);

    MultiheadAttention module(tgtSeqLen, nHeads);
    module.InputDimensions() = std::vector<size_t>({ embedDim, 2 * srcSeqLen + tgtSeqLen });
    module.ComputeOutputDimensions();
    arma::mat weights(module.WeightSize(), 1);
    weights.randu();
    module.SetWeights(weights.memptr());

    double error = JacobianTest(module, input);
    REQUIRE(error <= 1e-5);
  }
}


// Simple numerical gradient checker.
template<class FunctionType>
double _CheckGradient(FunctionType& function, const double eps = 1e-7)
{
  // Get gradients for the current parameters.
  arma::mat orgGradient, gradient, estGradient;
  function.Gradient(orgGradient);

  estGradient = arma::zeros(orgGradient.n_rows, orgGradient.n_cols);

  // Compute numeric approximations to gradient.
  for (size_t i = 0; i < orgGradient.n_elem; ++i)
  {
    double tmp = function.Parameters()(i);

    // Perturb parameter with a positive constant and get costs.
    function.Parameters()(i) += eps;
    double costPlus = function.Gradient(gradient);

    // Perturb parameter with a negative constant and get costs.
    function.Parameters()(i) -= (2 * eps);
    double costMinus = function.Gradient(gradient);

    // Restore the parameter value.
    function.Parameters()(i) = tmp;

    // Compute numerical gradients using the costs calculated above.
    estGradient(i) = (costPlus - costMinus) / (2 * eps);
    std::cout << "CheckGradient[" << i << "] costPlus=" << costPlus << ", costMinus=" << costMinus << ", est[" << i << "]=" << estGradient(i) << std::endl;
  }

  // Estimate error of gradient.
  return arma::norm(orgGradient - estGradient) /
         arma::norm(orgGradient + estGradient);
}

/**
 * Numerical gradient test for MultiheadAttention layer.
 */
TEST_CASE("GradientMultiheadAttentionTest", "[ANNLayerTest]")
{
  struct GradientFunction
  {
    GradientFunction() :
        tgtSeqLen(2),
        srcSeqLen(2),
        embedDim(4),
        nHeads(2),
        vocabSize(5),
        batchSize(2)
    {
      input = arma::randu(embedDim * (tgtSeqLen + 2 * srcSeqLen), batchSize);
      target = arma::zeros(vocabSize, batchSize);
      for (size_t i = 0; i < target.n_elem; ++i)
      {
        const size_t label = RandInt(1, vocabSize);
        target(i) = label;
      }

      attnMask = arma::zeros(tgtSeqLen, srcSeqLen);
      for (size_t i = 0; i < tgtSeqLen; ++i)
      {
        for (size_t j = 0; j < srcSeqLen; ++j)
        {
          if (i < j)
            attnMask(i, j) = std::numeric_limits<double>::lowest();
        }
      }

      keyPaddingMask = arma::zeros(1, srcSeqLen);
      keyPaddingMask(srcSeqLen - 1) = std::numeric_limits<double>::lowest();

      model = new FFN<NegativeLogLikelihood, XavierInitialization>();
      model->InputDimensions() = {embedDim, srcSeqLen * 2 + tgtSeqLen};
      model->ResetData(input, target);
      model->Add<MultiheadAttention>(tgtSeqLen, nHeads,
                                     attnMask, keyPaddingMask);
      model->Add<Linear>(vocabSize);
      model->Add<LogSoftMax>();
      model->Reset();

      arma::mat output(vocabSize, batchSize);
      model->Forward(input, output);
      input.print("input");
      output.print("initial output");
      count = 0;
    }

    ~GradientFunction()
    {
      delete model;
    }

    double Gradient(arma::mat& gradient)
    {
      ++count;
      std::cout << "[" << count << "]" << "Evaluating gradient" << std::endl;
      model->Parameters().print("parameters " + std::to_string(count));
      double error = model->Evaluate(model->Parameters(), 0, batchSize);
      std::cout << "[" << count << "]" << "error=" << error << std::endl;
      arma::mat output(vocabSize, batchSize);
      model->Forward(input, output);
      output.print("output " + std::to_string(count));
      model->Gradient(model->Parameters(), 0, gradient, batchSize);
      gradient.print("gradient " + std::to_string(count));
      return error;
    }

    arma::mat& Parameters() { return model->Parameters(); }

    FFN<NegativeLogLikelihood, XavierInitialization>* model;
    // MultiheadAttention* attnModule;

    arma::mat input, target, attnMask, keyPaddingMask;
    const size_t tgtSeqLen;
    const size_t srcSeqLen;
    const size_t embedDim;
    const size_t nHeads;
    const size_t vocabSize;
    const size_t batchSize;
    size_t count;
  } function;

  REQUIRE(_CheckGradient(function) <= 3e-06);
}
