#pragma once
#ifndef MATH_PARSER_H_
#define MATH_PARSER_H_

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
    EXPECTED_MORE_ARGUMENTS,
    IMAGINARY_NUMBER,
    UNEXPECTED_TOKEN,
  };

  struct Result {
    Status status;
    double result;
    std::string filtered_expression;
    ParsingErrorType parsing_error;
    EvaluationErrorType evaluation_error;
    int error_position;
    size_t error_length;

    Result(double result);
    Result(ParsingErrorType, std::string && = "", int = -1, int = 0);
    Result(EvaluationErrorType, std::string && = "", int = -1, int = 0);
  };

  Result evaluate_expression(const std::string &expression);

} // namespace MathParser

#endif // MATH_PARSER_H_