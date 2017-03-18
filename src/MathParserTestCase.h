#pragma once
#ifndef MATH_PARSER_TEST_CASE_H_
#define MATH_PARSER_TEST_CASE_H_

#include "MathParser.h"

using MathParser::EvaluationErrorType;
using MathParser::ParsingErrorType;

struct MathParserTestCase {
  std::string expression;
  MathParser::Status status = { };
  double current = std::numeric_limits<double>::quiet_NaN();;
  double result = std::numeric_limits<double>::quiet_NaN();
  MathParser::ParsingErrorType parsing_error = { };
  MathParser::EvaluationErrorType evaluation_error = { };
  MathParser::Config config = { };

  MathParserTestCase(const std::string &expression_, double result_, MathParser::Config config_);
  MathParserTestCase(const std::string &expression_, double result_, double current_ = std::numeric_limits<double>::quiet_NaN());
  MathParserTestCase(const std::string &expression_, MathParser::ParsingErrorType parsing_error_);
  MathParserTestCase(const std::string &expression_, MathParser::EvaluationErrorType evaluation_error_, double current_ = std::numeric_limits<double>::quiet_NaN());
};

enum class TrigFunctionType {
  COS = 0,
  SIN,
  TAN,
};

enum class TrigAngleUnits {
  DEGREES = 0,
  RADIANS,
};

MathParserTestCase trig_test_case(const std::string &expression, TrigFunctionType trig_function, double angle, TrigAngleUnits units = TrigAngleUnits::DEGREES);

#endif // MATH_PARSER_TEST_CASE_H_
