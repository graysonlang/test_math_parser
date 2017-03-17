#pragma once
#ifndef MATH_PARSER_H_
#define MATH_PARSER_H_

#include <climits> // std::numerical_limis::quiet_NaN()
#include <string>

namespace MathParser {

  enum class Status {
    SUCCESS = 0,
    PARSING_ERROR = 1,
    EVALUATION_ERROR = 2,
  };

  enum class ParsingErrorType {
    NONE = 0,
    EMPTY,
    MISMATCHED_PARENS,
    SYNTAX_ERROR,
  };

  enum class EvaluationErrorType {
    NONE = 0,
    DIVIDE_BY_ZERO,
    EXPECTED_CURRENT_VALUE,
    EXPECTED_MORE_ARGUMENTS,
    IMAGINARY_NUMBER,
    UNEXPECTED_TOKEN,
  };

  struct Config {
    bool use_degrees = true;
    Config(bool use_degrees_ = true) : use_degrees(use_degrees_) { }
  };

  struct Result {
    Status status;
    double result;
    std::string filtered_expression;
    ParsingErrorType parsing_error;
    EvaluationErrorType evaluation_error;
    size_t error_position;
    size_t error_length;

    Result(double result);
    Result(ParsingErrorType, std::string &&filtered_expression = "", size_t error_position = 0, size_t error_length = 0);
    Result(EvaluationErrorType, std::string &&filtered_expression = "", size_t error_position = 0, size_t error_length = 0);
  };

  Result evaluate_expression(const std::string &expression, Config config = { }, double current_value = std::numeric_limits<double>::quiet_NaN());
  Result evaluate_expression(const std::string &expression, double current_value, Config config = { });

} // namespace MathParser

#endif // MATH_PARSER_H_
